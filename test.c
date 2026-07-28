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

void test_failed() {
    exit(1);
}

void asset_result_float(const char* expr, float expected) {
    printf("Test: %s\n", expr);
    char error_msg[256];
    error_msg[0] = '\0';
    float ret = je_eval_float(expr, error_msg, sizeof(error_msg));
    if (error_msg[0] != '\0') {
        printf("Failed to evaluate expression '%s' with error: %s\n", expr, error_msg);
        test_failed();
    }
    if (!(fabs(ret - expected) < 0.0001f)) {
        printf("Test failed '%s', expected %.2f but got %.2f\n", expr, expected, ret);
        test_failed();
    }
}

void asset_result_bool(const char* expr, bool expected) {
    printf("Test: %s\n", expr);
    char error_msg[256];
    error_msg[0] = '\0';
    bool ret = je_eval_bool(expr, error_msg, sizeof(error_msg));
    if (error_msg[0] != '\0') {
        printf("Failed to evaluate expression '%s' with error: %s\n", expr, error_msg);
        test_failed();
    }
    if (!(ret == expected)) {
        printf("Test failed '%s', expected %i but got %i\n", expr, expected, ret);
        test_failed();
    }
}

void asset_result_int(const char* expr, int expected) {
    printf("Test: %s\n", expr);
    char error_msg[256];
    error_msg[0] = '\0';
    int ret = je_eval_int(expr, error_msg, sizeof(error_msg));
    if (error_msg[0] != '\0') {
        printf("Failed to evaluate expression '%s' with error: %s\n", expr, error_msg);
        test_failed();
    }
    if (!(ret == expected)) {
        printf("Test failed '%s', expected %i but got %i\n", expr, expected, ret);
        test_failed();
    }
}

void asset_result_string(const char* expr, const char* expected) {
    printf("Test: %s\n", expr);
    char error_msg[256];
    error_msg[0] = '\0';
    char* ret = je_eval_string(expr, error_msg, sizeof(error_msg));
    if (error_msg[0] != '\0') {
        printf("Failed to evaluate expression '%s' with error: %s\n", expr, error_msg);
        test_failed();
    }
    if (!(strcmp(ret, expected) == 0)) {
        printf("Test failed '%s', expected '%s' but got '%s'\n", expr, expected, ret);
        test_failed();
    }
    free(ret);
}

void run_tests() {
    
    // Float operators
    asset_result_float("2.0f + 1.0f", 3.0f);
    asset_result_float("2.0f - 1.0f", 1.0f);
    asset_result_float("2.0f * 2.0f", 4.0f);
    asset_result_float("2.0f / 2.0f", 1.0f);
    asset_result_bool("1.0f > 2.0f", false);
    asset_result_bool("1.0f >= 2.0f", false);
    asset_result_bool("1.0f < 2.0f", true);
    asset_result_bool("1.0f <= 2.0f", true);
    asset_result_bool("1.0f == 2.0f", false);
    asset_result_bool("1.0f != 2.0f", true);

    // Int operators
    asset_result_int("-1", -1);
    asset_result_int("+1", 1);
    asset_result_int("2 + 1", 3);
    asset_result_int("2 - 1", 1);
    asset_result_int("2 * 2", 4);
    asset_result_int("2 / 2", 1);
    asset_result_int("2 % 4", 2);
    asset_result_int("1 | 2", 1 | 2);
    asset_result_int("1 & 2", 1 & 2);
    asset_result_int("1 ^ 2", 1 ^ 2);
    asset_result_int("~2", ~2);
    asset_result_bool("1 > 2", false);
    asset_result_bool("1 >= 2", false);
    asset_result_bool("1 < 2", true);
    asset_result_bool("1 <= 2", true);
    asset_result_bool("1 == 2", false);
    asset_result_bool("1 != 2", true);

    // Bool operators
    asset_result_bool("true == false", false);
    asset_result_bool("true != false", true);
    asset_result_bool("true && true", true);
    asset_result_bool("true && false", false);
    asset_result_bool("true || true", true);
    asset_result_bool("true || false", true);
    asset_result_bool("false || false", false);
    asset_result_bool("!false", true);
    asset_result_bool("!true", false);

    // String operators
    asset_result_string("\"Test\"", "Test");
    asset_result_string("\"Hello\" + \"World\"", "HelloWorld");

    // Sub expressions
    asset_result_int("(1 + 2) * 3", 9);
    asset_result_int("1 + (2 * 3)", 7);
    asset_result_int("1 + 2 * 3", 7);
    asset_result_int("((1 * 2) + (2 * 3))", 8);

    // Casting 
    asset_result_bool("(bool)1", true);
    asset_result_bool("(bool)0", false);
    asset_result_bool("(bool)1.0f", true);
    asset_result_bool("(bool)0.0f", false);
    asset_result_bool("(bool)\"true\"", true);
    asset_result_bool("(bool)\"false\"", false);
    asset_result_bool("(bool)\"anything\"", true);
    asset_result_bool("(bool)\"1\"", true);
    asset_result_bool("(bool)\"0\"", false);
    asset_result_bool("(bool)\"\"", false);

    asset_result_float("(float)1", 1.0f);
    asset_result_float("(float)0", 0.0f);
    asset_result_float("(float)true", 1.0f);
    asset_result_float("(float)false", 0.0f);
    asset_result_float("(float)\"1.0\"", 1.0f);
    asset_result_float("(float)\"0.0\"", 0.0f);

    asset_result_int("(int)1.0f", 1);
    asset_result_int("(int)0.0f", 0);
    asset_result_int("(int)\"1\"", 1);
    asset_result_int("(int)\"0\"", 0);
    asset_result_int("(int)\"\"", 0);

    asset_result_string("(string)1", "1");
    asset_result_string("(string)0", "0");
    asset_result_string("(string)1.0f", "1.000000");
    asset_result_string("(string)0.0f", "0.000000");
    asset_result_string("(string)true", "1");
    asset_result_string("(string)false", "0");

    // Function calls
    asset_result_float("floor(1.5f)", 1.0f);
    asset_result_float("ceil(1.5f)", 2.0f);
    asset_result_float("min(1.0f, 2.0f)", 1.0f);
    asset_result_float("max(1.0f, 2.0f)", 2.0f);
    asset_result_string("__string_concat(\"Hello\", \"World\")", "HelloWorld");

    // Variables
    asset_result_float("PI", 3.14159265f);
}

int main(int argc, char* argv[]) {
    run_tests();
    printf("All tests passed.\n");
    return 0;
}
