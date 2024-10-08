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

## Installation
```
$ sudo make install
```

## Introduction
RPX (RPN eXtended) is a powerful calculator that uses Reverse Polish Notation (RPN). It supports real, complex, and matrix calculations, offering a wide range of mathematical operations and functions.

## Basic Concepts
- Uses Reverse Polish Notation (RPN)
- Operands are pushed onto a stack
- Operators act on the stack elements
- No parallel processing
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
- Use spaces to separate numbers and operators
- Negative numbers: Use 'm' operator (e.g., `5 m` for -5)
- Decimal numbers are supported
- Complex numbers: Use 'i' operator (e.g., `3m 4 i +` for -3+4i)
- Polar coordinates: Use 'p' operator (e.g., `1(\P2/)p` for 1i)
- Matrices: Use `[columns elements,]` format (e.g., `[2 1,2,3,4,]` for a 2x2 matrix)

## Operators and Functions
### Basic Arithmetic
- `+` (addition), `-` (subtraction), `*` (multiplication), `/` (division)
- `%` (modulo), `^` (exponentiation)

### Unary Operators
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

### Angle Conversion
- `r` (degree to radian), `d` (radian to degree)

### Matrix Operations
- `~` (Inverse Matrix)

## Special Features
### Constants
- `\E`: Euler's number
- `\P`: Pi

### Variable Operations
- `$` followed by a letter (a-z) for variable operations
- `u` to update variables (e.g., `3 5 + $x u`)

### Random Number
- `$R`: Random number (0 to 1 in real/complex mode)

### Function Operations
- `!` followed by a letter (a-z) for function call

### Builtin Operations
- `@a`: Reference to previous result (ANS)
- `@h`: Access to result history
- `@p`: Duplicate the top stack value
- `@s`: Access specific stack value

### Other
- `( )`: Specify the valid range of the operator
- `;`: Start of a comment (ignored until end of line)

## Commands
- `:d[a-z(name)][1-9(argc)]`: Define function
- `:tc`: Toggle between real and complex number mode
- `:tp`: Toggle between explicit and implicit function in plot
- `:o`: Optimize expression (e.g., remove unnecessary spaces)
- `:p`: Plot graph (argument is $1)

## Examples
1. Basic arithmetic: `3 4 + 2 *` -> 14.000000
2. Using constants: `\P 2 / s` -> 1.000000 (sin(Pi/2))
3. Complex expression: `2 3 ^ (4 5 *) + (6 7 /) -` -> 27.142857
4. Using previous result: `5 @a +` -> Adds 5 to previous result
5. Complex mode: `3 4i +` -> 3.000000 + 4.000000i
8. Matrix addition: `[2 1,2,3,4,][2,4,5,6,7,]+` -> [2 5,7,9,11,]
9. Matrix inverse: `[3 1,1,1m,2m,0,1,0,2,1,]~` -> [3 -0.5,-0.75,0.25,0.5,0.25,0.25,-1,-0.5,0.5,]
10. Plot fn: `:p $1s` -> Graph of sin(x)
11. Plot implicit fn: `:p $12^($22^)+1-` -> Circle of radius 1

## Error Handling
- Unknown operators or functions result in an error message
- Division by zero and other mathematical errors are not explicitly handled
- NaN results are not displayed

## Advanced Topics
### Matrix Input Details
- First element is the number of columns
- Elements separated by commas
- End matrix with `,]`
- Example: `[2 1,2,3,4,]` creates a 2x2 matrix [1 2; 3 4]
