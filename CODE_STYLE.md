## Coding Style
When you are going to contribute C code, please follow these coding style guidelines.

### Basics
All text, comments as well as variables or function names, are in English.

Use 4 spaces for indentation. Do not use tabs.

There is no strict limit for the line length. However, try to keep lines in a reasonable length (100 characters). A good reason to exceed the limit is, for example, a long string literal or keeping ternary expression on a single line. On the other hand, long lines caused by too many function parameters or too many indentation levels should be broken or the code should be refactored.

**More than 5 levels of indentation indicate a bad structure of the code and code refactoring should be considered.**

**Put at least one blank line between function definitions. More lines can be used e.g. to separate various groups of functions.**

No trailing spaces on lines. Use only the UNIX-style linebreaks (\n), not Windows-style (\r\n).

**Do not mix declarations and code within a block. It is also recommended to declare variables at the beginning of the innermost block to better clarify variable lifetime and keep them close to their use. This makes easier to find the variable type and initial value.**

**Declaration of each variable on a separate line is preferred. Only closely related variables of the same type could be declared together on a singleline.**

**Declare variables and function parameters as const if their value is not supposed to be changed during their lifetime.**

End source file with a newline character (most editors do that automatically).

Use -Wall and -Wextra compiler options. Treat compiler warnings as errors - if there is a warning, something is wrong.

### Naming
**Use underscores to separate words in an identifier (aka snake_case): multi_word_name.**

Use lowercase for most names. Use uppercase for macros and members of enumerations.

**Use names that explain the purpose of a function or object. Think twice when naming some important component so the role or the function of the component is clear across the whole code and it does not confuse with other components.**

Do not use names that begin with \_. If you need a name for "internal use only", use \_\_ as a suffix instead of a prefix.

**Try to avoid negative names - found is better than not_found.**

Consider a plural form of the name in case of naming arrays. There are natural exceptions to this consideration, such as const char *buffer.

**All (non-static) functions, typedefs, structures, unions and enums that represent interface of a C component within the same context must start with the same prefix:**
```
int bitset_create(...);

int bitset_destroy(...);

int bitset_set(...);
```
### Source and Header Files
Each source file must contain a Doxygen comment, with @file command (do not put a file name argument until providing documentation to a different file). The comment is supposed to provide, at least, a brief explanation of the purpose of the code and authors that participated on the code. Actually also some note regarding the license is present.
```
/**
 * @file
 * @author <name> <email>
 * @brief Schema tree implementation
 *
 * Copyright (c) to asi nehrozi xd
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
 ```
The #include directives should appear in the following order:

1. Own headers of the module/library/application, if any.
2. Standard C library headers and other system headers, preferably in alphabetical order. Occasionally one encounters a set of system headers that must be included in a particular order, in which case that order must take precedence.
3. Generic (non-standard) library headers.

The list of #include directives should be minimal (include only what you really use in the specific file). That applies to *.c source files and especially to header files. Including unnecessary files hurts compile-time performance. On the other hand, all the header files should be self-contained - i.e. they should compile on their own without requiring to include any additional header. Keeping the list minimal and self-containing can be a challenging task when the code changes, so there is [Include What You Use tool](https://include-what-you-use.org/ "Include What You Use tool") to help you. It is also useful to add a comment for a specific include in case it was added for a specific single declaration.

Each header file must contain an include guard to make the header file idempotent. Names of the guards are uppercase and they should be specific for the project by prefix or include path (CONFIG_H is not a good guard name). Characters /, ., and - in the name are substituted by an underscore (_).

Use the following exact form for include guards. GCC, and clang, recognize this idiom and avoid re-reading headers that use it. To avoid confusing GCC's and clang's header optimization, do not include any code before or after the include guards (comments and whitespace are OK). Do not combine any othe r preprocessor checks in the #ifndef <guard> expression.
```
#ifndef <guard>
#define <guard>
... All code ...
#endif /* <guard> */
```
Do not define the members of a struct or union in a (mainly public) header files, unless client code is actually intended to access them directly or if the definition is otherwise actually needed.

### Comments
**Focus mainly on what the code is supposed to do instead of how.**

**Use FIXME and TODO comments to mark code that needs some additional work.**

Use Doxygen to generate documentation of the code. Doxygen structural commands start with an at-sign (@). Remember to document the source file itself using the @file command. The only non-Doxygen comments should be inline comments describing the code snippets. Description of all functions, macros, unions, structures and their members, enums or types are written as Doxygen comments no matter if the object is part of public API or not.

Multiline comments are always in form of `/* ... */`. Doxygen comments start with `/**`.

For the case of single line comments, both formats (`/* ... */` as well as `// ...`) are allowed. However, keep consistency of the format within a single header/source file not only for the single line comments, but also for single line Doxygen comments (used e.g. for members of a structure). So, use together:

- `// ... and ///< ...`
- `/* ... */ and /**< ... */`

Provide a Doxygen comment before each function. Put the comment before a function declaration in a header file or before a function definition in case of static functions. The comment describes

- purpose of the function,
- possible limitations of its usage or side effects,
- each parameter of the function, including the attribute specifying the direction of the parameter ([in], [in,out] or [out]),
- return value.
```
/**
 * @brief Get a value of an unsigned integer.
 *
 * The @p value is read from a data @p field and converted from
 * "network byte order" to "host byte order".
 * @param[in]  field  Pointer to the data field (in "network byte order")
 * @param[in]  size   Size of the data field (min: 1 byte, max: 8 bytes)
 * @param[out] value  Pointer to a result variable
 * @return #CONVERTER_OK on success and fills the @p value.
 * @return #CONVERTER_ERR_ARG on failure (usually incorrect size of the field)
 *   and the @p value is not filled.
 */
int converter_get_int(const void *field, size_t size, uint64_t *value);
```
**Preprocessor directives #if 0 ... #endif can be used to comment out a block of code, but this is allowed only in development branches. Such blocks should not appear in branches intended for production.**

### Functions TODO edit this to match our preference
For both, function declaration as well as function definition, put the return type and function name with parameters list on separate lines. Opening brace in case of a function definition is also placed on a separate line. All these lines start in column 0.
```
static char * foo(const char *arg)
{
    ...
}
```
When you need to put the function parameters on multiple lines, start a new line with double indentation (with 8 spaces).
```
static int my_function(struct my_struct *p1, struct another_struct *p2,
        int size)
{
    ...
}
```
In the absence of good reasons for another order, the following parameter order is preferred. One notable exception is that data parameters and their corresponding size parameters should be paired.

1. The primary object being manipulated, if any (equivalent to the "this" pointer in C++).
2. Input-only parameters.
3. Input/output parameters.
4. Output-only parameters.
5. Status parameter.

Functions that destroy an instance of a dynamically-allocated type should accept and ignore a null pointer argument with no error. Code that calls such a function (including the C standard library function free()) should omit a null-pointer check. We find that this usually makes code easier to read.

Functions should not be explicitly marked inline, because it does not usually help code generation and it does suppress compilers warnings about unused functions. The only reasonable exception is in case the function is defined in a header file (which is very uncommon and there should be a good reason for it).

In public functions, always check the validity of input parameters and appropriately log and provide return code. For internal use, assert() can be more appropriate. Put some kind of error message in the assertion statement, which is printed if the assertion is tripped. This helps the poor debugger make sense of why an assertion is being made and enforced, and hopefully what to do about it.
```
const char * get_operand(struct operands *ops, int index)
{
    assert(ops && "get_operand() - ops cannot be NULL.");
    assert(index < ops->size && "get_operand() - index is out of range.");
    return ops->items[index];
}
```

### Types
Use typedefs sparingly. The code is clearer if the actual type is visible at the point of declaration. Do not declare a typedef for a pointer type, because this can be very confusing to the reader. A good reason for typedef can be to hide members of a structure in public API. A function type is also a good use for a typedef because it can clarify code. Again, the type should be a function type, not a pointer-to-function type. That way, the typedef name can be used to declare function prototypes (it cannot be used for function definitions because that is explicitly prohibited by C89 and C99).

Use the `int<N>_t` and `uint<N>_t` types from `<stdint.h>` for exact-width integer types. Use the PRId<N>, PRIu<N>, and PRIx<N> macros from <inttypes.h> for formatting them with printf() and related functions.

Bind a pointer declarator to the variable name, not the type name. Write int *x, not int* x and definitely not int * x.

Use bit-fields sparingly. Do not use bit-fields for a layout of network protocol fields or in other circumstances where the exact format is important to share the data with some other code. There are several properties (whether they overlap, their alignment, etc.) of bit fields that are implementation-defined.

Try to order structure members such that they pack well on a system with 2-byte short, 4-byte int, and 4- or 8-byte long and pointer types. Prefer clear organization over size-optimization unless you are convinced there is a size or speed benefit.


### Statements
Write only one statement per line.

**Avoid magic constants (e.g. when computing size for memory allocation). If a constant is used repeatedly, declare it as a macro, but, more importantly, always comment why the constant is used and what it represents.**

Put single space between if, while, for, etc. statements and the expression that follow them. On the other hand, function calls have no space between the function name and the opening parenthesis.

Enclose even a single statement in if, while, for, etc. into braces.
```
if (ret != 0) {
    return ret;
}
```
Opening code block brace is kept at the same line with the if, while, for or switch statements.
```
if (a) {
    x = exp(a);
} else {
    return 1;
}
```
Start switch's cases in the same column as the switch.
```
switch (conn->state) {
case 0:
    return "data found";
case 1:
    return "data not found";
default:
    return "unknown error";
}
```
Use `for (;;)` or `while (1)` to write an infinite loop.

Do not put gratuitous parentheses around the expression in a return statement, that is, write return 0; and not return(0);

If it helps to clarify the meaning of if and else branches (what can the reader expect in the corresponding branch), write comments into the block of the appropriate branch (do not join them into a single comment in one of the branches). If one branch is the normal or common case and the other branch is the uncommon or error case, put the common case after the if, not the else branch. This is a form of documentation. It also places the most important code in sequential order without forcing the reader to visually skip past less important details. Some compilers also assume that the "if" branch is the more common case so this can be a real form of optimization as well.

Don't put an else right after a (unconditional) return/break/continue/etc. Delete the else, it's unnecessary and increases indentation level.
```
int give_number()
{
    int x;
    ... some magic with x ...
    if (x &gt; 100) {
        ... some other magic with x ...
        return x;
    }
    ... some magic for small numbers ...
    return x;
}
```
Break and continue early to simplify code and reduce indentation. Instead of
```
while (1) {
    if (!x->is_terminator() && x->has_one_use()
            && do_other_thing(x)) {
        ... some long code ....
    }
}
```
better write:
```
while (1) {
    /* Terminators never need 'something' done to them because ... */
    if (x-&gt;is_terminator()) {
        return 0;
    }

    /* We conservatively avoid transforming instructions with multiple uses
     * because goats like cheese. */
    if (!x-&gt;has_one_use()) {
        return 0;
    }

    /* This is really just here for example. */
    if (!do_other_thing(X)) {
        return 0;
    }

  ... some long code ...
}
```
Use comments and blank lines to divide long functions into logical groups of statements.

### Expressions
Put one space on each side of infix binary and ternary operators:

`* / % + - << >> < <= > >= == != & ^ | && || ?: = += -= *= /= %= &= ^= |= <<= >>=`
If breaking line inside the if or while condition, add another 2 indentation levels for the additional line(s).

Do not parenthesize the operands of && and || unless operator precedence makes it necessary, or unless the operands are themselves expressions that use && and ||. Break the lines rather before && and || than after them. Thus:
```
if (!isdigit((unsigned char) s[0])
        || !isdigit((unsigned char) s[1])
        || !isdigit((unsigned char) s[2])) {
    printf("string %s does not start with 3-digit code\n", s);
}
```
but
```
if (rule && (!best || rule->priority > best->priority)) {
    best = rule;
}
```
Do parenthesize a subexpression that must be split across more than one line.
```
*idxp = ((l1_idx << PORT_ARRAY_L1_SHIFT)
         | (l2_idx << PORT_ARRAY_L2_SHIFT)
         | (l3_idx << PORT_ARRAY_L3_SHIFT));
```
When using a relational operator like < or ==, put an expression or variable on the left and a literal on the right - write x 0, not 0 x.

Do not put any white space around postfix, prefix, or grouping operators with exception notes below.
`() [] -> . ! ~ ++ -- + - * &`
- Put a space between the () used in a cast and the expression whose type is cast: (void *) 0.
- The `sizeof` operator is unique among C operators in that it accepts two very different kinds of operands: an expression or a type. In general, prefer to specify an expression: int *x = calloc(1, sizeof *x);
When the operand of sizeof is an expression, there is no need to parenthesize that operand, and please don't. There is an exception to this rule when you need to work with partially compatible structures:
```
struct a_s {
   uint8_t type;
}
struct b_s {
   uint8_t type;
   char *str;
}
struct c_s {
   uint8_t type;
   uint8_t *u8;
}
...
struct a_s *a;
switch (type) {
case 1:
    a = (struct a_s *) calloc(1, sizeof(struct b_s));
    break;
case 2:
    a = (struct a_s *) calloc(1, sizeof(struct c_s));
    break;
    ...
```
Goto statements should be used sparingly, but it is not essentially bad. The accompanying label should be alone on a line and to the left of the code that follows. The main places where it can be usefully employed are:

- to unify reaction to a similar situation coming from different places.
```
    for (...) {
        while (...) {
        ...
            if (disaster) {
                goto error;
            } 
        ...
        if (another_disaster) {
            goto error;
        }
    }
    ...
error:
    clean up the mess
```
- Error recovery while allocating resources:
```
void foo(void)
{
    if (!allocate_A()) {
        goto exit;
    }
    if (!allocate_B()) {
        goto cleanup_A;
    }
    if (!allocate_C()) {
        goto cleanup_B;
    }
    /* allocation has succeeded */
    compute();
    free_C();
cleanup_B:
    free_B();
cleanup_A:
    free_A();
exit:
    return;
}
```
- To break out of nested loops.
```
    for (...) {
        ...
        while (...) {
            ...
            if (something_breaking_outer_loop) {
                goto processing;
            }
        }
        ...
    }
processing:
    ...
```
Use ternary operator only to provide a value. Do not use it to replace if-else construct. If necessary to break the lines inside the expression with the ternary operator, break the lines rather before ? and/or : than after them.
```
return (x > y
        ? foo(x)
        : foo(y));
```
Use calloc() to initialize allocated memory instead of memset(). If the members of an allocated structure are supposed to be initiated to non-zero values, malloc() should be preferred to help compilers and other tools to detect uninitialized variables.
