# RPX: RPN eXtended Calculator

## Table of Contents
1. [Installation](#installation)
2. [Introduction](#introduction)
3. [Basic Concepts](#basic-concepts)
4. [Modes of Operation](#modes-of-operation)
5. [Input Format](#input-format)
6. [Operators and Functions](#operators-and-functions)
7. [Special Features](#special-features)
8. [Commands](#commands)
9. [Examples](#examples)
10. [Error Handling](#error-handling)
11. [Advanced Topics](#advanced-topics)

## Dependencies
### Required
- bash
- git
- make
- clang
- lld
- coreutils
### Optional
- ccache
- clang-format
- clang-tidy
- doxygen
- zig

## Installation
```
$ sudo make install
```

## Introduction
RPX (RPN eXtended) is a powerful calculator that uses Reverse Polish Notation (RPN). It supports real, complex, and matrix calculations, offering a wide range of mathematical operations and functions.

## Basic Concepts
- Uses Reverse Polish Notation (RPN)
- No parallel processing
- No GUI
- Low keystrokes

## Modes of Operation
RPX offers four modes of operation:

1. **Real Number Mode** (default)
   - Supports floating-point operations and transcendental functions
2. **Complex Number Mode**
   - Toggle with `:tc` command
   - Supports complex number operations
   - Supports matrix operations

## Input Format
- Whitespace is required for number separators, but ignored elsewhere
- Negative numbers: Use 'm' operator (e.g., `5 m` for -5)
- Decimal numbers are supported
- Complex numbers: Use 'i' operator (e.g., `3m 4 i +` for -3+4i)
- Polar coordinates: Use 'p' operator (e.g., `1(\P2/)p` for 1i)
- Matrices: Use `[columns elements,]` format (e.g., `[2 1,2,3,4,]` for a 2x2 matrix)
- Spaces are required only to delimit numbers (e.g., `5 3m4i+` for 5-3+4i)

## Operators and Functions
### Basic Arithmetic Operators
- `+` (addition), `-` (subtraction), `*` (multiplication), `/` (division)
- `%` (modulo), `^` (exponentiation)
- `=`, `<`, `>`, (comparison)

### Unary Functions
- `A` (absolute value)
- `m` (negate)
- `i` (multiply by i, for complex mode)

### Trigonometric Functions
- `s` (sin), `c` (cos), `t` (tan)
- `as` (asin), `ac` (acos), `at` (atan)

### Hyperbolic Functions
- `hs` (sinh), `hc` (cosh), `ht` (tanh)

### Logarithmic Functions
- `l2` (log2), `lc` (log10), `le` (ln)
- `L` (custom base log)

### Rounding Functions
- `C` (ceil), `F` (floor), `R` (round)

### Angle Conversion Functions
- `r` (degree to radian), `d` (radian to degree)

### Matrix Operation Functions
- `~` (Inverse Matrix)

### Conditional Branch
- `?` (Ternary operator)
usage: `<true case> <false case> <cond> ?`

## Special Features
### Constants
- `\E`: Euler's number
- `\P`: Pi

### Register
- `$[a-z]` followed by a letter (a-z) for register reference
- `&[a-z]` to update register (e.g., `3 5 + &x` for x=3+5)

### Function Operations
- `!` evaluate lambda expression at the top of the stack
Note: The order of function arguments is descending order.
usage:  `... <$3> <$2> <$1> <lambda> !`
e.g.) `4 5 {$1 $2 -}!` -> 5 - 4 = 1

### Matrix Input Details
- First element is the number of columns
- Elements separated by commas
- End matrix with `,]`
- Example: `[2 1,2,3,4,]` creates a 2x2 matrix [1 2; 3 4]
- Ignore the overhang
- Example: `[2 5,3m,6i,1s,99]` creates a 2x2 matrix [5 -3; 6i sin(1)]

### Builtin Operations (can be dangerous)
- `@a`: Reference to previous result (ANS)
- `@d`: Display top value of stack
- `@h`: Access to result history (e.g., `5 @h` for 5 previous history)
- `@n`: Push nan
- `@p`: Duplicate the top stack value
- `@r`: Random number between 0 and 1
- `@s`: Access specific stack value (e.g., `5 @h` for 5 previous stack value)

### Other
- `( )`: Specify the valid range of the operator
The operators, like lisp, support variable-length arguments, so unlike pure RPNs, they need parentheses.
tips: Parentheses do not specify priority. Operators are evaluated in definitive left-to-right order without exception and do not backtrack.
```
3 4 5 + 6 *   ; (3 + 4 + 5) * 6
------|+  |
----------|*

3 (4 5 +) 6 * ; 3 * (4 + 5) * 6
   ----|+   |
------------|*
```
- `;`: Start of a comment (ignored until end of line)
- ` `: Required at the number breaks, but ignored elsewhere

## Commands
- `:tc`: Toggle between real and complex number mode
- `:tp`: Toggle between explicit and implicit function in plot
- `:o`: Optimize expression (e.g., remove unnecessary spaces)
- `:p`: Plot graph (argument is $1, multidimensional is not supported)

## CommandLine Options
- `-h`: Show help
- `-r`: Evaluate following argument as expression
- `-q`: Quit
Arguments whose first letter is not '-' are interpreted as file name.

## Examples
1. Basic arithmetic: `3 4 + 2 *` -> $14$
2. Using constants: `\P 2 / s` -> $1$ ( $sin(Pi/2)$ )
3. Complex expression: `2 3 ^ (4 5 *) + (6 7 /) -` -> $27.142857$
4. Using previous result: `5 @a +` -> Adds 5 to previous result
5. Complex mode: `3 4i +` -> $3 + 4i$
8. Matrix addition: `[2 1,2,3,4,][2,4,5,6,7,]+` -> [2 5,7,9,11,]
9. Matrix inverse: `[3 1,1,1m,2m,0,1,0,2,1,]~` -> [3 -0.5,-0.75,0.25,0.5,0.25,0.25,-1,-0.5,0.5,]
10. Plot fn: `:p $1s` -> Graph of $sin(x)$
11. Plot implicit fn: `:p $12^($22^)+1-` -> Circle of radius $1$
12. Define fn: `{$1 2 ^} &f` -> $f(x) = x^2$
13. Define multi-arg fn: `{$1 $2 +} &g` -> $g(x, y) = x + y$
14. Call fn: `5 $f !` -> $f(5)$
15. Call multi-arg fn: `6 7 $g !` -> $g(7, 6)$
16. Lambda fn: `4 {$1 2 *} !` -> $8$
17. Lambda multi-arg fn: `5 6 {$1 $2 -} !` -> $1$
18. Conditional Branch: `3 4 (5 6 <) ?` -> $3$
19. Recursive fn: `5 {{1} {$1 1 - $f! $1 *} ($1 1 =) ? !} &f!` -> $120$
20. higher-order fn: `{$1 3 *} {5 $1!}!` -> $15$
21. Display help and exit: `rpx -h -q`
22. One-shot calculator: `rpx -r "1 1 +" -q`
23. One-shot graph plottor: `rpx -r ":spr\\P,\\Pm,1,1m" -r ":p $1s" -q`
24. Run script files: `rpx sample1.rpx sample2.rpx sample3.rpx`

## Error Handling
- Unknown operators or functions result in an error message
- Division by zero and other mathematical errors are not explicitly handled
