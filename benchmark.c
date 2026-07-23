// -----------------------------------------------------------------------
// jiteval.h - v1.0 - expression evaluation with optional JIT support
// Tim Leonard, Copyright 2026
//
// See jiteval.h for documentation
// -----------------------------------------------------------------------
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files(the “Software”), 
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and /or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included 
// in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS 
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// -----------------------------------------------------------------------
#define JITEVAL_IMPL
#include "jiteval.h"

#include <math.h>
#include <time.h>

// -----------------------------------------------------------------------
// Utility functions
// -----------------------------------------------------------------------

#ifdef JE_PLATFORM_WINDOWS

typedef struct perf_timer_t {
    LARGE_INTEGER time_start;
    LARGE_INTEGER time_stop;
} perf_timer_t;

void perf_timer_start(perf_timer_t* timer) {
    QueryPerformanceCounter(&timer->time_start);
}
void perf_timer_stop(perf_timer_t* timer) {
    QueryPerformanceCounter(&timer->time_stop);
}
double perf_timer_elapsed_ms(perf_timer_t* timer) {
    LARGE_INTEGER freq;
    double duration;
    QueryPerformanceFrequency(&freq);
    duration = (double)(timer->time_stop.QuadPart - timer->time_start.QuadPart) / (double)freq.QuadPart;
    return duration * 1000.0f;
}

#else

typedef struct perf_timer_t {
    struct timespec end_time;
    struct timespec start_time;
} perf_timer_t;

void perf_timer_start(perf_timer_t* timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
}
void perf_timer_stop(perf_timer_t* timer) {
    clock_gettime(CLOCK_MONOTONIC, &timer->end_time);
}
double perf_timer_elapsed_ms(perf_timer_t* timer) {
    long elapsed_ns = (timer->end_time.tv_sec - timer->start_time.tv_sec) * (long)1e9 + (timer->end_time.tv_nsec - timer->start_time.tv_nsec);
    return elapsed_ns / 1e6f;
}

#endif

// -----------------------------------------------------------------------
// Bound variables and functions
// -----------------------------------------------------------------------

const char* g_benchmark_expression = "sqr((((12 * var_float) / (32 * 2)) + 3) + 8)";

void func_sqr(je_context_t* ctx) {
    float a;
    je_get_parameter_float(ctx, 0, &a);
    je_return_float(ctx, a * a);
}

// -----------------------------------------------------------------------
// Main code
// -----------------------------------------------------------------------

void setup_context(je_context_t* ctx, int flags) {
    je_new_context(ctx, flags);

    je_bind_function        (ctx, "sqr",        false, &func_sqr, JE_TYPE_FLOAT, JE_TYPE_FLOAT, NULL);
    je_bind_variable_int    (ctx, "var_int",    false, 123);
    je_bind_variable_bool   (ctx, "var_bool",   false, false);
    je_bind_variable_string (ctx, "var_string", false, "abc");
    je_bind_variable_float  (ctx, "var_float",  false, 123.0f);
}

double run_benchmark(const char* name, int flags) {
    double elapsed = 0.0f;

    printf("============== %s ==============\n", name);

    je_context_t ctx;
    setup_context(&ctx, flags);

    {
        perf_timer_t timer;
        perf_timer_start(&timer);
        je_compile(&ctx, g_benchmark_expression);
        perf_timer_stop(&timer);
        printf("Compile took: %.8f ms\n", perf_timer_elapsed_ms(&timer));
    }

    {
        const int k_iterations = 1'000'000'000;

        float output_value = 0.0f;
        perf_timer_t timer;
        perf_timer_start(&timer);
        for (int i = 0; i < k_iterations; i++) {
            je_eval(&ctx);
            if (i == k_iterations - 1) {
                je_result_float(&ctx, &output_value);
            }
        }
        perf_timer_stop(&timer);
        elapsed = perf_timer_elapsed_ms(&timer);
        printf("Eval took %.8f ms (%.8f ms per iterations), result was %.8f\n", elapsed, elapsed / k_iterations, output_value);

        int permanent_mem_used = 0;
        int transient_mem_used = 0;
        int executable_mem_used = 0;
        je_memory_stats(&ctx, &permanent_mem_used, &transient_mem_used, &executable_mem_used);
        printf("Post memory used: Permanent=%i bytes, Transient=%i bytes, Executable=%i bytes\n", permanent_mem_used, transient_mem_used, executable_mem_used);
    }

    je_free_context(&ctx);

    return elapsed;
}

int main(int argc, char* argv[]) {
    double interpreted_time = run_benchmark("Interpreted", JE_FLAG_NO_JIT);
    double jit_time = run_benchmark("JIT Compiled", JE_FLAG_NONE);

    printf("\nJIT speed: %.2f x\n", (interpreted_time / jit_time));

    return 0;
}

