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

