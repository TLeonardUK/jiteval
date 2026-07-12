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

void func(je_context_t* ctx) {
    float a, b;
    je_get_parameter_float(ctx, 0, &a);
    je_get_parameter_float(ctx, 1, &b);
    je_return_float(ctx, a + b);
}

int main(int argc, char* argv[]) {
    je_context_t ctx;
    je_new_context(&ctx);
    
    je_bind_function(&ctx, "func", true, &func, JE_TYPE_FLOAT, JE_TYPE_FLOAT, JE_TYPE_FLOAT, NULL);
    je_bind_variable_float(&ctx, "PI", false, 3.14159f);
    je_compile(&ctx, "PI + func((0xF + -3.2), (32 + PI))");
    //je_compile(&ctx, "PI + (0xF + -3.2) / (32 + PI)");

    je_eval(&ctx);
    
    float result = 0.0f;
    je_result_float(&ctx, &result);
    printf("result = %f\n", result);
    
    je_bind_variable_float(&ctx, "PI", false, 3.0f);
    
    je_eval(&ctx);
    
    je_result_float(&ctx, &result);
    printf("result = %f\n", result);
    
    je_free_context(&ctx);
    
    return 0;
}

