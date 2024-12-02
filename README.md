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
- Use spaces to separate numbers and operators
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

## Special Features
### Constants
- `\E`: Euler's number
- `\P`: Pi

### Variable Operations
- `$[a-z]` followed by a letter (a-z) for variable reference
- `&[a-z]` to update variables (e.g., `3 5 + &x` for x=3+5)

### Random Number
- `$R`: Random number (0 to 1)

### Function Operations
- `!` followed by a letter (a-z) for function call
format:  `... <$3> <$2> <$1> !<fn>`

### Builtin Operations (can be dangerous)
- `@a`: Reference to previous result (ANS)
- `@h`: Access to result history (e.g., `5 @h` for 5 previous history)
- `@p`: Duplicate the top stack value
- `@s`: Access specific stack value (e.g., `5 @h` for 5 previous stack value)

### Other
- `( )`: Specify the valid range of the operator
- `;`: Start of a comment (ignored until end of line)

## Commands
- `:d[a-z(name)]`: Define function
- `:tc`: Toggle between real and complex number mode
- `:tp`: Toggle between explicit and implicit function in plot
- `:o`: Optimize expression (e.g., remove unnecessary spaces)
- `:p`: Plot graph (argument is $1)

## CommandLine Options
- `-h`: Show help
- `-r`: Evaluate following argument as expression
- `-q`: Quit
Arguments whose first letter is not '-' are interpreted as file name.

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
12. Display help and exit: `rpx -h -q`
13. One-shot calculator: `rpx -r "1 1 +" -q`
14. One-shot graph plottor: `rpx -r ":spr\\P,\\Pm,1,1m" -r ":p $1s" -q`
15. Run script files: `rpx sample1.rpx sample2.rpx sample3.rpx`

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
