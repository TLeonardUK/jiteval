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
//          Each context can only be compiled once, recreate the context if you want
//          to compile a new expression.
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
//          Bitwise:            &, |, ~, ^
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
//      Identifiers are case-sensitive.
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

// All failure results are negative, all success results are positive.

#define JE_RESULT_CANNOT_IMPLICITLY_CAST            (-17)   // Cannot implicitly cast between types
#define JE_RESULT_INCOMPATIBLE_TYPES                (-16)   // Operation attempted on incompatible types
#define JE_RESULT_UNDEFINED_IDENTIFIER              (-15)   // Undefined identifier
#define JE_RESULT_CANNOT_COMPILE_MULTIPLE_TIMES     (-14)   // Cannot compile an expression multiple times on a context.
#define JE_RESULT_UNKNOWN_TERM                      (-13)   // Unknown or unexpected term in expression
#define JE_RESULT_UNEXPECTED_EOF                    (-12)   // Unexpected end of token stream
#define JE_RESULT_UNEXPECTED_TOKEN                  (-11)   // Unexpected token
#define JE_RESULT_EMPTY_EXPRESSION                  (-10)   // No expression in source code.
#define JE_RESULT_UNEXPECTED_TRAILING_EXPRESSION    (-9)    // Finished parsing expression but still have tokens remaining
#define JE_RESULT_UNEXPECTED_ESCAPE_SEQUENCE        (-8)    // Found unknown or unexpected escape sequence in string.
#define JE_RESULT_UNEXPECTED_UNTERMINATED_STRING    (-7)    // Encountered end of expression before string terminator.
#define JE_RESULT_UNEXPECTED_CHARACTER              (-6)    // Unexpected character while tokenizing expression.
#define JE_RESULT_EOF                               (-5)    // Internal error, occurs when getting to end of token stream.
#define JE_RESULT_TYPE_CANNOT_CHANGE                (-4)    // Attempt was made to change the type of a constant that was already set.
#define JE_RESULT_CORRUPT                           (-3)    // Something has corrupted the internal state of the context.
#define JE_RESULT_OOM                               (-2)    // No memory is left in the internal allocator to satisfy operation.
#define JE_RESULT_FAILED                            (-1)    // Operation failed (generic)
#define JE_RESULT_SUCCESS                           (0)     // Operation succesful

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
#include <stdarg.h>
#include <assert.h>

#define JE_TYPE_UNSET                   (0)
#define JE_TYPE_INT                     (1)
#define JE_TYPE_BOOL                    (2)
#define JE_TYPE_FLOAT                   (3)
#define JE_TYPE_STRING                  (4)

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
#define JE_TOK_OP_BITWISE_XOR           (22)
#define JE_TOK_OP_PARENTHESIS_OPEN      (23)
#define JE_TOK_OP_PARENTHESIS_CLOSE     (24)
#define JE_TOK_OP_KEYWORD_INT           (25)
#define JE_TOK_OP_KEYWORD_FLOAT         (26)
#define JE_TOK_OP_KEYWORD_STRING        (27)
#define JE_TOK_OP_KEYWORD_BOOL          (28)

// These are generic node types, that will be converted into type-specific ones
// during semantic analysis.
#define JE_NODE_LOGICAL_NOT             (1)
#define JE_NODE_BITWISE_NOT             (2)
#define JE_NODE_MUL                     (3)
#define JE_NODE_DIV                     (4)
#define JE_NODE_MOD                     (5)
#define JE_NODE_SUB                     (6)
#define JE_NODE_ADD                     (7)
#define JE_NODE_LESS                    (8)
#define JE_NODE_GREATER                 (9)
#define JE_NODE_LE                      (10)
#define JE_NODE_GE                      (11)
#define JE_NODE_EQUAL                   (12)
#define JE_NODE_NOT_EQUAL               (13)
#define JE_NODE_BITWISE_AND             (14)
#define JE_NODE_BITWISE_OR              (15)
#define JE_NODE_LOGICAL_AND             (16)
#define JE_NODE_LOGICAL_OR              (17)
#define JE_NODE_CONSTANT                (18)
#define JE_NODE_NEG                     (19)
#define JE_NODE_POS                     (20)
#define JE_NODE_CAST_TO_BOOL            (21) 
#define JE_NODE_CAST_TO_INT             (22)
#define JE_NODE_CAST_TO_FLOAT           (23)
#define JE_NODE_CAST_TO_STRING          (24)
// These are the type-specific versions of the ndoes above.
#define JE_NODE_LOGICAL_NOT_BOOL        (25)
#define JE_NODE_BITWISE_NOT_INT         (26)
#define JE_NODE_MUL_FLOAT               (27)
#define JE_NODE_MUL_INT                 (28)
#define JE_NODE_DIV_FLOAT               (29)
#define JE_NODE_DIV_INT                 (30)
#define JE_NODE_MOD_INT                 (31)
#define JE_NODE_SUB_FLOAT               (32)
#define JE_NODE_SUB_INT                 (33)
#define JE_NODE_ADD_FLOAT               (34)
#define JE_NODE_ADD_INT                 (35)
#define JE_NODE_ADD_STRING              (36)
#define JE_NODE_LESS_FLOAT              (37)
#define JE_NODE_LESS_INT                (38)
#define JE_NODE_GREATER_FLOAT           (39)
#define JE_NODE_GREATER_INT             (40)
#define JE_NODE_LE_FLOAT                (41)
#define JE_NODE_LE_INT                  (42)
#define JE_NODE_GE_FLOAT                (43)
#define JE_NODE_GE_INT                  (44)
#define JE_NODE_EQUAL_BOOL              (45)
#define JE_NODE_EQUAL_INT               (46)
#define JE_NODE_EQUAL_FLOAT             (47)
#define JE_NODE_EQUAL_STRING            (48)
#define JE_NODE_NOT_EQUAL_BOOL          (49)
#define JE_NODE_NOT_EQUAL_INT           (50)
#define JE_NODE_NOT_EQUAL_FLOAT         (51)
#define JE_NODE_NOT_EQUAL_STRING        (52)
#define JE_NODE_BITWISE_AND_INT         (53)
#define JE_NODE_BITWISE_OR_INT          (54)
#define JE_NODE_LOGICAL_AND_BOOL        (55)
#define JE_NODE_LOGICAL_OR_BOOL         (56)
#define JE_NODE_CONSTANT_BOOL           (57)
#define JE_NODE_CONSTANT_INT            (58)
#define JE_NODE_CONSTANT_FLOAT          (59)
#define JE_NODE_CONSTANT_STRING         (60)
#define JE_NODE_NEG_FLOAT               (61)
#define JE_NODE_POS_FLOAT               (62)
#define JE_NODE_NEG_INT                 (63)
#define JE_NODE_POS_INT                 (64)
#define JE_NODE_FLOAT_LITERAL           (65)
#define JE_NODE_INT_LITERAL             (66)
#define JE_NODE_STRING_LITERAL          (67)
#define JE_NODE_BOOL_LITERAL            (68)
#define JE_NODE_CAST_INT_TO_STRING      (69)
#define JE_NODE_CAST_FLOAT_TO_STRING    (70)
#define JE_NODE_CAST_BOOL_TO_STRING     (71)
#define JE_NODE_CAST_STRING_TO_INT      (72)
#define JE_NODE_CAST_FLOAT_TO_INT       (73)
#define JE_NODE_CAST_BOOL_TO_INT        (74)
#define JE_NODE_CAST_INT_TO_FLOAT       (75)
#define JE_NODE_CAST_STRING_TO_FLOAT    (76)
#define JE_NODE_CAST_BOOL_TO_FLOAT      (77)
#define JE_NODE_CAST_INT_TO_BOOL        (78)
#define JE_NODE_CAST_STRING_TO_BOOL     (79)
#define JE_NODE_CAST_FLOAT_TO_BOOL      (80)

#define JE_MEM_ARENA_SIZE               (16 * 1024)
#define JE_MEM_ARENA_ALIGN              16
#define JE_MAX_STRING_CONSTANT_LENGTH   256
#define JE_MAX_OPERATOR_PRECEDENCE      9

typedef struct je_token_t {
    int         type;
    char*       source_ptr;                     // Pointer to where the token is in the source string.
    int         source_len;                     // Length of token in the source string.    
    union {
        int     int_value;
        int     bool_value;
        float   float_value;
        char*   string_value;
    };
} je_token_t;

typedef struct je_value_t {
    char*               name;
    int                 type;
    union {
        int             int_value;
        int             bool_value;
        float           float_value;
        char*           string_value;
    };
    size_t              string_value_len;
    struct je_value_t*  next;
} je_value_t;

typedef struct je_ast_node_t {
    int                     type : 8;
    int                     return_type : 7;
    int                     is_constant : 1;
    struct je_ast_node_t*   children[2];
    je_value_t              value;
    je_value_t*             constant;
} je_ast_node_t;

typedef struct je_context_t {
    char*           mem_arena;                      // Block of memory that all dynamically allocated memory is stored in. Used as a stack allocator.
    size_t          mem_arena_offset;               // Next locations in the mem_arena to allocate from.
    char*           transient_mem_arena;            // Block of memory that all transient allocations using during evaluation is stored in. Used as a stack allocator.
    size_t          transient_mem_arena_offset;     // Next locations in the transient_mem_arena_offset to allocate from.
    char*           source;                         // Pointer to source code string
    char*           read_ptr;                       // Read pointer into source string
    char*           rewind_read_ptr;                // Pointer to point in token stream we can rewind to with je_rewind_token_stream
    je_value_t*     constant_head;                  // Head of the constant linked list.
    je_value_t      result;                         // Result of the last evaluation call.
    int             error_code;                     // Last error that occured in the context.
    char            error_msg[512];                 // Error message from the last failing call.
    je_ast_node_t*  ast_root;                       // Root node in the ast graph.
    int             jit_compiled;                   // If the compiled expression is jit compiled.
    int             compiled;                       // If this context has been compiled.
} je_context_t;

int je_parse(je_context_t* context, je_ast_node_t** node, int precedence);
int je_eval_slow(je_context_t* context, je_ast_node_t* node, je_value_t* result);

int je_store_error(je_context_t* context, int error_code, const char* error_msg, ...) {  
    if (error_msg != NULL) {
        va_list va;
        va_start(va, error_msg);
        vsnprintf(context->error_msg, sizeof(context->error_msg), error_msg, va);
        va_end(va);
    } else {
        context->error_msg[0] = '\0';

        const char* error_code_msg = NULL;

        switch (error_code) {
            case JE_RESULT_EMPTY_EXPRESSION:        error_code_msg = "Empty expression"; break;
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

int je_alloc_transient(je_context_t* context, size_t size, char** ptr) {
    int align_padding = JE_MEM_ARENA_ALIGN - (((size_t)context->transient_mem_arena + context->transient_mem_arena_offset) % JE_MEM_ARENA_ALIGN);
    if (align_padding == JE_MEM_ARENA_ALIGN) {
        align_padding = 0;
    }
    size_t remaining_space = JE_MEM_ARENA_SIZE - context->transient_mem_arena_offset;
    if (remaining_space < size + JE_MEM_ARENA_ALIGN) {
        je_store_error(context, JE_RESULT_OOM, NULL);
        return context->error_code;
    }
    *ptr = context->transient_mem_arena + context->transient_mem_arena_offset + align_padding;
    context->transient_mem_arena_offset += size + align_padding;
    assert(((size_t)*ptr % JE_MEM_ARENA_ALIGN) == 0);
    return JE_RESULT_SUCCESS;
}

int je_alloc(je_context_t* context, size_t size, char** ptr) {
    int align_padding = JE_MEM_ARENA_ALIGN - (((size_t)context->mem_arena + context->mem_arena_offset) % JE_MEM_ARENA_ALIGN);
    if (align_padding == JE_MEM_ARENA_ALIGN) {
        align_padding = 0;
    }
    size_t remaining_space = JE_MEM_ARENA_SIZE - context->mem_arena_offset;
    if (remaining_space < size + JE_MEM_ARENA_ALIGN) {
        je_store_error(context, JE_RESULT_OOM, NULL);
        return context->error_code;
    }
    *ptr = context->mem_arena + context->mem_arena_offset + align_padding;
    context->mem_arena_offset += size + align_padding;
    assert(((size_t)*ptr % JE_MEM_ARENA_ALIGN) == 0);
    return JE_RESULT_SUCCESS;
}

int je_realloc_string(je_context_t* context, const char* value, je_value_t* result) {
    // If not already a string then initialize it as one.
    if (result->type != JE_TYPE_STRING) {
        result->type = JE_TYPE_STRING;
        result->string_value = NULL;
        result->string_value_len = 0;
    }
    // If we have enough space already just use the existing buffer.
    size_t len = strlen(value) + 1;
    if (result->string_value != NULL && result->string_value_len <= len) {
        strncpy(result->string_value, value, len);
        return JE_RESULT_SUCCESS;
    }
    // Otherwise alloc a new one.
    int ret = je_alloc(context, len, &result->string_value);
    if (ret < 0) {
        return ret;
    }
    strncpy(result->string_value, value, len);
    result->string_value_len = len;

    return JE_RESULT_SUCCESS;
}

int je_alloc_ast_node(je_context_t* context, je_ast_node_t** node) {
    int ret = je_alloc(context, sizeof(je_ast_node_t), (char**)node);
    if (ret < 0) {
        return ret;
    }
    memset(*node, 0, sizeof(je_ast_node_t));
    return JE_RESULT_SUCCESS;
}

const char* je_type_name(int type) {
    switch (type) {
        case JE_TYPE_UNSET:     return "unset";
        case JE_TYPE_STRING:    return "string";
        case JE_TYPE_INT:       return "int";
        case JE_TYPE_FLOAT:     return "float";
        case JE_TYPE_BOOL:      return "bool";
        default: {
            assert(0);
            return "unkown";
        }
    }
    return "";
}

const char* je_node_name(int type) {
    switch (type) {
        case JE_NODE_LOGICAL_NOT:           return "logical_not";
        case JE_NODE_BITWISE_NOT:           return "bitwise_not";
        case JE_NODE_MUL:                   return "mul";
        case JE_NODE_DIV:                   return "div";
        case JE_NODE_MOD:                   return "mod";
        case JE_NODE_SUB:                   return "sub";
        case JE_NODE_ADD:                   return "add";
        case JE_NODE_LESS:                  return "less";
        case JE_NODE_GREATER:               return "greater";
        case JE_NODE_LE:                    return "less_equal";
        case JE_NODE_GE:                    return "greater_equal";
        case JE_NODE_EQUAL:                 return "equal";
        case JE_NODE_NOT_EQUAL:             return "not_equal";
        case JE_NODE_BITWISE_AND:           return "bitwise_and";
        case JE_NODE_BITWISE_OR:            return "bitwise_or";
        case JE_NODE_LOGICAL_AND:           return "logical_and";
        case JE_NODE_LOGICAL_OR:            return "logical_or";
        case JE_NODE_CONSTANT:              return "constant";
        case JE_NODE_NEG:                   return "neg";
        case JE_NODE_POS:                   return "pos";
        case JE_NODE_CAST_TO_BOOL:          return "cast_to_bool";
        case JE_NODE_CAST_TO_INT:           return "cast_to_int";
        case JE_NODE_CAST_TO_FLOAT:         return "cast_to_float";
        case JE_NODE_CAST_TO_STRING:        return "cast_to_string";
        case JE_NODE_LOGICAL_NOT_BOOL:      return "logical_not_bool";
        case JE_NODE_BITWISE_NOT_INT:       return "bitwise_not_int";
        case JE_NODE_MUL_FLOAT:             return "mul_float";
        case JE_NODE_MUL_INT:               return "mul_int";
        case JE_NODE_DIV_FLOAT:             return "div_float";
        case JE_NODE_DIV_INT:               return "div_int";
        case JE_NODE_MOD_INT:               return "mod_int";
        case JE_NODE_SUB_FLOAT:             return "sub_float";
        case JE_NODE_SUB_INT:               return "sub_int";
        case JE_NODE_ADD_FLOAT:             return "add_float";
        case JE_NODE_ADD_INT:               return "add_int";
        case JE_NODE_ADD_STRING:            return "add_string";
        case JE_NODE_LESS_FLOAT:            return "less_float";
        case JE_NODE_LESS_INT:              return "less_int";
        case JE_NODE_GREATER_FLOAT:         return "greater_float";
        case JE_NODE_GREATER_INT:           return "greater_int";
        case JE_NODE_LE_FLOAT:              return "less_equal_float";
        case JE_NODE_LE_INT:                return "less_equal_int";
        case JE_NODE_GE_FLOAT:              return "greater_equal_float";
        case JE_NODE_GE_INT:                return "greater_equal_int";
        case JE_NODE_EQUAL_BOOL:            return "equal_bool";
        case JE_NODE_EQUAL_INT:             return "equal_int";
        case JE_NODE_EQUAL_FLOAT:           return "equal_float";
        case JE_NODE_EQUAL_STRING:          return "equal_string";
        case JE_NODE_NOT_EQUAL_BOOL:        return "not_equal_bool";
        case JE_NODE_NOT_EQUAL_INT:         return "not_equal_int";
        case JE_NODE_NOT_EQUAL_FLOAT:       return "not_equal_float";
        case JE_NODE_NOT_EQUAL_STRING:      return "not_equal_string";
        case JE_NODE_BITWISE_AND_INT:       return "bitwise_and_int";
        case JE_NODE_BITWISE_OR_INT:        return "bitwise_or_int";
        case JE_NODE_LOGICAL_AND_BOOL:      return "logical_and_bool";
        case JE_NODE_LOGICAL_OR_BOOL:       return "logical_or_bool";
        case JE_NODE_CONSTANT_BOOL:         return "constant_bool";
        case JE_NODE_CONSTANT_INT:          return "constant_int";
        case JE_NODE_CONSTANT_FLOAT:        return "constant_float";
        case JE_NODE_CONSTANT_STRING:       return "constant_string";
        case JE_NODE_NEG_FLOAT:             return "neg_float";
        case JE_NODE_POS_FLOAT:             return "pos_float";
        case JE_NODE_NEG_INT:               return "neg_int";
        case JE_NODE_POS_INT:               return "pos_int";
        case JE_NODE_FLOAT_LITERAL:         return "float_literal";
        case JE_NODE_INT_LITERAL:           return "int_literal";
        case JE_NODE_STRING_LITERAL:        return "string_literal";
        case JE_NODE_BOOL_LITERAL:          return "bool_literal";
        case JE_NODE_CAST_INT_TO_STRING:    return "cast_int_to_string";
        case JE_NODE_CAST_FLOAT_TO_STRING:  return "cast_float_to_string";
        case JE_NODE_CAST_BOOL_TO_STRING:   return "cast_bool_to_string";
        case JE_NODE_CAST_STRING_TO_INT:    return "cast_string_to_int";
        case JE_NODE_CAST_FLOAT_TO_INT:     return "cast_float_to_int";
        case JE_NODE_CAST_BOOL_TO_INT:      return "cast_bool_to_int";
        case JE_NODE_CAST_INT_TO_FLOAT:     return "cast_int_to_float";
        case JE_NODE_CAST_STRING_TO_FLOAT:  return "cast_string_to_float";
        case JE_NODE_CAST_BOOL_TO_FLOAT:    return "cast_bool_to_float";
        case JE_NODE_CAST_INT_TO_BOOL:      return "cast_int_to_bool";
        case JE_NODE_CAST_STRING_TO_BOOL:   return "cast_string_to_bool";
        case JE_NODE_CAST_FLOAT_TO_BOOL:    return "cast_float_to_bool";
        default: {
            assert(0); 
            return "unkown";
        }
    }
    return "";
}

je_value_t* je_find_constant(je_context_t* context, const char* name) {
    for (je_value_t* value = context->constant_head; value; value->next) {
        if (strcmp(value->name, name) == 0) {
            return value;
        }
    }
    return NULL;
}

int je_find_or_create_constant(je_context_t* context, const char* name, int type, je_value_t** constant) {
    *constant = je_find_constant(context, name);
    if (*constant != NULL) {
        if ((*constant)->type != type) {
            return je_store_error(context, JE_RESULT_TYPE_CANNOT_CHANGE, NULL);
        }
        return JE_RESULT_SUCCESS;
    }
    int ret = je_alloc(context, sizeof(je_value_t), (char**)constant);
    if (ret < 0) {
        return ret;
    }

    size_t len = strlen(name) + 1;
    ret = je_alloc(context, len, &(*constant)->name);
    if (ret < 0) {
        return ret;
    }
    strncpy((*constant)->name, name, len);

    return JE_RESULT_SUCCESS;    
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

    int ret = je_realloc_string(context, buffer, value);
    if (ret < 0) {
        return ret;
    }

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
            value->bool_value = (strcmp(value->string_value, "false") != 0 && strcmp(value->string_value, "0") != 0);
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
    context->transient_mem_arena = malloc(JE_MEM_ARENA_SIZE);
    if (context->transient_mem_arena == NULL) {
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
    int ret = je_find_or_create_constant(context, name, JE_TYPE_INT, &constant);
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
    int ret = je_find_or_create_constant(context, name, JE_TYPE_FLOAT, &constant);
    if (ret < 0) {
        return ret;
    }

    constant->type = JE_TYPE_FLOAT;
    constant->float_value = value;
    constant->next = context->constant_head;
    context->constant_head = constant;
    return JE_RESULT_SUCCESS;
}

int je_bind_constant_bool(je_context_t* context, const char* name, int value) {
    je_value_t* constant = NULL;
    int ret = je_find_or_create_constant(context, name, JE_TYPE_BOOL, &constant);
    if (ret < 0) {
        return ret;
    }

    constant->type = JE_TYPE_BOOL;
    constant->bool_value = value;
    constant->next = context->constant_head;
    context->constant_head = constant;
    return JE_RESULT_SUCCESS;
}

int je_bind_constant_string(je_context_t* context, const char* name, const char* value) {
    size_t value_length = strlen(value);

    je_value_t* constant = NULL;
    int ret = je_find_or_create_constant(context, name, JE_TYPE_STRING, &constant);
    if (ret < 0) {
        return ret;
    }

    ret = je_realloc_string(context, value, constant);
    if (ret < 0) {
        return ret;
    }

    constant->type = JE_TYPE_STRING;
    constant->next = context->constant_head;
    context->constant_head = constant;
    return JE_RESULT_SUCCESS;
}

int je_result_int(je_context_t* context, int* value) {
    if (context->result.type != JE_TYPE_INT) {
        int ret = je_coerce_to_int(context, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    *value = context->result.int_value;
    return JE_RESULT_SUCCESS;
}

int je_result_float(je_context_t* context, float* value) {
    if (context->result.type != JE_TYPE_FLOAT) {
        int ret = je_coerce_to_float(context, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    *value = context->result.float_value;
    return JE_RESULT_SUCCESS;
}

int je_result_string(je_context_t* context, const char** value) {
    if (context->result.type != JE_TYPE_STRING) {
        int ret = je_coerce_to_string(context, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    *value = context->result.string_value;
    return JE_RESULT_SUCCESS;
}

int je_result_bool(je_context_t* context, int* value) {
    if (context->result.type != JE_TYPE_BOOL) {
        int ret = je_coerce_to_bool(context, &context->result);
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
    tok->source_len = 1;
    char c = *context->read_ptr;
    if (c == '\0') {
        return JE_RESULT_EOF;
    }

    context->read_ptr++;

    switch (c) {
        case '*': tok->type = JE_TOK_OP_MUL;                break;
        case '/': tok->type = JE_TOK_OP_DIV;                break;
        case '%': tok->type = JE_TOK_OP_MOD;                break;
        case '+': tok->type = JE_TOK_OP_ADD;                break;
        case '-': tok->type = JE_TOK_OP_SUB;                break;
        case '(': tok->type = JE_TOK_OP_PARENTHESIS_OPEN;   break;
        case ')': tok->type = JE_TOK_OP_PARENTHESIS_CLOSE;  break;
        case '~': tok->type = JE_TOK_OP_BITWISE_NOT;        break;
        case '^': tok->type = JE_TOK_OP_BITWISE_XOR;        break;
        case '<': {
            if (context->read_ptr[0] == '=') {
                tok->type = JE_TOK_OP_LE;
                tok->source_len++;
                context->read_ptr++;
            } else {
                tok->type = JE_TOK_OP_LESS;
            }
            break;
        }
        case '>': {
            if (context->read_ptr[0] == '=') {
                tok->type = JE_TOK_OP_GE;
                tok->source_len++;
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
                tok->source_len++;
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
                tok->source_len++;
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
                tok->source_len++;
                context->read_ptr++;
            }
            else {
                tok->type = JE_TOK_OP_BITWISE_OR;
            }
            break;
        }
        case '"': {
            tok->type = JE_TOK_STRING;

            char string_buffer[JE_MAX_STRING_CONSTANT_LENGTH];
            string_buffer[0] = '\0';

            int len = 0;
            while (1) {
                c = context->read_ptr[0];
                context->read_ptr++;

                if (c == '\0') {
                    return je_store_error(context, JE_RESULT_UNEXPECTED_UNTERMINATED_STRING, "Unterminated string\n\t%s", tok->source_ptr);
                } else if (c == '"') {
                    break;
                } else if (c == '\\') {
                    char escape_c = context->read_ptr[0];
                    context->read_ptr++;
                    tok->source_len++;
                    switch (escape_c) {
                        case '"':   break;
                        case 'a':   string_buffer[len++] = '\a'; break;
                        case 'b':   string_buffer[len++] = '\b'; break;
                        case 'f':   string_buffer[len++] = '\f'; break;
                        case 'n':   string_buffer[len++] = '\n'; break;
                        case 'r':   string_buffer[len++] = '\r'; break;
                        case 't':   string_buffer[len++] = '\t'; break;
                        case 'v':   string_buffer[len++] = '\v'; break;
                        case '\\':  string_buffer[len++] = '\\'; break;
                        case '\?':  string_buffer[len++] = '?';  break;
                        // TODO: Maybe support hex/octal/unicode values here?
                        default: {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_ESCAPE_SEQUENCE, "Found unexpected escape sequence '%c'\n\t%s", escape_c, tok->source_ptr);
                        }
                    }
                } else {
                    string_buffer[len++] = c;
                }
            }

            // Copy string buffer to storage.
            int ret = je_alloc(context, len + 1, &tok->string_value);
            if (ret < 0) {
                return ret;
            }
            strncpy(tok->string_value, string_buffer, len + 1);

            break;
        }
        default: {
            // Identifier
            if ((c >= 'a' && c <= 'z') || 
                (c >= 'A' && c <= 'Z') || 
                 c == '_') {
                tok->type = JE_TOK_IDENTIFIER;
                do {
                    c = context->read_ptr[0];
                    if ( ! ((c >= 'a' && c <= 'z') || 
                            (c >= 'A' && c <= 'Z') || 
                            (c >= '0' && c <= '9') || 
                             c == '_') ) {
                        break;
                    }
                    tok->source_len++;
                    context->read_ptr++;
                } while (context->read_ptr[0]);

                if (strncmp(tok->source_ptr, "false", tok->source_len) == 0) {
                    tok->type = JE_TOK_BOOL;
                    tok->bool_value = 0;
                } else if (strncmp(tok->source_ptr, "true", tok->source_len) == 0) {
                    tok->type = JE_TOK_BOOL;
                    tok->bool_value = 1;
                } else if (strncmp(tok->source_ptr, "int", tok->source_len) == 0) {
                    tok->type = JE_TOK_OP_KEYWORD_INT;
                } else if (strncmp(tok->source_ptr, "float", tok->source_len) == 0) {
                    tok->type = JE_TOK_OP_KEYWORD_FLOAT;
                } else if (strncmp(tok->source_ptr, "bool", tok->source_len) == 0) {
                    tok->type = JE_TOK_OP_KEYWORD_BOOL;
                } else if (strncmp(tok->source_ptr, "string", tok->source_len) == 0) {
                    tok->type = JE_TOK_OP_KEYWORD_STRING;
                }
            // Number (or + / - operator)
            } else if ((c >= '0' && c <= '9') || c == '.') {
                char start_c = c;
                tok->type = JE_TOK_INT;
                int found_hex = 0;
                int found_exponent = 0;
                do {
                    c = context->read_ptr[0];
                    if (c >= '0' && c <= '9') {
                        // Just consume these
                    } else if (found_hex && ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
                        // Just consume these
                    } else if (c == '.') {
                        if (tok->type == JE_TOK_FLOAT) {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, "Floating point values cannot contain multiple radix's\n\t%s", tok->source_ptr);
                        }
                        tok->type = JE_TOK_FLOAT;
                    } else if (c == 'e') {
                        if (found_hex) {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, "Exponent values cannot have a hex prefix.\n\t%s", tok->source_ptr);
                        }
                        if (found_exponent) {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, "Number values cannot contain multiple exponent values\n\t%s", tok->source_ptr);
                        }
                        found_exponent = 1;
                        tok->type = JE_TOK_FLOAT;

                        if (context->read_ptr[1] == '-' || context->read_ptr[1] == '+') {
                            tok->source_len++;
                            context->read_ptr++;
                        }
                    } else if (c == 'f') {
                        context->read_ptr++; // Skip and ignore the f
                        tok->type = JE_TOK_FLOAT;
                        break;
                    } else if (c == 'x' || c == 'X') {
                        tok->type = JE_TOK_INT;
                        if (found_exponent) {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, "Exponent values cannot have a hex prefix.\n\t%s", tok->source_ptr);
                        }
                        if (found_hex) {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, "Number values cannot contain multiple hex prefixes\n\t%s", tok->source_ptr);
                        }
                        if (tok->source_ptr[0] != '0' || tok->source_len != 1) {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, "Invalid hex prefix in number\n\t%s", tok->source_ptr);
                        }
                        found_hex = 1;
                    } else {
                        break;
                    }
                    tok->source_len++;
                    context->read_ptr++;
                } while (context->read_ptr[0]);

                if (tok->type == JE_TOK_INT) {
                    c = context->read_ptr[0];
                    context->read_ptr[0] = '\0';
                    if (found_hex) {
                        tok->int_value = strtol(tok->source_ptr + 2, NULL, 16);
                    } else {
                        tok->int_value = strtol(tok->source_ptr, NULL, 10);
                    }
                    context->read_ptr[0] = c;
                } else if (tok->type == JE_TOK_FLOAT) {
                    c = context->read_ptr[0];
                    context->read_ptr[0] = '\0';
                    tok->float_value = strtof(tok->source_ptr, NULL);
                    context->read_ptr[0] = c;
                }
            // Unknown
            } else {
                return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, "Encountered unexpected character '%c'\n\t%s", c, tok->source_ptr);
            }
        }
    }

    return JE_RESULT_SUCCESS;
}

void je_store_token_rewind_point(je_context_t* context) {
    context->rewind_read_ptr = context->read_ptr;
}
void je_rewind_token_stream(je_context_t* context) {
    context->read_ptr = context->rewind_read_ptr;
}

int je_parse_term(je_context_t* context, je_ast_node_t** node) {

    je_token_t tok;
    int ret = je_read_token(context, &tok);
    if (ret == JE_RESULT_EOF) {
        return je_store_error(context, JE_RESULT_UNEXPECTED_EOF, "Unexpected end of token stream.");
    }
    else if (ret < 0) {
        return ret;
    }

    switch (tok.type) {
        case JE_TOK_OP_PARENTHESIS_OPEN: {
            ret = je_parse(context, node, JE_MAX_OPERATOR_PRECEDENCE);
            if (ret < 0) {
                return ret;
            }

            int ret = je_read_token(context, &tok);
            if (ret == JE_RESULT_EOF) {
                return je_store_error(context, JE_RESULT_UNEXPECTED_EOF, "Unexpected end of token stream, expecting closing parenthesis");
            } else if (ret < 0) {
                return ret;
            }

            if (tok.type != JE_TOK_OP_PARENTHESIS_CLOSE) {
                return je_store_error(context, JE_RESULT_UNEXPECTED_TOKEN, "Unexpected token, expecting closing parenthesis\n\t%s", tok.source_ptr);
            }
            break;
        }
        case JE_TOK_STRING: 
        case JE_TOK_IDENTIFIER: {
            ret = je_alloc_ast_node(context, node);
            if (ret < 0) {
                return ret;
            }
            (*node)->type = (tok.type == JE_TOK_IDENTIFIER ? JE_NODE_CONSTANT : JE_NODE_STRING_LITERAL);

            char c = *(tok.source_ptr + tok.source_len);
            *(tok.source_ptr + tok.source_len) = '\0';

            (*node)->value.type = JE_TYPE_UNSET;
            int ret = je_realloc_string(context, tok.source_ptr, &(*node)->value);
            if (ret < 0) {
                return ret;
            }

            *(tok.source_ptr + tok.source_len) = c;

            break;
        }
        case JE_TOK_FLOAT: {
            ret = je_alloc_ast_node(context, node);
            if (ret < 0) {
                return ret;
            }
            (*node)->type = JE_NODE_FLOAT_LITERAL;
            (*node)->value.float_value = tok.float_value;
            break;
        }
        case JE_TOK_INT: {
            ret = je_alloc_ast_node(context, node);
            if (ret < 0) {
                return ret;
            }
            (*node)->type = JE_NODE_INT_LITERAL;
            (*node)->value.int_value = tok.int_value;
            break;
        }
        case JE_TOK_BOOL: {
            ret = je_alloc_ast_node(context, node);
            if (ret < 0) {
                return ret;
            }
            (*node)->type = JE_NODE_INT_LITERAL;
            (*node)->value.bool_value = tok.bool_value;
            break;
        }
        default: {
            return je_store_error(context, JE_RESULT_UNKNOWN_TERM, "Unexpected term\n\t%s", tok.source_ptr);
        }
    }

    return JE_RESULT_SUCCESS;
}

int je_parse(je_context_t* context, je_ast_node_t** node, int precedence) {
    int nodeType = 0;
    int ret = 0;

    // Lowest precedence level is always a term.
    if (precedence == 0) {
        return je_parse_term(context, node);        
    }

    // Look for any unary tokens (! ~ (int) (float) (string) (bool)) before we try parsing
    // an lvalue as we won't have any lvalue in that case.
    int is_unary = 0;
    if (precedence == 1) {
        je_store_token_rewind_point(context);

        je_token_t unary_token;
        ret = je_read_token(context, &unary_token);
        if (ret == JE_RESULT_SUCCESS) {
            if (unary_token.type == JE_TOK_OP_NOT || 
                unary_token.type == JE_TOK_OP_BITWISE_NOT || 
                unary_token.type == JE_TOK_OP_ADD || 
                unary_token.type == JE_TOK_OP_SUB) {
                is_unary = 1;
            } else if (unary_token.type == JE_TOK_OP_PARENTHESIS_OPEN) {
                je_token_t type_token;
                ret = je_read_token(context, &type_token);
                if (ret == JE_RESULT_SUCCESS) {
                    je_token_t close_brace_token;
                    ret = je_read_token(context, &close_brace_token);
                    if (ret == JE_RESULT_SUCCESS) {
                        if (close_brace_token.type == JE_TOK_OP_PARENTHESIS_CLOSE &&
                            (
                                type_token.type == JE_TOK_OP_KEYWORD_BOOL ||
                                type_token.type == JE_TOK_OP_KEYWORD_INT ||
                                type_token.type == JE_TOK_OP_KEYWORD_FLOAT ||
                                type_token.type == JE_TOK_OP_KEYWORD_STRING
                            )) {
                            is_unary = 1;
                        }
                    }
                }
            }
        }

        je_rewind_token_stream(context);
    }

    // Parse the lvalue if we need one
    je_ast_node_t* lvalue = NULL;
    if (!is_unary) {
        int ret = je_parse(context, &lvalue, precedence - 1);
        if (ret < 0) {
            return ret;
        }
    }

    while (1) {
        char* start_read_ptr = context->read_ptr;

        je_token_t tok;
        ret = je_read_token(context, &tok);
        if (ret == JE_RESULT_EOF) {
            context->read_ptr = start_read_ptr;
            *node = lvalue;
            return JE_RESULT_SUCCESS;
        } else if (ret < 0) {
            return ret;
        }

        // Precedence levels are based on those from C

        // Precendence 1
        if (tok.type == JE_TOK_OP_NOT && precedence == 1) {
            nodeType = JE_NODE_LOGICAL_NOT;
        } else if (tok.type == JE_TOK_OP_BITWISE_NOT && precedence == 1) {
            nodeType = JE_NODE_BITWISE_NOT;
        } else if (tok.type == JE_TOK_OP_ADD && is_unary && precedence == 1) {
            nodeType = JE_NODE_POS;
        } else if (tok.type == JE_TOK_OP_SUB && is_unary && precedence == 1) {
            nodeType = JE_NODE_NEG;
        } else if (tok.type == JE_TOK_OP_PARENTHESIS_OPEN && is_unary && precedence == 1) {
            ret = je_read_token(context, &tok);
            if (ret < 0) {
                return ret;
            }

            if (tok.type == JE_TOK_OP_KEYWORD_BOOL) {
                nodeType = JE_NODE_CAST_TO_BOOL;
            } else if (tok.type == JE_TOK_OP_KEYWORD_INT) {
                nodeType = JE_NODE_CAST_TO_INT;
            } else if (tok.type == JE_TOK_OP_KEYWORD_FLOAT) {
                nodeType = JE_NODE_CAST_TO_FLOAT;
            } else if (tok.type == JE_TOK_OP_KEYWORD_STRING) {
                nodeType = JE_NODE_CAST_TO_STRING;
            } else {
                return je_store_error(context, JE_RESULT_UNEXPECTED_TOKEN, "Unexpected token, expected bool, int, float or string keyword\n\t%s", tok.source_ptr);
            }

            ret = je_read_token(context, &tok);
            if (ret < 0) {
                return ret;
            }

            if (tok.type != JE_TOK_OP_PARENTHESIS_CLOSE) {
                return je_store_error(context, JE_RESULT_UNEXPECTED_TOKEN, "Unexpected token, expected }\n\t%s", tok.source_ptr);
            }
        }
        // Precendence 2
        else if (tok.type == JE_TOK_OP_MUL && precedence == 2) {
            nodeType = JE_NODE_MUL;
        } else if (tok.type == JE_TOK_OP_DIV && precedence == 2) {
            nodeType = JE_NODE_DIV;
        } else if (tok.type == JE_TOK_OP_MOD && precedence == 2) {
            nodeType = JE_NODE_MOD;
        }
        // Precedence 3
        else if (tok.type == JE_TOK_OP_SUB && precedence == 3) {
            nodeType = JE_NODE_SUB;
        } else if (tok.type == JE_TOK_OP_ADD && precedence == 3) {
            nodeType = JE_NODE_ADD;
        }
        // Precedence 4
        else if (tok.type == JE_TOK_OP_LESS && precedence == 4) {
            nodeType = JE_NODE_LESS;
        } else if (tok.type == JE_TOK_OP_GREATER && precedence == 4) {
            nodeType = JE_NODE_GREATER;
        } else if (tok.type == JE_TOK_OP_LE && precedence == 4) {
            nodeType = JE_NODE_LE;
        } else if (tok.type == JE_TOK_OP_GE && precedence == 4) {
            nodeType = JE_NODE_GE;
        }
        // Precedence 5
        else if (tok.type == JE_TOK_OP_EQUAL && precedence == 5) {
            nodeType = JE_NODE_EQUAL;
        } else if (tok.type == JE_TOK_OP_NOT_EQUAL && precedence == 5) {
            nodeType = JE_NODE_NOT_EQUAL;
        }
        // Precedence 6
        else if (tok.type == JE_TOK_OP_BITWISE_AND && precedence == 6) {
            nodeType = JE_NODE_BITWISE_AND;
        }
        // Precedence 7
        else if (tok.type == JE_TOK_OP_BITWISE_OR && precedence == 7) {
            nodeType = JE_NODE_BITWISE_OR;
        }
        // Precedence 8
        else if (tok.type == JE_TOK_OP_LOGICAL_AND && precedence == 8) {
            nodeType = JE_NODE_LOGICAL_AND;
        }
        // Precedence 9
        else if (tok.type == JE_TOK_OP_LOGICAL_OR && precedence == 9) {
            nodeType = JE_NODE_LOGICAL_OR;
        }
        // Not relevant op for this precedence 
        else {
            context->read_ptr = start_read_ptr;
            *node = lvalue;
            return JE_RESULT_SUCCESS;
        }

        je_ast_node_t* rvalue = NULL;
        ret = je_parse(context, &rvalue, precedence - 1);
        if (ret < 0) {
            return ret;
        }

        ret = je_alloc_ast_node(context, node);
        if (ret < 0) {
            return ret;
        }

        (*node)->type = nodeType;
        (*node)->children[0] = lvalue;
        (*node)->children[1] = rvalue;

        lvalue = *node;
    }

    return JE_RESULT_SUCCESS;
}

int je_type_balance(je_ast_node_t* lvalue, je_ast_node_t* rvalue) {
    if (lvalue == NULL) {
        return rvalue->return_type;
    } else if (rvalue == NULL) {
        return lvalue->return_type;
    } else if (lvalue->return_type == JE_TYPE_STRING || rvalue->return_type == JE_TYPE_STRING) {
        return JE_TYPE_STRING;
    } else if (lvalue->return_type == JE_TYPE_FLOAT || rvalue->return_type == JE_TYPE_FLOAT) {
        return JE_TYPE_FLOAT;
    } else if (lvalue->return_type == JE_TYPE_BOOL || rvalue->return_type == JE_TYPE_BOOL) {
        return JE_TYPE_BOOL;
    }
    return JE_TYPE_INT;
}

int je_insert_cast(je_context_t* context, je_ast_node_t** node, int type) {
    int op_type = 0;
    switch (type) {
        case JE_TYPE_STRING: {
            switch ((*node)->return_type) {
                case JE_TYPE_INT:       op_type = JE_NODE_CAST_INT_TO_STRING; break;
                case JE_TYPE_STRING:    return JE_RESULT_SUCCESS;
                case JE_TYPE_FLOAT:     op_type = JE_NODE_CAST_FLOAT_TO_STRING; break;
                case JE_TYPE_BOOL:      op_type = JE_NODE_CAST_BOOL_TO_STRING; break;
            }
            break;
        }
        case JE_TYPE_INT: {
            switch ((*node)->return_type) {
                case JE_TYPE_INT:       return JE_RESULT_SUCCESS;
                case JE_TYPE_STRING:    op_type = JE_NODE_CAST_STRING_TO_INT; break;
                case JE_TYPE_FLOAT:     op_type = JE_NODE_CAST_FLOAT_TO_INT; break;
                case JE_TYPE_BOOL:      op_type = JE_NODE_CAST_BOOL_TO_INT; break;
            }
            break;
        }
        case JE_TYPE_FLOAT: {
            switch ((*node)->return_type) {
                case JE_TYPE_INT:       op_type = JE_NODE_CAST_INT_TO_FLOAT; break;
                case JE_TYPE_STRING:    op_type = JE_NODE_CAST_STRING_TO_FLOAT; break;
                case JE_TYPE_FLOAT:     return JE_RESULT_SUCCESS;
                case JE_TYPE_BOOL:      op_type = JE_NODE_CAST_BOOL_TO_FLOAT; break;
            }
            break;
        }
        case JE_TYPE_BOOL: {
            switch ((*node)->return_type) {
                case JE_TYPE_INT:       op_type = JE_NODE_CAST_INT_TO_BOOL; break;
                case JE_TYPE_STRING:    op_type = JE_NODE_CAST_STRING_TO_BOOL; break;
                case JE_TYPE_FLOAT:     op_type = JE_NODE_CAST_FLOAT_TO_BOOL; break;
                case JE_TYPE_BOOL:      return JE_RESULT_SUCCESS;
            }
            break;
        }
    }

    je_ast_node_t* cast_node;
    int ret = je_alloc_ast_node(context, &cast_node);
    if (ret < 0) {
        return ret;
    }

    cast_node->type = op_type;
    cast_node->return_type = type;
    cast_node->children[1] = *node; // casts are consider unary so we they go in the rvalue
    *node = cast_node;

    return JE_RESULT_SUCCESS;
}

int je_implicit_conversion(je_context_t* context, je_ast_node_t* node) {
    switch (node->return_type) {
        case JE_TYPE_STRING: {
            for (int i = 0; i < 2; i++) {
                if (node->children[i] != NULL && node->children[i]->return_type != JE_TYPE_STRING) {
                    return je_store_error(context, JE_RESULT_CANNOT_IMPLICITLY_CAST, "Cannot implicitly cast from '%s' to '%s'.", je_type_name(node->children[i]->return_type), je_type_name(JE_TYPE_FLOAT));
                }
            }
            break;
        }
        case JE_TYPE_INT: {
            for (int i = 0; i < 2; i++) {
                if (node->children[i] != NULL && node->children[i]->return_type != JE_TYPE_INT) {
                    if (node->children[i]->return_type == JE_TYPE_FLOAT) {
                        int ret = je_insert_cast(context, &node->children[i], JE_TYPE_INT);
                        if (ret < 0) {
                            return ret;
                        }
                    }
                    else {
                        return je_store_error(context, JE_RESULT_CANNOT_IMPLICITLY_CAST, "Cannot implicitly cast from '%s' to '%s'.", je_type_name(node->children[i]->return_type), je_type_name(JE_TYPE_FLOAT));
                    }
                }
            }
            break;
        }
        case JE_TYPE_FLOAT: {
            for (int i = 0; i < 2; i++) {
                if (node->children[i] != NULL && node->children[i]->return_type != JE_TYPE_FLOAT) {
                    if (node->children[i]->return_type == JE_TYPE_INT) {
                        int ret = je_insert_cast(context, &node->children[i], JE_TYPE_FLOAT);
                        if (ret < 0) {
                            return ret;
                        }
                    }
                    else {
                        return je_store_error(context, JE_RESULT_CANNOT_IMPLICITLY_CAST, "Cannot implicitly cast from '%s' to '%s'.", je_type_name(node->children[i]->return_type), je_type_name(JE_TYPE_FLOAT));
                    }
                }
            }
            break;
        }
        case JE_TYPE_BOOL: {
            for (int i = 0; i < 2; i++) {
                if (node->children[i]->return_type != JE_TYPE_BOOL) {
                    int ret = je_insert_cast(context, &node->children[i], JE_TYPE_BOOL);
                    if (ret < 0) {
                        return ret;
                    }
                }
            }
            break;
        }
    }
    return JE_RESULT_SUCCESS;
}

void je_print_ast(je_ast_node_t* node, int depth, int childIndex) {
    for (int i = 0; i < depth; i++) {
        printf("\t");
    }
    printf("[%i] ", childIndex);
    if (node->return_type != JE_TYPE_UNSET) {
        printf("[%s] ", je_type_name(node->return_type));
    }
    if (node->is_constant) {
        printf("const ");
    }
    switch (node->type) {
        case JE_NODE_CONSTANT_BOOL:     printf("%s (%i)", je_node_name(node->type), node->constant->bool_value); break;
        case JE_NODE_CONSTANT_STRING:   printf("%s (%s)", je_node_name(node->type), node->constant->string_value); break;
        case JE_NODE_CONSTANT_FLOAT:    printf("%s (%f)", je_node_name(node->type), node->constant->float_value); break;
        case JE_NODE_CONSTANT_INT:      printf("%s (%i)", je_node_name(node->type), node->constant->int_value); break;
        case JE_NODE_BOOL_LITERAL:      printf("%s (%i)", je_node_name(node->type), node->value.bool_value); break;
        case JE_NODE_STRING_LITERAL:    printf("%s (%s)", je_node_name(node->type), node->value.string_value); break;
        case JE_NODE_FLOAT_LITERAL:     printf("%s (%f)", je_node_name(node->type), node->value.float_value); break;
        case JE_NODE_INT_LITERAL:       printf("%s (%i)", je_node_name(node->type), node->value.int_value); break;
        default:                        printf("%s", je_node_name(node->type)); break;
    }
    printf("\n");
    for (int i = 0; i < 2; i++) {
        if (node->children[i] != NULL) {
            je_print_ast(node->children[i], depth + 1, i);
        }
    }
}

int je_semant(je_context_t* context, je_ast_node_t** node_ptr) {
    je_ast_node_t* node = *node_ptr;
    int ret = 0;

    // Recurse through all the children to draw up the return type.
    if (node->children[0] != NULL) {
        ret = je_semant(context, &node->children[0]);
        if (ret < 0) {
            return ret;
        }
    }
    if (node->children[1] != NULL) {
        ret = je_semant(context, &node->children[1]);
        if (ret < 0) {
            return ret;
        }
    }

    // Find the return type of the node.
    switch (node->type) {
        case JE_NODE_CONSTANT: {
            je_value_t* constant = je_find_constant(context, node->value.string_value);
            if (constant == NULL) {
                return je_store_error(context, JE_RESULT_UNDEFINED_IDENTIFIER, "Undefined identifier '%s'", node->value.string_value);
            }
            node->constant = constant;
            node->return_type = constant->type;
            switch (node->return_type) {
                case JE_TYPE_STRING:    node->type = JE_NODE_CONSTANT_STRING;   break;
                case JE_TYPE_FLOAT:     node->type = JE_NODE_CONSTANT_FLOAT;    break;
                case JE_TYPE_INT:       node->type = JE_NODE_CONSTANT_INT;      break;
                case JE_TYPE_BOOL:      node->type = JE_NODE_CONSTANT_BOOL;     break;
            }
            break;
        }
        case JE_NODE_FLOAT_LITERAL: {
            node->return_type = JE_TYPE_FLOAT;
            break;
        }
        case JE_NODE_INT_LITERAL: {
            node->return_type = JE_TYPE_INT;
            break;
        }
        case JE_NODE_STRING_LITERAL: {
            node->return_type = JE_TYPE_STRING;
            break;
        }
        case JE_NODE_BOOL_LITERAL: {
            node->return_type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_CAST_TO_FLOAT: {
            switch (node->children[1]->return_type) {
                case JE_TYPE_STRING:    node->type = JE_NODE_CAST_STRING_TO_FLOAT; break;
                case JE_TYPE_FLOAT:     *node_ptr = node->children[0]; break; // Replace node with lvalue as this is a noop.
                case JE_TYPE_INT:       node->type = JE_NODE_CAST_INT_TO_FLOAT; break;
                case JE_TYPE_BOOL:      node->type = JE_NODE_CAST_BOOL_TO_FLOAT; break;
            }
            node->return_type = JE_TYPE_FLOAT;
            break;
        }
        case JE_NODE_CAST_TO_INT: {
            switch (node->children[1]->return_type) {
                case JE_TYPE_STRING:    node->type = JE_NODE_CAST_STRING_TO_INT; break;
                case JE_TYPE_FLOAT:     node->type = JE_NODE_CAST_FLOAT_TO_INT; break;
                case JE_TYPE_INT:       *node_ptr = node->children[0]; break; // Replace node with lvalue as this is a noop.
                case JE_TYPE_BOOL:      node->type = JE_NODE_CAST_BOOL_TO_INT; break;
            }
            node->return_type = JE_TYPE_INT;
            break;
        }
        case JE_NODE_CAST_TO_STRING: {
            switch (node->children[1]->return_type) {
                case JE_TYPE_STRING:    *node_ptr = node->children[0]; break; // Replace node with lvalue as this is a noop.
                case JE_TYPE_FLOAT:     node->type = JE_NODE_CAST_FLOAT_TO_STRING; break;
                case JE_TYPE_INT:       node->type = JE_NODE_CAST_INT_TO_STRING; break;
                case JE_TYPE_BOOL:      node->type = JE_NODE_CAST_BOOL_TO_STRING; break;
            }
            node->return_type = JE_TYPE_STRING;
            break;
        }
        case JE_NODE_CAST_TO_BOOL: {
            switch (node->children[1]->return_type) {
                case JE_TYPE_STRING:    node->type = JE_NODE_CAST_STRING_TO_BOOL; break;
                case JE_TYPE_FLOAT:     node->type = JE_NODE_CAST_FLOAT_TO_BOOL; break;
                case JE_TYPE_INT:       node->type = JE_NODE_CAST_INT_TO_BOOL; break;
                case JE_TYPE_BOOL:      *node_ptr = node->children[0]; break; // Replace node with lvalue as this is a noop.
            }
            node->return_type = JE_TYPE_BOOL;
            break;
        }  
        case JE_NODE_LOGICAL_AND:
        case JE_NODE_LOGICAL_OR:
        case JE_NODE_LOGICAL_NOT: {
            node->return_type = JE_TYPE_BOOL;
            break;
        }      
        default: {
            node->return_type = je_type_balance(node->children[0], node->children[1]);
            break;
        }
    }

    // Add implicit conversion nodes if required.
    ret = je_implicit_conversion(context, node);
    if (ret < 0) {
        return ret;
    }

    // Check the operations are valid and convert to type specific operations.
    switch (node->type) {
        case JE_NODE_POS: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_POS_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_POS_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "unary + can only be used with float or int types");
            }
            break;
        }
        case JE_NODE_NEG: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_NEG_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_NEG_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "unary - can only be used with float or int types");
            }
            break;
        }
        case JE_NODE_SUB: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_SUB_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_SUB_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "- can only be used with float or int types");
            }
            break;
        }
        case JE_NODE_MUL: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_MUL_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_MUL_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "* can only be used with float or int types");
            }
            break;
        }
        case JE_NODE_ADD: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_ADD_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_ADD_INT;
            } else if (node->return_type == JE_TYPE_STRING) {
                node->type = JE_NODE_ADD_STRING;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "+ can only be used with float, int or string types");
            }
            break;
        }
        case JE_NODE_DIV: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_DIV_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_DIV_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "/ can only be used with float or int types");
            }
            break;
        }
        case JE_NODE_MOD: {
            if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_MOD_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "% can only be used with int types");
            }
            break;
        }
        case JE_NODE_BITWISE_NOT: {
            if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_BITWISE_NOT_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "~ can only be used with int types");
            }
            break;
        }
        case JE_NODE_LESS: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_LESS_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_LESS_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "< can only be used with bool types");
            }
            node->return_type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_GREATER: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_GREATER_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_GREATER_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, ">= can only be used with bool types");
            }
            node->return_type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_GE: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_GE_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_GE_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, ">= can only be used with bool types");
            }
            node->return_type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_LE: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_LE_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_LE_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "<= can only be used with bool types");
            }
            node->return_type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_NOT_EQUAL: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_NOT_EQUAL_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_NOT_EQUAL_INT;
            } else if (node->return_type == JE_TYPE_STRING) {
                node->type = JE_NODE_NOT_EQUAL_STRING;
            } else if (node->return_type == JE_TYPE_BOOL) {
                node->type = JE_NODE_NOT_EQUAL_BOOL;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "!= can only be used with bool types");
            }
            node->return_type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_EQUAL: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_EQUAL_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_EQUAL_INT;
            } else if (node->return_type == JE_TYPE_STRING) {
                node->type = JE_NODE_EQUAL_STRING;
            } else if (node->return_type == JE_TYPE_BOOL) {
                node->type = JE_NODE_EQUAL_BOOL;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "!= can only be used with bool types");
            }
            node->return_type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_BITWISE_AND: {
            if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_BITWISE_AND_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "& can only be used with int types");
            }
            break;
        }
        case JE_NODE_BITWISE_OR: {
            if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_BITWISE_OR_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "| can only be used with int types");
            }
            break;
        }
        case JE_NODE_LOGICAL_NOT: {
            if (node->return_type == JE_TYPE_BOOL) {
                node->type = JE_NODE_LOGICAL_NOT_BOOL;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "! can only be used with bool types");
            }
            break;
        }
        case JE_NODE_LOGICAL_AND: {
            if (node->return_type == JE_TYPE_BOOL) {
                node->type = JE_NODE_LOGICAL_AND_BOOL;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "&& can only be used with int types");
            }
            break;
        }
        case JE_NODE_LOGICAL_OR: {
            if (node->return_type == JE_TYPE_BOOL) {
                node->type = JE_NODE_LOGICAL_OR_BOOL;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, "|| can only be used with int types");
            }
            break;
        }
    }

    return JE_RESULT_SUCCESS;
}

void je_mark_nodes_constant(je_ast_node_t* node) {
    node->is_constant = 1;

    for (int i = 0; i < 2; i++) {
        if (node->children[i] != NULL) {
            je_mark_nodes_constant(node->children[i]);
            if (!node->children[i]->is_constant) {
                node->is_constant = 0;
            }
        }
    }

    // Values bound by the user are not const (despite being called constants ...)
    switch (node->type) {
        case JE_NODE_CONSTANT_BOOL:         
        case JE_NODE_CONSTANT_INT:          
        case JE_NODE_CONSTANT_FLOAT:        
        case JE_NODE_CONSTANT_STRING: {
            node->is_constant = 0;
            break;
        }
    }
}

void je_fold_node_constants(je_context_t* context, je_ast_node_t** node) {
    if ((*node)->is_constant) {
        // Evaluate result and replace node with a constant value.
        context->transient_mem_arena_offset = 0;
        int ret = je_eval_slow(context, *node, &(*node)->value);
        if (ret == JE_RESULT_SUCCESS) {
            for (int i = 0; i < 2; i++) {
                (*node)->children[i] = NULL;
            }
            switch ((*node)->value.type) {
                case JE_TYPE_BOOL:      (*node)->type = JE_NODE_BOOL_LITERAL;   break;
                case JE_TYPE_FLOAT:     (*node)->type = JE_NODE_FLOAT_LITERAL;  break;
                case JE_TYPE_STRING:    (*node)->type = JE_NODE_STRING_LITERAL; break;
                case JE_TYPE_INT:       (*node)->type = JE_NODE_INT_LITERAL;    break;
            }
            return;
        }
    }

    for (int i = 0; i < 2; i++) {
        if ((*node)->children[i] != NULL) {
            je_fold_node_constants(context, &(*node)->children[i]);
        }
    }
}

int je_fold_constants(je_context_t* context) {
    je_mark_nodes_constant(context->ast_root);    
    je_fold_node_constants(context, &context->ast_root);
    return JE_RESULT_SUCCESS;
}

int je_compile(je_context_t* context, const char* source) {
    if (context->compiled) {
        return JE_RESULT_CANNOT_COMPILE_MULTIPLE_TIMES;
    }
    context->compiled = 1;
    context->jit_compiled = 0;

    // Copy source to an internal buffer.
    size_t source_len = strlen(source);
    int ret = je_alloc(context, source_len + 1, &context->source);
    if (ret < 0) {
        return ret;
    }
    strncpy(context->source, source, source_len + 1);
    context->read_ptr = context->source;

    // Parse the first expression.
    ret = je_parse(context, &context->ast_root, JE_MAX_OPERATOR_PRECEDENCE);
    if (ret < 0) {
        return ret;
    }

    // Make sure we actually parsed something valid.
    if (context->ast_root == NULL) {
        return je_store_error(context, JE_RESULT_EMPTY_EXPRESSION, NULL);
    }
    if (context->read_ptr[0] != '\0') {
        return je_store_error(context, JE_RESULT_UNEXPECTED_TRAILING_EXPRESSION, "Unexpected trailing expression\n\t%s", context->read_ptr);
    }

    printf("==== PARSE ===\n");
    je_print_ast(context->ast_root, 0, 0);

    // Semantically analyze the ast to make sure its valid and insert implicit conversions/etc where required.
    ret = je_semant(context, &context->ast_root);
    if (ret < 0) {
        return ret;
    }

    printf("==== SEMANT ===\n");
    je_print_ast(context->ast_root, 0, 0);

    // Fold any constant operations for simple optimization.
    ret = je_fold_constants(context);
    if (ret < 0) {
        return ret;
    }

    printf("==== FOLD CONSTANTS ===\n");
    je_print_ast(context->ast_root, 0, 0);

    // TODO: Generate bytecode
    // TODO: JIT compile if available.

    return JE_RESULT_SUCCESS;
}

int je_eval_slow(je_context_t* context, je_ast_node_t* node, je_value_t* result) {
    je_value_t lvalue;
    je_value_t rvalue;
    int ret;

    if (node->children[0] != NULL) {
        ret = je_eval_slow(context, node->children[0], &lvalue);
        if (ret < 0) {
            return ret;
        }
    }
    if (node->children[1] != NULL) {
        ret = je_eval_slow(context, node->children[1], &rvalue);
        if (ret < 0) {
            return ret;
        }
    }

    switch (node->type) {
        case JE_NODE_LOGICAL_NOT_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = !rvalue.bool_value;
            break;
        }
        case JE_NODE_BITWISE_NOT_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = ~rvalue.int_value;
            break;
        }
        case JE_NODE_MUL_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = lvalue.float_value * rvalue.float_value;
            break;
        }
        case JE_NODE_MUL_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = lvalue.int_value * rvalue.int_value;
            break;
        }
        case JE_NODE_DIV_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = lvalue.float_value / rvalue.float_value;
            break;
        }
        case JE_NODE_DIV_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = lvalue.int_value / rvalue.int_value;
            break;
        }
        case JE_NODE_MOD_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = lvalue.int_value % rvalue.int_value;
            break;
        }
        case JE_NODE_SUB_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = lvalue.float_value - rvalue.float_value;
            break;
        }
        case JE_NODE_SUB_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = lvalue.int_value - rvalue.int_value;
            break;
        }
        case JE_NODE_ADD_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = lvalue.float_value + rvalue.float_value;
            break;
        }
        case JE_NODE_ADD_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = lvalue.int_value + rvalue.int_value;
            break;
        }
        case JE_NODE_ADD_STRING: {
            int size = snprintf(NULL, 0, "%s%s", lvalue.string_value, rvalue.string_value);
            int ret = je_alloc_transient(context, size + 1, &result->string_value);
            if (ret < 0) {
                return ret;
            }
            sprintf(result->string_value, "%s%s", lvalue.string_value, rvalue.string_value);
            result->string_value_len = size + 1;
            result->type = JE_TYPE_STRING;
            break;
        }
        case JE_NODE_LESS_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.float_value < rvalue.float_value);
            break;
        }
        case JE_NODE_LESS_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.int_value < rvalue.int_value);
            break;
        }
        case JE_NODE_GREATER_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.float_value > rvalue.float_value);
            break;
        }
        case JE_NODE_GREATER_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.int_value > rvalue.int_value);
            break;
        }
        case JE_NODE_LE_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.float_value <= rvalue.float_value);
            break;
        }
        case JE_NODE_LE_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.int_value <= rvalue.int_value);
            break;
        }
        case JE_NODE_GE_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.float_value >= rvalue.float_value);
            break;
        }
        case JE_NODE_GE_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.int_value >= rvalue.int_value);
            break;
        }
        case JE_NODE_EQUAL_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.bool_value == rvalue.bool_value);
            break;
        }
        case JE_NODE_EQUAL_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.int_value == rvalue.int_value);
            break;
        }
        case JE_NODE_EQUAL_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.float_value == rvalue.float_value);
            break;
        }
        case JE_NODE_EQUAL_STRING: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (strcmp(lvalue.string_value, rvalue.string_value) == 0);
            break;
        }
        case JE_NODE_NOT_EQUAL_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.bool_value != rvalue.bool_value);
            break;
        }
        case JE_NODE_NOT_EQUAL_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.int_value != rvalue.int_value);
            break;
        }
        case JE_NODE_NOT_EQUAL_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (lvalue.float_value != rvalue.float_value);
            break;
        }
        case JE_NODE_NOT_EQUAL_STRING: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (strcmp(lvalue.string_value, rvalue.string_value) != 0);
            break;
        }
        case JE_NODE_BITWISE_AND_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = lvalue.int_value & rvalue.int_value;
            break;
        }
        case JE_NODE_BITWISE_OR_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = lvalue.int_value | rvalue.int_value;
            break;
        }
        case JE_NODE_LOGICAL_AND_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = lvalue.bool_value && rvalue.bool_value;
            break;
        }
        case JE_NODE_LOGICAL_OR_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = lvalue.bool_value || rvalue.bool_value;
            break;
        }
        case JE_NODE_CONSTANT_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = node->constant->bool_value;
            break;
        }
        case JE_NODE_CONSTANT_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = node->constant->int_value;
            break;
        }
        case JE_NODE_CONSTANT_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = node->constant->float_value;
            break;
        }
        case JE_NODE_CONSTANT_STRING: {
            result->type = JE_TYPE_STRING;
            result->string_value = node->constant->string_value;
            break;
        }
        case JE_NODE_NEG_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = -rvalue.float_value;
            break;
        }
        case JE_NODE_POS_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = +rvalue.float_value;
            break;
        }
        case JE_NODE_NEG_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = -rvalue.int_value;
            break;
        }
        case JE_NODE_POS_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = +rvalue.int_value;
            break;
        }
        case JE_NODE_FLOAT_LITERAL: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = node->value.float_value;
            break;
        }
        case JE_NODE_INT_LITERAL: {
            result->type = JE_TYPE_INT;
            result->int_value = node->value.int_value;
            break;
        }
        case JE_NODE_STRING_LITERAL: {
            result->type = JE_TYPE_STRING;
            result->string_value = node->value.string_value;
            break;
        }
        case JE_NODE_BOOL_LITERAL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = node->value.bool_value;
            break;
        }
        case JE_NODE_CAST_INT_TO_STRING: {
            int size = snprintf(NULL, 0, "%i", rvalue.int_value);
            int ret = je_alloc_transient(context, size + 1, &result->string_value);
            if (ret < 0) {
                return ret;
            }
            sprintf(result->string_value, "%i", rvalue.int_value);

            result->string_value_len = size + 1;
            result->type = JE_TYPE_STRING;
            break;
        }
        case JE_NODE_CAST_FLOAT_TO_STRING: {
            int size = snprintf(NULL, 0, "%f", rvalue.float_value);
            int ret = je_alloc_transient(context, size + 1, &result->string_value);
            if (ret < 0) {
                return ret;
            }
            sprintf(result->string_value, "%f", rvalue.float_value);

            result->string_value_len = size + 1;
            result->type = JE_TYPE_STRING;
            break;
        }
        case JE_NODE_CAST_BOOL_TO_STRING: {
            result->string_value = (rvalue.bool_value ? "true" : "false");
            result->type = JE_TYPE_STRING;
            break;
        }
        case JE_NODE_CAST_STRING_TO_INT: {
            result->int_value = atoi(rvalue.string_value);
            result->type = JE_TYPE_INT;
            break;
        }
        case JE_NODE_CAST_FLOAT_TO_INT: {
            result->int_value = (int)rvalue.float_value;
            result->type = JE_TYPE_INT;
            break;
        }
        case JE_NODE_CAST_BOOL_TO_INT: {
            result->int_value = (rvalue.bool_value != 0 ? 1 : 0);
            result->type = JE_TYPE_INT;
            break;
        }
        case JE_NODE_CAST_INT_TO_FLOAT: {
            result->float_value = (float)rvalue.int_value;
            result->type = JE_TYPE_FLOAT;
            break;
        }
        case JE_NODE_CAST_STRING_TO_FLOAT: {
            result->float_value = (float)atof(rvalue.string_value);
            result->type = JE_TYPE_FLOAT;
            break;
        }
        case JE_NODE_CAST_BOOL_TO_FLOAT: {
            result->float_value = (rvalue.bool_value != 0 ? 1.0f : 0.0f);
            result->type = JE_TYPE_FLOAT;
            break;
        }
        case JE_NODE_CAST_INT_TO_BOOL: {
            result->bool_value = (rvalue.int_value != 0 ? 1 : 0);
            result->type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_CAST_STRING_TO_BOOL: {
            result->bool_value = (strcmp(rvalue.string_value, "false") != 0 && strcmp(rvalue.string_value, "0") != 0);
            result->type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_CAST_FLOAT_TO_BOOL: {
            result->bool_value = (rvalue.float_value != 0.0f ? 1 : 0);
            result->type = JE_TYPE_BOOL;
            break;
        }
        default: {
            assert(0);
            return JE_RESULT_FAILED;
        }
    }
    return JE_RESULT_SUCCESS;
}

int je_eval_jit(je_context_t* context, je_ast_node_t* node, je_value_t* result) {
    // TODO
    return JE_RESULT_FAILED;
}

int je_eval(je_context_t* context) {    
    context->transient_mem_arena_offset = 0;

    if (context->jit_compiled) {
        int ret = je_eval_jit(context, context->ast_root, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    else {
        int ret = je_eval_slow(context, context->ast_root, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    return JE_RESULT_SUCCESS;
}

#endif

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // JITEVAL_H