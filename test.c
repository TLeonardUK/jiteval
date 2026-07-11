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
    je_new_context(&ctx, JE_FLAG_NO_OPTIMIZATION | JE_FLAG_DEBUG_LOGGING);

    je_bind_function(&ctx, "func", false, &func, JE_TYPE_INT, JE_TYPE_INT, JE_TYPE_INT, NULL);
    je_bind_variable_int(&ctx, "val", false, 100);
    je_bind_variable_string(&ctx, "TEST", false, "b");
    je_compile(&ctx, "TEST != \"a\"");
    //je_compile(&ctx, "1 + 2");

    je_eval(&ctx);

    const char* result = "";
    je_result_string(&ctx, &result);
    printf("result = %s\n", result);

    je_free_context(&ctx);

    return 0;
}

