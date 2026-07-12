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

// -----------------------------------------------------------------------
// Utility functions
// -----------------------------------------------------------------------

#ifdef _WIN32
#include <windows.h>
typedef struct timer_t {
    LARGE_INTEGER time_start;
    LARGE_INTEGER time_stop;
} timer_t;

void timer_start(timer_t* timer) {
    QueryPerformanceCounter(&timer->time_start);
}
void timer_stop(timer_t* timer) {
    QueryPerformanceCounter(&timer->time_stop);
}
double timer_elapsed_ms(timer_t* timer) {
    LARGE_INTEGER freq;
    double duration;
    QueryPerformanceFrequency(&freq);
    duration = (double)(timer->time_stop.QuadPart - timer->time_start.QuadPart) / (double)freq.QuadPart;
    return duration * 1000.0f;
}
#else
void timer_start(timer_t* timer) {
}
void timer_stop(timer_t* timer) {
}
double timer_elapsed_ms(timer_t* timer) {
    return 0.0f;
}
#endif

// -----------------------------------------------------------------------
// Bound variables and functions
// -----------------------------------------------------------------------

void func_sin(je_context_t* ctx) {
    float a;
    je_get_parameter_float(ctx, 0, &a);
    je_return_float(ctx, (float)sin(a));
}

// -----------------------------------------------------------------------
// Main code
// -----------------------------------------------------------------------

void setup_context(je_context_t* ctx, int flags) {
    je_new_context(ctx, flags);

    je_bind_function(ctx, "sin", false, &func_sin, JE_TYPE_FLOAT, JE_TYPE_FLOAT, NULL);
    je_bind_variable_int(ctx, "var_int", false, 123);
    je_bind_variable_bool(ctx, "var_bool", false, false);
    je_bind_variable_string(ctx, "var_string", false, "abc");
    je_bind_variable_float(ctx, "var_float", false, 123.0f);
}

void run_benchmark(const char* name, int flags) {
    je_context_t ctx;
    setup_context(&ctx, flags);

    printf("============== %s ==============\n", name);

    {
        timer_t timer;
        timer_start(&timer);
        je_compile(&ctx, "((((12 * var_float) / (32 * 2)) + 3) + 8)");
        timer_stop(&timer);
        printf("Compile took: %.8f ms\n", timer_elapsed_ms(&timer));
    }

    {
        const int k_iterations = 1000000;

        timer_t timer;
        timer_start(&timer);
        for (int i = 0; i < k_iterations; i++) {
            je_eval(&ctx);
        }
        timer_stop(&timer);
        double elapsed = timer_elapsed_ms(&timer);
        printf("Eval took: %.8f ms (%.8f ms per iterations)\n", elapsed, elapsed / k_iterations);
    }

    je_free_context(&ctx);
}

int main(int argc, char* argv[]) {
    run_benchmark("Interpreted", JE_FLAG_NO_JIT);
    run_benchmark("JIT Compiled", JE_FLAG_NONE);
    return 0;
}

