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

// This is an example of a function that can be bound to a context and called
// by evaluated expressions. 
// 
// The signature is identical for all functions, parameters and return values
// are all processed by the je_get_parameter_* and je_return_* functions.
void func_add(je_context_t* ctx) {

    float a, b;

    // Note: Attempting to get parameters out of bounds will result in an error
    //       result being returned and the vaue being unchanged.
    je_get_parameter_float(ctx, 0, &a);
    je_get_parameter_float(ctx, 1, &b);

    // Note: Failure to return a value from a function is undefined behaviour.
    je_return_float(ctx, a + b);

}

int main(int argc, char* argv[]) {

    // Note: For brevity most error checking is skipped here. 
    //       All publically exposed je_* functions return a JE_RESULT_* value
    //       which should be checked in production code.

    // Allocate and create the context - the context stores all state specific
    // information like binded variables/function/etc. All memory is also allocated
    // at this point and no dynamic allocation happens beyond here (except for allocating JIT executable memory).
    je_context_t ctx;
    je_new_context(&ctx, JE_FLAG_NONE);
    
    // This binds the function "add" with a return type of float and two float parameters.
    // Marking a function as deterministic (the third argument) allows the function to be evaluated
    // at compile time and optimized away from evaluation.
    je_bind_function(&ctx, "add", true, &func_add, JE_TYPE_FLOAT, JE_TYPE_FLOAT, JE_TYPE_FLOAT, NULL);

    // This binds a variable to the context and allows it to be referenced in compiled expressions.
    // Like functions, variables can be marked as constant (the third argument), which allows it
    // to be evaluated at compile time and optimized away from evaluation.
    je_bind_variable_float(&ctx, "my_value", false, 100.0f);

    // Note: All binds must be complete before compile can be called.
    //       Variable values can be rebound between eval calls to change their value as long 
    //        as their type doesn't change.

    // This call does the heavy lifting; parsing, optimizing and optinally JIT compiling an
    // expression. 
    // 
    // Note: Contexts can only be compiled once. If you want to evaluate use multiple expressions you 
    //       need to use multiple contexts (or free/new the context for each one).
    int ret = je_compile(&ctx, "my_value * add(3.0f, my_value)");
    if (ret < 0) {
        printf("Compile error: %s\n", je_error_msg(&ctx));
        return;
    }

    // This call actually evaluates the expression and uses the current values of all functions
    // and variables. 
    //
    // Note: Contexts are not thread-safe, you should not attempt to manipulate or evaluate compiled
    //       contexts from multiple threads concurrently. 
    ret = je_eval(&ctx);
    if (ret < 0) {
        printf("Eval error: %s\n", je_error_msg(&ctx));
        return;
    }
    
    // Here the result of the evaluation is retrieved. If the expression returns a differnet type than
    // that which is requested, the value will be coerced to the expected type.
    float result = 0.0f;
    je_result_float(&ctx, &result);
    printf("Result: %f\n", result);
    
    // The value of the variable can be rebound between eval calls.
    je_bind_variable_float(&ctx, "my_value", false, 1.0f);
    
    // The expression is re-evaluated with the new values. 
    // Eval can be called as many times as needed once a context is compiled.
    ret = je_eval(&ctx);
    if (ret < 0) {
        printf("Eval error: %s\n", je_error_msg(&ctx));
        return;
    }
    
    // And again the result is retrieved with the new value.
    je_result_float(&ctx, &result);
    printf("Result: %f\n", result);
    
    // Release all the memory and clean up the context.
    je_free_context(&ctx);
    
    return 0;
}

