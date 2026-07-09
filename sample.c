#define JITEVAL_IMPL
#include "jiteval.h"

int main(int argc, char* argv[]) {
    je_context_t ctx;
    je_new_context(&ctx);
    
    je_bind_constant_float(&ctx, "PI", 3.14159f);
    je_compile(&ctx, "PI + (0xF + -3.2)");
    
    je_eval(&ctx);
    
    float result = 0.0f;
    je_result_float(&ctx, &result);
    printf("result = %f\n", result);
    
    je_bind_constant_float(&ctx, "PI", 3.0f);
    
    je_eval(&ctx);
    
    je_result_float(&ctx, &result);
    printf("result = %f\n", result);
    
    je_free_context(&ctx);
    
    return 0;
}

