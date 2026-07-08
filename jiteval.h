// -----------------------------------------------------------------------
// jiteval
// Copyright 2026 Tim Leonard
//
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
#ifndef JITEVAL_H
#define JITEVAL_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------
// jiteval.h - v1.0 - expression evaluation with optional JIT support
// Tim Leonard, July 2026
//
// Intended for when you want to parse evaluate some C-style expressions
// as quickly as possible.
//
// This is a single-header project. To integrate it into your code, include
// the header into one of your source files with JITEVAL_IMPL
// defined before it. eg.
//
// #define JITEVAL_IMPL
// #include <jiteval.h>
//
// DOCUMENTATION
// 
//      int je_new_context(je_context_t* context);
// 
//          Takes an opaque context object that is used by all other functions
//          to store stateful information. 
//
//          Returns a value of JE_RESULT_* describing the success or failure.
//
//      int je_free_context(je_context_t* context);
//
//          Takes an opaque context object that was previously initialized with
//          je_new_context and releases any memory held by it.
// 
//          Returns a value of JE_RESULT_* describing the success or failure.
// 
//      int je_bind_<type>(je_context_t* context, const char* name, <type> value)
//  
//          Binds identifiers to a given value in the context, which can be 
//          referenced in expressions that are evaluated.
// 
//          Constants cannot be unbound from a context once bound, but can
//          have their values changed inbetween je_eval calls as long
//          as their type does not change.
// 
//          String values are copied to an internal buffer and do not need to 
//          be alive beyond this call.
// 
//          Returns a value of JE_RESULT_* describing the success or failure.
// 
//      int je_compile(je_context_t* context, const char* source);
// 
//          Compiles an expression that the given context will evaluate
//          whenever je_eval is called.
// 
//          All constants used by an expression must be bound before je_compile
//          is called, but their values can be rebound between calls to 
//          je_eval as long as their type does not change.
// 
//          Source is copied to an internal buffer and does not need to exist
//          beyond this call.
// 
//          If a syntax error occurs, call je_error_msg for more information.
// 
//          Returns a value of JE_RESULT_* describing the success or failure.
// 
//      int je_eval(je_context_t* context);
// 
//          Evaluates the expession that was previously compiled with je_compile
//          on the context object. 
// 
//          Can be called multiple times with rebound constants without needing
//          to call je_compile again.
//  
//          Returns a value of JE_RESULT_* describing the success or failure.
// 
//      int je_result_<type>(je_context_t* context, <type>* result);
// 
//          Stores the resulting value of the last je_eval call in result parameter.
//          If the result was not the given type, the result will be coalesced to it.
// 
//          String results point to an internal buffer, which remains alive until
//          the next je_eval or je_free_context call.
// 
//          Returns a value of JE_RESULT_* describing the success or failure.
//
//      const char* je_error_msg(je_context_t* context);
// 
//          Gets an error message as a string from the last failing call.
// 
//          String returned remains alive until the next call on the context.
// 
//          If no error occured NULL is returned.
// 
// SYNTAX
//      
//      The syntax of expressions follows that of standard C expressions, with the
//      same operator precedence.
// 
//      Supported operators are: 
// 
//          Arithmatic:         *, /, -, +, %
//          Comparison:         <, >, >=, <=, ==, !=, 
//          Logical:            &&, ||
//          Unary:              ! - +
//          Bitwise:            &, |, ~
//          Sub Expressions:    ( )
//          Casting:            (int) / (float) / (string) / (bool)
// 
//      Supported types are: 
// 
//          int, float, string, bool
// 
//      Numbers notation support:
//          
//          floating point
//          hex
//          integer 
//          scientific 
// 
//      Strings support escaping via the standard C escape characters.
// 
//      Identifiers are case-insensitive.
// 
//      Expressions are strongly typed. 
//
// LICENSE
//
//      See top of file for license information.
//
// VERSION HISTORY
//
//      (2026-07-07)    1.0    First release
//
// SAMPLE CODE
// 
//      See sample.c in the same directory as this file.
// 
// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
// HEADER
// -----------------------------------------------------------------------

#define JE_RESULT_EOF                   (-5)    // Internal error, occurs when getting to end of token stream.
#define JE_RESULT_TYPE_CANNOT_CHANGE    (-4)    // Attempt was made to change the type of a constant that was already set.
#define JE_RESULT_CORRUPT               (-3)    // Something has corrupted the internal state of the context.
#define JE_RESULT_OOM                   (-2)    // No memory is left in the internal allocator to satisfy operation.
#define JE_RESULT_FAILED                (-1)    // Operation failed (generic)
#define JE_RESULT_SUCCESS               (0)     // Operation succesful

typedef struct je_context_t je_context_t;

int je_new_context(je_context_t* context);
int je_free_context(je_context_t* context);

int je_bind_constant_int(je_context_t* context, const char* name, int value);
int je_bind_constant_float(je_context_t* context, const char* name, float value);
int je_bind_constant_string(je_context_t* context, const char* name, const char* value);
int je_bind_constant_bool(je_context_t* context, const char* name, int value);

int je_compile(je_context_t* context, const char* source);
int je_eval(je_context_t* context);

int je_result_int(je_context_t* context, int* value);
int je_result_float(je_context_t* context, float* value);
int je_result_string(je_context_t* context, const char** value);
int je_result_bool(je_context_t* context, int* value);

const char* je_error_msg(je_context_t* context);

// -----------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------

#ifdef __INTELLISENSE__
#define JITEVAL_IMPL
#endif

#ifdef JITEVAL_IMPL

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#define JE_TYPE_INT                     (0)
#define JE_TYPE_BOOL                    (1)
#define JE_TYPE_FLOAT                   (2)
#define JE_TYPE_STRING                  (3)

#define JE_TOK_IDENTIFIER               (0)
#define JE_TOK_FLOAT                    (1)
#define JE_TOK_INT                      (2)
#define JE_TOK_STRING                   (3)
#define JE_TOK_BOOL                     (4)
#define JE_TOK_OP_MUL                   (5)
#define JE_TOK_OP_DIV                   (6)
#define JE_TOK_OP_SUB                   (7)
#define JE_TOK_OP_ADD                   (8)
#define JE_TOK_OP_MOD                   (9)
#define JE_TOK_OP_LESS                  (10)
#define JE_TOK_OP_GREATER               (11)
#define JE_TOK_OP_LE                    (12)
#define JE_TOK_OP_GE                    (13)
#define JE_TOK_OP_EQUAL                 (14)
#define JE_TOK_OP_NOT_EQUAL             (15)
#define JE_TOK_OP_NOT                   (16)
#define JE_TOK_OP_LOGICAL_AND           (17)
#define JE_TOK_OP_LOGICAL_OR            (18)
#define JE_TOK_OP_BITWISE_AND           (19)
#define JE_TOK_OP_BITWISE_OR            (20)
#define JE_TOK_OP_BITWISE_NOT           (21)
#define JE_TOK_OP_PARENTHESIS_OPEN      (22)
#define JE_TOK_OP_PARENTHESIS_CLOSE     (23)
#define JE_TOK_OP_KEYWORD_INT           (24)
#define JE_TOK_OP_KEYWORD_FLOAT         (25)
#define JE_TOK_OP_KEYWORD_STRING        (26)
#define JE_TOK_OP_KEYWORD_BOOL          (27)

#define JE_MEM_ARENA_SIZE               (16 * 1024)

typedef struct je_token_t {
    int         type;
    const char* source_ptr;                     // Pointer to where the token is in the source string.
    int         source_len;                     // Length of token in the source string.
} je_token_t;

typedef struct je_value_t {
    const char*         name;
    int                 type;
    union {
        int             int_value;
        int             bool_value;
        float           float_value;
        char*           string_value;
    };
    struct je_value_t*  next;
} je_value_t;

typedef struct je_context_t {
    char*                   mem_arena;          // Block of memory that all dynamically allocated memory is stored in. Used as a stack allocator.
    size_t                  mem_arena_offset;   // Next locations in the mem_arena to allocate from.
    char*                   source;             // Pointer to source code string
    char*                   read_ptr;           // Read pointer into source string
    struct je_value_t*      constant_head;      // Head of the constant linked list.
    je_value_t              result;             // Result of the last evaluation call.
    int                     error_code;         // Last error that occured in the context.
    char                    error_msg[512];     // Error message from the last failing call.
} je_context_t;

int je_store_error(je_context_t* context, int error_code, const char* error_msg) {
    if (error_msg != NULL) {
        strncpy(context->error_msg, error_msg, sizeof(context->error_msg));
    } else {
        context->error_msg[0] = '\0';

        const char* error_code_msg = NULL;
        switch (error_code) {
            case JE_RESULT_TYPE_CANNOT_CHANGE:      error_code_msg = "Type of constant cannot be changed from what it was originally set to.";  break;
            case JE_RESULT_CORRUPT:                 error_code_msg = "Internal state of the context is corrupt";                                break;
            case JE_RESULT_OOM:                     error_code_msg = "Out of memory";                                                           break;
            case JE_RESULT_FAILED:                  error_code_msg = "Operation failed";                                                        break;
        }

        if (error_code_msg != NULL) {
            strncpy(context->error_msg, error_code_msg, sizeof(context->error_msg));
        }
    }
    context->error_code = error_code;
    return error_code;
}

int je_alloc(je_context_t* context, size_t size, char** ptr) {
    size_t remaining_space = JE_MEM_ARENA_SIZE - context->mem_arena_offset;
    if (remaining_space < size) {
        je_store_error(context, JE_RESULT_OOM, NULL);
        return context->error_code;
    }
    *ptr = context->mem_arena + context->mem_arena_offset;
    context->mem_arena_offset += size;
    return JE_RESULT_SUCCESS;
}

je_value_t* je_find_constant(je_context_t* context, const char* name) {
    for (je_value_t* value = context->constant_head; value; value->next) {
        if (stricmp(value->name, name) == 0) {
            return value;
        }
    }
    return NULL;
}

int je_coerce_to_int(je_context_t* context, je_value_t* value) {
    switch (value->type) {
        case JE_TYPE_BOOL: {
            value->type = JE_TYPE_INT;
            value->int_value = (int)value->bool_value;
            break;
        }
        case JE_TYPE_INT: {
            // noop
            break;
        }
        case JE_TYPE_FLOAT: {
            value->type = JE_TYPE_INT;
            value->int_value = (int)value->float_value;
            break;
        }
        case JE_TYPE_STRING: {
            value->type = JE_TYPE_INT;
            value->int_value = atoi(value->string_value);
            break;
        }
        default: {
            return je_store_error(context, JE_RESULT_CORRUPT, NULL);
        }
    }
    return JE_RESULT_SUCCESS;
}

int je_coerce_to_float(je_context_t* context, je_value_t* value) {
    switch (value->type) {
        case JE_TYPE_BOOL: {
            value->type = JE_TYPE_FLOAT;
            value->float_value = (float)value->bool_value;
            break;
        }
        case JE_TYPE_INT: {
            value->type = JE_TYPE_FLOAT;
            value->float_value = (float)value->int_value;
            break;
        }
        case JE_TYPE_FLOAT: {
            // noop
            break;
        }
        case JE_TYPE_STRING: {
            value->type = JE_TYPE_FLOAT;
            value->float_value = (float)atof(value->string_value);
            break;
        }
        default: {
            return je_store_error(context, JE_RESULT_CORRUPT, NULL);
        }
    }
    return JE_RESULT_SUCCESS;
}

int je_coerce_to_string(je_context_t* context, je_value_t* value) {
    char buffer[32];
    size_t len = 0;
    int ret = 0;

    switch (value->type) {
        case JE_TYPE_BOOL: {
            strncpy(buffer, value->bool_value ? "true" : "false", sizeof(buffer));
            break;
        }
        case JE_TYPE_INT: {
            sprintf(buffer, "%i", value->int_value);
            break;
        }
        case JE_TYPE_FLOAT: {
            sprintf(buffer, "%f", value->float_value);
            break;
        }
        case JE_TYPE_STRING: {
            // noop
            return JE_RESULT_SUCCESS;
        }
        default: {
            return je_store_error(context, JE_RESULT_CORRUPT, NULL);
        }
    }

    len = strlen(buffer) + 1;
    ret = je_alloc(context, len, &value->string_value);
    if (ret < 0) {
        return ret;
    }
    strncpy(value->string_value, buffer, len);

    return JE_RESULT_SUCCESS;
}

int je_coerce_to_bool(je_context_t* context, je_value_t* value) {
    switch (value->type) {
        case JE_TYPE_BOOL: {
            // noop
            break;
        }
        case JE_TYPE_INT: {
            value->type = JE_TYPE_BOOL;
            value->bool_value = (value->int_value != 0 ? 1 : 0);
            break;
        }
        case JE_TYPE_FLOAT: {
            value->type = JE_TYPE_BOOL;
            value->bool_value = (fabs(value->float_value) > FLT_EPSILON ? 1 : 0);
            break;
        }
        case JE_TYPE_STRING: {
            value->type = JE_TYPE_BOOL;
            value->bool_value = (stricmp(value->string_value, "false") != 0 && stricmp(value->string_value, "0") != 0);
            break;
        }
        default: {
            return je_store_error(context, JE_RESULT_CORRUPT, NULL);
        }
    }
    return JE_RESULT_SUCCESS;
}

int je_new_context(je_context_t* context) {
    memset(context, 0, sizeof(struct je_context_t));
    context->mem_arena = malloc(JE_MEM_ARENA_SIZE);
    if (context->mem_arena == NULL) {
        return JE_RESULT_OOM;
    }
    return JE_RESULT_SUCCESS;
}

int je_free_context(je_context_t* context) {
    if (context->mem_arena != NULL) {
        free(context->mem_arena);
        context->mem_arena = NULL;
    }
    return JE_RESULT_SUCCESS;
}

int je_bind_constant_int(je_context_t* context, const char* name, int value) {
    je_value_t* constant = NULL;
    int ret = 0;

    constant = je_find_constant(context, name);
    if (constant != NULL) {
        if (constant->type != JE_TYPE_INT) {
            return je_store_error(context, JE_RESULT_TYPE_CANNOT_CHANGE, NULL);
        }
        constant->int_value = value;
        return JE_RESULT_SUCCESS;
    }

    ret = je_alloc(context, sizeof(je_value_t), (char**)&constant);
    if (ret < 0) {
        return ret;
    }

    constant->type = JE_TYPE_INT;
    constant->int_value = value;
    constant->next = context->constant_head;
    context->constant_head = constant;
    return JE_RESULT_SUCCESS;
}

int je_bind_constant_float(je_context_t* context, const char* name, float value) {
    je_value_t* constant = NULL;
    int ret = 0;

    ret = je_find_or_create_constant(context, name, JE_TYPE_FLOAT, &constant);
    if (ret < 0) {
        return ret;
    }

    /*
    constant = je_find_constant(context, name);
    if (constant != NULL) {
        if (constant->type != JE_TYPE_FLOAT) {
            return je_store_error(context, JE_RESULT_TYPE_CANNOT_CHANGE, NULL);
        }
        constant->float_value = value;
        return JE_RESULT_SUCCESS;
    }

    ret = je_alloc(context, sizeof(je_value_t), (char**)&constant);
    if (ret < 0) {
        return ret;
    }
    */

    constant->type = JE_TYPE_FLOAT;
    constant->float_value = value;
    constant->next = context->constant_head;
    context->constant_head = constant;
    return JE_RESULT_SUCCESS;
}

int je_bind_constant_string(je_context_t* context, const char* name, const char* value) {
    je_value_t* constant = NULL;
    int ret = 0;
    size_t value_length = 0;

    value_length = strlen(value);

    constant = je_find_constant(context, name);
    if (constant != NULL) {
        if (constant->type != JE_TYPE_STRING) {
            return je_store_error(context, JE_RESULT_TYPE_CANNOT_CHANGE, NULL);
        }

        ret = je_alloc(context, value_length + 1, &constant->string_value);
        if (ret < 0) {
            return ret;
        }
        strcpy(constant->string_value, value);

        return JE_RESULT_SUCCESS;
    }

    ret = je_alloc(context, sizeof(je_value_t), (char**)&constant);
    if (ret < 0) {
        return ret;
    }
    ret = je_alloc(context, value_length + 1, &constant->string_value);
    if (ret < 0) {
        return ret;
    }
    strcpy(constant->string_value, value);

    constant->type = JE_TYPE_STRING;
    constant->next = context->constant_head;
    context->constant_head = constant;
    return JE_RESULT_SUCCESS;
}

int je_bind_constant_bool(je_context_t* context, const char* name, int value) {
    je_value_t* constant = NULL;
    int ret = 0;

    constant = je_find_constant(context, name);
    if (constant != NULL) {
        if (constant->type != JE_TYPE_BOOL) {
            return je_store_error(context, JE_RESULT_TYPE_CANNOT_CHANGE, NULL);
        }
        constant->bool_value = value;
        return JE_RESULT_SUCCESS;
    }

    ret = je_alloc(context, sizeof(je_value_t), (char**)&constant);
    if (ret < 0) {
        return ret;
    }
    constant->type = JE_TYPE_BOOL;
    constant->bool_value = value;
    constant->next = context->constant_head;
    context->constant_head = constant;
    return JE_RESULT_SUCCESS;
}

int je_result_int(je_context_t* context, int* value) {
    int ret = 0;
    if (context->result.type != JE_TYPE_INT) {
        ret = je_coerce_to_int(context, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    *value = context->result.int_value;
    return JE_RESULT_SUCCESS;
}

int je_result_float(je_context_t* context, float* value) {
    int ret = 0;
    if (context->result.type != JE_TYPE_FLOAT) {
        ret = je_coerce_to_float(context, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    *value = context->result.float_value;
    return JE_RESULT_SUCCESS;
}

int je_result_string(je_context_t* context, const char** value) {
    int ret = 0;
    if (context->result.type != JE_TYPE_STRING) {
        ret = je_coerce_to_string(context, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    *value = context->result.string_value;
    return JE_RESULT_SUCCESS;
}

int je_result_bool(je_context_t* context, int* value) {
    int ret = 0;
    if (context->result.type != JE_TYPE_BOOL) {
        ret = je_coerce_to_bool(context, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    *value = context->result.int_value;
    return JE_RESULT_SUCCESS;
}

const char* je_error_msg(je_context_t* context) {
    return context->error_msg;
}

int je_read_token(je_context_t* context, je_token_t* tok) {
    
    // Consume whitespace before token.
    while (1) {
        if (*context->read_ptr != ' ' &&
            *context->read_ptr != '\t' &&
            *context->read_ptr != '\n' &&
            *context->read_ptr != '\r') {
            break;
        }
        context->read_ptr++;
    }

    tok->source_ptr = context->read_ptr;
    char c = *context->read_ptr;
    if (c == '\0') {
        return JE_RESULT_EOF;
    }

    context->read_ptr++;

    switch (c) {
        case '*': tok->type = JE_TOK_OP_MUL;                break;
        case '/': tok->type = JE_TOK_OP_DIV;                break;
        case '-': tok->type = JE_TOK_OP_SUB;                break;
        case '+': tok->type = JE_TOK_OP_ADD;                break;
        case '%': tok->type = JE_TOK_OP_MOD;                break;
        case '(': tok->type = JE_TOK_OP_PARENTHESIS_OPEN;   break;
        case ')': tok->type = JE_TOK_OP_PARENTHESIS_CLOSE;  break;
        case '~': tok->type = JE_TOK_OP_BITWISE_NOT;        break;
        case '<': {
            if (context->read_ptr[0] == '=') {
                tok->type = JE_TOK_OP_LE;
                context->read_ptr++;
            } else {
                tok->type = JE_TOK_OP_LESS;
            }
            break;
        }
        case '>': {
            if (context->read_ptr[0] == '=') {
                tok->type = JE_TOK_OP_GE;
                context->read_ptr++;
            }
            else {
                tok->type = JE_TOK_OP_GREATER;
            }
            break;
        }
        case '!': {
            if (context->read_ptr[0] == '=') {
                tok->type = JE_TOK_OP_NOT_EQUAL;
                context->read_ptr++;
            }
            else {
                tok->type = JE_TOK_OP_NOT;
            }
            break;
        }
        case '&': {
            if (context->read_ptr[0] == '&') {
                tok->type = JE_TOK_OP_LOGICAL_AND;
                context->read_ptr++;
            }
            else {
                tok->type = JE_TOK_OP_BITWISE_AND;
            }
            break;
        }
        case '|': {
            if (context->read_ptr[0] == '|') {
                tok->type = JE_TOK_OP_LOGICAL_OR;
                context->read_ptr++;
            }
            else {
                tok->type = JE_TOK_OP_BITWISE_OR;
            }
            break;
        }
        default: {
            if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
                // identifier
            } else if (c >= '0' && c <= '9') {
                // number
            } else {
                return 0;
            }
        }
    }

    return JE_RESULT_SUCCESS;
}

int je_eval(je_context_t* context) {    
    // TODO
    return JE_RESULT_SUCCESS;
}

int je_compile(je_context_t* context, const char* source) {
    je_token_t tok;
    int ret = 0;
    size_t source_len = strlen(source);

    // Copy source to an internal buffer.
    ret = je_alloc(context, source_len + 1, &context->source);
    if (ret < 0) {
        return ret;
    }
    strncpy(context->source, source, source_len + 1);

    // Parse the first expression.

    // If any tokens remain, then we have a syntax error ...


    while (1) {
        ret = je_read_token(context, &tok);
        if (ret == JE_RESULT_EOF) {
            break;
        }
        else if (ret < 0) {
            return ret;
        }

        // todo: parse
    }

    // build jit if possible.

    return JE_RESULT_SUCCESS;
}

#endif

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // JITEVAL_H