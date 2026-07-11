#define JITEVAL_IMPL
#include "jiteval.h"

void func(je_context_t* ctx) {
    int a, b;
    je_get_parameter_int(ctx, 0, &a);
    je_get_parameter_int(ctx, 1, &b);
    je_return_int(ctx, a + b);
}

int main(int argc, char* argv[]) {
    je_context_t ctx;
    je_new_context(&ctx, JE_FLAG_NO_OPTIMIZATION);

    je_bind_function(&ctx, "func", false, &func, JE_TYPE_INT, JE_TYPE_INT, JE_TYPE_INT, NULL);
    je_bind_variable_int(&ctx, "val", false, 100);
    je_compile(&ctx, "func(1, 2)");
    //je_compile(&ctx, "1 + 2");

    je_eval(&ctx);

    int result = 0;
    je_result_int(&ctx, &result);
    printf("result = %i\n", result);

    je_free_context(&ctx);

    return 0;
}

