// -----------------------------------------------------------------------
// jiteval.h - v1.0 - expression evaluation with optional JIT support
// Tim Leonard, Copyright 2026
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
//      <type> je_eval_<type>(const char* expression, const char* error_msg = 0, int error_msg_len = 0);
//          
//          Shorthand that can be used standalone to interpret an expression without 
//          setting up a context.
// 
//          Useful for throw-away expressions, but less optimal for expressions that
//          are going to be re-used as the expression needs to be compiled for each call.
//
//          If a buffer is passed to error_msg it will be filled if an error occurs.
// 
//          Return value is the result of the expression coerced to the given type.
// 
//          For je_eval_string the result must be free'd by the caller.
// 
//      int je_new_context(je_context_t* context, int flags);
// 
//          Takes an opaque context object that is used by all other functions
//          to store stateful information. 
// 
//          The various JE_FLAG_* defines can be passed in as a bitmask to the flags
//          parameter to control various aspects of the contexts behaviour.
//          Pass in JE_FLAG_NONE if you want to use the defaults.
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
//      int je_bind_variable_<type>(je_context_t* context, const char* name, int is_constant, <type> value)
//  
//          Binds identifiers to a given value in the context, which can be 
//          referenced in expressions that are evaluated.
// 
//          Variables cannot be unbound from a context once bound, but can
//          have their values changed inbetween je_eval calls as long
//          as their type does not change.
// 
//          Variables marked as constant allows them to be optimized out
//          during compilation but means they cannot be rebound.
// 
//          String values are copied to an internal buffer and do not need to 
//          be alive beyond this call.
// 
//          Returns a value of JE_RESULT_* describing the success or failure.
// 
//      int je_bind_function(je_context_t* context, const char* name, int is_deterministic, je_func_ptr_t func, int return_type, ...)
//
//          Binds a function that can be called from expressions when
//          they are evaluated.
// 
//          Functions marked as deterministic are expected to always return the same value when 
//          the same input is provided. This can be used to call them during compilation and 
//          optimize them away during evaluation.
// 
//          Varidic arguments are a list of parameter types the function takes, the last argument
//          should always be NULL to deliminate the end of parameters.
// 
//          Returns a value of JE_RESULT_* describing the success or failure.
// 
//      int je_get_parameter_<type>(je_context_t* context, int index, <type>* result)
//
//          When called from inside a bound function returns the parameter at the given
//          index that was passed into the function call.
// 
//          Returns a value of JE_RESULT_* describing the success or failure.
//     
//      int je_return_<type>(je_context_t* context, int index, <type> result)
//
//          When called from inside a bound function sets the return value 
//          of the function to the given value.
// 
//          Returns a value of JE_RESULT_* describing the success or failure.
// 
//      int je_compile(je_context_t* context, const char* source);
// 
//          Compiles an expression that the given context will evaluate
//          whenever je_eval is called.
// 
//          All variables/functions used by an expression must be bound before je_compile
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
//          Can be called multiple times with rebound variables/functions without 
//          needing to call je_compile again.
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
//      void je_memory_stats(je_context_t* context, uint64_t* permanent_mem_used, uint64_t* transient_mem_used, uint64_t* executable_mem_used );
// 
//          Gets how much memory was used internally for various things.
// 
//          Permanent memory is never released until the context is freed.
//          Transient memory is reset each time je_eval is executed.
//          Executable memory is used for JIT and never released until the context is freed.
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
//          Functions 
//          Variables
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
//      Identifiers are case-sensitive. All Identifiers beginning with __ are reserved for intrinsics.
// 
//      Expressions are strongly typed. 
//
// LICENSE
//
//      Permission is hereby granted, free of charge, to any person obtaining a 
//      copy of this software and associated documentation files(the �Software�), 
//      to deal in the Software without restriction, including without limitation 
//      the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//      and /or sell copies of the Software, and to permit persons to whom the 
//      Software is furnished to do so, subject to the following conditions :
// 
//      The above copyright notice and this permission notice shall be included 
//      in all copies or substantial portions of the Software.
// 
//      THE SOFTWARE IS PROVIDED �AS IS�, WITHOUT WARRANTY OF ANY KIND, EXPRESS 
//      OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
//      MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//      IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
//      CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
//      TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
//      SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// VERSION HISTORY
//
//      (2026-07-12)    0.1    First release
//
// SAMPLE CODE
// 
//      See sample.c in the same directory as this file.
// 
// -----------------------------------------------------------------------
#ifndef JITEVAL_H
#define JITEVAL_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Handy macro that will go down the arm64 path on x64 for simpler
// debugging of instruction encoding.
//#define JE_DEBUG_FAKE_ARM64
//#define JE_DEBUG_FAKE_JIT_AVAILABLE

// When enabled all contexts are forced to have the given flags
//#define JE_DEBUG_FORCE_FLAGS (JE_FLAG_DEBUG_JIT_DISASSEMBLY | JE_FLAG_NO_OPTIMIZATION)

// -----------------------------------------------------------------------
// PLATFORM DETERMINATION OPTIONS
// -----------------------------------------------------------------------

#define JE_VERSION_MAJOR 0
#define JE_VERSION_MINOR 1

// Platform determination
#if defined(_WIN32)
    #define JE_PLATFORM_WINDOWS
    #define JE_PLATFORM_NAME "Windows"
#elif defined(__linux__)
    #define JE_PLATFORM_LINUX
    #define JE_PLATFORM_NAME "Linux"
#elif defined(__APPLE__)
    #define JE_PLATFORM_MACOS
    #define JE_PLATFORM_NAME "MacOS"
#elif defined(ARDUINO)
    #define JE_PLATFORM_ARDUINO
    #define JE_PLATFORM_NAME "Arduino"
#else
    #error Unknown platform
#endif

// Compiler determination
#if defined(_MSC_VER)
    // Also captures using clang as a frontend for msvc.
    #define JE_COMPILER_MSVC
    #define JE_COMPILER_NAME "MSVC"
#elif defined(__clang__)
    #define JE_COMPILER_CLANG
    #define JE_COMPILER_NAME "Clang"
#elif defined(__GNUC__)
    #define JE_COMPILER_GCC
    #define JE_COMPILER_NAME "GCC"
#else
    #error Unknown compiler
#endif

// ISA determination.
#if defined(_M_ARM64) || defined(__aarch64__) || defined(JE_DEBUG_FAKE_ARM64)
    #define JE_ISA_ARM64
    #define JE_ISA_NAME "ARM64"
#elif defined(_M_X64) || defined(__x86_64__)
    #define JE_ISA_X64
    #define JE_ISA_NAME "X64"
#elif defined(_M_IX86) || defined(__i386__)
    #define JE_ISA_X86
    #define JE_ISA_NAME "X86"
#elif defined(_M_ARM) || defined(__arm__)
    #define JE_ISA_ARM32
    #define JE_ISA_NAME "ARM32"
#elif defined(__AVR__)
    #define JE_ISA_AVR
    #define JE_ISA_NAME "AVR"
#elif defined(__XTENSA__)
    #define JE_ISA_XTENSA
    #define JE_ISA_NAME "XTENSA"
#else
    #error Unknown ISA
#endif

// Determine if JIT is available for the current ISA/Compiler/Platform combination.
#if defined(JE_PLATFORM_WINDOWS)
    #if defined(JE_COMPILER_MSVC) || defined(JE_COMPILER_GCC) || defined(JE_COMPILER_CLANG)
        #if defined(JE_ISA_X64) || defined(JE_ISA_X86) || defined(JE_ISA_ARM64)
            #define JE_JIT_AVAILABLE
        #endif
    #endif
#elif defined(JE_PLATFORM_LINUX)
    #if defined(JE_COMPILER_GCC) || defined(JE_COMPILER_CLANG)
        #if defined(JE_ISA_X64) || defined(JE_ISA_X86) || defined(JE_ISA_ARM64)
            #define JE_JIT_AVAILABLE
        #endif
    #endif
#endif

#if defined(JE_DEBUG_FAKE_JIT_AVAILABLE) && !defined(JE_JIT_AVAILABLE)
    #define JE_JIT_AVAILABLE
#endif

#ifdef JE_JIT_AVAILABLE
    #ifdef JE_COMPILER_MSVC
        #if defined(JE_ISA_X64)
            #define JE_CALLING_CONVENTION_MSVC
        #elif defined(JE_ISA_X86)
            #define JE_CALLING_CONVENTION_C
        #elif defined(JE_ISA_ARM32)
            #define JE_CALLING_CONVENTION_ARM32
        #elif defined(JE_ISA_ARM64)
            #define JE_CALLING_CONVENTION_ARM64
        #endif
    #elif defined(JE_COMPILER_GCC) || defined(JE_COMPILER_CLANG)
        #if defined(JE_ISA_X64)
            #define JE_CALLING_CONVENTION_SYSTEMV
        #elif defined(JE_ISA_X86)
            #define JE_CALLING_CONVENTION_C
        #elif defined(JE_ISA_ARM32)
            #define JE_CALLING_CONVENTION_ARM32
        #elif defined(JE_ISA_ARM64)
            #define JE_CALLING_CONVENTION_ARM64
        #endif
    #endif

    #if !defined(JE_CALLING_CONVENTION_MSVC) && \
        !defined(JE_CALLING_CONVENTION_C) && \
        !defined(JE_CALLING_CONVENTION_SYSTEMV) && \
        !defined(JE_CALLING_CONVENTION_ARM32) && \
        !defined(JE_CALLING_CONVENTION_ARM64)
        #error Unknown calling convention
    #endif
#endif

// -----------------------------------------------------------------------
// COMPILE TIME OPTIONS
// -----------------------------------------------------------------------

// Define these before your JITEVAL_IMPL include to modify various 
// limits of the code

// A context allocates two fixed memory arenas up front that it uses
// as permanent and transient stack allocators. If you are trying to
// parse a very complex expression you may need to further increase this.
// 
// Maximum dynamic memory allocation of a context is double this, plus
// any additional memory needed for JIT compiled memory pages.
//
// Maximum size of arena is UINT16_MAX as uint16_t's are used for offsets
// within the arena.
#ifndef JE_MEM_ARENA_SIZE
#define JE_MEM_ARENA_SIZE               (2 * 1024)
#endif

// Alignment of allocations from the memory arena. In general should not
// require modification, but if your platform requites specific alignment
// for performance you can modify this.
#ifndef JE_MEM_ARENA_ALIGN
#ifdef JE_ISA_XTENSA
#define JE_MEM_ARENA_ALIGN              (8)
#else
#define JE_MEM_ARENA_ALIGN              (1)
#endif
#endif

// Maximum length of a constant string in an expression. This expands the
// size of a handful of fixed size arrays, take care when increasing as it will
// have an effect on how much memory is needed in the arena.
#ifndef JE_MAX_STRING_CONSTANT_LENGTH
#define JE_MAX_STRING_CONSTANT_LENGTH   (256)
#endif

// If enabled then hashes will be used for function/variable/etc names rather than strings
// This reduces memory usage but will make error messages using the name less helpful. 
// This is expected to only be enabled in production environments where the expressions
// are constant and errors are unexpected.
#ifndef JE_USE_NAME_HASHES
#define JE_USE_NAME_HASHES              (1)
#endif

// -----------------------------------------------------------------------
// HEADER
// -----------------------------------------------------------------------

// Note: All failure results are negative, all success results are positive.

#define JE_RESULT_VALUE_IS_CONSTANT                 (-24)   // Function or variable is constant and cannot be changed.
#define JE_RESULT_PARAMETER_INDEX_OUT_OF_BOUNDS     (-23)   // Attempt to get a parameter index beyond the number of parameters the function takes.
#define JE_RESULT_WRONG_PARAMETER_TYPE              (-22)   // Attempt to get the value of a function in the wrong type.
#define JE_RESULT_NOT_IN_FUNCTION                   (-21)   // Attempt to call one of the parameter/return functions that have to be called in a function
#define JE_RESULT_INCORRECT_FUNC_RETURN_TYPE        (-20)   // Function called returned a type that it wasn't defined as returning.
#define JE_RESULT_INCORRECT_PARAMETER_COUNT         (-19)   // Tried to call function with incorrect parameter count.
#define JE_RESULT_MAX_PARAMETER_COUNT_EXCEEDED      (-18)   // Maximum number of parameters to a bound function has been exceeded.
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
#define JE_RESULT_TYPE_CANNOT_CHANGE                (-4)    // Attempt was made to change the type of a variable that was already set.
#define JE_RESULT_CORRUPT                           (-3)    // Something has corrupted the internal state of the context.
#define JE_RESULT_OOM                               (-2)    // No memory is left in the internal allocator to satisfy operation.
#define JE_RESULT_FAILED                            (-1)    // Operation failed (generic)
#define JE_RESULT_SUCCESS                           (0)     // Operation succesful

#define JE_TYPE_UNSET                               (0)     // Used as a sentinel value, not expected to be passed in by public API.
#define JE_TYPE_INT                                 (1)
#define JE_TYPE_BOOL                                (2)
#define JE_TYPE_FLOAT                               (3)
#define JE_TYPE_STRING                              (4)

#define JE_FLAG_NONE                                (0)     // Default behaviour, no changes.
#define JE_FLAG_NO_OPTIMIZATION                     (1)     // Disables any optimization passes on the expression.
#define JE_FLAG_NO_JIT                              (2)     // Disables jit compiling of the expression 
#define JE_FLAG_DEBUG_LOGGING                       (4)     // Prints out the AST at different stages of compilation, among other things, used for debugging.
#define JE_FLAG_DEBUG_MEM_LOGGING                   (8)     // Prints out memory allocations / arena states.
#define JE_FLAG_NO_UTILITY_FUNCTIONS                (16)    // Prevents registration of the basic utility functions normally provided (min/max/abs/etc)
#define JE_FLAG_DEBUG_JIT_DISASSEMBLY               (32)    // Prints out dissassembly of the JIT instruction stream

typedef struct je_context_t je_context_t;
typedef void (*je_func_t)(je_context_t* context);
typedef void (*je_jit_func_t)();

int je_eval_int(const char* expression, char* error_msg, int error_msg_len);
float je_eval_float(const char* expression, char* error_msg, int error_msg_len);
bool je_eval_bool(const char* expression, char* error_msg, int error_msg_len);
char* je_eval_string(const char* expression, char* error_msg, int error_msg_len);

int je_new_context(je_context_t* context, int flags);
int je_free_context(je_context_t* context);

int je_bind_variable_int(je_context_t* context, const char* name, bool is_constant, int value);
int je_bind_variable_float(je_context_t* context, const char* name, bool is_constant, float value);
int je_bind_variable_string(je_context_t* context, const char* name, bool is_constant, const char* value);
int je_bind_variable_bool(je_context_t* context, const char* name, bool is_constant, int value);

int je_bind_function(je_context_t* context, const char* name, bool is_deterministic, je_func_t func, int return_type, ...);

int je_compile(je_context_t* context, const char* source);
int je_eval(je_context_t* context);

int je_result_int(je_context_t* context, int* value);
int je_result_float(je_context_t* context, float* value);
int je_result_string(je_context_t* context, const char** value);
int je_result_bool(je_context_t* context, int* value);

const char* je_error_msg(je_context_t* context);
void je_memory_stats(je_context_t* context, int* permanent_mem_used, int* transient_mem_used, int* executable_mem_used);

// -----------------------------------------------------------------------
// IMPLEMENTATION
// -----------------------------------------------------------------------

#ifdef __INTELLISENSE__
    #define JITEVAL_IMPL
#endif

#ifdef JITEVAL_IMPL

#ifdef JE_COMPILER_MSVC
    #define _CRT_SECURE_NO_WARNINGS 1
    #define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

#ifdef JE_PLATFORM_WINDOWS
    #include <Windows.h>
#endif


#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#if defined(JE_PLATFORM_LINUX)
    #include <sys/mman.h>
#endif

// -----------------------------------------------------------------------
// DEFINES
// -----------------------------------------------------------------------

#define JE_MAX_OPERATOR_PRECEDENCE      (10)

// Maximum string length that can be manipulated by the expression.
#define JE_MAX_STRING_BIT_LENGTH        (13)
#define JE_MAX_STRING_LENGTH            (2 ^ JE_MAX_STRING_BIT_LENGTH);
#define JE_MAX_PARAMETERS               (8)

#define JE_FLOAT_EPSILON                (0.000001f)

enum je_token_type_t {
    JE_TOK_IDENTIFIER,             
    JE_TOK_FLOAT,
    JE_TOK_INT,
    JE_TOK_STRING,
    JE_TOK_BOOL,
    JE_TOK_OP_COMMA,
    JE_TOK_OP_MUL,
    JE_TOK_OP_DIV,
    JE_TOK_OP_SUB,
    JE_TOK_OP_ADD,
    JE_TOK_OP_MOD,
    JE_TOK_OP_LESS,
    JE_TOK_OP_GREATER,
    JE_TOK_OP_LE,
    JE_TOK_OP_GE,
    JE_TOK_OP_EQUAL,
    JE_TOK_OP_NOT_EQUAL,
    JE_TOK_OP_NOT,
    JE_TOK_OP_LOGICAL_AND,
    JE_TOK_OP_LOGICAL_OR,
    JE_TOK_OP_BITWISE_AND,
    JE_TOK_OP_BITWISE_OR,
    JE_TOK_OP_BITWISE_NOT,
    JE_TOK_OP_BITWISE_XOR,
    JE_TOK_OP_PARENTHESIS_OPEN,
    JE_TOK_OP_PARENTHESIS_CLOSE,
    JE_TOK_OP_KEYWORD_INT,
    JE_TOK_OP_KEYWORD_FLOAT,
    JE_TOK_OP_KEYWORD_STRING,
    JE_TOK_OP_KEYWORD_BOOL
};

enum je_node_type_t {
    // These are generic node types, that will be converted into type-specific ones
    // during semantic analysis.
    JE_NODE_LOGICAL_NOT,
    JE_NODE_BITWISE_NOT,
    JE_NODE_MUL,
    JE_NODE_DIV,
    JE_NODE_MOD,
    JE_NODE_SUB,
    JE_NODE_ADD,
    JE_NODE_LESS,
    JE_NODE_GREATER,
    JE_NODE_LE,
    JE_NODE_GE,
    JE_NODE_EQUAL,
    JE_NODE_NOT_EQUAL,
    JE_NODE_BITWISE_AND,
    JE_NODE_BITWISE_OR,
    JE_NODE_BITWISE_XOR,
    JE_NODE_LOGICAL_AND,
    JE_NODE_LOGICAL_OR,
    JE_NODE_VARIABLE,
    JE_NODE_NEG,
    JE_NODE_POS,
    JE_NODE_CAST_TO_BOOL,
    JE_NODE_CAST_TO_INT,
    JE_NODE_CAST_TO_FLOAT,
    JE_NODE_CAST_TO_STRING,
    JE_NODE_FUNCTION_CALL,
    // These are the type-specific versions of the ndoes above.
    JE_NODE_LOGICAL_NOT_BOOL,
    JE_NODE_BITWISE_NOT_INT,
    JE_NODE_MUL_FLOAT,
    JE_NODE_MUL_INT,
    JE_NODE_DIV_FLOAT,
    JE_NODE_DIV_INT,
    JE_NODE_MOD_INT,
    JE_NODE_SUB_FLOAT,
    JE_NODE_SUB_INT,
    JE_NODE_ADD_FLOAT,
    JE_NODE_ADD_INT,
    JE_NODE_ADD_STRING,
    JE_NODE_LESS_FLOAT,
    JE_NODE_LESS_INT,
    JE_NODE_GREATER_FLOAT,
    JE_NODE_GREATER_INT,
    JE_NODE_LE_FLOAT,
    JE_NODE_LE_INT,
    JE_NODE_GE_FLOAT,
    JE_NODE_GE_INT,
    JE_NODE_EQUAL_BOOL,
    JE_NODE_EQUAL_INT,
    JE_NODE_EQUAL_FLOAT,
    JE_NODE_EQUAL_STRING,
    JE_NODE_NOT_EQUAL_BOOL,
    JE_NODE_NOT_EQUAL_INT,
    JE_NODE_NOT_EQUAL_FLOAT,
    JE_NODE_NOT_EQUAL_STRING,
    JE_NODE_BITWISE_AND_INT,
    JE_NODE_BITWISE_OR_INT,
    JE_NODE_BITWISE_XOR_INT,
    JE_NODE_LOGICAL_AND_BOOL,
    JE_NODE_LOGICAL_OR_BOOL,
    JE_NODE_VARIABLE_BOOL,
    JE_NODE_VARIABLE_INT,
    JE_NODE_VARIABLE_FLOAT,
    JE_NODE_VARIABLE_STRING,
    JE_NODE_NEG_FLOAT,
    JE_NODE_POS_FLOAT,
    JE_NODE_NEG_INT,
    JE_NODE_POS_INT,
    JE_NODE_FLOAT_LITERAL,
    JE_NODE_INT_LITERAL,
    JE_NODE_STRING_LITERAL,
    JE_NODE_BOOL_LITERAL,
    JE_NODE_CAST_INT_TO_STRING,
    JE_NODE_CAST_FLOAT_TO_STRING,
    JE_NODE_CAST_BOOL_TO_STRING,
    JE_NODE_CAST_STRING_TO_INT,
    JE_NODE_CAST_FLOAT_TO_INT,
    JE_NODE_CAST_BOOL_TO_INT,
    JE_NODE_CAST_INT_TO_FLOAT,
    JE_NODE_CAST_STRING_TO_FLOAT,
    JE_NODE_CAST_BOOL_TO_FLOAT,
    JE_NODE_CAST_INT_TO_BOOL,
    JE_NODE_CAST_STRING_TO_BOOL,
    JE_NODE_CAST_FLOAT_TO_BOOL,
    JE_NODE_FUNCTION_CALL_INT,
    JE_NODE_FUNCTION_CALL_FLOAT,
    JE_NODE_FUNCTION_CALL_BOOL,
    JE_NODE_FUNCTION_CALL_STRING
};

enum je_mem_tag_t {
    JE_MEM_TAG_NAME,
    JE_MEM_TAG_VALUE,
    JE_MEM_TAG_VARIABLE_DEF,
    JE_MEM_TAG_FUNCTION_DEF,
    JE_MEM_TAG_AST_NODE,
    JE_MEM_TAG_STRING,
    JE_MEM_TAG_NODE_CHILD_ARRAY,
    JE_MEM_TAG_TYPE_DATA,
    JE_MEM_TAG_PARAM_TYPES,
    JE_MEM_TAG_COUNT,
};

// -----------------------------------------------------------------------
// TYPES
// -----------------------------------------------------------------------

#ifdef JE_COMPILER_MSVC
// Smaller size gets us more speed than the miss-alignment penalty.
#pragma pack(push, 1) 
#endif

typedef struct je_name_t {
    uint16_t                        next_offset;
    uint16_t                        name_offset;
} je_name_t;

typedef struct je_token_t {
    int                             type;
    const char*                     source_ptr;                         // Pointer to where the token is in the source string.
    int                             source_len;                         // Length of token in the source string.    
    union {
        int                         int_value;
        int                         bool_value;
        float                       float_value;
        char*                       string_value;
    };
} je_token_t;

typedef struct je_value_t {
    union {
        int                         int_value;
        int                         bool_value;
        float                       float_value;
        char*                       string_value;
    };
    uint8_t                         type : 4;
    uint8_t                         pad0 : 4;
} je_value_t;

typedef struct je_variable_def_t {
    // 2
    uint16_t                        next_offset;
    // 2
    uint16_t                        name_index;
    // 1
    uint8_t                         type : 4;
    uint8_t                         is_constant : 4;

#if JE_MEM_ARENA_ALIGN > 1
    // Ensure variable following this is aligned.
    uint8_t                         pad0[JE_MEM_ARENA_ALIGN - (5 % JE_MEM_ARENA_ALIGN)];
#endif

    // Note: Struct is over-allocated, the value of the variable 
    //       immediately follows it. Type is used to disambiguate.
} je_variable_def_t;

typedef struct je_func_def_t {
    // 4/8
    je_func_t                       function;
    // 2
    uint16_t                        next_offset;
    // 2
    uint16_t                        name_index;
    // 2
    uint16_t                        param_types_data;                        // If less than 4 parameters the types are encoded in this, if more its an offset to an array of uint16_t
    // 1
    uint8_t                         return_type : 4;
    uint8_t                         param_count : 3;
    uint8_t                         is_deterministic : 1;
} je_func_def_t;

typedef struct je_ast_node_t {     
    // 2 - could re-encode as relative to drop one byte
    uint16_t                        child_offset;
    // 2
    uint16_t                        type_data;                              // Offset to either a variable, function or value. What it points to depends on the node type.
    // 1
    uint8_t                         type : 8;
    // 1
    uint8_t                         child_count : 4;
    uint8_t                         pad0 : 4;
    // 1
    uint8_t                         return_type : 4;
    uint8_t                         param_count : 3;
    uint8_t                         is_constant : 1;
} je_ast_node_t;

typedef struct je_jit_register_allocation_t {
    uint32_t                        alloc_index;                            // Constantly incrementing number across all registers, used to see what is most "new"
    uint16_t                        alloc_count;                            // Number of times the register has been allocated, if > 1 registers are pushed on to the stack to avoid spilling.
} je_jit_register_allocation_t;

typedef struct je_context_t {
    char                            error_msg[128];                         // Error message from the last failing call.

#ifdef JE_JIT_AVAILABLE
    je_jit_register_allocation_t    jit_register_allocation[20];            // Allocation of registers used when JIT compiling code.
#endif

    uint16_t                        mem_tag_allocations[JE_MEM_TAG_COUNT];  // Tracks how much memory is allocated by tag (does not reset transient!).

    je_value_t                      result;                                 // Result of the last evaluation call.
    je_value_t                      function_params[JE_MAX_PARAMETERS];     // Parameters passed into the last function called.
    je_value_t                      function_result;                        // Return value from function called

    const char*                     source;                               // Pointer to source code string
    const char*                     read_ptr;                               // Read pointer into source string
    je_variable_def_t*              variable_head;                          // Head of the variable linked list.
    je_func_def_t*                  function_head;                          // Head of the function linked list.
    je_name_t*                      name_head;                              // Head of the name linked list.
    je_ast_node_t*                  ast_root;                               // Root node in the ast graph.
    je_func_def_t*                  active_function;                        // Pointer to the function currently being called.
    char                            mem_arena[JE_MEM_ARENA_SIZE];           // Block of memory that all persistent allocated memory is stored in. Used as a stack allocator.
    char                            transient_mem_arena[JE_MEM_ARENA_SIZE]; // Block of memory that all transient allocations using during evaluation is stored in. Used as a stack allocator.
#ifdef JE_JIT_AVAILABLE
    char*                           jit_write_buffer;                       // Start of buffer to generate JIT code into.
    char*                           jit_executable_memory;                  // Executable memory containing the JIT'd code.
    char*                           jit_write_ptr;                          // Current write pointer for generating JIT code.
    char*                           jit_instruction_start;                  // Stores a pointer into the write buffer where the current instruction starts.
#endif

    int                             error_code;                             // Last error that occured in the context.
#ifdef JE_JIT_AVAILABLE
    int                             jit_executable_memory_length;           // Use of executable memory region allocated for JIT.
    int                             jit_write_buffer_len;                   // Length of write buffer.
    int                             jit_register_allocation_counter;        // Count of every time a register was allocated.
#endif

#ifdef JE_JIT_AVAILABLE
    uint16_t                        jit_instruction_num;                    // Number of instructions in the JIT code
    uint16_t                        jit_code_bytes;                         // Number of bytes the JIT code takes up.
    uint16_t                        jit_stack_bytes;                        // How many bytes are currently pushed to the stack during jit emission.
#endif

    uint16_t                        mem_arena_offset;                       // Next locations in the mem_arena to allocate from.
    uint16_t                        transient_mem_arena_offset;             // Next locations in the transient_mem_arena_offset to allocate from.

    uint8_t                         flags;                                  // Flags controling the contexts behaviour.
    bool                            mem_arena_frozen : 1;                   // If set to true we will assert if allocations are attempted. Used during jit compiling.
    bool                            compiled : 1;                           // If this context has been compiled.
#ifdef JE_JIT_AVAILABLE
    bool                            jit_compiled : 1;                       // If the compiled expression is jit compiled.
    bool                            jit_write_buffer_overflow : 1;          // If we ran out of space while emitting JIT code.
#endif
} je_context_t;

#ifdef JE_COMPILER_MSVC
// Smaller size gets us more speed than the miss-alignment penalty.
#pragma pack(pop)
#endif

// -----------------------------------------------------------------------
// UTILITY FUNCTIONS
// -----------------------------------------------------------------------

uint32_t je_hash32(const char* input) {
    uint32_t hash = 0;
    for (unsigned char* p = (unsigned char*)input; *p != '\0'; p++) {
        hash = 37 * hash + *p;
    }
    return hash;
}

uint16_t je_hash16(const char* input) {
    return je_hash32(input) % UINT16_MAX;
}

const char* je_mem_tag_name(int tag) {
    switch (tag) {        
        case JE_MEM_TAG_NAME:               return "je_name_t";
        case JE_MEM_TAG_VALUE:              return "je_value_t";
        case JE_MEM_TAG_VARIABLE_DEF:       return "je_variable_def_t";
        case JE_MEM_TAG_FUNCTION_DEF:       return "je_func_def_t";
        case JE_MEM_TAG_AST_NODE:           return "je_ast_node_t";
        case JE_MEM_TAG_STRING:             return "string";
        case JE_MEM_TAG_NODE_CHILD_ARRAY:   return "node child array";
        case JE_MEM_TAG_TYPE_DATA:          return "node type data";
        case JE_MEM_TAG_PARAM_TYPES:        return "func param types array";
        default: assert(0);
    }
    return "unknown";
}

void je_jit_free(je_context_t* context);
int je_parse(je_context_t* context, je_ast_node_t** node, int precedence);
int je_eval_slow(je_context_t* context, je_ast_node_t* node, je_value_t* result);
void je_print_ast(je_context_t* context, je_ast_node_t* node, int depth, int childIndex);

int je_store_error(je_context_t* context, int error_code, const char* location, const char* error_msg, ...) {  
    if (error_msg != NULL) {
        va_list va;
        va_start(va, error_msg);
        vsnprintf(context->error_msg, sizeof(context->error_msg), error_msg, va);
        va_end(va);
    } else {
        context->error_msg[0] = '\0';

        const char* error_code_msg = NULL;

        switch (error_code) {
            case JE_RESULT_EMPTY_EXPRESSION:                error_code_msg = "Empty expression";                                                                        break;
            case JE_RESULT_TYPE_CANNOT_CHANGE:              error_code_msg = "Type of variable cannot be changed from what it was originally set to.";                  break;
            case JE_RESULT_CORRUPT:                         error_code_msg = "Internal state of the context is corrupt";                                                break;
            case JE_RESULT_OOM:                             error_code_msg = "Out of memory";                                                                           break;
            case JE_RESULT_FAILED:                          error_code_msg = "Operation failed";                                                                        break;
            case JE_RESULT_NOT_IN_FUNCTION:                 error_code_msg = "Function call is invalid outside a function";                                             break;
            case JE_RESULT_INCORRECT_FUNC_RETURN_TYPE:      error_code_msg = "Attempted to return data type that function isn't defined as returning";                  break;
            case JE_RESULT_WRONG_PARAMETER_TYPE:            error_code_msg = "Attempted to get function parameter as the wrong type";                                   break;
            case JE_RESULT_PARAMETER_INDEX_OUT_OF_BOUNDS:   error_code_msg = "Attempted to get a parameter index beyond the number of parameters the function takes";   break;
            case JE_RESULT_VALUE_IS_CONSTANT:               error_code_msg = "Function or variable is constant and cannot be changed.";                                 break;
        }

        if (error_code_msg != NULL) {
            strncpy(context->error_msg, error_code_msg, sizeof(context->error_msg));
        }
    }

    // If we have enough buffer space then append the location description.
    if (location != NULL) {
        int bytes_remaining = (sizeof(context->error_msg) - 1) - (int)strlen(context->error_msg);
        int indent = (int)(location - context->source) + 1;
        int bytes_required = 1 + (int)strlen(context->source) + 1 + indent + 1;
        if (bytes_remaining >= bytes_required) {
            strcat(context->error_msg, "\n");
            strcat(context->error_msg, context->source);
            strcat(context->error_msg, "\n");
            for (int i = 0; i < indent; i++) {
                strcat(context->error_msg, " ");
            }
            strcat(context->error_msg, "^");
        }
    }

    context->error_code = error_code;
    return error_code;
}

int je_alloc_transient(je_context_t* context, size_t size, char** ptr, int tag) {
    int align_padding = JE_MEM_ARENA_ALIGN - (((size_t)context->transient_mem_arena + context->transient_mem_arena_offset) % JE_MEM_ARENA_ALIGN);
    if (align_padding == JE_MEM_ARENA_ALIGN) {
        align_padding = 0;
    }
    size_t remaining_space = JE_MEM_ARENA_SIZE - context->transient_mem_arena_offset;
    if (remaining_space < size + JE_MEM_ARENA_ALIGN) {
        je_store_error(context, JE_RESULT_OOM, NULL, NULL);
        return context->error_code;
    }
    if (context->flags & JE_FLAG_DEBUG_MEM_LOGGING) {
        printf("je_alloc_transient size=%i tag=%s offset=%i padding=%i\n", (int)size, je_mem_tag_name(tag), (int)context->mem_arena_offset, align_padding);
    }
    *ptr = context->transient_mem_arena + context->transient_mem_arena_offset + align_padding;
    context->transient_mem_arena_offset += (uint16_t)(size + align_padding);
    context->mem_tag_allocations[tag] += (uint16_t)(size);
    assert(((size_t)*ptr % JE_MEM_ARENA_ALIGN) == 0);
    return JE_RESULT_SUCCESS;
}

int je_alloc(je_context_t* context, size_t size, char** ptr, int tag) {
    assert(!context->mem_arena_frozen);

    int align_padding = JE_MEM_ARENA_ALIGN - (((size_t)context->mem_arena + context->mem_arena_offset) % JE_MEM_ARENA_ALIGN);
    if (align_padding == JE_MEM_ARENA_ALIGN) {
        align_padding = 0;
    }
    size_t remaining_space = JE_MEM_ARENA_SIZE - context->mem_arena_offset;
    if (remaining_space < size + JE_MEM_ARENA_ALIGN) {
        je_store_error(context, JE_RESULT_OOM, NULL, NULL);
        return context->error_code;
    }
    if (context->flags & JE_FLAG_DEBUG_MEM_LOGGING) {
        printf("je_alloc size=%i tag=%s offset=%i padding=%i\n", (int)size, je_mem_tag_name(tag), (int)context->mem_arena_offset, align_padding);
    }
    *ptr = context->mem_arena + context->mem_arena_offset + align_padding;
    context->mem_arena_offset += (uint16_t)(size + align_padding);
    context->mem_tag_allocations[tag] += (uint16_t)(size);
    assert(((size_t)*ptr % JE_MEM_ARENA_ALIGN) == 0);
    return JE_RESULT_SUCCESS;
}

int je_realloc_string(je_context_t* context, const char* value, je_value_t* result, int length, bool transient) {
    size_t len = (length >= 0 ? length : strlen(value)) + 1;

    int ret = 0;
    if ( transient ) {
        ret = je_alloc_transient(context, len, &result->string_value, JE_MEM_TAG_STRING);
    } else {
        ret = je_alloc(context, len, &result->string_value, JE_MEM_TAG_STRING);
    }
    if (ret < 0) {
        return ret;
    }
    strncpy(result->string_value, value, len - 1);
    result->string_value[len - 1] = '\0';

    return JE_RESULT_SUCCESS;
}

int je_alloc_ast_node(je_context_t* context, je_ast_node_t** node) {
    int ret = je_alloc(context, sizeof(je_ast_node_t), (char**)node, JE_MEM_TAG_AST_NODE);
    if (ret < 0) {
        return ret;
    }
    memset(*node, 0, sizeof(je_ast_node_t));
    return JE_RESULT_SUCCESS;
}

int je_alloc_node_children(je_context_t* context, je_ast_node_t* node, int count) {
    if (node->child_count >= count) {
        node->child_count = count;
        return JE_RESULT_SUCCESS;
    }
    uint16_t* children_offsets;
    int ret = je_alloc(context, sizeof(uint16_t) * count, (char**)&children_offsets, JE_MEM_TAG_NODE_CHILD_ARRAY);
    if (ret < 0) {
        return ret;
    }
    memset(children_offsets, 0, sizeof(uint16_t) * count);
    node->child_offset = (uint16_t)((char*)children_offsets - (char*)context->mem_arena);
    node->child_count = count;
    return JE_RESULT_SUCCESS;
}

je_ast_node_t* je_get_node_child(je_context_t* context, je_ast_node_t* node, int index) {
    uint16_t* children_offsets = (uint16_t*)(context->mem_arena + node->child_offset);
    uint16_t child_offset = children_offsets[index];
    if (child_offset == 0) {
        return NULL;
    }
    return (je_ast_node_t*)(context->mem_arena + child_offset);
}

void je_set_node_child(je_context_t* context, je_ast_node_t* node, int index, je_ast_node_t* child) {
    uint16_t* children_offsets = (uint16_t*)(context->mem_arena + node->child_offset);
    uint16_t child_offset = (uint16_t)((char*)child - (char*)context->mem_arena);
    if (child == NULL) {
        children_offsets[index] = 0;
    } else {
        children_offsets[index] = child_offset;
    }
}

void je_set_func_param_type(je_context_t* context, je_func_def_t* func, int index, int type) {
    int nibble =  (index & 1);
    uint16_t* encoded = 0;
    if (func->param_count > 4) {
        uint16_t* param_types = (uint16_t*)(context->mem_arena + func->param_types_data);
        encoded = &param_types[index / 2];
    } else {
        encoded = &func->param_types_data;
    }

    if (nibble) {
        *encoded = (*encoded & 0xFF00) | type;
    } else {
        *encoded = (*encoded & 0x00FF) | (type << 8);
    }
}

int je_get_func_param_type(je_context_t* context, je_func_def_t* func, int index) {
    int nibble = (index & 1);
    uint16_t* encoded = 0;
    if (func->param_count > 4) {
        uint16_t* param_types = (uint16_t*)(context->mem_arena + func->param_types_data);
        encoded = &param_types[index / 2];
    }
    else {
        encoded = &func->param_types_data;
    }

    if (nibble) {
        return (int)(*encoded & 0x00FF);
    } else {
        return (int)((*encoded & 0xFF00) >> 8);
    }
}

uint16_t je_get_name_index(je_context_t* context, const char* name, uint16_t* index) {
#if JE_USE_NAME_HASHES
    * index = je_hash16(name);
    return JE_RESULT_SUCCESS;
#else
    uint16_t i = 0;
    je_name_t* last_name = NULL;
    for (je_name_t* ptr = context->name_head; ptr; /*empty*/) {
        const char* ptr_name = (context->mem_arena + ptr->name_offset);
        if (strcmp(ptr_name, name) == 0) {
            *index = i;
            return JE_RESULT_SUCCESS;
        }
        last_name = ptr;
        
        i++;
        if (ptr->next_offset == 0) {
            break;
        } else {
            ptr = (je_name_t*)(context->mem_arena + ptr->next_offset);
        }
    }

    je_name_t* new_name;
    int ret = je_alloc(context, sizeof(je_name_t), (char**)&new_name, JE_MEM_TAG_NAME);
    if (ret < 0) {
        return ret;
    }

    size_t len = strlen(name);
    char* name_ptr;
    ret = je_alloc(context, len + 1, &name_ptr, JE_MEM_TAG_STRING);
    if (ret < 0) {
        return ret;
    }
    strcpy(name_ptr, name);
    new_name->name_offset = (uint32_t)(name_ptr - context->mem_arena);

    new_name->next_offset = 0;
    if (last_name != NULL) {
        last_name->next_offset = (uint16_t)((char*)new_name - context->mem_arena);
    } else {
        context->name_head = new_name;
    }

    *index = i;
    return JE_RESULT_SUCCESS;
#endif
}

const char* je_get_name(je_context_t* context, uint16_t index) {
#if JE_USE_NAME_HASHES
    return "hashed";
#else
    int i = 0;
    for (je_name_t* ptr = context->name_head; ptr; i++) {
        const char* name = (context->mem_arena + ptr->name_offset);
        if (i == index) {
            return name;
        }
        if (ptr->next_offset == 0) {
            break;
        } else {
            ptr = (je_name_t*)(context->mem_arena + ptr->next_offset);
        }
    }
    return "";
#endif
}

const char* je_token_name(int type) {
    switch (type) {
        case JE_TOK_IDENTIFIER:             return "identifier";
        case JE_TOK_FLOAT:                  return "float literal";
        case JE_TOK_INT:                    return "int literal";
        case JE_TOK_STRING:                 return "string literal";
        case JE_TOK_BOOL:                   return "bool literal";
        case JE_TOK_OP_COMMA:               return ",";
        case JE_TOK_OP_MUL:                 return "*";
        case JE_TOK_OP_DIV:                 return "/";
        case JE_TOK_OP_SUB:                 return "-";
        case JE_TOK_OP_ADD:                 return "+";
        case JE_TOK_OP_MOD:                 return "%";
        case JE_TOK_OP_LESS:                return "<";
        case JE_TOK_OP_GREATER:             return ">";
        case JE_TOK_OP_LE:                  return "<=";
        case JE_TOK_OP_GE:                  return ">=";
        case JE_TOK_OP_EQUAL:               return "==";
        case JE_TOK_OP_NOT_EQUAL:           return "!=";
        case JE_TOK_OP_NOT:                 return "!";
        case JE_TOK_OP_LOGICAL_AND:         return "&&";
        case JE_TOK_OP_LOGICAL_OR:          return "||";
        case JE_TOK_OP_BITWISE_AND:         return "&";
        case JE_TOK_OP_BITWISE_OR:          return "|";
        case JE_TOK_OP_BITWISE_NOT:         return "~";
        case JE_TOK_OP_BITWISE_XOR:         return "^";
        case JE_TOK_OP_PARENTHESIS_OPEN:    return "(";
        case JE_TOK_OP_PARENTHESIS_CLOSE:   return ")";
        case JE_TOK_OP_KEYWORD_INT:         return "int";
        case JE_TOK_OP_KEYWORD_FLOAT:       return "float";
        case JE_TOK_OP_KEYWORD_STRING:      return "string";
        case JE_TOK_OP_KEYWORD_BOOL:        return "bool";
        default: {
            assert(0);
            return "unkown";
        }
    }
    return "";
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
        case JE_NODE_BITWISE_XOR:           return "bitwise_xor";
        case JE_NODE_LOGICAL_AND:           return "logical_and";
        case JE_NODE_LOGICAL_OR:            return "logical_or";
        case JE_NODE_VARIABLE:              return "variable";
        case JE_NODE_NEG:                   return "neg";
        case JE_NODE_POS:                   return "pos";
        case JE_NODE_CAST_TO_BOOL:          return "cast_to_bool";
        case JE_NODE_CAST_TO_INT:           return "cast_to_int";
        case JE_NODE_CAST_TO_FLOAT:         return "cast_to_float";
        case JE_NODE_CAST_TO_STRING:        return "cast_to_string";
        case JE_NODE_FUNCTION_CALL:         return "function_call";
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
        case JE_NODE_BITWISE_XOR_INT:       return "bitwise_xor_int";
        case JE_NODE_LOGICAL_AND_BOOL:      return "logical_and_bool";
        case JE_NODE_LOGICAL_OR_BOOL:       return "logical_or_bool";
        case JE_NODE_VARIABLE_BOOL:         return "variable_bool";
        case JE_NODE_VARIABLE_INT:          return "variable_int";
        case JE_NODE_VARIABLE_FLOAT:        return "variable_float";
        case JE_NODE_VARIABLE_STRING:       return "variable_string";
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
        case JE_NODE_FUNCTION_CALL_BOOL:    return "function_call_bool";
        case JE_NODE_FUNCTION_CALL_STRING:  return "function_call_string";
        case JE_NODE_FUNCTION_CALL_FLOAT:   return "function_call_float";
        case JE_NODE_FUNCTION_CALL_INT:     return "function_call_int";
        default: {
            assert(0); 
            return "unkown";
        }
    }
    return "";
}

je_variable_def_t* je_find_variable(je_context_t* context, uint16_t name_index) {
    for (je_variable_def_t* value = context->variable_head; value; /*empty*/) {
        if (value->name_index == name_index) {
            return value;
        }

        if (value->next_offset == 0) {
            break;
        } else {
            value = (je_variable_def_t*)(context->mem_arena + value->next_offset);
        }
    }
    return NULL;
}

int je_find_or_create_variable(je_context_t* context, const char* name, int type, je_variable_def_t** variable) {
    uint16_t index;
    int ret = je_get_name_index(context, name, &index);
    if (ret < 0) {
        return ret;
    }
    *variable = je_find_variable(context, index);
    if (*variable != NULL) {
        if ((*variable)->type != type) {
            return je_store_error(context, JE_RESULT_TYPE_CANNOT_CHANGE, NULL, NULL);
        }
        if ((*variable)->is_constant) {
            return je_store_error(context, JE_RESULT_VALUE_IS_CONSTANT, NULL, NULL);
        }
        return JE_RESULT_SUCCESS;
    }

    int data_size = 0;
    switch (type) {
        case JE_TYPE_BOOL:      data_size = sizeof(int);    break;
        case JE_TYPE_INT:       data_size = sizeof(int);    break;
        case JE_TYPE_STRING:    data_size = sizeof(char*);  break;
        case JE_TYPE_FLOAT:     data_size = sizeof(float);  break;
        default:                assert(false);              break;
    }

    ret = je_alloc(context, sizeof(je_variable_def_t) + data_size, (char**)variable, JE_MEM_TAG_VARIABLE_DEF);
    if (ret < 0) {
        return ret;
    }
    (*variable)->name_index = index;
    return JE_RESULT_SUCCESS;    
}

char** je_get_variable_string(je_variable_def_t* variable) {
    assert(variable->type == JE_TYPE_STRING);
    char* ptr = ((char*)variable) + sizeof(je_variable_def_t);
    return (char**)ptr;
}

float* je_get_variable_float(je_variable_def_t* variable) {
    assert(variable->type == JE_TYPE_FLOAT);
    char* ptr = ((char*)variable) + sizeof(je_variable_def_t);
    return (float*)ptr;
}

int* je_get_variable_int(je_variable_def_t* variable) {
    assert(variable->type == JE_TYPE_INT);
    char* ptr = ((char*)variable) + sizeof(je_variable_def_t);
    return (int*)ptr;
}

int* je_get_variable_bool(je_variable_def_t* variable) {
    assert(variable->type == JE_TYPE_BOOL);
    char* ptr = ((char*)variable) + sizeof(je_variable_def_t);
    return (int*)ptr;
}

void je_set_variable_string(je_variable_def_t* variable, char* value) {
    *je_get_variable_string(variable) = value;
}

void je_set_variable_float(je_variable_def_t* variable, float value) {
    *je_get_variable_float(variable) = value;
}

void je_set_variable_int(je_variable_def_t* variable, int value) {
    *je_get_variable_int(variable) = value;
}

void je_set_variable_bool(je_variable_def_t* variable, int value) {
    *je_get_variable_bool(variable) = value;
}

je_func_def_t* je_find_function(je_context_t* context, uint16_t name_index) {
    for (je_func_def_t* value = context->function_head; value; /*empty*/) {
        if (value->name_index == name_index) {
            return value;
        }

        if (value->next_offset == 0) {
            break;
        } else {
            value = (je_func_def_t*)(context->mem_arena + value->next_offset);
        }
    }
    return NULL;
}

int je_find_or_create_function(je_context_t* context, const char* name, je_func_def_t** func) {
    uint16_t index;

    int ret = je_get_name_index(context, name, &index);
    if (ret < 0) {
        return ret;
    }
    *func = je_find_function(context, index);
    if (*func != NULL) {
        if ((*func)->is_deterministic) {
            return je_store_error(context, JE_RESULT_VALUE_IS_CONSTANT, NULL, NULL);
        }
        return JE_RESULT_SUCCESS;
    }
    ret = je_alloc(context, sizeof(je_func_def_t), (char**)func, JE_MEM_TAG_FUNCTION_DEF);
    if (ret < 0) {
        return ret;
    }
    (*func)->name_index = index;
    return JE_RESULT_SUCCESS;    
}

float* je_get_ast_node_float(je_context_t* context, je_ast_node_t* node) {
    return (float*)(context->mem_arena + node->type_data);
}

int* je_get_ast_node_int(je_context_t* context, je_ast_node_t* node) {
    return (int*)(context->mem_arena + node->type_data);
}

char** je_get_ast_node_string(je_context_t* context, je_ast_node_t* node) {
    return (char**)(context->mem_arena + node->type_data);
}

int* je_get_ast_node_bool(je_context_t* context, je_ast_node_t* node) {
    return je_get_ast_node_int(context, node);
}

je_func_def_t* je_get_ast_node_function(je_context_t* context, je_ast_node_t* node) {
    return je_find_function(context, (uint16_t)node->type_data);
}

je_variable_def_t* je_get_ast_node_variable(je_context_t* context, je_ast_node_t* node) {
    return je_find_variable(context, (uint16_t)node->type_data);
}

int je_get_ast_node_name_index(je_context_t* context, je_ast_node_t* node) {
    return node->type_data;
}

int je_set_ast_node_float(je_context_t* context, je_ast_node_t* node, float value) {
    float* alloc_value;
    int ret = je_alloc(context, sizeof(float), (char**)&alloc_value, JE_MEM_TAG_TYPE_DATA);
    if (ret < 0) {
        return ret;
    }
    *alloc_value = value;
    node->type_data = (uint16_t)((char*)alloc_value - context->mem_arena);
    return JE_RESULT_SUCCESS;
}

int je_set_ast_node_int(je_context_t* context, je_ast_node_t* node, int value) {
    int* alloc_value;
    int ret = je_alloc(context, sizeof(int), (char**)&alloc_value, JE_MEM_TAG_TYPE_DATA);
    if (ret < 0) {
        return ret;
    }
    *alloc_value = value;
    node->type_data = (uint16_t)((char*)alloc_value - context->mem_arena);
    return JE_RESULT_SUCCESS;
}

int je_set_ast_node_string(je_context_t* context, je_ast_node_t* node, char* value) {
    char** alloc_value;
    int ret = je_alloc(context, sizeof(char*), (char**)&alloc_value, JE_MEM_TAG_TYPE_DATA);
    if (ret < 0) {
        return ret;
    }
    *alloc_value = value;
    node->type_data = (uint16_t)((char*)alloc_value - context->mem_arena);
    return JE_RESULT_SUCCESS;
}

int je_set_ast_node_bool(je_context_t* context, je_ast_node_t* node, bool value) {
    return je_set_ast_node_int(context, node, value);
}

int je_set_ast_node_function(je_context_t* context, je_ast_node_t* node, je_func_def_t* value) {
    node->type_data = value->name_index;
    return JE_RESULT_SUCCESS;
}

int je_set_ast_node_variable(je_context_t* context, je_ast_node_t* node, je_variable_def_t* value) {
    node->type_data = value->name_index;
    return JE_RESULT_SUCCESS;
}

int je_set_ast_node_name_index(je_context_t* context, je_ast_node_t* node, int value) {
    node->type_data = value;
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
            return je_store_error(context, JE_RESULT_CORRUPT, NULL, NULL);
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
            return je_store_error(context, JE_RESULT_CORRUPT, NULL, NULL);
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
            return je_store_error(context, JE_RESULT_CORRUPT, NULL, NULL);
        }
    }

    int ret = je_realloc_string(context, buffer, value, -1, true);
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
            value->bool_value = (value->int_value != 0);
            break;
        }
        case JE_TYPE_FLOAT: {
            value->type = JE_TYPE_BOOL;
            value->bool_value = (fabs(value->float_value) > JE_FLOAT_EPSILON ? 1 : 0);
            break;
        }
        case JE_TYPE_STRING: {
            value->type = JE_TYPE_BOOL;
            value->bool_value = (strcmp(value->string_value, "false") != 0 && strcmp(value->string_value, "0") != 0);
            break;
        }
        default: {
            return je_store_error(context, JE_RESULT_CORRUPT, NULL, NULL);
        }
    }
    return JE_RESULT_SUCCESS;
}

// -----------------------------------------------------------------------
// PUBLIC FUNCTIONS
// -----------------------------------------------------------------------

int je_get_parameter_int(je_context_t* context, int index, int* result);
int je_get_parameter_float(je_context_t* context, int index, float* result);
int je_get_parameter_bool(je_context_t* context, int index, int* result);
int je_get_parameter_string(je_context_t* context, int index, const char** result);
int je_return_int(je_context_t* context, int result);
int je_return_float(je_context_t* context, float result);
int je_return_bool(je_context_t* context, int result);
int je_return_string(je_context_t* context, const char* result);

const char* je_error_msg(je_context_t* context) {
    return context->error_msg;
}

void je_memory_stats(je_context_t* context, int* permanent_mem_used, int* transient_mem_used, int* executable_mem_used) {
    if (permanent_mem_used != NULL) {
        *permanent_mem_used = (int)context->mem_arena_offset;
    }
    if (transient_mem_used != NULL) {
        *transient_mem_used = (int)context->transient_mem_arena_offset;
    }
#ifdef JE_JIT_AVAILABLE
    if (executable_mem_used != NULL) {
        *executable_mem_used = (int)context->jit_executable_memory_length;
    }
#else
    *executable_mem_used = 0;
#endif
}

void je_intrinsic_int_to_string(je_context_t* ctx) {
    char buffer[32];
    int value;
    je_get_parameter_int(ctx, 0, &value);
    sprintf(buffer, "%i", value);
    je_return_string(ctx, buffer);
}

void je_intrinsic_float_to_string(je_context_t* ctx) {
    char buffer[32];
    float value;
    je_get_parameter_float(ctx, 0, &value);
    sprintf(buffer, "%f", value);
    je_return_string(ctx, buffer);
}

void je_intrinsic_bool_to_string(je_context_t* ctx) {
    int value;
    je_get_parameter_bool(ctx, 0, &value);
    je_return_string(ctx, value ? "true" : "false");
}

void je_intrinsic_string_to_int(je_context_t* ctx) {
    const char* value;
    je_get_parameter_string(ctx, 0, &value);
    je_return_int(ctx, atoi(value));
}

void je_intrinsic_string_to_float(je_context_t* ctx) {
    const char* value;
    je_get_parameter_string(ctx, 0, &value);
    je_return_float(ctx, (float)atof(value));
}

void je_intrinsic_string_to_bool(je_context_t* ctx) {
    const char* value;
    je_get_parameter_string(ctx, 0, &value);
    je_return_bool(ctx, strcmp(value, "") != 0 && strcmp(value, "false") != 0 && strcmp(value, "0") != 0);
}

void je_intrinsic_string_concat(je_context_t* ctx) {
    const char* a;
    const char* b;
    je_get_parameter_string(ctx, 0, &a);
    je_get_parameter_string(ctx, 1, &b);

    int a_len = (int)strlen(a);
    int b_len = (int)strlen(b);
    int size = a_len + b_len + 1;

    char* buffer;
    int ret = je_alloc_transient(ctx, size, &buffer, JE_MEM_TAG_STRING);
    if (ret != JE_RESULT_SUCCESS) {
        assert(false);
    }

    memcpy(buffer, a, a_len);
    memcpy(buffer + a_len, b, b_len);
    buffer[size - 1] = '\0';

    je_return_string(ctx, buffer);
}

void je_intrinsic_string_equal(je_context_t* ctx) {
    const char* a;
    const char* b;
    je_get_parameter_string(ctx, 0, &a);
    je_get_parameter_string(ctx, 1, &b);
    je_return_bool(ctx, strcmp(a, b) == 0);
}

void je_intrinsic_string_not_equal(je_context_t* ctx) {
    const char* a;
    const char* b;
    je_get_parameter_string(ctx, 0, &a);
    je_get_parameter_string(ctx, 1, &b);
    je_return_bool(ctx, strcmp(a, b) != 0);
}

void je_intrinsic_floor(je_context_t* ctx) {
    float a;
    je_get_parameter_float(ctx, 0, &a);
    je_return_float(ctx, (float)floor(a));
}

void je_intrinsic_ceil(je_context_t* ctx) {
    float a;
    je_get_parameter_float(ctx, 0, &a);
    je_return_float(ctx, (float)ceil(a));
}

void je_intrinsic_round(je_context_t* ctx) {
    float a;
    je_get_parameter_float(ctx, 0, &a);
    je_return_float(ctx, (float)round(a));
}

void je_intrinsic_min(je_context_t* ctx) {
    float a;
    float b;
    je_get_parameter_float(ctx, 0, &a);
    je_get_parameter_float(ctx, 1, &b);
    je_return_float(ctx, a < b ? a : b);
}

void je_intrinsic_max(je_context_t* ctx) {
    float a;
    float b;
    je_get_parameter_float(ctx, 0, &a);
    je_get_parameter_float(ctx, 1, &b);
    je_return_float(ctx, a < b ? b : a);
}

void je_intrinsic_abs(je_context_t* ctx) {
    float a;
    je_get_parameter_float(ctx, 0, &a);
    je_return_float(ctx, a < 0.0f ? -a : a);
}

int je_eval_int(const char* expression, char* error_msg, int error_msg_len) {
    je_context_t ctx;
    int ret = je_new_context(&ctx, JE_FLAG_NONE);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0;
    }

    ret = je_compile(&ctx, expression);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0;
    }

    ret = je_eval(&ctx);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0;
    }

    int result = 0;
    ret = je_result_int(&ctx, &result);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0;
    }

    je_free_context(&ctx);
    return result;
}

float je_eval_float(const char* expression, char* error_msg, int error_msg_len) {
    je_context_t ctx;
    int ret = je_new_context(&ctx, JE_FLAG_NONE);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0.0f;
    }

    ret = je_compile(&ctx, expression);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0.0f;
    }

    ret = je_eval(&ctx);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0.0f;
    }

    float result = 0.0f;
    ret = je_result_float(&ctx, &result);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0.0f;
    }

    je_free_context(&ctx);
    return result;
}

bool je_eval_bool(const char* expression, char* error_msg, int error_msg_len) {
    je_context_t ctx;
    int ret = je_new_context(&ctx, JE_FLAG_NONE);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0;
    }

    ret = je_compile(&ctx, expression);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0;
    }

    ret = je_eval(&ctx);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0;
    }

    int result = 0;
    ret = je_result_bool(&ctx, &result);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return 0;
    }

    je_free_context(&ctx);
    return result;
}

char* je_eval_string(const char* expression, char* error_msg, int error_msg_len) {
    je_context_t ctx;
    int ret = je_new_context(&ctx, JE_FLAG_NONE);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return NULL;
    }

    ret = je_compile(&ctx, expression);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return NULL;
    }

    ret = je_eval(&ctx);
    if (ret < 0) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return NULL;
    }

    const char* result = NULL;
    ret = je_result_string(&ctx, &result);
    if (ret < 0 || result == NULL) {
        if (error_msg != NULL) {
            strncpy(error_msg, je_error_msg(&ctx), error_msg_len);
        }
        je_free_context(&ctx);
        return NULL;
    }

    size_t result_len = strlen(result);
    char* buffer = malloc(result_len + 1);
    strncpy(buffer, result, result_len + 1);

    je_free_context(&ctx);
    return buffer;
}

int je_new_context(je_context_t* context, int flags) {

#ifdef JE_DEBUG_FORCE_FLAGS
    flags |= JE_DEBUG_FORCE_FLAGS;
#endif

    if (flags & JE_FLAG_DEBUG_LOGGING) {
#ifdef JE_JIT_AVAILABLE
        const char* jit_available_str = "Yes";
#else
        const char* jit_available_str = "No";
#endif
        printf("JitEval: ISA=%s Platform=%s Compiler=%s JitAvailable=%s\n", JE_ISA_NAME, JE_PLATFORM_NAME, JE_COMPILER_NAME, jit_available_str);
    }

    memset(context, 0, sizeof(struct je_context_t));
    context->flags = flags;

    // We use offsets in various places with 0 as a sentinel/null value, so always start offsets at 1.
    context->mem_arena_offset = 1;
    context->transient_mem_arena_offset = 1;

    // Register all intrinsic functions.
    je_bind_function(context, "__int_to_string",    true, &je_intrinsic_int_to_string,      JE_TYPE_STRING,     JE_TYPE_INT,        NULL);
    je_bind_function(context, "__float_to_string",  true, &je_intrinsic_float_to_string,    JE_TYPE_STRING,     JE_TYPE_FLOAT,      NULL);
    je_bind_function(context, "__bool_to_string",   true, &je_intrinsic_bool_to_string,     JE_TYPE_STRING,     JE_TYPE_BOOL,       NULL);
    je_bind_function(context, "__string_to_int",    true, &je_intrinsic_string_to_int,      JE_TYPE_INT,        JE_TYPE_STRING,     NULL);
    je_bind_function(context, "__string_to_float",  true, &je_intrinsic_string_to_float,    JE_TYPE_FLOAT,      JE_TYPE_STRING,     NULL);
    je_bind_function(context, "__string_to_bool",   true, &je_intrinsic_string_to_bool,     JE_TYPE_BOOL,       JE_TYPE_STRING,     NULL);
    je_bind_function(context, "__string_concat",    true, &je_intrinsic_string_concat,      JE_TYPE_STRING,     JE_TYPE_STRING,     JE_TYPE_STRING,     NULL);
    je_bind_function(context, "__string_equal",     true, &je_intrinsic_string_equal,       JE_TYPE_BOOL,       JE_TYPE_STRING,     JE_TYPE_STRING,     NULL);
    je_bind_function(context, "__string_not_equal", true, &je_intrinsic_string_not_equal,   JE_TYPE_BOOL,       JE_TYPE_STRING,     JE_TYPE_STRING,     NULL);

    // Basic utility functions
    if ((flags & JE_FLAG_NO_UTILITY_FUNCTIONS) == 0) {

        je_bind_function(context, "floor",          true, &je_intrinsic_floor,              JE_TYPE_FLOAT,      JE_TYPE_FLOAT,      NULL);
        je_bind_function(context, "ceil",           true, &je_intrinsic_ceil,               JE_TYPE_FLOAT,      JE_TYPE_FLOAT,      NULL);
        je_bind_function(context, "round",          true, &je_intrinsic_round,              JE_TYPE_FLOAT,      JE_TYPE_FLOAT,      NULL);
        je_bind_function(context, "min",            true, &je_intrinsic_min,                JE_TYPE_FLOAT,      JE_TYPE_FLOAT,      JE_TYPE_FLOAT,      NULL);
        je_bind_function(context, "max",            true, &je_intrinsic_max,                JE_TYPE_FLOAT,      JE_TYPE_FLOAT,      JE_TYPE_FLOAT,      NULL);
        je_bind_function(context, "abs",            true, &je_intrinsic_abs,                JE_TYPE_FLOAT,      JE_TYPE_FLOAT,      NULL);

        je_bind_variable_float  (context,     "PI",              true, 3.14159265f);
        je_bind_variable_string (context,    "__platform",       true, JE_PLATFORM_NAME);
        je_bind_variable_string (context,    "__compiler",       true, JE_COMPILER_NAME);
        je_bind_variable_string (context,    "__isa",            true, JE_ISA_NAME);
        je_bind_variable_int    (context,    "__version_major",  true, JE_VERSION_MAJOR);
        je_bind_variable_int    (context,    "__version_minor",  true, JE_VERSION_MINOR);
    }

    return JE_RESULT_SUCCESS;
}

int je_free_context(je_context_t* context) {
    if (context->flags & JE_FLAG_DEBUG_MEM_LOGGING) {
        for (int i = 0; i < JE_MEM_TAG_COUNT; i++) {
            printf("%s : %i bytes\n", je_mem_tag_name(i), (int)context->mem_tag_allocations[i]);
        }
    }
#ifdef JE_JIT_AVAILABLE
    if (context->jit_compiled) {
        je_jit_free(context);
    }
#endif
    return JE_RESULT_SUCCESS;
}

int je_bind_variable_int(je_context_t* context, const char* name, bool is_constant, int value) {
    je_variable_def_t* variable = NULL;
    int ret = je_find_or_create_variable(context, name, JE_TYPE_INT, &variable);
    if (ret < 0) {
        return ret;
    }

    variable->is_constant = is_constant;
    variable->type = JE_TYPE_INT;
    variable->next_offset = (uint16_t)(context->variable_head ? ((char*)context->variable_head - (char*)context->mem_arena) : 0);
    context->variable_head = variable;

    je_set_variable_int(variable, value);

    return JE_RESULT_SUCCESS;
}

int je_bind_variable_float(je_context_t* context, const char* name, bool is_constant, float value) {
    je_variable_def_t* variable = NULL;
    int ret = je_find_or_create_variable(context, name, JE_TYPE_FLOAT, &variable);
    if (ret < 0) {
        return ret;
    }

    variable->is_constant = is_constant;
    variable->type = JE_TYPE_FLOAT;
    variable->next_offset = (uint16_t)(context->variable_head ? ((char*)context->variable_head - (char*)context->mem_arena) : 0);
    context->variable_head = variable;

    je_set_variable_float(variable, value);

    return JE_RESULT_SUCCESS;
}

int je_bind_variable_bool(je_context_t* context, const char* name, bool is_constant, int value) {
    je_variable_def_t* variable = NULL;
    int ret = je_find_or_create_variable(context, name, JE_TYPE_BOOL, &variable);
    if (ret < 0) {
        return ret;
    }

    variable->is_constant = is_constant;
    variable->type = JE_TYPE_BOOL;
    variable->next_offset = (uint16_t)(context->variable_head ? ((char*)context->variable_head - (char*)context->mem_arena) : 0);
    context->variable_head = variable;

    je_set_variable_bool(variable, value);

    return JE_RESULT_SUCCESS;
}

int je_bind_variable_string(je_context_t* context, const char* name, bool is_constant, const char* value) {
    je_variable_def_t* variable = NULL;
    int ret = je_find_or_create_variable(context, name, JE_TYPE_STRING, &variable);
    if (ret < 0) {
        return ret;
    }

    variable->is_constant = is_constant;
    variable->type = JE_TYPE_STRING;

    int len = (int)strlen(value);
    char* copy;
    ret = je_alloc(context, len + 1, &copy, JE_MEM_TAG_STRING);
    if (ret < 0) {
        return ret;
    }

    memcpy(copy, value, len + 1);

    je_set_variable_string(variable, copy);

    variable->next_offset = (uint16_t)(context->variable_head ? ((char*)context->variable_head - (char*)context->mem_arena) : 0);
    context->variable_head = variable;

    return JE_RESULT_SUCCESS;
}

int je_bind_function(je_context_t* context, const char* name, bool is_deterministic, je_func_t func, int return_type, ...) {
    je_func_def_t* function = NULL;
    int ret = je_find_or_create_function(context, name, &function);
    if (ret < 0) {
        return ret;
    }

    function->param_count = 0;
    function->return_type = return_type;
    function->is_deterministic = is_deterministic;
    function->function = func;

    int param_types[JE_MAX_PARAMETERS];

    va_list list;
    va_start(list, return_type);
    while (1) {
        int arg_type = va_arg(list, int);
        if (arg_type == 0) {
            break;
        }
        if (function->param_count == JE_MAX_PARAMETERS) {
            return JE_RESULT_MAX_PARAMETER_COUNT_EXCEEDED;
        }
        param_types[function->param_count++] = arg_type;
    }
    va_end(list);

    // If we have more than 4 parameters we need more space to encode our parameter types.
    if (function->param_count > 4) {
        uint16_t* allocation;
        int allocation_size = (function->param_count + 1) / 2;
        ret = je_alloc(context, allocation_size, (char**)&allocation, JE_MEM_TAG_PARAM_TYPES);
        if (ret < 0) {
            return ret;
        }
        memset(allocation, 0, allocation_size);
        function->param_types_data = (uint16_t)((char*)allocation - (char*)context->mem_arena);
    } else {
        function->param_types_data = 0;
    }

    for (int i = 0; i < function->param_count; i++) {
        je_set_func_param_type(context, function, i, param_types[i]);
    }

    function->next_offset = (uint16_t)(context->function_head ? ((char*)context->function_head - (char*)context->mem_arena) : 0);
    context->function_head = function;

    return JE_RESULT_SUCCESS;
}

int je_get_parameter_int(je_context_t* context, int index, int* result) {
    if (context->active_function == NULL) {
        return je_store_error(context, JE_RESULT_NOT_IN_FUNCTION, NULL, NULL);
    }
    if (index >= context->active_function->param_count) {
        return je_store_error(context, JE_RESULT_PARAMETER_INDEX_OUT_OF_BOUNDS, NULL, NULL);
    }
    if (je_get_func_param_type(context, context->active_function, index) != JE_TYPE_INT) {
        return je_store_error(context, JE_RESULT_WRONG_PARAMETER_TYPE, NULL, NULL);
    }

    *result = context->function_params[index].int_value;

    return JE_RESULT_SUCCESS;
}

int je_get_parameter_float(je_context_t* context, int index, float* result) {
    if (context->active_function == NULL) {
        return je_store_error(context, JE_RESULT_NOT_IN_FUNCTION, NULL, NULL);
    }
    if (index >= context->active_function->param_count) {
        return je_store_error(context, JE_RESULT_PARAMETER_INDEX_OUT_OF_BOUNDS, NULL, NULL);
    }
    if (je_get_func_param_type(context, context->active_function, index) != JE_TYPE_FLOAT) {
        return je_store_error(context, JE_RESULT_WRONG_PARAMETER_TYPE, NULL, NULL);
    }

    *result = context->function_params[index].float_value;

    return JE_RESULT_SUCCESS;
}

int je_get_parameter_bool(je_context_t* context, int index, int* result) {
    if (context->active_function == NULL) {
        return je_store_error(context, JE_RESULT_NOT_IN_FUNCTION, NULL, NULL);
    }
    if (index >= context->active_function->param_count) {
        return je_store_error(context, JE_RESULT_PARAMETER_INDEX_OUT_OF_BOUNDS, NULL, NULL);
    }
    if (je_get_func_param_type(context, context->active_function, index) != JE_TYPE_BOOL) {
        return je_store_error(context, JE_RESULT_WRONG_PARAMETER_TYPE, NULL, NULL);
    }

    *result = context->function_params[index].int_value;

    return JE_RESULT_SUCCESS;
}

int je_get_parameter_string(je_context_t* context, int index, const char** result) {
    if (context->active_function == NULL) {
        return je_store_error(context, JE_RESULT_NOT_IN_FUNCTION, NULL, NULL);
    }
    if (index >= context->active_function->param_count) {
        return je_store_error(context, JE_RESULT_PARAMETER_INDEX_OUT_OF_BOUNDS, NULL, NULL);
    }
    if (je_get_func_param_type(context, context->active_function, index) != JE_TYPE_STRING) {
        return je_store_error(context, JE_RESULT_WRONG_PARAMETER_TYPE, NULL, NULL);
    }

    *result = context->function_params[index].string_value;

    return JE_RESULT_SUCCESS;
}

int je_return_int(je_context_t* context, int result) {
    if (context->active_function == NULL) {
        return je_store_error(context, JE_RESULT_NOT_IN_FUNCTION, NULL, NULL);
    }
    if (context->active_function->return_type != JE_TYPE_INT) {
        return je_store_error(context, JE_RESULT_INCORRECT_FUNC_RETURN_TYPE, NULL, NULL);
    }

    context->function_result.type = JE_TYPE_INT;
    context->function_result.int_value = result;

    return JE_RESULT_SUCCESS;
}

int je_return_float(je_context_t* context, float result) {
    if (context->active_function == NULL) {
        return je_store_error(context, JE_RESULT_NOT_IN_FUNCTION, NULL, NULL);
    }
    if (context->active_function->return_type != JE_TYPE_FLOAT) {
        return je_store_error(context, JE_RESULT_INCORRECT_FUNC_RETURN_TYPE, NULL, NULL);
    }

    context->function_result.type = JE_TYPE_FLOAT;
    context->function_result.float_value = result;

    return JE_RESULT_SUCCESS;
}

int je_return_bool(je_context_t* context, int result) {
    if (context->active_function == NULL) {
        return je_store_error(context, JE_RESULT_NOT_IN_FUNCTION, NULL, NULL);
    }
    if (context->active_function->return_type != JE_TYPE_BOOL) {
        return je_store_error(context, JE_RESULT_INCORRECT_FUNC_RETURN_TYPE, NULL, NULL);
    }

    context->function_result.type = JE_TYPE_BOOL;
    context->function_result.bool_value = result;

    return JE_RESULT_SUCCESS;
}

int je_return_string(je_context_t* context, const char* result) {
    if (context->active_function == NULL) {
        return je_store_error(context, JE_RESULT_NOT_IN_FUNCTION, NULL, NULL);
    }
    if (context->active_function->return_type != JE_TYPE_STRING) {
        return je_store_error(context, JE_RESULT_INCORRECT_FUNC_RETURN_TYPE, NULL, NULL);
    }

    context->function_result.type = JE_TYPE_STRING;

    size_t len = strlen(result) + 1;
    int ret = je_alloc_transient(context, len, &context->function_result.string_value, JE_MEM_TAG_STRING);
    if (ret < 0) {
        return ret;
    }
    strncpy(context->function_result.string_value, result, len);

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

// -----------------------------------------------------------------------
// LEXICAL ANALYSIS
// -----------------------------------------------------------------------

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
        case ',': tok->type = JE_TOK_OP_COMMA;              break;
        case '(': tok->type = JE_TOK_OP_PARENTHESIS_OPEN;   break;
        case ')': tok->type = JE_TOK_OP_PARENTHESIS_CLOSE;  break;
        case '~': tok->type = JE_TOK_OP_BITWISE_NOT;        break;
        case '^': tok->type = JE_TOK_OP_BITWISE_XOR;        break;
        case '=': {
            if (context->read_ptr[0] == '=') {
                tok->type = JE_TOK_OP_EQUAL;
                tok->source_len++;
                context->read_ptr++;
            } else {
                return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok->source_ptr, "Encountered unexpected character '%c'", c);
            }
            break;
        }
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
                    return je_store_error(context, JE_RESULT_UNEXPECTED_UNTERMINATED_STRING, tok->source_ptr, "Unterminated string");
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
                            return je_store_error(context, JE_RESULT_UNEXPECTED_ESCAPE_SEQUENCE, tok->source_ptr, "Found unexpected escape sequence '%c'", escape_c);
                        }
                    }
                } else {
                    string_buffer[len++] = c;
                    tok->source_len++;
                }
            }

            // Copy string buffer to storage.
            int ret = je_alloc(context, len + 1, &tok->string_value, JE_MEM_TAG_STRING);
            if (ret < 0) {
                return ret;
            }
            strncpy(tok->string_value, string_buffer, len + 1);
            tok->string_value[len] = '\0';

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
                    tok->bool_value = false;
                } else if (strncmp(tok->source_ptr, "true", tok->source_len) == 0) {
                    tok->type = JE_TOK_BOOL;
                    tok->bool_value = true;
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
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok->source_ptr, "Floating point values cannot contain multiple radix's");
                        }
                        tok->type = JE_TOK_FLOAT;
                    } else if (c == 'e') {
                        if (found_hex) {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok->source_ptr, "Exponent values cannot have a hex prefix");
                        }
                        if (found_exponent) {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok->source_ptr, "Number values cannot contain multiple exponent values");
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
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok->source_ptr, "Exponent values cannot have a hex prefix");
                        }
                        if (found_hex) {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok->source_ptr, "Number values cannot contain multiple hex prefixes");
                        }
                        if (tok->source_ptr[0] != '0' || tok->source_len != 1) {
                            return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok->source_ptr, "Invalid hex prefix in number");
                        }
                        found_hex = 1;
                    } else {
                        break;
                    }
                    tok->source_len++;
                    context->read_ptr++;
                } while (context->read_ptr[0]);

                if (tok->type == JE_TOK_INT) {
                    char buffer[32];
                    size_t len = context->read_ptr - tok->source_ptr;
                    if (len >= sizeof(buffer) - 1) {
                        return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok->source_ptr, "Number value too long to parse");
                    }
                    memcpy(buffer, tok->source_ptr, len);
                    buffer[len] = '\0';

                    if (found_hex) {
                        tok->int_value = strtol(buffer + 2, NULL, 16);
                    } else {
                        tok->int_value = strtol(buffer, NULL, 10);
                    }
                } else if (tok->type == JE_TOK_FLOAT) {
                    char buffer[32];
                    size_t len = context->read_ptr - tok->source_ptr;
                    if (len >= sizeof(buffer) - 1) {
                        return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok->source_ptr, "Number value too long to parse");
                    }
                    memcpy(buffer, tok->source_ptr, len);
                    buffer[len] = '\0';

                    tok->float_value = (float)atof(buffer);
                }
            // Unknown
            } else {
                return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok->source_ptr, "Encountered unexpected character '%c'");
            }
        }
    }

    return JE_RESULT_SUCCESS;
}

int je_peek_token(je_context_t* context, je_token_t* tok) {
    const char* read_ptr = context->read_ptr;
    int ret = je_read_token(context, tok);
    context->read_ptr = read_ptr;
    return ret;
}

int je_expect_token(je_context_t* context, je_token_t* tok, int type) {
    int ret = je_read_token(context, tok);
    if (ret == JE_RESULT_EOF) {
        return je_store_error(context, JE_RESULT_UNEXPECTED_EOF, NULL, "Unexpected end of token stream, expecting '%s'", je_token_name(type));
    }
    else if (ret < 0) {
        return ret;
    }
    if (tok->type != type) {
        return je_store_error(context, JE_RESULT_UNEXPECTED_TOKEN, tok->source_ptr, "Unexpected token '%s', expecting '%s'", je_token_name(tok->type), je_token_name(type));
    }
    return JE_RESULT_SUCCESS;
}

// -----------------------------------------------------------------------
// PARSING
// -----------------------------------------------------------------------

int je_parse_term(je_context_t* context, je_ast_node_t** node) {

    je_token_t tok;
    int ret = je_read_token(context, &tok);
    if (ret == JE_RESULT_EOF) {
        return je_store_error(context, JE_RESULT_UNEXPECTED_EOF, NULL, "Unexpected end of token stream.");
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
            ret = je_expect_token(context, &tok, JE_TOK_OP_PARENTHESIS_CLOSE);
            if (ret < 0) {
                return ret;
            }
            break;
        }
        case JE_TOK_STRING: {
            ret = je_alloc_ast_node(context, node);
            if (ret < 0) {
                return ret;
            }
            (*node)->type = JE_NODE_STRING_LITERAL;

            je_value_t value;
            int ret = je_realloc_string(context, tok.string_value, &value, -1, false);
            if (ret < 0) {
                return ret;
            }

            ret = je_set_ast_node_string(context, *node, value.string_value);
            if (ret < 0) {
                return ret;
            }

            break;
        }
        case JE_TOK_IDENTIFIER: {
            ret = je_alloc_ast_node(context, node);
            if (ret < 0) {
                return ret;
            }

            // Copy name into memory.
            char buffer[64];
            if (tok.source_len >= sizeof(buffer) - 1) {
                return je_store_error(context, JE_RESULT_UNEXPECTED_CHARACTER, tok.source_ptr, "Identifier too long to parse");
            }
            memcpy(buffer, tok.source_ptr, tok.source_len);
            buffer[tok.source_len] = '\0';

            uint16_t name_index = 0;
            ret = je_get_name_index(context, buffer, &name_index);
            if (ret < 0) {
                return ret;
            }

            je_set_ast_node_name_index(context, *node, name_index);

            bool is_function_call = false;
            ret = je_peek_token(context, &tok);
            if (ret == JE_RESULT_SUCCESS) {
                if (tok.type == JE_TOK_OP_PARENTHESIS_OPEN) {
                    is_function_call = true;
                    je_read_token(context, &tok);
                }
            }

            if (is_function_call) {
                (*node)->type = JE_NODE_FUNCTION_CALL;

                je_ast_node_t* children[JE_MAX_PARAMETERS];

                // Read all parameters
                while (1) {
                    ret = je_peek_token(context, &tok);
                    if (ret == JE_RESULT_SUCCESS && tok.type == JE_TOK_OP_PARENTHESIS_CLOSE) {
                        break;
                    }

                    if ((*node)->param_count > 0) {
                        ret = je_expect_token(context, &tok, JE_TOK_OP_COMMA);
                        if (ret < 0) {
                            return ret;
                        }
                    }

                    if ((*node)->param_count >= JE_MAX_PARAMETERS) {
                        return je_store_error(context, JE_RESULT_MAX_PARAMETER_COUNT_EXCEEDED, tok.source_ptr, "Function call '%s' exceeded max number of parameters (%i)", je_get_name(context, name_index), JE_MAX_PARAMETERS);
                    }

                    ret = je_parse(context, &children[(*node)->param_count++], JE_MAX_OPERATOR_PRECEDENCE);
                    if (ret < 0) {
                        return ret;
                    }
                }

                ret = je_alloc_node_children(context, *node, (*node)->param_count);
                if (ret < 0) {
                    return ret;
                }

                for (int i = 0; i < (*node)->param_count; i++) {
                    je_set_node_child(context, *node, i, children[i]);
                }

                // Read closing brace.
                ret = je_expect_token(context, &tok, JE_TOK_OP_PARENTHESIS_CLOSE);
                if (ret < 0) {
                    return ret;
                }

            } else {
                (*node)->type = JE_NODE_VARIABLE;
            }

            break;
        }
        case JE_TOK_FLOAT: {
            ret = je_alloc_ast_node(context, node);
            if (ret < 0) {
                return ret;
            }
            (*node)->type = JE_NODE_FLOAT_LITERAL;
            
            ret = je_set_ast_node_float(context, *node, tok.float_value);
            if (ret < 0) {
                return ret;
            }

            break;
        }
        case JE_TOK_INT: {
            ret = je_alloc_ast_node(context, node);
            if (ret < 0) {
                return ret;
            }
            (*node)->type = JE_NODE_INT_LITERAL;

            ret = je_set_ast_node_int(context, *node, tok.int_value);
            if (ret < 0) {
                return ret;
            }

            break;
        }
        case JE_TOK_BOOL: {
            ret = je_alloc_ast_node(context, node);
            if (ret < 0) {
                return ret;
            }
            (*node)->type = JE_NODE_INT_LITERAL;
            
            ret = je_set_ast_node_bool(context, *node, tok.bool_value);
            if (ret < 0) {
                return ret;
            }
            break;
        }
        default: {
            return je_store_error(context, JE_RESULT_UNKNOWN_TERM, tok.source_ptr, "Unexpected term");
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
    bool is_unary = false;
    if (precedence == 1) {
        const char* rewind_point = context->read_ptr;

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
                            is_unary = true;
                        }
                    }
                }
            }
        }

        context->read_ptr = rewind_point;
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
        const char* start_read_ptr = context->read_ptr;

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
                return je_store_error(context, JE_RESULT_UNEXPECTED_TOKEN, tok.source_ptr, "Unexpected token, expected bool, int, float or string keyword");
            }

            ret = je_expect_token(context, &tok, JE_TOK_OP_PARENTHESIS_CLOSE);
            if (ret < 0) {
                return ret;
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
        else if (tok.type == JE_TOK_OP_BITWISE_XOR && precedence == 7) {
            nodeType = JE_NODE_BITWISE_XOR;
        }
        // Precedence 7
        else if (tok.type == JE_TOK_OP_BITWISE_OR && precedence == 8) {
            nodeType = JE_NODE_BITWISE_OR;
        }
        // Precedence 8
        else if (tok.type == JE_TOK_OP_LOGICAL_AND && precedence == 9) {
            nodeType = JE_NODE_LOGICAL_AND;
        }
        // Precedence 9
        else if (tok.type == JE_TOK_OP_LOGICAL_OR && precedence == 10) {
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

        ret = je_alloc_node_children(context, *node, 2);
        if (ret < 0) {
            return ret;
        }

        (*node)->type = nodeType;
        je_set_node_child(context, *node, 0, lvalue);
        je_set_node_child(context, *node, 1, rvalue);

        lvalue = *node;
    }

    return JE_RESULT_SUCCESS;
}

// -----------------------------------------------------------------------
// SEMANTIC ANALYSIS
// -----------------------------------------------------------------------

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

int je_insert_cast(je_context_t* context, je_ast_node_t* node, je_ast_node_t* parent, int parent_child_index, int type) {
    int op_type = 0;
    switch (type) {
        case JE_TYPE_STRING: {
            switch (node->return_type) {
                case JE_TYPE_INT:       op_type = JE_NODE_CAST_INT_TO_STRING; break;
                case JE_TYPE_STRING:    return JE_RESULT_SUCCESS;
                case JE_TYPE_FLOAT:     op_type = JE_NODE_CAST_FLOAT_TO_STRING; break;
                case JE_TYPE_BOOL:      op_type = JE_NODE_CAST_BOOL_TO_STRING; break;
            }
            break;
        }
        case JE_TYPE_INT: {
            switch (node->return_type) {
                case JE_TYPE_INT:       return JE_RESULT_SUCCESS;
                case JE_TYPE_STRING:    op_type = JE_NODE_CAST_STRING_TO_INT; break;
                case JE_TYPE_FLOAT:     op_type = JE_NODE_CAST_FLOAT_TO_INT; break;
                case JE_TYPE_BOOL:      op_type = JE_NODE_CAST_BOOL_TO_INT; break;
            }
            break;
        }
        case JE_TYPE_FLOAT: {
            switch (node->return_type) {
                case JE_TYPE_INT:       op_type = JE_NODE_CAST_INT_TO_FLOAT; break;
                case JE_TYPE_STRING:    op_type = JE_NODE_CAST_STRING_TO_FLOAT; break;
                case JE_TYPE_FLOAT:     return JE_RESULT_SUCCESS;
                case JE_TYPE_BOOL:      op_type = JE_NODE_CAST_BOOL_TO_FLOAT; break;
            }
            break;
        }
        case JE_TYPE_BOOL: {
            switch (node->return_type) {
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

    ret = je_alloc_node_children(context, cast_node, 2);
    if (ret < 0) {
        return ret;
    }

    cast_node->type = op_type;
    cast_node->return_type = type;
    je_set_node_child(context, cast_node, 1, node); // casts are consider unary so we they go in the rvalue
    je_set_node_child(context, parent, parent_child_index, cast_node);

    return JE_RESULT_SUCCESS;
}

int je_implicit_conversion_child(je_context_t* context, je_ast_node_t* child, je_ast_node_t* parent, int parent_child_index, int to_type) {
    switch (to_type) {
        case JE_TYPE_STRING: {
            if (child->return_type != JE_TYPE_STRING) {
                return je_store_error(context, JE_RESULT_CANNOT_IMPLICITLY_CAST, NULL, "Cannot implicitly cast from '%s' to '%s'.", je_type_name(child->return_type), je_type_name(to_type));
            }
            break;
        }
        case JE_TYPE_INT: {
            if (child->return_type != JE_TYPE_INT) {
                if (child->return_type == JE_TYPE_FLOAT) {
                    int ret = je_insert_cast(context, child, parent, parent_child_index, JE_TYPE_INT);
                    if (ret < 0) {
                        return ret;
                    }
                } else {
                    return je_store_error(context, JE_RESULT_CANNOT_IMPLICITLY_CAST, NULL, "Cannot implicitly cast from '%s' to '%s'.", je_type_name(child->return_type), je_type_name(to_type));
                }
            }
            break;
        }
        case JE_TYPE_FLOAT: {
            if (child->return_type != JE_TYPE_FLOAT) {
                if (child->return_type == JE_TYPE_INT) {
                    int ret = je_insert_cast(context, child, parent, parent_child_index, JE_TYPE_FLOAT);
                    if (ret < 0) {
                        return ret;
                    }
                } else {
                    return je_store_error(context, JE_RESULT_CANNOT_IMPLICITLY_CAST, NULL, "Cannot implicitly cast from '%s' to '%s'.", je_type_name(child->return_type), je_type_name(to_type));
                }
            }
            break;
        }
        case JE_TYPE_BOOL: {
            if (child->return_type != JE_TYPE_BOOL) {
                int ret = je_insert_cast(context, child, parent, parent_child_index, JE_TYPE_BOOL);
                if (ret < 0) {
                    return ret;
                }
            }
            break;
        }
    }
    return JE_RESULT_SUCCESS;
}

int je_implicit_conversion(je_context_t* context, je_ast_node_t* node) {
    for (int i = 0; i < node->child_count; i++) {
        je_ast_node_t* child = je_get_node_child(context, node, i);
        if (child != NULL) {
            int target_type = node->return_type;
            if (node->type == JE_NODE_FUNCTION_CALL ||
                node->type == JE_NODE_FUNCTION_CALL_STRING ||
                node->type == JE_NODE_FUNCTION_CALL_BOOL ||
                node->type == JE_NODE_FUNCTION_CALL_INT ||
                node->type == JE_NODE_FUNCTION_CALL_FLOAT) {                
                target_type = je_get_func_param_type(context, je_get_ast_node_function(context, node), i);
            }
            je_implicit_conversion_child(context, child, node, i, target_type);
        }
    }
    return JE_RESULT_SUCCESS;
}

void je_print_ast(je_context_t* context, je_ast_node_t* node, int depth, int childIndex) {
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
        case JE_NODE_FUNCTION_CALL_BOOL:    
        case JE_NODE_FUNCTION_CALL_STRING:  
        case JE_NODE_FUNCTION_CALL_FLOAT:   
        case JE_NODE_FUNCTION_CALL_INT: {
            printf("%s (%u)", je_node_name(node->type), je_get_ast_node_function(context, node)->name_index);
            break;
        }
        case JE_NODE_VARIABLE_BOOL:         printf("%s (%i)", je_node_name(node->type), *je_get_variable_bool(je_get_ast_node_variable(context, node))); break;
        case JE_NODE_VARIABLE_STRING:       printf("%s (%s)", je_node_name(node->type), *je_get_variable_string(je_get_ast_node_variable(context, node))); break;
        case JE_NODE_VARIABLE_FLOAT:        printf("%s (%f)", je_node_name(node->type), *je_get_variable_float(je_get_ast_node_variable(context, node))); break;
        case JE_NODE_VARIABLE_INT:          printf("%s (%i)", je_node_name(node->type), *je_get_variable_int(je_get_ast_node_variable(context, node))); break;
        case JE_NODE_BOOL_LITERAL:          printf("%s (%i)", je_node_name(node->type), *je_get_ast_node_bool(context, node)); break;
        case JE_NODE_STRING_LITERAL:        printf("%s (%s)", je_node_name(node->type), *je_get_ast_node_string(context, node)); break;
        case JE_NODE_FLOAT_LITERAL:         printf("%s (%f)", je_node_name(node->type), *je_get_ast_node_float(context, node)); break;
        case JE_NODE_INT_LITERAL:           printf("%s (%i)", je_node_name(node->type), *je_get_ast_node_int(context, node)); break;
        default:                            printf("%s", je_node_name(node->type)); break;
    }
    printf("\n");
    for (int i = 0; i < node->child_count; i++) {
        je_ast_node_t* child = je_get_node_child(context, node, i);
        if (child != NULL) {
            je_print_ast(context, child, depth + 1, i);
        }
    }
}

int je_semant(je_context_t* context, je_ast_node_t* node, je_ast_node_t* parent, int parent_child_index) {
    int ret = 0;

    // Recurse through all the children to draw up the return type.
    for (int i = 0; i < node->child_count; i++) {
        je_ast_node_t* child = je_get_node_child(context, node, i);
        if (child != NULL) {
            ret = je_semant(context, child, node, i);
            if (ret < 0) {
                return ret;
            }
        }
    }

    // Find the return type of the node.
    bool needs_implicit_cast = true;
    switch (node->type) {
        case JE_NODE_VARIABLE: {
            int name_index = je_get_ast_node_name_index(context, node);
            je_variable_def_t* variable = je_find_variable(context, name_index);
            if (variable == NULL) {
                return je_store_error(context, JE_RESULT_UNDEFINED_IDENTIFIER, NULL, "Undefined identifier '%s'", je_get_name(context, name_index));
            }
            ret = je_set_ast_node_variable(context, node, variable);
            if (ret < 0) {
                return ret;
            }
            node->return_type = variable->type;
            switch (node->return_type) {
                case JE_TYPE_STRING:    node->type = JE_NODE_VARIABLE_STRING;   break;
                case JE_TYPE_FLOAT:     node->type = JE_NODE_VARIABLE_FLOAT;    break;
                case JE_TYPE_INT:       node->type = JE_NODE_VARIABLE_INT;      break;
                case JE_TYPE_BOOL:      node->type = JE_NODE_VARIABLE_BOOL;     break;
            }
            break;
        }
        case JE_NODE_FUNCTION_CALL: {
            int name_index = je_get_ast_node_name_index(context, node);
            je_func_def_t* function = je_find_function(context, name_index);
            if (function == NULL) {
                return je_store_error(context, JE_RESULT_UNDEFINED_IDENTIFIER, NULL, "Undefined identifier '%s'", je_get_name(context, name_index));
            }
            if (function->param_count != node->param_count) {
                return je_store_error(context, JE_RESULT_INCORRECT_PARAMETER_COUNT, NULL, "Incorrect number of parameters for function '%s'", je_get_name(context, name_index));
            }
            ret = je_set_ast_node_function(context, node, function);
            if (ret < 0) {
                return ret;
            }
            node->return_type = function->return_type;
            switch (node->return_type) {
                case JE_TYPE_STRING:    node->type = JE_NODE_FUNCTION_CALL_STRING;   break;
                case JE_TYPE_FLOAT:     node->type = JE_NODE_FUNCTION_CALL_FLOAT;    break;
                case JE_TYPE_INT:       node->type = JE_NODE_FUNCTION_CALL_INT;      break;
                case JE_TYPE_BOOL:      node->type = JE_NODE_FUNCTION_CALL_BOOL;     break;
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
            je_ast_node_t* child = je_get_node_child(context, node, 1);
            switch (child->return_type) {
                case JE_TYPE_STRING:    node->type = JE_NODE_CAST_STRING_TO_FLOAT; break;
                case JE_TYPE_FLOAT: {
                    // Replace node with lvalue as this is a noop.
                    je_set_node_child(context, parent, parent_child_index, je_get_node_child(context, node, 0));
                    break;
                }
                case JE_TYPE_INT:       node->type = JE_NODE_CAST_INT_TO_FLOAT; break;
                case JE_TYPE_BOOL:      node->type = JE_NODE_CAST_BOOL_TO_FLOAT; break;
            }
            node->return_type = JE_TYPE_FLOAT;
            needs_implicit_cast = false;
            break;
        }
        case JE_NODE_CAST_TO_INT: {
            je_ast_node_t* child = je_get_node_child(context, node, 1);
            switch (child->return_type) {
                case JE_TYPE_STRING:    node->type = JE_NODE_CAST_STRING_TO_INT; break;
                case JE_TYPE_FLOAT:     node->type = JE_NODE_CAST_FLOAT_TO_INT; break;
                case JE_TYPE_INT: {
                    // Replace node with lvalue as this is a noop.
                    je_set_node_child(context, parent, parent_child_index, je_get_node_child(context, node, 0));
                    break;
                }
                case JE_TYPE_BOOL:      node->type = JE_NODE_CAST_BOOL_TO_INT; break;
            }
            node->return_type = JE_TYPE_INT;
            needs_implicit_cast = false;
            break;
        }
        case JE_NODE_CAST_TO_STRING: {
            je_ast_node_t* child = je_get_node_child(context, node, 1);
            switch (child->return_type) {
                case JE_TYPE_STRING: {
                    // Replace node with lvalue as this is a noop.
                    je_set_node_child(context, parent, parent_child_index, je_get_node_child(context, node, 0));
                    break;
                }
                case JE_TYPE_FLOAT:     node->type = JE_NODE_CAST_FLOAT_TO_STRING; break;
                case JE_TYPE_INT:       node->type = JE_NODE_CAST_INT_TO_STRING; break;
                case JE_TYPE_BOOL:      node->type = JE_NODE_CAST_BOOL_TO_STRING; break;
            }
            node->return_type = JE_TYPE_STRING;
            needs_implicit_cast = false;
            break;
        }
        case JE_NODE_CAST_TO_BOOL: {
            je_ast_node_t* child = je_get_node_child(context, node, 1);
            switch (child->return_type) {
                case JE_TYPE_STRING:    node->type = JE_NODE_CAST_STRING_TO_BOOL; break;
                case JE_TYPE_FLOAT:     node->type = JE_NODE_CAST_FLOAT_TO_BOOL; break;
                case JE_TYPE_INT:       node->type = JE_NODE_CAST_INT_TO_BOOL; break;
                case JE_TYPE_BOOL: {
                    // Replace node with lvalue as this is a noop.
                    je_set_node_child(context, parent, parent_child_index, je_get_node_child(context, node, 0));
                    break;
                }
            }
            node->return_type = JE_TYPE_BOOL;
            needs_implicit_cast = false;
            break;
        }  
        case JE_NODE_LOGICAL_AND:
        case JE_NODE_LOGICAL_OR:
        case JE_NODE_LOGICAL_NOT: {
            node->return_type = JE_TYPE_BOOL;
            break;
        }      
        default: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            node->return_type = je_type_balance(lvalue, rvalue);
            break;
        }
    }
    
    // Replaces certain notes with intrinsics.
    const char* intrinsic_function_name = "";
    switch (node->type) {
        case JE_NODE_CAST_BOOL_TO_STRING:   intrinsic_function_name = "__bool_to_string";   break;
        case JE_NODE_CAST_INT_TO_STRING:    intrinsic_function_name = "__int_to_string";    break;
        case JE_NODE_CAST_FLOAT_TO_STRING:  intrinsic_function_name = "__float_to_string";  break;
        case JE_NODE_CAST_STRING_TO_BOOL:   intrinsic_function_name = "__string_to_bool";   break;
        case JE_NODE_CAST_STRING_TO_FLOAT:  intrinsic_function_name = "__string_to_float";  break;
        case JE_NODE_CAST_STRING_TO_INT:    intrinsic_function_name = "__string_to_int";    break;
        case JE_NODE_ADD: {
            if (node->return_type == JE_TYPE_STRING) {
                intrinsic_function_name = "__string_concat";
            }
            break;
        }
        case JE_NODE_EQUAL: {
            if (node->return_type == JE_TYPE_STRING) {
                intrinsic_function_name = "__string_equal";
            }
            break;
        }
        case JE_NODE_NOT_EQUAL: {
            if (node->return_type == JE_TYPE_STRING) {
                intrinsic_function_name = "__string_not_equal";
            }
            break;
        }
    }
    if (intrinsic_function_name[0] != '\0') {
        uint16_t index;
        int ret = je_get_name_index(context, intrinsic_function_name, &index);
        if (ret < 0) {
            return ret;
        }

        je_func_def_t* func = je_find_function(context, index);
        assert(func);

        switch (func->return_type) {
            case JE_TYPE_STRING:    node->type = JE_NODE_FUNCTION_CALL_STRING;  break;
            case JE_TYPE_INT:       node->type = JE_NODE_FUNCTION_CALL_INT;     break;
            case JE_TYPE_FLOAT:     node->type = JE_NODE_FUNCTION_CALL_FLOAT;   break;
            case JE_TYPE_BOOL:      node->type = JE_NODE_FUNCTION_CALL_BOOL;    break;
        }

        ret = je_set_ast_node_function(context, node, func);
        if (ret < 0) {
            return ret;
        }

        node->param_count = func->param_count;

        // Casts only have an rvalue so shift that to the first index for our intrinsic parameter.
        je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
        if (lvalue == NULL) {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            je_set_node_child(context, node, 0, rvalue);
            je_set_node_child(context, node, 1, NULL);
        }
    }

    // Add implicit conversion nodes if required.
    if (needs_implicit_cast) {
        ret = je_implicit_conversion(context, node);
        if (ret < 0) {
            return ret;
        }
    }

    // Check the operations are valid and convert to type specific operations.
    switch (node->type) {
        case JE_NODE_POS: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_POS_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_POS_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "unary + can only be used with float or int types");
            }
            break;
        }
        case JE_NODE_NEG: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_NEG_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_NEG_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "unary - can only be used with float or int types");
            }
            break;
        }
        case JE_NODE_SUB: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_SUB_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_SUB_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "- can only be used with float or int types");
            }
            break;
        }
        case JE_NODE_MUL: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_MUL_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_MUL_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "* can only be used with float or int types");
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
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "+ can only be used with float, int or string types");
            }
            break;
        }
        case JE_NODE_DIV: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_DIV_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_DIV_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "/ can only be used with float or int types");
            }
            break;
        }
        case JE_NODE_MOD: {
            if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_MOD_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "% can only be used with int types");
            }
            break;
        }
        case JE_NODE_BITWISE_NOT: {
            if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_BITWISE_NOT_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "~ can only be used with int types");
            }
            break;
        }
        case JE_NODE_LESS: {
            if (node->return_type == JE_TYPE_FLOAT) {
                node->type = JE_NODE_LESS_FLOAT;
            } else if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_LESS_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "< can only be used with bool types");
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
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, ">= can only be used with bool types");
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
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, ">= can only be used with bool types");
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
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "<= can only be used with bool types");
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
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "!= can only be used with bool types");
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
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "!= can only be used with bool types");
            }
            node->return_type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_BITWISE_AND: {
            if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_BITWISE_AND_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "& can only be used with int types");
            }
            break;
        }
        case JE_NODE_BITWISE_OR: {
            if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_BITWISE_OR_INT;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "| can only be used with int types");
            }
            break;
        }
        case JE_NODE_BITWISE_XOR: {
            if (node->return_type == JE_TYPE_INT) {
                node->type = JE_NODE_BITWISE_XOR_INT;
            }
            else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "^ can only be used with int types");
            }
            break;
        }
        case JE_NODE_LOGICAL_NOT: {
            if (node->return_type == JE_TYPE_BOOL) {
                node->type = JE_NODE_LOGICAL_NOT_BOOL;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "! can only be used with bool types");
            }
            break;
        }
        case JE_NODE_LOGICAL_AND: {
            if (node->return_type == JE_TYPE_BOOL) {
                node->type = JE_NODE_LOGICAL_AND_BOOL;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "&& can only be used with int types");
            }
            break;
        }
        case JE_NODE_LOGICAL_OR: {
            if (node->return_type == JE_TYPE_BOOL) {
                node->type = JE_NODE_LOGICAL_OR_BOOL;
            } else {
                return je_store_error(context, JE_RESULT_INCOMPATIBLE_TYPES, NULL, "|| can only be used with int types");
            }
            break;
        }
        case JE_NODE_FUNCTION_CALL_STRING: {
            node->return_type = JE_TYPE_STRING;
            break;
        }
        case JE_NODE_FUNCTION_CALL_INT: {
            node->return_type = JE_TYPE_INT;
            break;
        }
        case JE_NODE_FUNCTION_CALL_FLOAT: {
            node->return_type = JE_TYPE_FLOAT;
            break;
        }
        case JE_NODE_FUNCTION_CALL_BOOL: {
            node->return_type = JE_TYPE_BOOL;
            break;
        }
    }
    
    return JE_RESULT_SUCCESS;
}

// -----------------------------------------------------------------------
// OPTIMIZATION
// -----------------------------------------------------------------------

void je_mark_nodes_constant(je_context_t* context, je_ast_node_t* node) {
    node->is_constant = true;

    for (int i = 0; i < node->child_count; i++) {
        je_ast_node_t* child = je_get_node_child(context, node, i);
        if (child != NULL) {
            je_mark_nodes_constant(context, child);
            if (!child->is_constant) {
                node->is_constant = 0;
            }
        }
    }

    // Values bound by the user are not const (despite being called constants ...)
    switch (node->type) {
        case JE_NODE_VARIABLE_BOOL:         
        case JE_NODE_VARIABLE_INT:          
        case JE_NODE_VARIABLE_FLOAT:        
        case JE_NODE_VARIABLE_STRING: {
            node->is_constant = je_get_ast_node_variable(context, node)->is_constant;
            break;
        }
        case JE_NODE_FUNCTION_CALL_BOOL:         
        case JE_NODE_FUNCTION_CALL_INT:          
        case JE_NODE_FUNCTION_CALL_FLOAT:        
        case JE_NODE_FUNCTION_CALL_STRING: {
            node->is_constant = je_get_ast_node_function(context, node)->is_deterministic;
            break;
        }
    }
}

int je_fold_node_constants(je_context_t* context, je_ast_node_t* node) {
    if (node->is_constant) {
        // Evaluate result and replace node with a variable value.
        context->transient_mem_arena_offset = 0;

        je_value_t value;
        int ret = je_eval_slow(context, node, &value);
        if (ret == JE_RESULT_SUCCESS) {
            for (int i = 0; i < node->child_count; i++) {
                je_set_node_child(context, node, i, NULL);
            }
            node->child_count = 0;
            int ret = 0;
            switch (value.type) {
                case JE_TYPE_BOOL: {
                    node->type = JE_NODE_BOOL_LITERAL;   
                    ret = je_set_ast_node_bool(context, node, value.bool_value);
                    break;
                }
                case JE_TYPE_FLOAT: {
                    node->type = JE_NODE_FLOAT_LITERAL;
                    ret = je_set_ast_node_float(context, node, value.float_value);
                    break;
                }
                case JE_TYPE_STRING: {
                    node->type = JE_NODE_STRING_LITERAL;
                    ret = je_set_ast_node_string(context, node, value.string_value);
                    break;
                }
                case JE_TYPE_INT: {
                    node->type = JE_NODE_INT_LITERAL;
                    ret = je_set_ast_node_int(context, node, value.int_value);
                    break;
                }
            }
            if (ret < 0) {
                return ret;
            }
            return JE_RESULT_SUCCESS;
        }
    }

    for (int i = 0; i < node->child_count; i++) {
        je_ast_node_t* child = je_get_node_child(context, node, i);
        if (child != NULL) {
            int ret = je_fold_node_constants(context, child);
            if (ret < 0) {
                return ret;
            }
        }
    }

    return JE_RESULT_SUCCESS;
}

int je_fold_constants(je_context_t* context) {
    je_mark_nodes_constant(context, context->ast_root);    
    int ret = je_fold_node_constants(context, context->ast_root);
    if (ret < 0) {
        return ret;
    }
    return JE_RESULT_SUCCESS;
}

// -----------------------------------------------------------------------
// EVALUATION
// -----------------------------------------------------------------------

int je_eval_slow(je_context_t* context, je_ast_node_t* node, je_value_t* result) {
    je_value_t values[JE_MAX_PARAMETERS];
    int ret;

    for (int i = 0; i < node->child_count; i++) {
        je_ast_node_t* child = je_get_node_child(context, node, i);
        if (child != NULL) {
            ret = je_eval_slow(context, child, &values[i]);
            if (ret < 0) {
                return ret;
            }
        }
    }

    switch (node->type) {
        case JE_NODE_LOGICAL_NOT_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = !values[1].bool_value;
            break;
        }
        case JE_NODE_BITWISE_NOT_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = ~values[1].int_value;
            break;
        }
        case JE_NODE_MUL_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = values[0].float_value * values[1].float_value;
            break;
        }
        case JE_NODE_MUL_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = values[0].int_value * values[1].int_value;
            break;
        }
        case JE_NODE_DIV_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = values[0].float_value / values[1].float_value;
            break;
        }
        case JE_NODE_DIV_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = values[0].int_value / values[1].int_value;
            break;
        }
        case JE_NODE_MOD_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = values[0].int_value % values[1].int_value;
            break;
        }
        case JE_NODE_SUB_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = values[0].float_value - values[1].float_value;
            break;
        }
        case JE_NODE_SUB_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = values[0].int_value - values[1].int_value;
            break;
        }
        case JE_NODE_ADD_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = values[0].float_value + values[1].float_value;
            break;
        }
        case JE_NODE_ADD_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = values[0].int_value + values[1].int_value;
            break;
        }
        case JE_NODE_LESS_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].float_value < values[1].float_value);
            break;
        }
        case JE_NODE_LESS_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].int_value < values[1].int_value);
            break;
        }
        case JE_NODE_GREATER_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].float_value > values[1].float_value);
            break;
        }
        case JE_NODE_GREATER_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].int_value > values[1].int_value);
            break;
        }
        case JE_NODE_LE_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].float_value <= values[1].float_value);
            break;
        }
        case JE_NODE_LE_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].int_value <= values[1].int_value);
            break;
        }
        case JE_NODE_GE_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].float_value >= values[1].float_value);
            break;
        }
        case JE_NODE_GE_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].int_value >= values[1].int_value);
            break;
        }
        case JE_NODE_EQUAL_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].bool_value == values[1].bool_value);
            break;
        }
        case JE_NODE_EQUAL_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].int_value == values[1].int_value);
            break;
        }
        case JE_NODE_EQUAL_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].float_value == values[1].float_value);
            break;
        }
        case JE_NODE_NOT_EQUAL_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].bool_value != values[1].bool_value);
            break;
        }
        case JE_NODE_NOT_EQUAL_INT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].int_value != values[1].int_value);
            break;
        }
        case JE_NODE_NOT_EQUAL_FLOAT: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = (values[0].float_value != values[1].float_value);
            break;
        }
        case JE_NODE_BITWISE_AND_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = values[0].int_value & values[1].int_value;
            break;
        }
        case JE_NODE_BITWISE_OR_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = values[0].int_value | values[1].int_value;
            break;
        }
        case JE_NODE_BITWISE_XOR_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = values[0].int_value ^ values[1].int_value;
            break;
        }
        case JE_NODE_LOGICAL_AND_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = values[0].bool_value && values[1].bool_value;
            break;
        }
        case JE_NODE_LOGICAL_OR_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = values[0].bool_value || values[1].bool_value;
            break;
        }
        case JE_NODE_VARIABLE_BOOL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = *je_get_variable_bool(je_get_ast_node_variable(context, node));
            break;
        }
        case JE_NODE_VARIABLE_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = *je_get_variable_int(je_get_ast_node_variable(context, node));
            break;
        }
        case JE_NODE_VARIABLE_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = *je_get_variable_float(je_get_ast_node_variable(context, node));
            break;
        }
        case JE_NODE_VARIABLE_STRING: {
            result->type = JE_TYPE_STRING;
            result->string_value = *je_get_variable_string(je_get_ast_node_variable(context, node));
            break;
        }
        case JE_NODE_NEG_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = -values[1].float_value;
            break;
        }
        case JE_NODE_POS_FLOAT: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = +values[1].float_value;
            break;
        }
        case JE_NODE_NEG_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = -values[1].int_value;
            break;
        }
        case JE_NODE_POS_INT: {
            result->type = JE_TYPE_INT;
            result->int_value = +values[1].int_value;
            break;
        }
        case JE_NODE_FLOAT_LITERAL: {
            result->type = JE_TYPE_FLOAT;
            result->float_value = *je_get_ast_node_float(context, node);
            break;
        }
        case JE_NODE_INT_LITERAL: {
            result->type = JE_TYPE_INT;
            result->int_value = *je_get_ast_node_int(context, node);
            break;
        }
        case JE_NODE_STRING_LITERAL: {
            result->type = JE_TYPE_STRING;
            result->string_value = *je_get_ast_node_string(context, node);
            break;
        }
        case JE_NODE_BOOL_LITERAL: {
            result->type = JE_TYPE_BOOL;
            result->bool_value = *je_get_ast_node_bool(context, node);
            break;
        }
        case JE_NODE_CAST_FLOAT_TO_INT: {
            result->int_value = (int)values[1].float_value;
            result->type = JE_TYPE_INT;
            break;
        }
        case JE_NODE_CAST_BOOL_TO_INT: {
            result->int_value = (values[1].bool_value != 0 ? 1 : 0);
            result->type = JE_TYPE_INT;
            break;
        }
        case JE_NODE_CAST_INT_TO_FLOAT: {
            result->float_value = (float)values[1].int_value;
            result->type = JE_TYPE_FLOAT;
            break;
        }
        case JE_NODE_CAST_BOOL_TO_FLOAT: {
            result->float_value = (values[1].bool_value != 0 ? 1.0f : 0.0f);
            result->type = JE_TYPE_FLOAT;
            break;
        }
        case JE_NODE_CAST_INT_TO_BOOL: {
            result->bool_value = (values[1].int_value != 0 ? 1 : 0);
            result->type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_CAST_FLOAT_TO_BOOL: {
            result->bool_value = (values[1].float_value != 0.0f ? 1 : 0);
            result->type = JE_TYPE_BOOL;
            break;
        }
        case JE_NODE_FUNCTION_CALL_STRING:
        case JE_NODE_FUNCTION_CALL_FLOAT:
        case JE_NODE_FUNCTION_CALL_INT: 
        case JE_NODE_FUNCTION_CALL_BOOL: {
            context->active_function = je_get_ast_node_function(context, node);
            context->function_result.type = JE_TYPE_UNSET;
            for (int i = 0; i < JE_MAX_PARAMETERS; i++) {
                context->function_params[i] = values[i];
            }

            context->active_function->function(context);

            if (context->function_result.type != context->active_function->return_type) {
                return je_store_error(context, JE_RESULT_INCORRECT_FUNC_RETURN_TYPE, NULL, "Call to function returned incorrect type '%s' expecting '%s'.", 
                    je_type_name(context->function_result.type), je_type_name(context->active_function->return_type));
            }
            
            context->active_function = NULL;

            *result = context->function_result;
            break;
        }
        default: {
            assert(0);
            return JE_RESULT_FAILED;
        }
    }
    return JE_RESULT_SUCCESS;
}

#ifdef JE_JIT_AVAILABLE
int je_eval_jit(je_context_t* context, je_ast_node_t* node, je_value_t* result) {
#ifndef JE_DEBUG_FAKE_JIT_AVAILABLE
    je_jit_func_t func = (je_jit_func_t)context->jit_executable_memory;
    func();
#endif
    return JE_RESULT_SUCCESS;
}
#endif

int je_eval(je_context_t* context) {    
    context->transient_mem_arena_offset = 0;

#ifdef JE_JIT_AVAILABLE
    if (context->jit_compiled) {
        int ret = je_eval_jit(context, context->ast_root, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    else 
#endif
    {
        int ret = je_eval_slow(context, context->ast_root, &context->result);
        if (ret < 0) {
            return ret;
        }
    }
    return JE_RESULT_SUCCESS;
}

// -----------------------------------------------------------------------
// COMPILING
// -----------------------------------------------------------------------

#ifdef JE_JIT_AVAILABLE
int je_compile_jit(je_context_t* context);
#endif // JE_JIT_AVAILABLE

int je_compile(je_context_t* context, const char* source) {
    if (context->compiled) {
        return JE_RESULT_CANNOT_COMPILE_MULTIPLE_TIMES;
    }
    context->compiled = true;
#ifdef JE_JIT_AVAILABLE
    context->jit_compiled = false;
#endif
    context->source = source;
    context->read_ptr = source;

    if (context->flags & JE_FLAG_DEBUG_MEM_LOGGING) {
        printf("je_context_t = %i\n", (int)sizeof(je_context_t));
        printf("je_name_t = %i\n", (int)sizeof(je_name_t));
        printf("je_func_def_t = %i\n", (int)sizeof(je_func_def_t));
        printf("je_variable_def_t = %i\n", (int)sizeof(je_variable_def_t));
        printf("je_value_t = %i\n", (int)sizeof(je_value_t));
        printf("je_ast_node_t = %i\n", (int)sizeof(je_ast_node_t));
        printf("Pre-Compile Size: %i\n", (int)context->mem_arena_offset);
    }

    // Parse the first expression.
    int ret = je_parse(context, &context->ast_root, JE_MAX_OPERATOR_PRECEDENCE);
    if (ret < 0) {
        return ret;
    }

    // Make sure we actually parsed something valid.
    if (context->ast_root == NULL) {
        return je_store_error(context, JE_RESULT_EMPTY_EXPRESSION, NULL, NULL);
    }
    if (context->read_ptr[0] != '\0') {
        return je_store_error(context, JE_RESULT_UNEXPECTED_TRAILING_EXPRESSION, context->read_ptr, "Unexpected trailing expression");
    }

    if (context->flags & JE_FLAG_DEBUG_LOGGING) {
        printf("==== AST After Parse ===\n");
        je_print_ast(context, context->ast_root, 0, 0);
    }

    // Semantically analyze the ast to make sure its valid and insert implicit conversions/etc where required.
    ret = je_semant(context, context->ast_root, NULL, 0);
    if (ret < 0) {
        return ret;
    }

    if (context->flags & JE_FLAG_DEBUG_LOGGING) {
        printf("==== AST after Semantic Analysis ===\n");
        je_print_ast(context, context->ast_root, 0, 0);
    }

    // Fold any constant operations for simple optimization.
    if ((context->flags & JE_FLAG_NO_OPTIMIZATION) == 0) {
        ret = je_fold_constants(context);
        if (ret < 0) {
            return ret;
        }

        if (context->flags & JE_FLAG_DEBUG_LOGGING) {
            printf("==== AST after Constant Folding ===\n");
            je_print_ast(context, context->ast_root, 0, 0);
        }
    }

#ifdef JE_JIT_AVAILABLE

    if ((context->flags & JE_FLAG_NO_JIT) == 0) {
        ret = je_compile_jit(context);
        if (ret < 0) {
            return ret;
        }

        if (context->flags & JE_FLAG_DEBUG_LOGGING) {
            printf("==== JIT Metrics ===\n");
            printf("Instructions: %i\n", context->jit_instruction_num);
            printf("Code Bytes: %i\n", context->jit_code_bytes);
        }
    }

#endif

    return JE_RESULT_SUCCESS;
}

#ifdef JE_JIT_AVAILABLE

// -----------------------------------------------------------------------
// PLATFORM SPECIFIC CODE
// -----------------------------------------------------------------------

#ifdef JE_JIT_AVAILABLE
#ifdef JE_PLATFORM_WINDOWS
int je_alloc_executable(je_context_t* context, const char* code, int code_size, char** output, int* allocated_size) {
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);

    int pageSize = system_info.dwPageSize;
    int memorySize = ((code_size + (pageSize - 1)) / pageSize) * pageSize;
    DWORD dummy;

    *output = (char*)VirtualAlloc(NULL, memorySize, MEM_COMMIT, PAGE_READWRITE);
    memcpy(*output, code, code_size);
    VirtualProtect(*output, memorySize, PAGE_EXECUTE_READ, &dummy);

    if (allocated_size != NULL) {
        *allocated_size = memorySize;
    }

    return *output != NULL ? JE_RESULT_SUCCESS : JE_RESULT_FAILED;
}

int je_free_executable(je_context_t* context, void* memory, int code_size) {
    if (!VirtualFree(memory, 0, MEM_RELEASE)) {
        return JE_RESULT_FAILED;
    }
    return JE_RESULT_SUCCESS;
}
#elif defined(JE_PLATFORM_LINUX)
int je_alloc_executable(je_context_t* context, const char* code, int code_size, char** output, int* allocated_size) {
    *output = mmap(0, code_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (*output == (char*)-1) {
        return JE_RESULT_FAILED;
    }
    memcpy(*output, code, code_size);
    if (mprotect(*output, code_size, PROT_READ | PROT_EXEC) == -1) {
        return JE_RESULT_FAILED;
    }
    if (allocated_size != NULL) {
        *allocated_size = code_size;
    }
    return JE_RESULT_SUCCESS;
}

int je_free_executable(je_context_t* context, void* memory, int code_size) {
    if (munmap(memory, code_size) != 0) {
        return JE_RESULT_FAILED;
    }
    return JE_RESULT_SUCCESS;
}
#endif
#endif // JE_JIT_AVAILABLE

// -----------------------------------------------------------------------
// ISA Agnostic Utility Functions
// -----------------------------------------------------------------------

void je_jit_emit_comment(je_context_t* context, const char* format, ...) {
    if (context->flags & JE_FLAG_DEBUG_JIT_DISASSEMBLY) {
        char buffer[128];
        va_list va;
        va_start(va, format);
        vsnprintf(buffer, sizeof(buffer), format, va);
        va_end(va);

        printf("\n\t// %s\n", buffer);
    }
}

void je_jit_emit_bytes_1(je_context_t* context, uint8_t byte1) {
    int jit_length = (int)(context->jit_write_ptr - context->jit_write_buffer);
    int remaining_space = context->jit_write_buffer_len - jit_length;
    if (remaining_space < 1) {
        context->jit_write_buffer_overflow = true;
        return;
    }
    *(context->jit_write_ptr++) = byte1;
}

void je_jit_emit_bytes_2(je_context_t* context, uint8_t byte1, uint8_t byte2) {
    int jit_length = (int)(context->jit_write_ptr - context->jit_write_buffer);
    int remaining_space = context->jit_write_buffer_len - jit_length;
    if (remaining_space < 2) {
        context->jit_write_buffer_overflow = true;
        return;
    }
    *(context->jit_write_ptr++) = byte1;
    *(context->jit_write_ptr++) = byte2;
}

void je_jit_emit_bytes_3(je_context_t* context, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
    int jit_length = (int)(context->jit_write_ptr - context->jit_write_buffer);
    int remaining_space = context->jit_write_buffer_len - jit_length;
    if (remaining_space < 3) {
        context->jit_write_buffer_overflow = true;
        return;
    }
    *(context->jit_write_ptr++) = byte1;
    *(context->jit_write_ptr++) = byte2;
    *(context->jit_write_ptr++) = byte3;
}

void je_jit_emit_bytes_4(je_context_t* context, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4) {
    int jit_length = (int)(context->jit_write_ptr - context->jit_write_buffer);
    int remaining_space = context->jit_write_buffer_len - jit_length;
    if (remaining_space < 4) {
        context->jit_write_buffer_overflow = true;
        return;
    }
    *(context->jit_write_ptr++) = byte1;
    *(context->jit_write_ptr++) = byte2;
    *(context->jit_write_ptr++) = byte3;
    *(context->jit_write_ptr++) = byte4;
}

void je_jit_emit_bytes_8(je_context_t* context, uint8_t byte1, uint8_t byte2, uint8_t byte3, uint8_t byte4, uint8_t byte5, uint8_t byte6, uint8_t byte7, uint8_t byte8) {
    int jit_length = (int)(context->jit_write_ptr - context->jit_write_buffer);
    int remaining_space = context->jit_write_buffer_len - jit_length;
    if (remaining_space < 8) {
        context->jit_write_buffer_overflow = true;
        return;
    }
    *(context->jit_write_ptr++) = byte1;
    *(context->jit_write_ptr++) = byte2;
    *(context->jit_write_ptr++) = byte3;
    *(context->jit_write_ptr++) = byte4;
    *(context->jit_write_ptr++) = byte5;
    *(context->jit_write_ptr++) = byte6;
    *(context->jit_write_ptr++) = byte7;
    *(context->jit_write_ptr++) = byte8;
}

void je_jit_emit_bytes(je_context_t* context, uint8_t* bytes, uint32_t length) {
    int jit_length = (int)(context->jit_write_ptr - context->jit_write_buffer);
    int remaining_space = context->jit_write_buffer_len - jit_length;
    if (remaining_space < (int)length) {
        context->jit_write_buffer_overflow = true;
        return;
    }
    for (int i = 0; i < (int)length; i++) {
        *(context->jit_write_ptr++) = bytes[i];
    }
}

void je_jit_start_instruction(je_context_t* context) {
    context->jit_instruction_start = context->jit_write_ptr;
}

void je_jit_end_instruction(je_context_t* context, const char* mnemonic, ...) {
    if (context->flags & JE_FLAG_DEBUG_JIT_DISASSEMBLY) {
        int instruction_length = (int)(context->jit_write_ptr - context->jit_instruction_start);

        char buffer[128];
        va_list va;
        va_start(va, mnemonic);
        vsnprintf(buffer, sizeof(buffer), mnemonic, va);
        va_end(va);

        printf("\t");
        for (int i = 0; i < 12; i++) {
            if (i >= instruction_length) { 
                printf("   ");
            } else {
                printf("%s%02x", i == 0 ? "" : " ", *(uint8_t*)(context->jit_instruction_start + i));
            }
        }
        printf("%s\n", buffer);
    }

    context->jit_instruction_num++;
}

// -----------------------------------------------------------------------
// X86 / X64 JIT
// -----------------------------------------------------------------------

#if defined(JE_ISA_X64) || defined(JE_ISA_X86)

#define JE_JIT_X86_REG_EAX                                      (0)
#define JE_JIT_X86_REG_ECX                                      (1)
#define JE_JIT_X86_REG_EDX                                      (2)
#define JE_JIT_X86_REG_EBX                                      (3)
#define JE_JIT_X86_REG_ESP                                      (4)
#define JE_JIT_X86_REG_EBP                                      (5)
#define JE_JIT_X86_REG_ESI                                      (6)
#define JE_JIT_X86_REG_EDI                                      (7)

// These indexes are offset by 10 for easier management, offset needs
// subtracting when encoding register index.
#define JE_JIT_X86_REG_XMM0                                     (10)
#define JE_JIT_X86_REG_XMM1                                     (11)
#define JE_JIT_X86_REG_XMM2                                     (12)
#define JE_JIT_X86_REG_XMM3                                     (13)
#define JE_JIT_X86_REG_XMM4                                     (14)
#define JE_JIT_X86_REG_XMM5                                     (15)
#define JE_JIT_X86_REG_XMM6                                     (16)
#define JE_JIT_X86_REG_XMM7                                     (17)

#define JE_JIT_X86_MEMORY_ADDRESSING_NO_DISPLACEMENT            (0x0)
#define JE_JIT_X86_MEMORY_ADDRESSING_WITH_8BIT_DISPLACEMENT     (0x1)
#define JE_JIT_X86_MEMORY_ADDRESSING_WITH_16BIT_DISPLACEMENT    (0x2)
#define JE_JIT_X86_REGISTER_DIRECT_ADDRESSING                   (0x3)

// Some good references if unfamiliar with x86 instruction encoding.
// http://ref.x86asm.net/coder64-abc.html
// https://pyokagan.name/blog/2019-09-20-x86encoding/

typedef struct je_jit_x86_modrm_t {
    uint8_t rm : 3;
    uint8_t reg : 3;
    uint8_t mod : 2;
} je_jit_x86_modrm_t;

typedef struct je_jit_x86_rex_t {
    uint8_t b : 1;
    uint8_t x : 1;
    uint8_t r : 1;
    uint8_t w : 1;
    uint8_t prefix : 4;
} je_jit_x86_rex_t;

void je_jit_x86_emit_prologue(je_context_t* context);
void je_jit_x86_emit_epilogue(je_context_t* context, int result_reg);
int je_jit_x86_emit_node(je_context_t* context, je_ast_node_t* node);
int je_jit_x86_alloc_alu_reg(je_context_t* context);
int je_jit_x86_alloc_xmm_reg(je_context_t* context);
void je_jit_x86_free_reg(je_context_t* context, int reg);

const char* je_jit_x86_reg_name(int reg, int width) {
#ifdef JE_ISA_X86
    if (width == 64) {
        width = 32;
    }
#endif
    switch (reg) {        
        case JE_JIT_X86_REG_EAX: {
            if (width == 8)         return "al";
            else if (width == 16)   return "ax";
            else if (width == 32)   return "eax";
            else if (width == 64)   return "rax";
        }
        case JE_JIT_X86_REG_ECX: {
            if (width == 8)         return "cl";
            else if (width == 16)   return "cx";
            else if (width == 32)   return "ecx";
            else if (width == 64)   return "rcx";
        }
        case JE_JIT_X86_REG_EDX: {
            if (width == 8)         return "dl";
            else if (width == 16)   return "dx";
            else if (width == 32)   return "edx";
            else if (width == 64)   return "rdx";
        }
        case JE_JIT_X86_REG_EBX: {
            if (width == 8)         return "bl";
            else if (width == 16)   return "bx";
            else if (width == 32)   return "ebx";
            else if (width == 64)   return "rbx";
        }
        case JE_JIT_X86_REG_ESP: {
            if (width == 8)         return "spl";
            else if (width == 16)   return "sp";
            else if (width == 32)   return "esp";
            else if (width == 64)   return "rsp";
        }
        case JE_JIT_X86_REG_EBP: {
            if (width == 8)         return "bpl";
            else if (width == 16)   return "bp";
            else if (width == 32)   return "ebp";
            else if (width == 64)   return "rbp";
        }
        case JE_JIT_X86_REG_ESI: {
            if (width == 8)         return "sil";
            else if (width == 16)   return "si";
            else if (width == 32)   return "esi";
            else if (width == 64)   return "rsi";
        }
        case JE_JIT_X86_REG_EDI: {
            if (width == 8)         return "dil";
            else if (width == 16)   return "di";
            else if (width == 32)   return "edi";
            else if (width == 64)   return "rdi";
        }
        case JE_JIT_X86_REG_XMM0: return "xmm0";
        case JE_JIT_X86_REG_XMM1: return "xmm1";
        case JE_JIT_X86_REG_XMM2: return "xmm2";
        case JE_JIT_X86_REG_XMM3: return "xmm3";
        case JE_JIT_X86_REG_XMM4: return "xmm4";
        case JE_JIT_X86_REG_XMM5: return "xmm5";
        case JE_JIT_X86_REG_XMM6: return "xmm6";
        case JE_JIT_X86_REG_XMM7: return "xmm7";
    }
    return "unknown-register";
}

int je_compile_jit(je_context_t* context) {
    int ret;
    
    // Use remaining space in the mem arena for jit compiling.
    context->jit_write_buffer = context->mem_arena + context->mem_arena_offset;
    context->jit_write_buffer_len = (int)(JE_MEM_ARENA_SIZE - context->mem_arena_offset);
    context->jit_write_ptr = context->jit_write_buffer;
    context->jit_write_buffer_overflow = false;
    context->jit_instruction_num = 0;
    context->jit_code_bytes = 0;

    context->mem_arena_frozen = true;
    je_jit_x86_emit_prologue(context);
    int return_reg = je_jit_x86_emit_node(context, context->ast_root);
    je_jit_x86_emit_epilogue(context, return_reg);
    context->mem_arena_frozen = false;

    // Ran out of space while trying to emit all JIT code.
    if (context->jit_write_buffer_overflow) {
        return JE_RESULT_OOM;
    }

    // Emplace the compiled code into executable memory.
    int jit_length = (int)(context->jit_write_ptr - context->jit_write_buffer);
    ret = je_alloc_executable(context, context->jit_write_buffer, jit_length, &context->jit_executable_memory, &context->jit_executable_memory_length);
    if (ret < 0) {
        return ret;
    }

    context->jit_code_bytes = jit_length;
    context->jit_compiled = true;
    return JE_RESULT_SUCCESS;
}

void je_jit_free(je_context_t* context) {
    je_free_executable(context, context->jit_executable_memory, context->jit_code_bytes);
}

void je_jit_x86_emit_imm8(je_context_t* context, uint8_t immediate) {
    uint8_t* bytes = (uint8_t*)&immediate;
    je_jit_emit_bytes_1(context, bytes[0]);
}

void je_jit_x86_emit_imm32(je_context_t* context, int immediate) {
    uint8_t* bytes = (uint8_t*)&immediate;
    je_jit_emit_bytes_4(context, bytes[0], bytes[1], bytes[2], bytes[3]);
}

void je_jit_x86_emit_imm64(je_context_t* context, uint64_t immediate) {
    uint8_t* bytes = (uint8_t*)&immediate;
    je_jit_emit_bytes_8(context, bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7]);
}

uint8_t je_jit_x86_encode_modrm(int mod, int reg, int rm) {
    je_jit_x86_modrm_t modrm;
    modrm.mod = mod;
    modrm.reg = reg;
    modrm.rm = rm;
    return *((uint8_t*)&modrm);
}

uint8_t je_jit_x86_encode_rex(int w, int r, int x, int b) {
    je_jit_x86_rex_t rex;
    rex.prefix = 0x4;
    rex.w = w;
    rex.r = r;
    rex.x = x;
    rex.b = b;
    return *((uint8_t*)&rex);
}

void je_jit_x86_emit_nop(je_context_t* context) {
    je_jit_start_instruction(context);
    je_jit_emit_bytes_1(context, 0x90); // nop
    je_jit_end_instruction(context, "nop");
}

void je_jit_x86_emit_ret(je_context_t* context) {
    je_jit_start_instruction(context);
    je_jit_emit_bytes_1(context, 0xC3); // ret
    je_jit_end_instruction(context, "ret");
}

void je_jit_x86_emit_pop_r32(je_context_t* context, int src) {
    je_jit_start_instruction(context);
    je_jit_emit_bytes_1(context, 0x58 + src); // POP r64/16
    je_jit_end_instruction(context, "pop %s", je_jit_x86_reg_name(src, 64));
    context->jit_stack_bytes -= 4;
}

void je_jit_x86_emit_push_r32(je_context_t* context, int src) {
    je_jit_start_instruction(context);
    je_jit_emit_bytes_1(context, 0x50 + src); // PUSH r64/16
    je_jit_end_instruction(context, "push %s", je_jit_x86_reg_name(src, 32));
    context->jit_stack_bytes += 4;
}

void je_jit_x86_emit_push_imm32(je_context_t* context, int imm32) {
    je_jit_start_instruction(context);
    je_jit_emit_bytes_1(context, 0x68); // PUSH imm32
    je_jit_x86_emit_imm32(context, imm32);
    je_jit_end_instruction(context, "push 0x%08x", imm32);
    context->jit_stack_bytes += 4;
}

void je_jit_x86_emit_push_r64(je_context_t* context, int src) {
    uint8_t rex_byte = je_jit_x86_encode_rex(1, 0, 0, 0);
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x6, src);
    uint8_t opcode_byte = 0xFF; // PUSH  r/m64/16
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, rex_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "push %s", je_jit_x86_reg_name(src, 64));
    context->jit_stack_bytes += 8;
}

void je_jit_x86_emit_pop_r64(je_context_t* context, int src) {
    uint8_t rex_byte = je_jit_x86_encode_rex(1, 0, 0, 0);
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, src);
    uint8_t opcode_byte = 0x8F; // POP r/m64/16
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, rex_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "pop %s", je_jit_x86_reg_name(src, 64));
    context->jit_stack_bytes -= 8;
}

void je_jit_x86_emit_add_r32_r32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, src, dst);
    uint8_t opcode_byte = 0x01; // ADD r/m16/32/64 r16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "add %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_add_r32_imm32(je_context_t* context, int dst, int imm32) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0, dst);
    uint8_t opcode_byte = 0x81; // ADD r/m16/32/64 imm16/32
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_x86_emit_imm32(context, imm32);
    je_jit_end_instruction(context, "add %s, 0x%08x", je_jit_x86_reg_name(dst, 32), imm32);
}

void je_jit_x86_emit_add_r64_imm32(je_context_t* context, int dst, int imm32) {
    uint8_t rex_byte = je_jit_x86_encode_rex(1, 0, 0, 0);
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0, dst);
    uint8_t opcode_byte = 0x81; // ADD r/m16/32/64 imm16/32
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, rex_byte, opcode_byte, modrm_byte);
    je_jit_x86_emit_imm32(context, imm32);
    je_jit_end_instruction(context, "add %s, 0x%08x", je_jit_x86_reg_name(dst, 32), imm32);
}

void je_jit_x86_emit_sub_r32_imm32(je_context_t* context, int dst, int imm32) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 5, dst);
    uint8_t opcode_byte = 0x81; // SUB r/m16/32/64 imm16/32
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_x86_emit_imm32(context, imm32);
    je_jit_end_instruction(context, "sub %s, 0x%08x", je_jit_x86_reg_name(dst, 32), imm32);
}

void je_jit_x86_emit_sub_r64_imm32(je_context_t* context, int dst, int imm32) {
    uint8_t rex_byte = je_jit_x86_encode_rex(1, 0, 0, 0);
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 5, dst);
    uint8_t opcode_byte = 0x81; // SUB r/m16/32/64 imm16/32
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, rex_byte, opcode_byte, modrm_byte);
    je_jit_x86_emit_imm32(context, imm32);
    je_jit_end_instruction(context, "sub %s, 0x%08x", je_jit_x86_reg_name(dst, 32), imm32);
}

void je_jit_x86_emit_sub_r32_r32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, src, dst);
    uint8_t opcode_byte = 0x29; // SUB r/m16/32/64 r16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "sub %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_imul_r32_r32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, dst, src);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0xAF; // IMUL r16/32/64 r/m16/32/64 
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "imul %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_mov_r32_imm32(je_context_t* context, int dst, int imm32) {
    uint8_t opcode_byte = 0xB8 + dst; // MOV r16/32/64 imm16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_1(context, opcode_byte);
    je_jit_x86_emit_imm32(context, imm32);
    je_jit_end_instruction(context, "mov %s, 0x%08x", je_jit_x86_reg_name(dst, 32), imm32);
}

void je_jit_x86_emit_mov_r32_r32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, dst, src);
    uint8_t opcode_byte = 0x8B; // MOV r16/32/64 r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "mov %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_mov_r64_r64(je_context_t* context, int dst, int src) {
    uint8_t rex_byte = je_jit_x86_encode_rex(1, 0, 0, 0);
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, dst, src);
    uint8_t opcode_byte = 0x8B; // MOV r16/32/64 r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, rex_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "mov %s, %s", je_jit_x86_reg_name(dst, 64), je_jit_x86_reg_name(src, 64));
}

void je_jit_x86_emit_mov_r64_imm64(je_context_t* context, int dst, uint64_t imm64) {
    uint8_t rex_byte = je_jit_x86_encode_rex(1, 0, 0, 0);
    uint8_t opcode_byte = 0xB8 + dst; // MOV r16/32/64 imm16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, rex_byte, opcode_byte);
    je_jit_x86_emit_imm64(context, imm64);
    je_jit_end_instruction(context, "movabs %s, 0x%016llx", je_jit_x86_reg_name(dst, 64), imm64);
}

void je_jit_x86_emit_mov_r32_direct_r64_addr(je_context_t* context, int dst, int src) {    
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_MEMORY_ADDRESSING_NO_DISPLACEMENT, dst, src);
    uint8_t opcode_byte = 0x8B; // MOV r16/32/64 r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "mov %s, dword ptr [%s]", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 64));
}

void je_jit_x86_emit_mov_r64_direct_r64_addr(je_context_t* context, int dst, int src) {
    uint8_t rex_byte = je_jit_x86_encode_rex(1, 0, 0, 0);
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_MEMORY_ADDRESSING_NO_DISPLACEMENT, dst, src);
    uint8_t opcode_byte = 0x8B; // MOV r16/32/64 r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, rex_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "mov %s, qword ptr [%s]", je_jit_x86_reg_name(dst, 64), je_jit_x86_reg_name(src, 64));
}

void je_jit_x86_emit_mov_r64_addr_r32_direct(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_MEMORY_ADDRESSING_NO_DISPLACEMENT, src, dst);
    uint8_t opcode_byte = 0x89; // MOV r/m16/32/64 r16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "mov dword ptr [%s], %s", je_jit_x86_reg_name(dst, 64), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_mov_r64_addr_r64_direct(je_context_t* context, int dst, int src) {
    uint8_t rex_byte = je_jit_x86_encode_rex(1, 0, 0, 0);
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_MEMORY_ADDRESSING_NO_DISPLACEMENT, src, dst);
    uint8_t opcode_byte = 0x89; // MOV r/m16/32/64 r16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, rex_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "mov qword ptr [%s], %s", je_jit_x86_reg_name(dst, 64), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_mov_r32_direct_r32_addr(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_MEMORY_ADDRESSING_NO_DISPLACEMENT, dst, src);
    uint8_t opcode_byte = 0x8B; // MOV r16/32/64 r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "mov %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_mov_r32_addr_r32_direct(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_MEMORY_ADDRESSING_NO_DISPLACEMENT, src, dst);
    uint8_t opcode_byte = 0x89; // MOV r/m16/32/64 r16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "mov %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_idiv_eax_r32(je_context_t* context, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x7, src);
    uint8_t opcode_byte = 0xF7; // IDIV rDX rAX r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "idiv %s", je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_xor_r32_r32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, src, dst);
    uint8_t opcode_byte = 0x31; // XOR r/m16/32/64 	r16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "xor %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_not_r32(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x2, dst);
    uint8_t opcode_byte = 0xF7; // NOT r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "not %s", je_jit_x86_reg_name(dst, 32));
}

void je_jit_x86_emit_and_r32_r32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, dst, src);
    uint8_t opcode_byte = 0x23; // AND r16/32/64 r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "and %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_and_r32_imm32(je_context_t* context, int dst, uint32_t imm32) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0, dst);
    uint8_t opcode_byte = 0x81; // AND r/m16/32/64 imm16/32
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_x86_emit_imm32(context, imm32);
    je_jit_end_instruction(context, "and %s, 0x%08x", je_jit_x86_reg_name(dst, 32), imm32);
}

void je_jit_x86_emit_and_r64_imm64_sign_extended(je_context_t* context, int dst, uint32_t imm32) {
    uint8_t rex_byte = je_jit_x86_encode_rex(1, 0, 0, 0);
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0, dst);
    uint8_t opcode_byte = 0x81; // AND r/m16/32/64 imm16/32
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, rex_byte, opcode_byte, modrm_byte);
    je_jit_x86_emit_imm32(context, imm32);
    je_jit_end_instruction(context, "add %s, 0x%08x", je_jit_x86_reg_name(dst, 64), imm32);
}

void je_jit_x86_emit_or_r32_r32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, dst, src);
    uint8_t opcode_byte = 0x0B; // OR r16/32/64 r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "or %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_neg_r32(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x3, dst);
    uint8_t opcode_byte = 0xF7; // NEG r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "neg %s", je_jit_x86_reg_name(dst, 32));
}

void je_jit_x86_emit_cmp_r32_r32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, dst, src);
    uint8_t opcode_byte = 0x3B; // CMP r16/32/64 r/m16/32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "cmp %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_cmp_r32_imm32(je_context_t* context, int dst, int imm32) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x7, dst);
    uint8_t opcode_byte = 0x81; // CMP r/m16/32/64 imm16/32
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_x86_emit_imm32(context, imm32);
    je_jit_end_instruction(context, "cmp %s, 0x%08x", je_jit_x86_reg_name(dst, 32), imm32 );
}

void je_jit_x86_emit_setl_r8(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, dst);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0x9C; // SETL r/m8
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "setl %s", je_jit_x86_reg_name(dst, 8));
}

void je_jit_x86_emit_setle_r8(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, dst);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0x9E; // SETLE r/m8
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "setle %s", je_jit_x86_reg_name(dst, 8));
}

void je_jit_x86_emit_setg_r8(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, dst);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0x9F; // SETG r/m8
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "setg %s", je_jit_x86_reg_name(dst, 8));
}

void je_jit_x86_emit_setge_r8(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, dst);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0x9D; // SETGE r/m8
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "setge %s", je_jit_x86_reg_name(dst, 8));
}

void je_jit_x86_emit_setb_r8(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, dst);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0x92; // SETB r/m8
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "setb %s", je_jit_x86_reg_name(dst, 8));
}

void je_jit_x86_emit_setbe_r8(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, dst);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0x96; // SETBE r/m8
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "setbe %s", je_jit_x86_reg_name(dst, 8));
}

void je_jit_x86_emit_seta_r8(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, dst);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0x97; // SETA r/m8
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "seta %s", je_jit_x86_reg_name(dst, 8));
}

void je_jit_x86_emit_setae_r8(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, dst);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0x93; // SETAE r/m8
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "setae %s", je_jit_x86_reg_name(dst, 8));
}

void je_jit_x86_emit_sete_r8(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, dst);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0x94; // SETE r/m8
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "sete %s", je_jit_x86_reg_name(dst, 8));
}

void je_jit_x86_emit_setne_r8(je_context_t* context, int dst) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x0, dst);
    uint8_t prefix_byte = 0x0F;
    uint8_t opcode_byte = 0x95; // SETNE r/m8
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, prefix_byte, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "setna %s", je_jit_x86_reg_name(dst, 8));
}

void je_jit_x86_emit_call(je_context_t* context, int addr_reg) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, 0x2, addr_reg);
    uint8_t opcode_byte = 0xFF; // CALL r/m64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_2(context, opcode_byte, modrm_byte);
    je_jit_end_instruction(context, "call %s", je_jit_x86_reg_name(addr_reg, 32));
}

void je_jit_x86_emit_xorps_xmm32_xmm32(je_context_t* context, int reg1, int reg2) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, reg1 - JE_JIT_X86_REG_XMM0, reg2 - JE_JIT_X86_REG_XMM0);
    uint8_t opcode_byte1 = 0x0F;
    uint8_t opcode_byte2 = 0x57; // XORPS xmm xmm/m128
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, opcode_byte1, opcode_byte2, modrm_byte);
    je_jit_end_instruction(context, "xorps %s, %s", je_jit_x86_reg_name(reg1, 32), je_jit_x86_reg_name(reg2, 32));
}

void je_jit_x86_emit_addss_xmm32_xmm32(je_context_t* context, int reg1, int reg2) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, reg1 - JE_JIT_X86_REG_XMM0, reg2 - JE_JIT_X86_REG_XMM0);
    uint8_t rep_prefix = 0xF3;
    uint8_t opcode_byte1 = 0x0F;
    uint8_t opcode_byte2 = 0x58; // ADDSS xmm xmm/m32	
    je_jit_start_instruction(context);
    je_jit_emit_bytes_4(context, rep_prefix, opcode_byte1, opcode_byte2, modrm_byte);
    je_jit_end_instruction(context, "addss %s, %s", je_jit_x86_reg_name(reg1, 32), je_jit_x86_reg_name(reg2, 32));
}

void je_jit_x86_emit_subss_xmm32_xmm32(je_context_t* context, int reg1, int reg2) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, reg1 - JE_JIT_X86_REG_XMM0, reg2 - JE_JIT_X86_REG_XMM0);
    uint8_t rep_prefix = 0xF3;
    uint8_t opcode_byte1 = 0x0F;
    uint8_t opcode_byte2 = 0x5C; // SUBSS xmm xmm/m32	
    je_jit_start_instruction(context);
    je_jit_emit_bytes_4(context, rep_prefix, opcode_byte1, opcode_byte2, modrm_byte);
    je_jit_end_instruction(context, "subss %s, %s", je_jit_x86_reg_name(reg1, 32), je_jit_x86_reg_name(reg2, 32));
}

void je_jit_x86_emit_mulss_xmm32_xmm32(je_context_t* context, int reg1, int reg2) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, reg1 - JE_JIT_X86_REG_XMM0, reg2 - JE_JIT_X86_REG_XMM0);
    uint8_t rep_prefix = 0xF3;
    uint8_t opcode_byte1 = 0x0F;
    uint8_t opcode_byte2 = 0x59; // MULSS xmm xmm/m32	
    je_jit_start_instruction(context);
    je_jit_emit_bytes_4(context, rep_prefix, opcode_byte1, opcode_byte2, modrm_byte);
    je_jit_end_instruction(context, "mulss %s, %s", je_jit_x86_reg_name(reg1, 32), je_jit_x86_reg_name(reg2, 32));
}

void je_jit_x86_emit_divss_xmm32_xmm32(je_context_t* context, int reg1, int reg2) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, reg1 - JE_JIT_X86_REG_XMM0, reg2 - JE_JIT_X86_REG_XMM0);
    uint8_t rep_prefix = 0xF3;
    uint8_t opcode_byte1 = 0x0F;
    uint8_t opcode_byte2 = 0x5E; // DIVSS xmm xmm/m32	
    je_jit_start_instruction(context);
    je_jit_emit_bytes_4(context, rep_prefix, opcode_byte1, opcode_byte2, modrm_byte);
    je_jit_end_instruction(context, "divss %s, %s", je_jit_x86_reg_name(reg1, 32), je_jit_x86_reg_name(reg2, 32));
}

void je_jit_x86_emit_comiss_xmm32_xmm32(je_context_t* context, int reg1, int reg2) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, reg1 - JE_JIT_X86_REG_XMM0, reg2 - JE_JIT_X86_REG_XMM0);
    uint8_t opcode_byte1 = 0x0F;
    uint8_t opcode_byte2 = 0x2F; // COMISS xmm xmm/m32
    je_jit_start_instruction(context);
    je_jit_emit_bytes_3(context, opcode_byte1, opcode_byte2, modrm_byte);
    je_jit_end_instruction(context, "comiss %s, %s", je_jit_x86_reg_name(reg1, 32), je_jit_x86_reg_name(reg2, 32));
}

void je_jit_x86_emit_movss_xmm32_r32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_MEMORY_ADDRESSING_NO_DISPLACEMENT, dst - JE_JIT_X86_REG_XMM0, src);
    uint8_t rep_prefix = 0xF3;
    uint8_t opcode_byte1 = 0x0F;
    uint8_t opcode_byte2 = 0x10; // movss xmm xmm/m32
    je_jit_start_instruction(context);
    // where is dst set
    je_jit_emit_bytes_4(context, rep_prefix, opcode_byte1, opcode_byte2, modrm_byte);
    je_jit_end_instruction(context, "movss %s, dword ptr [%s]", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 64));
}

void je_jit_x86_emit_movss_r32_xmm32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_MEMORY_ADDRESSING_NO_DISPLACEMENT, dst, src - JE_JIT_X86_REG_XMM0);
    uint8_t rep_prefix = 0xF3;
    uint8_t opcode_byte1 = 0x0F;
    uint8_t opcode_byte2 = 0x11; // movss xmm/m32 xmm
    je_jit_start_instruction(context);
    je_jit_emit_bytes_4(context, rep_prefix, opcode_byte1, opcode_byte2, modrm_byte);
    je_jit_end_instruction(context, "movss dword ptr [%s], %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 64));
}

void je_jit_x86_emit_cvtsi2ss_xmm32_r32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, dst - JE_JIT_X86_REG_XMM0, src);
    uint8_t rep_prefix = 0xF3;
    uint8_t opcode_byte1 = 0x0F;
    uint8_t opcode_byte2 = 0x2A; // CVTSI2SS xmm r/m32/64
    je_jit_start_instruction(context);
    je_jit_emit_bytes_4(context, rep_prefix, opcode_byte1, opcode_byte2, modrm_byte);
    je_jit_end_instruction(context, "cvtsi2ss %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

void je_jit_x86_emit_cvtss2si_r32_xmm32(je_context_t* context, int dst, int src) {
    uint8_t modrm_byte = je_jit_x86_encode_modrm(JE_JIT_X86_REGISTER_DIRECT_ADDRESSING, dst, src - JE_JIT_X86_REG_XMM0);
    uint8_t rep_prefix = 0xF3;
    uint8_t opcode_byte1 = 0x0F;
    uint8_t opcode_byte2 = 0x2C; // CVTTSS2SI r32/64 xmm/m32
    je_jit_start_instruction(context);
    je_jit_emit_bytes_4(context, rep_prefix, opcode_byte1, opcode_byte2, modrm_byte);
    je_jit_end_instruction(context, "cvttss2si %s, %s", je_jit_x86_reg_name(dst, 32), je_jit_x86_reg_name(src, 32));
}

int je_jit_x86_alloc_exact_reg(je_context_t* context, int reg) {
    int alloc_index = context->jit_register_allocation_counter++;

    // Is it available?
    if (context->jit_register_allocation[reg].alloc_count == 0) {
        context->jit_register_allocation[reg].alloc_count++;
        context->jit_register_allocation[reg].alloc_index = alloc_index;
        return reg;
    }

    // We don't curently support spilling xmm registers, this shouldn't happen in practice
    // with the number of xmm registers we have available.
    assert(reg < JE_JIT_X86_REG_XMM0);

    // Spill to stack.
    context->jit_register_allocation[reg].alloc_count++;
    context->jit_register_allocation[reg].alloc_index = alloc_index;

    // Push original value to stack.
    je_jit_x86_emit_push_r32(context, reg);

    return reg;
}

int je_jit_x86_alloc_alu_reg(je_context_t* context) {
    int gp_registers[3] = { JE_JIT_X86_REG_EAX, JE_JIT_X86_REG_ECX, JE_JIT_X86_REG_EDX };
    int gp_registers_num = sizeof(gp_registers) / sizeof(*gp_registers);

    int alloc_index = context->jit_register_allocation_counter++;

    // Find one thats available.
    for (int i = 0; i < gp_registers_num; i++) {
        int reg = gp_registers[i];
        if (context->jit_register_allocation[reg].alloc_count == 0) {
            context->jit_register_allocation[reg].alloc_count++;
            context->jit_register_allocation[reg].alloc_index = alloc_index;
            return reg;
        }
    }

    // If non are available, find the oldest one to spill to the stack.
    uint32_t oldest_index = UINT32_MAX;
    int oldest_reg = 0;
    for (int i = 0; i < gp_registers_num; i++) {
        int reg = gp_registers[i];
        if (context->jit_register_allocation[reg].alloc_index < oldest_index) {
            oldest_index = context->jit_register_allocation[reg].alloc_index;
            oldest_reg = reg;
        }
    }

    // Spill to stack.
    context->jit_register_allocation[oldest_reg].alloc_count++;
    context->jit_register_allocation[oldest_reg].alloc_index = alloc_index;

    // Push original value to stack.
    je_jit_x86_emit_push_r32(context, oldest_reg);

    return oldest_reg;
}

int je_jit_x86_alloc_xmm_reg(je_context_t* context) {
    int gp_registers[8] = { JE_JIT_X86_REG_XMM0, JE_JIT_X86_REG_XMM1, JE_JIT_X86_REG_XMM2, 
                                JE_JIT_X86_REG_XMM3, JE_JIT_X86_REG_XMM4, JE_JIT_X86_REG_XMM5, 
                                JE_JIT_X86_REG_XMM6, JE_JIT_X86_REG_XMM7 };
    int gp_registers_num = sizeof(gp_registers) / sizeof(*gp_registers);

    int alloc_index = context->jit_register_allocation_counter++;

    // Find one thats available.
    for (int i = 0; i < gp_registers_num; i++) {
        int reg = gp_registers[i];
        if (context->jit_register_allocation[reg].alloc_count == 0) {
            context->jit_register_allocation[reg].alloc_count++;
            context->jit_register_allocation[reg].alloc_index = alloc_index;
            return reg;
        }
    }

    // We don't curently support spilling xmm registers.
    assert(false);
    return 0;
}

int je_jit_e86_find_alu_reg_excluding(je_context_t* context, int reg1, int reg2) {
    int gp_registers[3] = { JE_JIT_X86_REG_EAX, JE_JIT_X86_REG_ECX, JE_JIT_X86_REG_EDX };
    int gp_registers_num = sizeof(gp_registers) / sizeof(*gp_registers);
    for (int i = 0; i < gp_registers_num; i++) {
        int reg = gp_registers[i];
        if (reg != reg1 && reg != reg2) {
            return reg;
        }
    }
    return JE_JIT_X86_REG_EAX;
}

void je_jit_x86_free_reg(je_context_t* context, int reg) {
    context->jit_register_allocation[reg].alloc_count--;

    // If this is a spill register, restore the last value.
    if (context->jit_register_allocation[reg].alloc_count > 0) {
        assert(reg < JE_JIT_X86_REG_XMM0); // xmm registers don't support spilling.
        je_jit_x86_emit_pop_r32(context, reg);
    }
}

bool je_jit_x86_reg_allocated(je_context_t* context, int reg) {
    return (context->jit_register_allocation[reg].alloc_count > 0);
}

void je_jit_x86_emit_prologue(je_context_t* context) {
    je_jit_emit_comment(context, "Prologue");
#if defined(JE_CALLING_CONVENTION_MSVC)
    // In theory we are meant to store volatile registers here, but with the way
    // we do JIT, I don't think we have an occassion where thats actually needed.
#elif defined(JE_CALLING_CONVENTION_C)
    je_jit_x86_emit_push_r32(context, JE_JIT_X86_REG_EBP);
    je_jit_x86_emit_mov_r32_r32(context, JE_JIT_X86_REG_EBP, JE_JIT_X86_REG_ESP);
#elif defined(JE_CALLING_CONVENTION_SYSTEMV)
    // In theory we are meant to store volatile registers here, but with the way
    // we do JIT, I don't think we have an occassion where thats actually needed.
#else
    #error compiler-specific implementation required
#endif
}

void je_jit_x86_emit_epilogue(je_context_t* context, int return_reg) {
    je_jit_emit_comment(context, "Store return value");
    // Move the return value to the result struct.
    context->result.type = context->ast_root->return_type;
    switch (context->ast_root->return_type) {
        case JE_TYPE_BOOL: 
        case JE_TYPE_INT: {
            int addr_reg = je_jit_x86_alloc_alu_reg(context);

#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)&context->result.int_value;
            je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
            je_jit_x86_emit_mov_r64_addr_r32_direct(context, addr_reg, return_reg);
#else
            uint32_t address = (uint32_t)&context->result.int_value;
            je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_addr_r32_direct(context, addr_reg, return_reg);
#endif

            je_jit_x86_free_reg(context, addr_reg);
            break;
        }    
        case JE_TYPE_STRING: {
            int addr_reg = je_jit_x86_alloc_alu_reg(context);

#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)&context->result.string_value;
            je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
            je_jit_x86_emit_mov_r64_addr_r64_direct(context, addr_reg, return_reg);
#else
            uint32_t address = (uint32_t)&context->result.string_value;
            je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_addr_r32_direct(context, addr_reg, return_reg);
#endif

            je_jit_x86_free_reg(context, addr_reg);
            break;
        }    
        case JE_TYPE_FLOAT: {
            int alu_reg = je_jit_x86_alloc_alu_reg(context);

#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)&context->result.float_value;
            je_jit_x86_emit_mov_r64_imm64(context, alu_reg, address);
#else
            uint32_t address = (uint32_t)&context->result.float_value;
            je_jit_x86_emit_mov_r32_imm32(context, alu_reg, address);
#endif

            je_jit_x86_emit_movss_r32_xmm32(context, alu_reg, return_reg);
            je_jit_x86_free_reg(context, alu_reg);
            break;
        }
    }

    je_jit_emit_comment(context, "Epilogue");
#if defined(JE_CALLING_CONVENTION_MSVC)
    // Restore volatile registers (if we ever use them...)
#elif defined(JE_CALLING_CONVENTION_C)
    je_jit_x86_emit_pop_r32(context, JE_JIT_X86_REG_EBP);
#elif defined(JE_CALLING_CONVENTION_SYSTEMV)
    // Restore volatile registers (if we ever use them...)
#else
    #error compiler-specific implementation required
#endif

    je_jit_x86_emit_ret(context);
}
 
// Return value is the register the result is in if applicable.
int je_jit_x86_emit_function_call(je_context_t* context, je_ast_node_t* node) {

    je_func_def_t* function = je_get_ast_node_function(context, node);

    // Store every parameter in the context.
    for (int i = 0; i < function->param_count; i++) {
        je_ast_node_t* child = je_get_node_child(context, node, i);
        int reg1 = je_jit_x86_emit_node(context, child);
        int type = je_get_func_param_type(context, function, i);

        je_jit_emit_comment(context, "Storing %s parameter %i", je_type_name(type), i);
        switch (type) {
            case JE_TYPE_INT: {
                int addr_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
                uint64_t address = (uint64_t)&context->function_params[i].int_value;
                je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
                je_jit_x86_emit_mov_r64_addr_r32_direct(context, addr_reg, reg1);
#else
                uint32_t address = (uint32_t)&context->function_params[i].int_value;
                je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
                je_jit_x86_emit_mov_r32_addr_r32_direct(context, addr_reg, reg1);
#endif
                je_jit_x86_free_reg(context, addr_reg);
                break;
            }
            case JE_TYPE_BOOL: {
                int addr_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
                uint64_t address = (uint64_t)&context->function_params[i].bool_value;
                je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
                je_jit_x86_emit_mov_r64_addr_r32_direct(context, addr_reg, reg1);
#else
                uint32_t address = (uint32_t)&context->function_params[i].bool_value;
                je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
                je_jit_x86_emit_mov_r32_addr_r32_direct(context, addr_reg, reg1);
#endif
                je_jit_x86_free_reg(context, addr_reg);
                break;
            }
            case JE_TYPE_FLOAT: {
                int addr_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
                uint64_t address = (uint64_t)&context->function_params[i].float_value;
                je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
                je_jit_x86_emit_movss_r32_xmm32(context, addr_reg, reg1);
#else
                uint32_t address = (uint32_t)&context->function_params[i].float_value;
                je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
                je_jit_x86_emit_movss_r32_xmm32(context, addr_reg, reg1);
#endif
                je_jit_x86_free_reg(context, addr_reg);
                break;
            }
            case JE_TYPE_STRING: {
                int addr_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
                uint64_t address = (uint64_t)&context->function_params[i].string_value;
                je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
                je_jit_x86_emit_mov_r64_addr_r64_direct(context, addr_reg, reg1);
#else
                uint32_t address = (uint32_t)&context->function_params[i].string_value;
                je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
                je_jit_x86_emit_mov_r32_addr_r32_direct(context, addr_reg, reg1);
#endif
                je_jit_x86_free_reg(context, addr_reg);
                break;
            }
        }

        je_jit_x86_free_reg(context, reg1);
    }

    // This block is compiler specific as they all use slightly different conventions
    // on x64 for passing arguments :cry:
#if defined(JE_CALLING_CONVENTION_MSVC)
    // Store registers we are going to use.
    je_jit_x86_emit_push_r64(context, JE_JIT_X86_REG_ECX);
    je_jit_x86_emit_push_r64(context, JE_JIT_X86_REG_EDX);

    // Ensure stack is aligned.
    je_jit_emit_comment(context, "Stack alignment");
    je_jit_x86_emit_push_r64(context, JE_JIT_X86_REG_EBP);
    je_jit_x86_emit_mov_r64_r64(context, JE_JIT_X86_REG_EBP, JE_JIT_X86_REG_ESP);
    je_jit_x86_emit_and_r64_imm64_sign_extended(context, JE_JIT_X86_REG_ESP, 0xFFFFFFF0);

    // Store the active_function pointer
    je_jit_emit_comment(context, "Storing active_function");
    uint64_t address = (uint64_t)&context->active_function;
    je_jit_x86_emit_mov_r64_imm64(context, JE_JIT_X86_REG_ECX, address);
    je_jit_x86_emit_mov_r64_imm64(context, JE_JIT_X86_REG_EDX, (uint64_t)function);
    je_jit_x86_emit_mov_r64_addr_r64_direct(context, JE_JIT_X86_REG_ECX, JE_JIT_X86_REG_EDX);

    // Push the context agument (first integer argumnet goes to ECX under MSVC).
    je_jit_emit_comment(context, "Passing context argument");
    address = (uint64_t)context;
    je_jit_x86_emit_mov_r64_imm64(context, JE_JIT_X86_REG_ECX, address);
    // Call function
    je_jit_emit_comment(context, "Calling function");
    address = (uint64_t)function->function;
    je_jit_x86_emit_mov_r64_imm64(context, JE_JIT_X86_REG_EDX, address);
    je_jit_x86_emit_call(context, JE_JIT_X86_REG_EDX);
    je_jit_x86_emit_nop(context);

    // Restore alignment.
    je_jit_emit_comment(context, "Restore stack alignment");
    je_jit_x86_emit_mov_r64_r64(context, JE_JIT_X86_REG_ESP, JE_JIT_X86_REG_EBP);
    je_jit_x86_emit_pop_r64(context, JE_JIT_X86_REG_EBP);

    // Restore registers we used.
    je_jit_x86_emit_pop_r64(context, JE_JIT_X86_REG_EDX);
    je_jit_x86_emit_pop_r64(context, JE_JIT_X86_REG_ECX);
#elif defined(JE_CALLING_CONVENTION_C)
    // Store registers we are going to use.
    je_jit_x86_emit_push_r32(context, JE_JIT_X86_REG_ECX);
    je_jit_x86_emit_push_r32(context, JE_JIT_X86_REG_EDX);

    // Ensure stack is aligned.
    je_jit_emit_comment(context, "Stack alignment");
    je_jit_x86_emit_push_r32(context, JE_JIT_X86_REG_EBP);
    je_jit_x86_emit_mov_r32_r32(context, JE_JIT_X86_REG_EBP, JE_JIT_X86_REG_ESP);
    je_jit_x86_emit_and_r32_imm32(context, JE_JIT_X86_REG_ESP, 0xFFFFFFF0);

    // Store the active_function pointer
    je_jit_emit_comment(context, "Storing active_function");
    uint32_t address = (uint32_t)&context->active_function;
    je_jit_x86_emit_mov_r32_imm32(context, JE_JIT_X86_REG_ECX, address);
    je_jit_x86_emit_mov_r32_imm32(context, JE_JIT_X86_REG_EDX, (uint32_t)function);
    je_jit_x86_emit_mov_r32_addr_r32_direct(context, JE_JIT_X86_REG_ECX, JE_JIT_X86_REG_EDX);

    // Push the context agument
    je_jit_emit_comment(context, "Passing context argument");
    address = (uint32_t)context;
    je_jit_x86_emit_push_imm32(context, address);

    // Call function
    je_jit_emit_comment(context, "Calling function");
    address = (uint32_t)function->function;
    je_jit_x86_emit_mov_r32_imm32(context, JE_JIT_X86_REG_EDX, address);
    je_jit_x86_emit_call(context, JE_JIT_X86_REG_EDX);
    je_jit_x86_emit_nop(context);

    // Pop value off stack.
    je_jit_x86_emit_add_r32_imm32(context, JE_JIT_X86_REG_ESP, 4);

    // Restore alignment.
    je_jit_emit_comment(context, "Restore stack alignment");
    je_jit_x86_emit_mov_r32_r32(context, JE_JIT_X86_REG_ESP, JE_JIT_X86_REG_EBP);
    je_jit_x86_emit_pop_r32(context, JE_JIT_X86_REG_EBP);

    // Restore registers we used.
    je_jit_x86_emit_pop_r32(context, JE_JIT_X86_REG_EDX);
    je_jit_x86_emit_pop_r32(context, JE_JIT_X86_REG_ECX);
#elif defined(JE_CALLING_CONVENTION_SYSTEMV)
    // Store registers we are going to use.
    je_jit_x86_emit_push_r64(context, JE_JIT_X86_REG_ECX);
    je_jit_x86_emit_push_r64(context, JE_JIT_X86_REG_EDX);

    // Ensure stack is aligned.
    je_jit_emit_comment(context, "Stack alignment");
    je_jit_x86_emit_push_r64(context, JE_JIT_X86_REG_EBP);
    je_jit_x86_emit_mov_r64_r64(context, JE_JIT_X86_REG_EBP, JE_JIT_X86_REG_ESP);
    je_jit_x86_emit_and_r64_imm64_sign_extended(context, JE_JIT_X86_REG_ESP, 0xFFFFFFF0);

    // Store the active_function pointer
    je_jit_emit_comment(context, "Storing active_function");
    uint64_t address = (uint64_t)&context->active_function;
    je_jit_x86_emit_mov_r64_imm64(context, JE_JIT_X86_REG_ECX, address);
    je_jit_x86_emit_mov_r64_imm64(context, JE_JIT_X86_REG_EDX, (uint64_t)function);
    je_jit_x86_emit_mov_r64_addr_r64_direct(context, JE_JIT_X86_REG_ECX, JE_JIT_X86_REG_EDX);

    // Push the context agument (first integer argumnet goes to RDI under SystemV).
    je_jit_emit_comment(context, "Passing context argument");
    address = (uint64_t)context;
    je_jit_x86_emit_mov_r64_imm64(context, JE_JIT_X86_REG_EDI, address);
    // Call function
    je_jit_emit_comment(context, "Calling function");
    address = (uint64_t)function->function;
    je_jit_x86_emit_mov_r64_imm64(context, JE_JIT_X86_REG_EDX, address);
    je_jit_x86_emit_call(context, JE_JIT_X86_REG_EDX);
    je_jit_x86_emit_nop(context);

    // Restore alignment.
    je_jit_emit_comment(context, "Restore stack alignment");
    je_jit_x86_emit_mov_r64_r64(context, JE_JIT_X86_REG_ESP, JE_JIT_X86_REG_EBP);
    je_jit_x86_emit_pop_r64(context, JE_JIT_X86_REG_EBP);

    // Restore registers we used.
    je_jit_x86_emit_pop_r64(context, JE_JIT_X86_REG_EDX);
    je_jit_x86_emit_pop_r64(context, JE_JIT_X86_REG_ECX);
#else
    #error compiler-specific implementation required
#endif

    // Move result into return register.
    int ret_reg = 0;

    je_jit_emit_comment(context, "Retrieving %s return value", je_type_name(function->return_type));
    switch (function->return_type) {
        case JE_TYPE_INT: {
            ret_reg = je_jit_x86_alloc_alu_reg(context);
            int addr_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)&context->function_result.int_value;
            je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_direct_r64_addr(context, ret_reg, addr_reg);
#else
            uint32_t address = (uint32_t)&context->function_result.int_value;
            je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_direct_r32_addr(context, ret_reg, addr_reg);
#endif
            je_jit_x86_free_reg(context, addr_reg);
            break;
        }
        case JE_TYPE_BOOL: {
            ret_reg = je_jit_x86_alloc_alu_reg(context);
            int addr_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)&context->function_result.bool_value;
            je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_direct_r64_addr(context, ret_reg, addr_reg);
#else
            uint32_t address = (uint32_t)&context->function_result.bool_value;
            je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_direct_r32_addr(context, ret_reg, addr_reg);
#endif
            je_jit_x86_free_reg(context, addr_reg);
            break;
        }
        case JE_TYPE_FLOAT: {
            ret_reg = je_jit_x86_alloc_xmm_reg(context);
            int addr_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)&context->function_result.float_value;
            je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
            je_jit_x86_emit_movss_xmm32_r32(context, ret_reg, addr_reg);
#else
            uint32_t address = (uint32_t)&context->function_result.float_value;
            je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
            je_jit_x86_emit_movss_xmm32_r32(context, ret_reg, addr_reg);
#endif
            je_jit_x86_free_reg(context, addr_reg);
            break;
        }
        case JE_TYPE_STRING: {
            ret_reg = je_jit_x86_alloc_alu_reg(context);
            int addr_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)&context->function_result.string_value;
            je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
            je_jit_x86_emit_mov_r64_direct_r64_addr(context, ret_reg, addr_reg);
#else
            uint32_t address = (uint32_t)&context->function_result.string_value;
            je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_direct_r32_addr(context, ret_reg, addr_reg);
#endif
            je_jit_x86_free_reg(context, addr_reg);
            break;
        }
    }

    return ret_reg;
}

// Return value is the register the result is in if applicable.
int je_jit_x86_emit_node(je_context_t* context, je_ast_node_t* node) {
    je_jit_emit_comment(context, "%s", je_node_name(node->type));
    switch (node->type) {
        // ------------------------------------------------------------------------------
        // Integer operations
        // ------------------------------------------------------------------------------
        case JE_NODE_BITWISE_NOT_INT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_not_r32(context, reg1);
            return reg1;
        }
        case JE_NODE_MUL_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_imul_r32_r32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_MOD_INT: 
        case JE_NODE_DIV_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);

            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            
            // If reg2 is eax, move it elsewhere we need it for the source operand.
            if (reg2 == JE_JIT_X86_REG_EAX) {
                int new_reg = je_jit_x86_alloc_exact_reg(context, je_jit_e86_find_alu_reg_excluding(context, reg2, reg1));
                je_jit_x86_emit_mov_r32_r32(context, new_reg, reg2);
                je_jit_x86_free_reg(context, reg2);
                reg2 = new_reg;
            }

            // If reg1 is not eax, move it into eax.
            if (reg1 != JE_JIT_X86_REG_EAX) {
                int eax_reg = je_jit_x86_alloc_exact_reg(context, JE_JIT_X86_REG_EAX); 
                je_jit_x86_emit_mov_r32_r32(context, eax_reg, reg1);
                je_jit_x86_free_reg(context, reg1);
                reg1 = eax_reg;
            }

            // If RDX is in use, we need to store it off as the remainder ends up in it.
            bool restore_edx = false;
            if (je_jit_x86_reg_allocated(context, JE_JIT_X86_REG_EDX)) {
                je_jit_x86_emit_push_r32(context, JE_JIT_X86_REG_EDX);
            }

            je_jit_x86_emit_xor_r32_r32(context, JE_JIT_X86_REG_EDX, JE_JIT_X86_REG_EDX);
            je_jit_x86_emit_idiv_eax_r32(context, reg2);

            // If we are doing a mod, then move EDX into the output reg.
            if (node->type == JE_NODE_MOD_INT) {
                je_jit_x86_emit_mov_r32_r32(context, reg1, JE_JIT_X86_REG_EDX);
            }

            if (restore_edx) {
                je_jit_x86_emit_pop_r32(context, JE_JIT_X86_REG_EDX);
            }

            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_SUB_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_sub_r32_r32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_ADD_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_add_r32_r32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_LESS_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cmp_r32_r32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_setl_r8(context, reg1);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_LE_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cmp_r32_r32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_setle_r8(context, reg1);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_GREATER_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cmp_r32_r32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_setg_r8(context, reg1);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_GE_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cmp_r32_r32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_setge_r8(context, reg1);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_EQUAL_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cmp_r32_r32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_sete_r8(context, reg1);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_NOT_EQUAL_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cmp_r32_r32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_setne_r8(context, reg1);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_BITWISE_AND_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_and_r32_r32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_BITWISE_OR_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_or_r32_r32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_BITWISE_XOR_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_xor_r32_r32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_VARIABLE_INT: {
            int addr_reg = je_jit_x86_alloc_alu_reg(context);
            int dst_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)je_get_variable_int(je_get_ast_node_variable(context, node));
            je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_direct_r64_addr(context, dst_reg, addr_reg);
#else
            uint32_t address = (uint32_t)je_get_variable_int(je_get_ast_node_variable(context, node));
            je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_direct_r32_addr(context, dst_reg, addr_reg);
#endif
            je_jit_x86_free_reg(context, addr_reg);
            return dst_reg;
        }
        case JE_NODE_NEG_INT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_neg_r32(context, reg1);
            return reg1;
        }
        case JE_NODE_POS_INT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, rvalue);
            // TODO: This is basically a nop, why do we have it?
            return reg1;
        }
        case JE_NODE_INT_LITERAL: {
            int reg = je_jit_x86_alloc_alu_reg(context);
            je_jit_x86_emit_mov_r32_imm32(context, reg, *je_get_ast_node_int(context, node));
            return reg;
        }
        case JE_NODE_CAST_INT_TO_FLOAT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_alloc_xmm_reg(context);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cvtsi2ss_xmm32_r32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_CAST_INT_TO_BOOL: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cmp_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_setne_r8(context, reg1);
            return reg1;
        }
        case JE_NODE_FUNCTION_CALL_INT: {
            return je_jit_x86_emit_function_call(context, node);
        }

        // ------------------------------------------------------------------------------
        // Boolean Operations
        // ------------------------------------------------------------------------------
        case JE_NODE_LOGICAL_NOT_BOOL: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cmp_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_sete_r8(context, reg1);
            return reg1;
        }
        case JE_NODE_LOGICAL_AND_BOOL: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_and_r32_r32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_LOGICAL_OR_BOOL: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_or_r32_r32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_EQUAL_BOOL: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cmp_r32_r32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_sete_r8(context, reg1);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_NOT_EQUAL_BOOL: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cmp_r32_r32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_setne_r8(context, reg1);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_VARIABLE_BOOL: {
            int addr_reg = je_jit_x86_alloc_alu_reg(context);
            int dst_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)je_get_variable_bool(je_get_ast_node_variable(context, node));
            je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_direct_r64_addr(context, dst_reg, addr_reg);
#else
            uint32_t address = (uint32_t)je_get_variable_bool(je_get_ast_node_variable(context, node));
            je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_direct_r32_addr(context, dst_reg, addr_reg);
#endif
            je_jit_x86_free_reg(context, addr_reg);
            break;
        }
        case JE_NODE_BOOL_LITERAL: {
            int reg = je_jit_x86_alloc_alu_reg(context);
            je_jit_x86_emit_mov_r32_imm32(context, reg, *je_get_ast_node_bool(context, node));
            return reg;
        }
        case JE_NODE_CAST_BOOL_TO_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            // This is essentially a nop, they are stored identically.
            return reg1;
        }
        case JE_NODE_CAST_BOOL_TO_FLOAT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_alloc_xmm_reg(context);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cvtsi2ss_xmm32_r32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_FUNCTION_CALL_BOOL: {
            return je_jit_x86_emit_function_call(context, node);
        }

        // ------------------------------------------------------------------------------
        // Float Operations
        // ------------------------------------------------------------------------------
        case JE_NODE_MUL_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_mulss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_DIV_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_divss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_SUB_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_subss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_ADD_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_addss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_LESS_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            int ret_reg = je_jit_x86_alloc_alu_reg(context);
            je_jit_x86_emit_comiss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, ret_reg, 0);
            je_jit_x86_emit_setb_r8(context, ret_reg);
            je_jit_x86_free_reg(context, reg2);
            je_jit_x86_free_reg(context, reg1);
            return ret_reg;
        }
        case JE_NODE_GREATER_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            int ret_reg = je_jit_x86_alloc_alu_reg(context);
            je_jit_x86_emit_comiss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, ret_reg, 0);
            je_jit_x86_emit_seta_r8(context, ret_reg);
            je_jit_x86_free_reg(context, reg2);
            je_jit_x86_free_reg(context, reg1);
            return ret_reg;
        }
        case JE_NODE_LE_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            int ret_reg = je_jit_x86_alloc_alu_reg(context);
            je_jit_x86_emit_comiss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, ret_reg, 0);
            je_jit_x86_emit_setbe_r8(context, ret_reg);
            je_jit_x86_free_reg(context, reg2);
            je_jit_x86_free_reg(context, reg1);
            return ret_reg;
        }
        case JE_NODE_GE_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            int ret_reg = je_jit_x86_alloc_alu_reg(context);
            je_jit_x86_emit_comiss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, ret_reg, 0);
            je_jit_x86_emit_setae_r8(context, ret_reg);
            je_jit_x86_free_reg(context, reg2);
            je_jit_x86_free_reg(context, reg1);
            return ret_reg;
        }
        case JE_NODE_EQUAL_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            int ret_reg = je_jit_x86_alloc_alu_reg(context);
            je_jit_x86_emit_comiss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, ret_reg, 0);
            je_jit_x86_emit_sete_r8(context, ret_reg);
            je_jit_x86_free_reg(context, reg2);
            je_jit_x86_free_reg(context, reg1);
            return ret_reg;
        }
        case JE_NODE_NOT_EQUAL_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, lvalue);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            int ret_reg = je_jit_x86_alloc_alu_reg(context);
            je_jit_x86_emit_comiss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_emit_mov_r32_imm32(context, ret_reg, 0);
            je_jit_x86_emit_setne_r8(context, ret_reg);
            je_jit_x86_free_reg(context, reg2);
            je_jit_x86_free_reg(context, reg1);
            return ret_reg;
        }
        case JE_NODE_VARIABLE_FLOAT: {
            int alu_reg = je_jit_x86_alloc_alu_reg(context);
            int xmm_reg = je_jit_x86_alloc_xmm_reg(context);

#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)je_get_variable_float(je_get_ast_node_variable(context, node));
            je_jit_x86_emit_mov_r64_imm64(context, alu_reg, address);
#else
            uint32_t address = (uint32_t)je_get_variable_float(je_get_ast_node_variable(context, node));
            je_jit_x86_emit_mov_r32_imm32(context, alu_reg, address);
#endif

            je_jit_x86_emit_movss_xmm32_r32(context, xmm_reg, alu_reg);
            je_jit_x86_free_reg(context, alu_reg);
            return xmm_reg;
            break;
        }
        case JE_NODE_NEG_FLOAT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_alloc_xmm_reg(context);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_xorps_xmm32_xmm32(context, reg1, reg1);
            je_jit_x86_emit_subss_xmm32_xmm32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_POS_FLOAT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_emit_node(context, rvalue);
            // TODO: This is basically a nop, why do we have it?
            return reg1;
        }
        case JE_NODE_FLOAT_LITERAL: {
            int alu_reg = je_jit_x86_alloc_alu_reg(context);
            int xmm_reg = je_jit_x86_alloc_xmm_reg(context);

#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)je_get_ast_node_float(context, node);
            je_jit_x86_emit_mov_r64_imm64(context, alu_reg, address);
#else
            uint32_t address = (uint32_t)je_get_ast_node_float(context, node);
            je_jit_x86_emit_mov_r32_imm32(context, alu_reg, address);
#endif

            je_jit_x86_emit_movss_xmm32_r32(context, xmm_reg, alu_reg);
            je_jit_x86_free_reg(context, alu_reg);
            return xmm_reg;
        }
        case JE_NODE_CAST_FLOAT_TO_INT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_alloc_alu_reg(context);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cvtss2si_r32_xmm32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);            
            return reg1;
        }
        case JE_NODE_CAST_FLOAT_TO_BOOL: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_x86_alloc_alu_reg(context);
            int reg2 = je_jit_x86_emit_node(context, rvalue);
            je_jit_x86_emit_cvtss2si_r32_xmm32(context, reg1, reg2);
            je_jit_x86_free_reg(context, reg2);

            je_jit_x86_emit_cmp_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_mov_r32_imm32(context, reg1, 0);
            je_jit_x86_emit_setne_r8(context, reg1);
            return reg1;
        }
        case JE_NODE_FUNCTION_CALL_FLOAT: {
            return je_jit_x86_emit_function_call(context, node);
        }

        // ------------------------------------------------------------------------------
        // String Operations
        // ------------------------------------------------------------------------------
        case JE_NODE_VARIABLE_STRING: {
            int addr_reg = je_jit_x86_alloc_alu_reg(context);
            int dst_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)je_get_variable_string(je_get_ast_node_variable(context, node));
            je_jit_x86_emit_mov_r64_imm64(context, addr_reg, address);
            je_jit_x86_emit_mov_r64_direct_r64_addr(context, dst_reg, addr_reg);
#else
            uint32_t address = (uint32_t)je_get_variable_string(je_get_ast_node_variable(context, node));
            je_jit_x86_emit_mov_r32_imm32(context, addr_reg, address);
            je_jit_x86_emit_mov_r32_direct_r32_addr(context, dst_reg, addr_reg);
#endif
            je_jit_x86_free_reg(context, addr_reg);
            return dst_reg;
        }
        case JE_NODE_STRING_LITERAL: {
            int dst_reg = je_jit_x86_alloc_alu_reg(context);
#ifdef JE_ISA_X64
            uint64_t address = (uint64_t)*je_get_ast_node_string(context, node);
            je_jit_x86_emit_mov_r64_imm64(context, dst_reg, address);
#else
            uint32_t address = (uint32_t)*je_get_ast_node_string(context, node);
            je_jit_x86_emit_mov_r32_imm32(context, dst_reg, address);
#endif
            return dst_reg;
        }
        case JE_NODE_FUNCTION_CALL_STRING: {
            return je_jit_x86_emit_function_call(context, node);
        }

    }
    assert(false);
    return 0;
}

// -----------------------------------------------------------------------
// ARM32 / ARM64 JIT
// -----------------------------------------------------------------------
#elif defined(JE_ISA_ARM32) || defined(JE_ISA_ARM64)

#define JE_JIT_ARM_REG_X0       (0)
#define JE_JIT_ARM_REG_X1       (1)
#define JE_JIT_ARM_REG_X2       (2)
#define JE_JIT_ARM_REG_X3       (3)
#define JE_JIT_ARM_REG_X4       (4)
#define JE_JIT_ARM_REG_X5       (5)
#define JE_JIT_ARM_REG_X6       (6)
#define JE_JIT_ARM_REG_X7       (7)
#define JE_JIT_ARM_REG_X8       (8)
#define JE_JIT_ARM_REG_X9       (9)
#define JE_JIT_ARM_REG_X10      (10)
#define JE_JIT_ARM_REG_X11      (11)
#define JE_JIT_ARM_REG_X12      (12)
#define JE_JIT_ARM_REG_X13      (13)
#define JE_JIT_ARM_REG_X14      (14)
#define JE_JIT_ARM_REG_X15      (15)
#define JE_JIT_ARM_REG_X16      (16)
#define JE_JIT_ARM_REG_X17      (17)
#define JE_JIT_ARM_REG_X18      (18)
#define JE_JIT_ARM_REG_X19      (19)
#define JE_JIT_ARM_REG_X20      (20)
#define JE_JIT_ARM_REG_X21      (21)
#define JE_JIT_ARM_REG_X22      (22)
#define JE_JIT_ARM_REG_X23      (23)
#define JE_JIT_ARM_REG_X24      (24)
#define JE_JIT_ARM_REG_X25      (25)
#define JE_JIT_ARM_REG_X26      (26)
#define JE_JIT_ARM_REG_X27      (27)
#define JE_JIT_ARM_REG_X28      (28)
#define JE_JIT_ARM_REG_X29      (29)
#define JE_JIT_ARM_REG_X30      (30)
#define JE_JIT_ARM_REG_X31      (31)
#define JE_JIT_ARM_REG_SP       (31) // Alias of X31
#define JE_JIT_ARM_REG_XZR      (31) // Alias of X31

// These are offset for easier handling, subtract by V0 to get the correct
// index when encoding instructions.
#define JE_JIT_ARM_REG_V0       (32)
#define JE_JIT_ARM_REG_V1       (33)
#define JE_JIT_ARM_REG_V2       (34)
#define JE_JIT_ARM_REG_V3       (35)
#define JE_JIT_ARM_REG_V4       (36)
#define JE_JIT_ARM_REG_V5       (37)
#define JE_JIT_ARM_REG_V6       (38)
#define JE_JIT_ARM_REG_V7       (39)
#define JE_JIT_ARM_REG_V8       (40)
#define JE_JIT_ARM_REG_V9       (41)
#define JE_JIT_ARM_REG_V10      (42)
#define JE_JIT_ARM_REG_V11      (43)
#define JE_JIT_ARM_REG_V12      (44)
#define JE_JIT_ARM_REG_V13      (45)
#define JE_JIT_ARM_REG_V14      (46)
#define JE_JIT_ARM_REG_V15      (47)
#define JE_JIT_ARM_REG_V16      (48)
#define JE_JIT_ARM_REG_V17      (49)
#define JE_JIT_ARM_REG_V18      (50)
#define JE_JIT_ARM_REG_V19      (51)
#define JE_JIT_ARM_REG_V20      (52)
#define JE_JIT_ARM_REG_V21      (53)
#define JE_JIT_ARM_REG_V22      (54)
#define JE_JIT_ARM_REG_V23      (55)
#define JE_JIT_ARM_REG_V24      (56)
#define JE_JIT_ARM_REG_V25      (57)
#define JE_JIT_ARM_REG_V26      (58)
#define JE_JIT_ARM_REG_V27      (59)
#define JE_JIT_ARM_REG_V28      (60)
#define JE_JIT_ARM_REG_V29      (61)
#define JE_JIT_ARM_REG_V30      (62)
#define JE_JIT_ARM_REG_V31      (63)

// Some misc useful links
// https://aengelke.net/disarm.html
// http://163.238.35.161/~zhangs/arm64simulator/
// https://support.arm.com/documentation/ddi0596/2020-12/Base-Instructions

void je_jit_arm_emit_prologue(je_context_t* context);
void je_jit_arm_emit_epilogue(je_context_t* context, int result_reg);
int  je_jit_arm_emit_node(je_context_t* context, je_ast_node_t* node);
int  je_jit_arm_alloc_reg(je_context_t* context);
void je_jit_arm_free_reg(je_context_t* context, int reg);

int je_compile_jit(je_context_t* context) {
    int ret;
    
    // Use remaining space in the mem arena for jit compiling.
    context->jit_write_buffer = context->mem_arena + context->mem_arena_offset;
    context->jit_write_buffer_len = (int)(JE_MEM_ARENA_SIZE - context->mem_arena_offset);
    context->jit_write_ptr = context->jit_write_buffer;
    context->jit_write_buffer_overflow = false;
    context->jit_instruction_num = 0;
    context->jit_code_bytes = 0;

    context->mem_arena_frozen = true;
    je_jit_arm_emit_prologue(context);
    int return_reg = je_jit_arm_emit_node(context, context->ast_root);
    je_jit_arm_emit_epilogue(context, return_reg);
    context->mem_arena_frozen = false;

    // Ran out of space while trying to emit all JIT code.
    if (context->jit_write_buffer_overflow) {
        return JE_RESULT_OOM;
    }

    // Emplace the compiled code into executable memory.
    int jit_length = (int)(context->jit_write_ptr - context->jit_write_buffer);
    ret = je_alloc_executable(context, context->jit_write_buffer, jit_length, &context->jit_executable_memory, &context->jit_executable_memory_length);
    if (ret < 0) {
        return ret;
    }

    context->jit_code_bytes = jit_length;
    context->jit_compiled = true;
    return JE_RESULT_SUCCESS;
}

void je_jit_free(je_context_t* context) {
    je_free_executable(context, context->jit_executable_memory, context->jit_code_bytes);
}

void je_jit_arm_emit_add_sub_imm12(je_context_t* context, int dst, int src, int imm12, bool is_add) {
    struct {
        uint32_t Rd     : 5;
        uint32_t Rn     : 5;
        uint32_t imm12  : 12;
        uint32_t sh     : 1;
        uint32_t opcode : 6;
        uint32_t S      : 1;
        uint32_t op     : 1;
        uint32_t sf     : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);
    assert(imm12 < 4096);

    bitfield.Rd         = dst;
    bitfield.Rn         = src;
    bitfield.imm12      = imm12;
    bitfield.sh         = 0;
    bitfield.opcode     = 0b100010;
    bitfield.S          = 0;
    bitfield.op         = is_add ? 0 : 1;
    bitfield.sf         = 1;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, is_add ? "add x%i, x%i, #%i" : "sub x%i, x%i, #%i", dst, src, imm12);
}

void je_jit_arm_emit_sub_imm12(je_context_t* context, int dst, int src, int imm12) {
    je_jit_arm_emit_add_sub_imm12(context, dst, src, imm12, false);
}

void je_jit_arm_emit_add_imm12(je_context_t* context, int dst, int src, int imm12) {
    je_jit_arm_emit_add_sub_imm12(context, dst, src, imm12, true);
}

void je_jit_arm_emit_add_sub_r32(je_context_t* context, int dst, int reg1, int reg2, bool is_add) {
    struct {
        uint32_t Rd      : 5;
        uint32_t Rn      : 5;
        uint32_t imm6    : 6;
        uint32_t Rm      : 5;
        uint32_t fixed1  : 1;
        uint32_t shift   : 2;
        uint32_t opcode  : 5;
        uint32_t S       : 1;
        uint32_t op      : 1;
        uint32_t sf      : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = dst;
    bitfield.Rn = reg1;
    bitfield.imm6 = 0;
    bitfield.Rm = reg2;
    bitfield.fixed1 = 0;
    bitfield.shift = 0;
    bitfield.opcode = 0b01011;
    bitfield.S = 0;
    bitfield.op = is_add ? 0 : 1;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, is_add ? "add w%i, w%i, w%i" : "sub w%i, w%i, w%i", dst, reg1, reg2);
}

void je_jit_arm_emit_sub_r32(je_context_t* context, int dst, int reg1, int reg2) {
    je_jit_arm_emit_add_sub_r32(context, dst, reg1, reg2, false);
}

void je_jit_arm_emit_add_r32(je_context_t* context, int dst, int reg1, int reg2) {
    je_jit_arm_emit_add_sub_r32(context, dst, reg1, reg2, true);
}

void je_jit_arm_emit_ldp_stp_addr(je_context_t* context, int reg1, int reg2, int dst_addr_reg, bool is_load) {
    struct {
        uint32_t Rt     : 5;
        uint32_t Rn     : 5;
        uint32_t Rt2    : 5;
        uint32_t imm7   : 7;
        uint32_t L      : 1;
        uint32_t opcode : 9;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rt = reg1;
    bitfield.Rn = dst_addr_reg;
    bitfield.Rt2 = reg2;
    bitfield.imm7 = 0;
    bitfield.L = is_load ? 1 : 0;
    bitfield.opcode = 0b101010010;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, is_load ? "ldp x%i, x%i, [x%i]" : "stp x%i, x%i, [x%i]", reg1, reg2, dst_addr_reg);
}

void je_jit_arm_emit_stp_addr(je_context_t* context, int reg1, int reg2, int dst_addr_reg) {
    je_jit_arm_emit_ldp_stp_addr(context, reg1, reg2, dst_addr_reg, false);
}

void je_jit_arm_emit_ldp_addr(je_context_t* context, int reg1, int reg2, int dst_addr_reg) {
    je_jit_arm_emit_ldp_stp_addr(context, reg1, reg2, dst_addr_reg, true);
}

void je_jit_arm_emit_ret(je_context_t* context) {
    struct {
        uint32_t Rm     : 5;
        uint32_t Rn     : 5;
        uint32_t M      : 1;
        uint32_t A      : 1;
        uint32_t fixed1 : 9;
        uint32_t op     : 2;
        uint32_t Z      : 2;
        uint32_t fixed2 : 7;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rm = 0;
    bitfield.Rn = JE_JIT_ARM_REG_X30; // link register
    bitfield.M = 0;
    bitfield.A = 0;
    bitfield.fixed1 = 0b111110000;
    bitfield.op = 0b10;
    bitfield.Z = 0;
    bitfield.fixed2 = 0b1101011;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "ret");
}

void je_jit_arm_emit_blr(je_context_t* context, int addr_reg) {
    struct {
        uint32_t Rm : 5;
        uint32_t Rn : 5;
        uint32_t M : 1;
        uint32_t A : 1;
        uint32_t fixed1 : 9;
        uint32_t op : 2;
        uint32_t Z : 2;
        uint32_t fixed2 : 7;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rm = 0;
    bitfield.Rn = addr_reg;
    bitfield.M = 0;
    bitfield.A = 0;
    bitfield.fixed1 = 0b111110000;
    bitfield.op = 0b01;
    bitfield.Z = 0;
    bitfield.fixed2 = 0b1101011;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "blr x%i", addr_reg);
}

void je_jit_arm_emit_mov(je_context_t* context, int dst, int src) {
    // ARM just encodes this as add dst, src, #0
    je_jit_arm_emit_add_imm12(context, dst, src, 0);
}

void je_jit_arm_emit_orn_r32(je_context_t* context, int dst, int reg1, int reg2) {
    struct {
        uint32_t Rd     : 5;
        uint32_t Rn     : 5;
        uint32_t imm6   : 6;
        uint32_t Rm     : 5;
        uint32_t N      : 1;
        uint32_t shift  : 2;
        uint32_t opc    : 7;
        uint32_t sf     : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = dst;
    bitfield.Rn = reg1;
    bitfield.imm6 = 0;
    bitfield.Rm = reg2;
    bitfield.N = 1;
    bitfield.shift = 0;
    bitfield.opc = 0b0101010;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "orn w%i, w%i, w%i", dst, reg1, reg2);
}

void je_jit_arm_emit_and_r32(je_context_t* context, int dst, int reg1, int reg2) {
    struct {
        uint32_t Rd     : 5;
        uint32_t Rn     : 5;
        uint32_t imm6   : 6;
        uint32_t Rm     : 5;
        uint32_t N      : 1;
        uint32_t shift  : 2;
        uint32_t opc    : 7;
        uint32_t sf     : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = dst;
    bitfield.Rn = reg1;
    bitfield.imm6 = 0;
    bitfield.Rm = reg2;
    bitfield.N = 0;
    bitfield.shift = 0;
    bitfield.opc = 0b0001010;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "and w%i, w%i, w%i", dst, reg1, reg2);
}

void je_jit_arm_emit_orr_r32(je_context_t* context, int dst, int reg1, int reg2) {
    struct {
        uint32_t Rd     : 5;
        uint32_t Rn     : 5;
        uint32_t imm6   : 6;
        uint32_t Rm     : 5;
        uint32_t N      : 1;
        uint32_t shift  : 2;
        uint32_t opc    : 7;
        uint32_t sf     : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = dst;
    bitfield.Rn = reg1;
    bitfield.imm6 = 0;
    bitfield.Rm = reg2;
    bitfield.N = 0;
    bitfield.shift = 0;
    bitfield.opc = 0b0101010;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "orr w%i, w%i, w%i", dst, reg1, reg2);
}

void je_jit_arm_emit_eor_r32(je_context_t* context, int dst, int reg1, int reg2) {
    struct {
        uint32_t Rd     : 5;
        uint32_t Rn     : 5;
        uint32_t imm6   : 6;
        uint32_t Rm     : 5;
        uint32_t N      : 1;
        uint32_t shift  : 2;
        uint32_t opc    : 7;
        uint32_t sf     : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = dst;
    bitfield.Rn = reg1;
    bitfield.imm6 = 0;
    bitfield.Rm = reg2;
    bitfield.N = 0;
    bitfield.shift = 0;
    bitfield.opc = 0b1001010;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "eor w%i, w%i, w%i", dst, reg1, reg2);
}

void je_jit_arm_emit_madd_r32(je_context_t* context, int dst, int reg1, int reg2, int reg3) {
    struct {
        uint32_t Rd      : 5;
        uint32_t Rn      : 5;
        uint32_t Ra      : 5;
        uint32_t o0      : 1;
        uint32_t Rm      : 5;
        uint32_t opcode  : 10;
        uint32_t sf      : 1;   
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = dst;
    bitfield.Rn = reg1;
    bitfield.Ra = reg3;
    bitfield.o0 = 0;
    bitfield.Rm = reg2;
    bitfield.opcode = 0b0011011000;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "madd w%i, w%i, w%i, w%i", dst, reg1, reg2, reg3);
}

void je_jit_arm_emit_sdiv_r32(je_context_t* context, int dst, int reg1, int reg2) {
    struct {
        uint32_t Rd     : 5;
        uint32_t Rn     : 5;
        uint32_t op     : 6;
        uint32_t Rm     : 5;
        uint32_t opcode : 10;
        uint32_t sf     : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = dst;
    bitfield.Rn = reg1;
    bitfield.op = 0b000011;
    bitfield.Rm = reg2;
    bitfield.opcode = 0b0011010110;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "sdiv w%i, w%i, w%i", dst, reg1, reg2);
}

void je_jit_arm_emit_msub_r32(je_context_t* context, int dst, int reg1, int reg2, int reg3) {
    struct {
        uint32_t Rd     : 5;
        uint32_t Rn     : 5;
        uint32_t Ra     : 5;
        uint32_t o0     : 1;
        uint32_t Rm     : 5;
        uint32_t opcode : 10;
        uint32_t sf     : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = dst;
    bitfield.Rn = reg1;
    bitfield.Ra = reg3;
    bitfield.o0 = 1;
    bitfield.Rm = reg2;
    bitfield.opcode = 0b0011011000;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "msub w%i, w%i, w%i, w%i", dst, reg1, reg2, reg3);
}

void je_jit_arm_emit_cmp_r32(je_context_t* context, int reg1, int reg2) {
    // This is just encoded as a sub xzr, x0, x1
    // Just subtracts and discards the result, and sets the relevant flags.

    struct {
        uint32_t Rd      : 5;
        uint32_t Rn      : 5;
        uint32_t imm6    : 6;
        uint32_t Rm      : 5;
        uint32_t fixed1  : 1;
        uint32_t shift   : 2;
        uint32_t opcode  : 5;
        uint32_t S       : 1;
        uint32_t op      : 1;
        uint32_t sf      : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = JE_JIT_ARM_REG_XZR;
    bitfield.Rn = reg1;
    bitfield.imm6 = 0;
    bitfield.Rm = reg2;
    bitfield.fixed1 = 0;
    bitfield.shift = 0;
    bitfield.opcode = 0b01011;
    bitfield.S = 1;
    bitfield.op = 1;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "cmp w%i, w%i", reg1, reg2);
}

void je_jit_arm_emit_cset_r32(je_context_t* context, int reg1, int condition) {
    struct {
        uint32_t Rd      : 5;
        uint32_t Rn      : 5;
        uint32_t o2      : 1;
        uint32_t fixed1  : 1;
        uint32_t cond    : 4;
        uint32_t Rm      : 5;
        uint32_t op      : 10;
        uint32_t sf      : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = reg1;
    bitfield.Rn = JE_JIT_ARM_REG_XZR;
    bitfield.o2 = 1;
    bitfield.fixed1 = 0;
    bitfield.cond = condition;
    bitfield.Rm = JE_JIT_ARM_REG_XZR;
    bitfield.op = 0b0011010100;
    bitfield.sf = 0;

    const char* condition_str = "";
    switch (condition) {
        case 0b1011: condition_str = "l";   break;
        case 0b1101: condition_str = "leq"; break;
        case 0b1100: condition_str = "g";   break;
        case 0b1010: condition_str = "geq"; break;
        case 0b0000: condition_str = "eq";  break;
        case 0b0001: condition_str = "neq"; break;
    }

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "cset w%i, %s", reg1, condition_str);
}

void je_jit_arm_emit_cset_less_r32(je_context_t* context, int reg1) {
    je_jit_arm_emit_cset_r32(context, reg1, 0b1011);
}

void je_jit_arm_emit_cset_less_equal_r32(je_context_t* context, int reg1) {
    je_jit_arm_emit_cset_r32(context, reg1, 0b1101);
}

void je_jit_arm_emit_cset_greater_r32(je_context_t* context, int reg1) {
    je_jit_arm_emit_cset_r32(context, reg1, 0b1100);
}

void je_jit_arm_emit_cset_greater_equal_r32(je_context_t* context, int reg1) {
    je_jit_arm_emit_cset_r32(context, reg1, 0b1010);
}

void je_jit_arm_emit_cset_equal_r32(je_context_t* context, int reg1) {
    je_jit_arm_emit_cset_r32(context, reg1, 0b0000);
}

void je_jit_arm_emit_cset_not_equal_r32(je_context_t* context, int reg1) {
    je_jit_arm_emit_cset_r32(context, reg1, 0b0001);
}

void je_jit_arm_emit_scvtf_r32(je_context_t* context, int vreg, int xreg) {
    struct {
        uint32_t Rd     : 5;
        uint32_t Rn     : 5;
        uint32_t opcode : 9;
        uint32_t rmode  : 2;
        uint32_t fixed1 : 1;
        uint32_t ftype  : 2;
        uint32_t fixed2 : 7;
        uint32_t sf     : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd     = vreg - JE_JIT_ARM_REG_V0;
    bitfield.Rn     = xreg;
    bitfield.opcode = 0b010000000;
    bitfield.rmode  = 0b00;
    bitfield.fixed1 = 1;
    bitfield.ftype  = 0b00;
    bitfield.fixed2 = 0b0011110;
    bitfield.sf     = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "scvtf s%i, w%i", bitfield.Rd, bitfield.Rn);
}

void je_jit_arm_emit_fcvtzs_r32(je_context_t* context, int xreg, int vreg) {
    struct {
        uint32_t Rd     : 5;
        uint32_t Rn     : 5;
        uint32_t opcode : 9;
        uint32_t rmode  : 2;
        uint32_t fixed1 : 1;
        uint32_t ftype  : 2;
        uint32_t fixed2 : 7;
        uint32_t sf     : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd     = xreg;
    bitfield.Rn     = vreg - JE_JIT_ARM_REG_V0;
    bitfield.opcode = 0;
    bitfield.rmode  = 0b11;
    bitfield.fixed1 = 1;
    bitfield.ftype  = 0b00;
    bitfield.fixed2 = 0b0011110;
    bitfield.sf     = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "fcvtzs w%i, s%i", bitfield.Rd, bitfield.Rn);
}

void je_jit_arm_emit_movz_r32_imm16(je_context_t* context, int reg, uint16_t value) {
    struct {
        uint32_t Rd      : 5;
        uint32_t imm16   : 16;
        uint32_t hw      : 2;
        uint32_t opcode  : 8;
        uint32_t sf      : 1; 
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = reg;
    bitfield.imm16 = value;
    bitfield.hw = 0;
    bitfield.opcode = 0b10100101;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "movz w%i, #0x%04x, lsl #%i", reg, value, 0);
}

void je_jit_arm_emit_movk_r32_imm16(je_context_t* context, int reg, uint16_t value, int shift) {
    struct {
        uint32_t Rd      : 5;
        uint32_t imm16   : 16;
        uint32_t hw      : 2;
        uint32_t opcode  : 8;
        uint32_t sf      : 1; 
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = reg;
    bitfield.imm16 = value;
    bitfield.hw = shift / 16;
    bitfield.opcode = 0b11100101;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "movk w%i, #0x%04x, lsl #%i", reg, value, shift);
}

void je_jit_arm_emit_movz_r64_imm16(je_context_t* context, int reg, uint16_t value) {
    struct {
        uint32_t Rd      : 5;
        uint32_t imm16   : 16;
        uint32_t hw      : 2;
        uint32_t opcode  : 8;
        uint32_t sf      : 1; 
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = reg;
    bitfield.imm16 = value;
    bitfield.hw = 0;
    bitfield.opcode = 0b10100101;
    bitfield.sf = 1;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "movz x%i, #0x%04x, lsl #%i", reg, value, 0);
}

void je_jit_arm_emit_movk_r64_imm16(je_context_t* context, int reg, uint16_t value, int shift) {
    struct {
        uint32_t Rd      : 5;
        uint32_t imm16   : 16;
        uint32_t hw      : 2;
        uint32_t opcode  : 8;
        uint32_t sf      : 1; 
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rd = reg;
    bitfield.imm16 = value;
    bitfield.hw = shift / 16;
    bitfield.opcode = 0b11100101;
    bitfield.sf = 1;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "movk x%i, #0x%04x, lsl #%i", reg, value, shift);
}

void je_jit_arm_emit_mov_r32_imm32(je_context_t* context, int reg, uint32_t value) {
    uint16_t high_bits = (value >> 16) & 0xFFFF;
    uint16_t low_bits = value & 0xFFFF;
    je_jit_arm_emit_movz_r32_imm16(context, reg, low_bits);
    je_jit_arm_emit_movk_r32_imm16(context, reg, high_bits, 16);
}

void je_jit_arm_emit_mov_r32_imm64(je_context_t* context, int reg, uint64_t value) {
    uint16_t bits0 = (value >> 48) & 0xFFFF;
    uint16_t bits1 = (value >> 32) & 0xFFFF;
    uint16_t bits2 = (value >> 16) & 0xFFFF;
    uint16_t bits3 = value & 0xFFFF;
    je_jit_arm_emit_movz_r64_imm16(context, reg, bits3);
    je_jit_arm_emit_movk_r64_imm16(context, reg, bits2, 16);
    je_jit_arm_emit_movk_r64_imm16(context, reg, bits1, 32);
    je_jit_arm_emit_movk_r64_imm16(context, reg, bits0, 48);
}

void je_jit_arm_emit_ldr_r32(je_context_t* context, int reg, int addr_reg) {
    struct {
        uint32_t Rt      : 5;
        uint32_t Rn      : 5;
        uint32_t imm12   : 12;
        uint32_t opcode  : 8;
        uint32_t size    : 2;
    } bitfield;
    assert(sizeof(bitfield) == 4);
    
    bitfield.Rt = reg;
    bitfield.Rn = addr_reg;
    bitfield.imm12 = 0;
    bitfield.opcode = 0b11100101;
    bitfield.size = 0b10;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "ldr w%i, [x%i]", reg, addr_reg);
}

void je_jit_arm_emit_ldr_r64(je_context_t* context, int reg, int addr_reg) {
    struct {
        uint32_t Rt      : 5;
        uint32_t Rn      : 5;
        uint32_t imm12   : 12;
        uint32_t opcode  : 8;
        uint32_t size    : 2;
    } bitfield;
    assert(sizeof(bitfield) == 4);
    
    bitfield.Rt = reg;
    bitfield.Rn = addr_reg;
    bitfield.imm12 = 0;
    bitfield.opcode = 0b11100101;
    bitfield.size = 0b11;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "ldr x%i, [x%i]", reg, addr_reg);
}

void je_jit_arm_emit_ldr_s32(je_context_t* context, int reg, int addr_reg) {
    struct {
        uint32_t Rt         : 5;
        uint32_t Rn         : 5;
        uint32_t imm12      : 12;
        uint32_t fixed      : 10;   
    } bitfield;
    assert(sizeof(bitfield) == 4);
    
    bitfield.Rt = reg - JE_JIT_ARM_REG_V0;
    bitfield.Rn = addr_reg;
    bitfield.imm12 = 0;
    bitfield.fixed = 0b1011110101;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "ldr s%i, [x%i]", bitfield.Rt, bitfield.Rn);
}

void je_jit_arm_emit_str_r32(je_context_t* context, int reg, int addr_reg) {
    struct {
        uint32_t Rt     : 5;
        uint32_t Rn     : 5;
        uint32_t imm12  : 12;
        uint32_t opcode : 8;
        uint32_t size   : 2;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rt = reg;
    bitfield.Rn = addr_reg;
    bitfield.imm12 = 0;
    bitfield.opcode = 0b11100100;
    bitfield.size = 0b10;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "str w%i, [x%i]", reg, addr_reg);
}

void je_jit_arm_emit_str_r64(je_context_t* context, int reg, int addr_reg) {
    struct {
        uint32_t Rt     : 5;
        uint32_t Rn     : 5;
        uint32_t imm12  : 12;
        uint32_t opcode : 8;
        uint32_t size   : 2;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rt = reg;
    bitfield.Rn = addr_reg;
    bitfield.imm12 = 0;
    bitfield.opcode = 0b11100100;
    bitfield.size = 0b11;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "str x%i, [x%i]", reg, addr_reg);
}

void je_jit_arm_emit_str_s32(je_context_t* context, int reg, int addr_reg) {
    struct {
        uint32_t Rt     : 5;
        uint32_t Rn     : 5;
        uint32_t imm12  : 12;
        uint32_t opcode : 8;
        uint32_t size   : 2;
    } bitfield;
    assert(sizeof(bitfield) == 4);

    bitfield.Rt = reg - JE_JIT_ARM_REG_V0;
    bitfield.Rn = addr_reg;
    bitfield.imm12 = 0;
    bitfield.opcode = 0b11110100;
    bitfield.size = 0b10;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "str s%i, [x%i]", bitfield.Rt, bitfield.Rn);
}

void je_jit_arm_emit_fneg_r32(je_context_t* context, int reg1, int reg2) {
    struct {
        uint32_t Rd      : 5;
        uint32_t Rn      : 5;
        uint32_t opcode  : 6;
        uint32_t fixed   : 16;
    } bitfield;
    assert(sizeof(bitfield) == 4);
    
    bitfield.Rd = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.Rn = reg2 - JE_JIT_ARM_REG_V0;
    bitfield.opcode = 0b010000;
    bitfield.fixed = 0x1E21;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "fneg s%i, s%i", bitfield.Rd, bitfield.Rn);
}

void je_jit_arm_emit_fmul_r32(je_context_t* context, int reg1, int reg2) {
    struct {
        uint32_t Rd         : 5;
        uint32_t Rn         : 5;
        uint32_t opcode     : 6;
        uint32_t Rm         : 5;
        uint32_t fixed1     : 1;
        uint32_t ftype      : 2;
        uint32_t fixed2     : 8;
    } bitfield;
    assert(sizeof(bitfield) == 4);
    
    bitfield.Rd = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.Rn = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.opcode = 0b000010;
    bitfield.Rm = reg2 - JE_JIT_ARM_REG_V0;
    bitfield.fixed1 = 1;
    bitfield.ftype = 0b00;
    bitfield.fixed2 = 0b00011110;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "fmul s%i, s%i, s%i", bitfield.Rd, bitfield.Rn, bitfield.Rm);
}

void je_jit_arm_emit_fdiv_r32(je_context_t* context, int reg1, int reg2) {
    struct {
        uint32_t Rd         : 5;
        uint32_t Rn         : 5;
        uint32_t opcode     : 6;
        uint32_t Rm         : 5;
        uint32_t fixed1     : 1;
        uint32_t ftype      : 2;
        uint32_t fixed2     : 8;
    } bitfield;
    assert(sizeof(bitfield) == 4);
    
    bitfield.Rd = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.Rn = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.opcode = 0b000110;
    bitfield.Rm = reg2 - JE_JIT_ARM_REG_V0;
    bitfield.fixed1 = 1;
    bitfield.ftype = 0b00;
    bitfield.fixed2 = 0b00011110;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "fdiv s%i, s%i, s%i", bitfield.Rd, bitfield.Rn, bitfield.Rm);
}

void je_jit_arm_emit_fsub_r32(je_context_t* context, int reg1, int reg2) {
    struct {
        uint32_t Rd         : 5;
        uint32_t Rn         : 5;
        uint32_t opcode     : 6;
        uint32_t Rm         : 5;
        uint32_t fixed1     : 1;
        uint32_t ftype      : 2;
        uint32_t fixed2     : 8;
    } bitfield;
    assert(sizeof(bitfield) == 4);
    
    bitfield.Rd = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.Rn = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.opcode = 0b001110;
    bitfield.Rm = reg2 - JE_JIT_ARM_REG_V0;
    bitfield.fixed1 = 1;
    bitfield.ftype = 0b00;
    bitfield.fixed2 = 0b00011110;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "fsub s%i, s%i, s%i", bitfield.Rd, bitfield.Rn, bitfield.Rm);
}

void je_jit_arm_emit_fadd_r32(je_context_t* context, int reg1, int reg2) {
    struct {
        uint32_t Rd         : 5;
        uint32_t Rn         : 5;
        uint32_t opcode     : 6;
        uint32_t Rm         : 5;
        uint32_t fixed1     : 1;
        uint32_t ftype      : 2;
        uint32_t fixed2     : 8;
    } bitfield;
    assert(sizeof(bitfield) == 4);
    
    bitfield.Rd = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.Rn = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.opcode = 0b001010;
    bitfield.Rm = reg2 - JE_JIT_ARM_REG_V0;
    bitfield.fixed1 = 1;
    bitfield.ftype = 0b00;
    bitfield.fixed2 = 0b00011110;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "fadd s%i, s%i, s%i", bitfield.Rd, bitfield.Rn, bitfield.Rm);
}

void je_jit_arm_emit_fcmp_r32(je_context_t* context, int reg1, int reg2) {
    struct {
        uint32_t opc        : 5;
        uint32_t Rn         : 5;
        uint32_t opcode     : 6;
        uint32_t Rm         : 5;
        uint32_t fixed1     : 1;
        uint32_t ftype      : 2;
        uint32_t fixed2     : 8;
    } bitfield;
    assert(sizeof(bitfield) == 4);
    
    bitfield.opc = 0;
    bitfield.Rn = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.opcode = 0b001000;
    bitfield.Rm = reg2 - JE_JIT_ARM_REG_V0;
    bitfield.fixed1 = 1;
    bitfield.ftype = 0b00;
    bitfield.fixed2 = 0b00011110;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "fcmp s%i, s%i", bitfield.Rn, bitfield.Rm);
}

void je_jit_arm_emit_fmov_r32(je_context_t* context, int reg1, int reg2) {
    struct {
        uint32_t Rd       : 5;
        uint32_t Rn       : 5;
        uint32_t fixed1   : 6;
        uint32_t opcode   : 3;
        uint32_t rmode    : 2;
        uint32_t fixed2   : 1;
        uint32_t ftype    : 2;
        uint32_t fixed3   : 7;
        uint32_t sf       : 1;
    } bitfield;
    assert(sizeof(bitfield) == 4);
    
    bitfield.Rd = reg1 - JE_JIT_ARM_REG_V0;
    bitfield.Rn = reg2;
    bitfield.fixed1 = 0;
    bitfield.opcode = 0b111;
    bitfield.rmode = 0b00;
    bitfield.fixed2 = 1;
    bitfield.ftype = 0b00; // Single precision
    bitfield.fixed3 = 0b0011110;
    bitfield.sf = 0;

    je_jit_start_instruction(context);
    je_jit_emit_bytes(context, (uint8_t*)&bitfield, 4);
    je_jit_end_instruction(context, "fmov s%i, x%i", bitfield.Rd, bitfield.Rn);
}

int je_jit_arm_alloc_x_reg(je_context_t* context) {
    int gp_registers[9] = {
        JE_JIT_ARM_REG_X9,
        JE_JIT_ARM_REG_X10,
        JE_JIT_ARM_REG_X11,
        JE_JIT_ARM_REG_X12,
        JE_JIT_ARM_REG_X13,
        JE_JIT_ARM_REG_X14,
        JE_JIT_ARM_REG_X15,
        JE_JIT_ARM_REG_X16,
        JE_JIT_ARM_REG_X17    
    };
    int gp_registers_num = sizeof(gp_registers) / sizeof(*gp_registers);

    int alloc_index = context->jit_register_allocation_counter++;

    // Find one thats available.
    for (int i = 0; i < gp_registers_num; i++) {
        int reg = gp_registers[i];
        if (context->jit_register_allocation[reg].alloc_count == 0) {
            context->jit_register_allocation[reg].alloc_count++;
            context->jit_register_allocation[reg].alloc_index = alloc_index;
            return reg;
        }
    }

    // We don't support spilling registers on arm as we have soo many gp registers. 
    // If we get to this point something has gone wrong in our code generation.
    assert(false);
    return 0;
}

int je_jit_arm_alloc_v_reg(je_context_t* context) {
    int gp_registers[9] = {
        JE_JIT_ARM_REG_V0,
        JE_JIT_ARM_REG_V1,
        JE_JIT_ARM_REG_V2,
        JE_JIT_ARM_REG_V3,
        JE_JIT_ARM_REG_V4,
        JE_JIT_ARM_REG_V5,
        JE_JIT_ARM_REG_V6,
        JE_JIT_ARM_REG_V7,
        JE_JIT_ARM_REG_V8
    };
    int gp_registers_num = sizeof(gp_registers) / sizeof(*gp_registers);

    int alloc_index = context->jit_register_allocation_counter++;

    // Find one thats available.
    for (int i = 0; i < gp_registers_num; i++) {
        int reg = gp_registers[i];
        if (context->jit_register_allocation[reg].alloc_count == 0) {
            context->jit_register_allocation[reg].alloc_count++;
            context->jit_register_allocation[reg].alloc_index = alloc_index;
            return reg;
        }
    }

    // We don't support spilling registers on arm as we have soo many gp registers. 
    // If we get to this point something has gone wrong in our code generation.
    assert(false);
    return 0;
}

void je_jit_arm_free_reg(je_context_t* context, int reg) {
    context->jit_register_allocation[reg].alloc_count--;
}

bool je_jit_arm_reg_allocated(je_context_t* context, int reg) {
    return (context->jit_register_allocation[reg].alloc_count > 0);
}

void je_jit_arm_emit_prologue(je_context_t* context) {
    // Link the frame record
    je_jit_emit_comment(context, "Prologue");
    je_jit_arm_emit_sub_imm12(context, JE_JIT_ARM_REG_SP, JE_JIT_ARM_REG_SP, 16);
    je_jit_arm_emit_stp_addr(context, JE_JIT_ARM_REG_X29, JE_JIT_ARM_REG_X30, JE_JIT_ARM_REG_SP);
    je_jit_arm_emit_mov(context, JE_JIT_ARM_REG_X29, JE_JIT_ARM_REG_SP);
}

void je_jit_arm_emit_epilogue(je_context_t* context, int return_reg) {   
    je_jit_emit_comment(context, "Store return value");

    // Move the return value to the result struct.
    context->result.type = context->ast_root->return_type;
    switch (context->ast_root->return_type) {
        case JE_TYPE_INT: {
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)&context->result.int_value;
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_str_r32(context, return_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            break;
        }
        case JE_TYPE_BOOL: {
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)&context->result.bool_value;
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_str_r32(context, return_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            break;
        }
        case JE_TYPE_FLOAT: {
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)&context->result.float_value;
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_str_s32(context, return_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            break;
        }
        case JE_TYPE_STRING: {
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)&context->result.string_value;
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_str_r64(context, return_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            break;
        }
    }

    // Unlink the frame record
    je_jit_emit_comment(context, "Epilogue");
    je_jit_arm_emit_ldp_addr(context, JE_JIT_ARM_REG_X29, JE_JIT_ARM_REG_X30, JE_JIT_ARM_REG_SP);
    je_jit_arm_emit_add_imm12(context, JE_JIT_ARM_REG_SP, JE_JIT_ARM_REG_SP, 16);
    je_jit_arm_emit_ret(context);
}
 
// Return value is the register the result is in if applicable.
int je_jit_arm_emit_function_call(je_context_t* context, je_ast_node_t* node) {

    je_func_def_t* function = je_get_ast_node_function(context, node);

    // Store every parameter in the context.
    for (int i = 0; i < function->param_count; i++) {
        je_ast_node_t* child = je_get_node_child(context, node, i);
        int reg1 = je_jit_arm_emit_node(context, child);
        int type = je_get_func_param_type(context, function, i);

        je_jit_emit_comment(context, "Storing %s parameter %i", je_type_name(type), i);
        switch (type) {
            case JE_TYPE_INT: {
                int addr_reg = je_jit_arm_alloc_x_reg(context);
                uint64_t address = (uint64_t)&context->function_params[i].int_value;
                je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
                je_jit_arm_emit_str_r32(context, reg1, addr_reg);
                je_jit_arm_free_reg(context, addr_reg);
                break;
            }
            case JE_TYPE_BOOL: {
                int addr_reg = je_jit_arm_alloc_x_reg(context);
                uint64_t address = (uint64_t)&context->function_params[i].bool_value;
                je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
                je_jit_arm_emit_str_r32(context, reg1, addr_reg);
                je_jit_arm_free_reg(context, addr_reg);
                break;
            }
            case JE_TYPE_FLOAT: {
                int addr_reg = je_jit_arm_alloc_x_reg(context);
                uint64_t address = (uint64_t)&context->function_params[i].float_value;
                je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
                je_jit_arm_emit_str_s32(context, reg1, addr_reg);
                je_jit_arm_free_reg(context, addr_reg);
                break;
            }
            case JE_TYPE_STRING: {
                int addr_reg = je_jit_arm_alloc_x_reg(context);
                uint64_t address = (uint64_t)&context->function_params[i].string_value;
                je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
                je_jit_arm_emit_str_r64(context, reg1, addr_reg);
                je_jit_arm_free_reg(context, addr_reg);
                break;
            }
        }

        je_jit_arm_free_reg(context, reg1);
    }

#if defined(JE_CALLING_CONVENTION_ARM64)
    int active_func_addr_reg = je_jit_arm_alloc_x_reg(context);
    int func_addr_reg = je_jit_arm_alloc_x_reg(context);

    // Store the active_function pointer
    je_jit_emit_comment(context, "Storing active_function");
    uint64_t address = (uint64_t)&context->active_function;
    je_jit_arm_emit_mov_r32_imm64(context, active_func_addr_reg, address);
    je_jit_arm_emit_mov_r32_imm64(context, func_addr_reg, (uint64_t)function);
    je_jit_arm_emit_str_r64(context, func_addr_reg, active_func_addr_reg);

    // Push the context argument which goes into X0
    je_jit_emit_comment(context, "Passing context argument");
    address = (uint64_t)context;
    je_jit_arm_emit_mov_r32_imm64(context, JE_JIT_ARM_REG_X0, address);

    // Call function
    je_jit_emit_comment(context, "Calling function");
    address = (uint64_t)function->function;
    je_jit_arm_emit_mov_r32_imm64(context, func_addr_reg, address);
    je_jit_arm_emit_blr(context, func_addr_reg);

    je_jit_arm_free_reg(context, func_addr_reg);
    je_jit_arm_free_reg(context, active_func_addr_reg);
#else
    #error compiler-specific implementation required
#endif

    // Move result into return register.
    int ret_reg = 0;

    je_jit_emit_comment(context, "Retrieving %s return value", je_type_name(function->return_type));
    switch (function->return_type) {
        case JE_TYPE_INT: {
            ret_reg = je_jit_arm_alloc_x_reg(context);
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)&context->function_result.int_value;
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_ldr_r32(context, ret_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            break;
        }
        case JE_TYPE_BOOL: {
            ret_reg = je_jit_arm_alloc_x_reg(context);
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)&context->function_result.bool_value;
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_ldr_r32(context, ret_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            break;
        }
        case JE_TYPE_FLOAT: {
            ret_reg = je_jit_arm_alloc_v_reg(context);
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)&context->function_result.float_value;
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_ldr_s32(context, ret_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            break;
        }
        case JE_TYPE_STRING: {
            ret_reg = je_jit_arm_alloc_x_reg(context);
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)&context->function_result.string_value;
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_ldr_r64(context, ret_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            break;
        }
    }

    return ret_reg;
}

// Return value is the register the result is in if applicable.
int je_jit_arm_emit_node(je_context_t* context, je_ast_node_t* node) {
    je_jit_emit_comment(context, "%s", je_node_name(node->type));
    switch (node->type) {
        // ------------------------------------------------------------------------------
        // Integer operations
        // ------------------------------------------------------------------------------
        case JE_NODE_BITWISE_NOT_INT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_orn_r32(context, reg1, JE_JIT_ARM_REG_XZR, reg1);
            return reg1;
        }
        case JE_NODE_MUL_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_madd_r32(context, reg1, reg1, reg2, JE_JIT_ARM_REG_XZR);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_MOD_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            int reg3 = je_jit_arm_alloc_x_reg(context);
            je_jit_arm_emit_sdiv_r32(context, reg3, reg1, reg2);
            je_jit_arm_emit_msub_r32(context, reg3, reg3, reg2, reg1);
            je_jit_arm_free_reg(context, reg2);
            je_jit_arm_free_reg(context, reg1);
            return reg3;
        }
        case JE_NODE_DIV_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_sdiv_r32(context, reg1, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_SUB_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_sub_r32(context, reg1, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_ADD_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_add_r32(context, reg1, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_LESS_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_cmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_less_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_LE_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_cmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_less_equal_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_GREATER_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_cmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_greater_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_GE_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_cmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_greater_equal_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_EQUAL_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_cmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_equal_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_NOT_EQUAL_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_cmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_not_equal_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_BITWISE_AND_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_and_r32(context, reg1, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_BITWISE_OR_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_orr_r32(context, reg1, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_BITWISE_XOR_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_eor_r32(context, reg1, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_VARIABLE_INT: {
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            int dst_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)je_get_variable_int(je_get_ast_node_variable(context, node));
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_ldr_r32(context, dst_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            return dst_reg;
        }
        case JE_NODE_NEG_INT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_sub_r32(context, reg1, JE_JIT_ARM_REG_XZR, reg1);
            return reg1;
        }
        case JE_NODE_POS_INT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            // TODO: This is basically a nop, why do we have it?
            return reg1;
        }
        case JE_NODE_INT_LITERAL: {
            int reg = je_jit_arm_alloc_x_reg(context);
            uint32_t value = *((uint32_t*)je_get_ast_node_int(context, node));
            je_jit_arm_emit_mov_r32_imm32(context, reg, value);
            return reg;
        }
        case JE_NODE_CAST_INT_TO_FLOAT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            int reg2 = je_jit_arm_alloc_v_reg(context);
            je_jit_arm_emit_scvtf_r32(context, reg2, reg1);
            je_jit_arm_free_reg(context, reg1);
            return reg2;
        }
        case JE_NODE_CAST_INT_TO_BOOL: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_cmp_r32(context, reg1, JE_JIT_ARM_REG_XZR);
            je_jit_arm_emit_cset_not_equal_r32(context, reg1);
            return reg1;
        }
        case JE_NODE_FUNCTION_CALL_INT: {
            return je_jit_arm_emit_function_call(context, node);
        }

        // ------------------------------------------------------------------------------
        // Boolean Operations
        // ------------------------------------------------------------------------------
        case JE_NODE_LOGICAL_NOT_BOOL: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_orn_r32(context, reg1, JE_JIT_ARM_REG_XZR, reg1);
            return reg1;
        }
        case JE_NODE_LOGICAL_AND_BOOL: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_and_r32(context, reg1, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_LOGICAL_OR_BOOL: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_orr_r32(context, reg1, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_EQUAL_BOOL: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_cmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_equal_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_NOT_EQUAL_BOOL: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_cmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_not_equal_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_VARIABLE_BOOL: {
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            int dst_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)je_get_variable_bool(je_get_ast_node_variable(context, node));
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_ldr_r32(context, dst_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            return dst_reg;
        }
        case JE_NODE_BOOL_LITERAL: {
            int reg = je_jit_arm_alloc_x_reg(context);
            uint32_t value = *((uint32_t*)je_get_ast_node_bool(context, node));
            je_jit_arm_emit_mov_r32_imm32(context, reg, value);
            return reg;
        }
        case JE_NODE_CAST_BOOL_TO_INT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            // This is essentially a nop, they are stored identically.
            return reg1;
        }
        case JE_NODE_CAST_BOOL_TO_FLOAT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            int reg2 = je_jit_arm_alloc_v_reg(context);
            je_jit_arm_emit_scvtf_r32(context, reg2, reg1);
            je_jit_arm_free_reg(context, reg1);
            return reg2;
        }
        case JE_NODE_FUNCTION_CALL_BOOL: {
            return je_jit_arm_emit_function_call(context, node);
        }

        // ------------------------------------------------------------------------------
        // Float Operations
        // ------------------------------------------------------------------------------
        case JE_NODE_MUL_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fmul_r32(context, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_DIV_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fdiv_r32(context, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_SUB_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fsub_r32(context, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_ADD_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fadd_r32(context, reg1, reg2);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_LESS_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fcmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_less_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_GREATER_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fcmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_greater_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_LE_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fcmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_less_equal_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_GE_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fcmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_greater_equal_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_EQUAL_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fcmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_equal_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_NOT_EQUAL_FLOAT: {
            je_ast_node_t* lvalue = je_get_node_child(context, node, 0);
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, lvalue);
            int reg2 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fcmp_r32(context, reg1, reg2);
            je_jit_arm_emit_cset_not_equal_r32(context, reg1);
            je_jit_arm_free_reg(context, reg2);
            return reg1;
        }
        case JE_NODE_VARIABLE_FLOAT: {
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            int dst_reg = je_jit_arm_alloc_v_reg(context);
            uint64_t address = (uint64_t)je_get_variable_float(je_get_ast_node_variable(context, node));
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_ldr_s32(context, dst_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            return dst_reg;
        }
        case JE_NODE_NEG_FLOAT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            je_jit_arm_emit_fneg_r32(context, reg1, reg1);
            return reg1;
        }
        case JE_NODE_POS_FLOAT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            // TODO: This is basically a nop, why do we have it?
            return reg1;
        }
        case JE_NODE_FLOAT_LITERAL: {
            int reg1 = je_jit_arm_alloc_x_reg(context);
            int reg2 = je_jit_arm_alloc_v_reg(context);
            uint32_t value = *((uint32_t*)je_get_ast_node_float(context, node));
            je_jit_arm_emit_mov_r32_imm32(context, reg1, value);
            je_jit_arm_emit_fmov_r32(context, reg2, reg1);
            je_jit_arm_free_reg(context, reg1);
            return reg2;
        }
        case JE_NODE_CAST_FLOAT_TO_INT: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            int reg2 = je_jit_arm_alloc_x_reg(context);
            je_jit_arm_emit_fcvtzs_r32(context, reg2, reg1);
            je_jit_arm_free_reg(context, reg1);
            return reg2;
        }
        case JE_NODE_CAST_FLOAT_TO_BOOL: {
            je_ast_node_t* rvalue = je_get_node_child(context, node, 1);
            int reg1 = je_jit_arm_emit_node(context, rvalue);
            int reg2 = je_jit_arm_alloc_x_reg(context);
            je_jit_arm_emit_fcvtzs_r32(context, reg2, reg1);
            je_jit_arm_free_reg(context, reg1);
            return reg2;
        }
        case JE_NODE_FUNCTION_CALL_FLOAT: {
            return je_jit_arm_emit_function_call(context, node);
        }

        // ------------------------------------------------------------------------------
        // String Operations
        // ------------------------------------------------------------------------------
        case JE_NODE_VARIABLE_STRING: {
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            int dst_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)je_get_variable_string(je_get_ast_node_variable(context, node));
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            je_jit_arm_emit_ldr_r64(context, dst_reg, addr_reg);
            je_jit_arm_free_reg(context, addr_reg);
            return dst_reg;
        }
        case JE_NODE_STRING_LITERAL: {
            int addr_reg = je_jit_arm_alloc_x_reg(context);
            uint64_t address = (uint64_t)*je_get_ast_node_string(context, node);
            je_jit_arm_emit_mov_r32_imm64(context, addr_reg, address);
            return addr_reg;
        }
        case JE_NODE_FUNCTION_CALL_STRING: {
            return je_jit_arm_emit_function_call(context, node);
        }

    }
    assert(false);
    return 0;
}

#endif // JE_ISA_X86/64/arm32/arm64
#endif // JE_JIT_AVAILABLE

#endif

#ifdef __cplusplus
} // extern "C" {
#endif

#endif // JITEVAL_H