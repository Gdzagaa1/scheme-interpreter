# Scheme Interpreter in C

A complete Scheme programming language interpreter built from scratch in C. This interpreter can parse, evaluate, and execute Scheme code with support for functions, variables, arithmetic, and list operations.

## Features

It can execute the following constructs:

1) Lambda functions
2) In-place lambda function calls
3) define
4) Arithmetic + - * / operations
5) Logical operations and and or
6) if/else
7) List functions: car, cdr, cons, map, append
8) List description without execution: ‘(1 2 3)
9) Execution functions: apply, eval
10) Recursive function execution
11) Helper functions: null?, length
12) equal?
13) load

## How to Use

### Compile and Run
```bash
gcc -o scheme interpreter.c
./scheme
```

### Memory Management
The interpreter handles memory allocation and cleanup automatically.

## How It Works

The interpreter processes code in three stages:

1. **Tokenization**: Breaks input into pieces (numbers, symbols, parentheses)
2. **Parsing**: Builds a tree structure from tokens
3. **Evaluation**: Executes the parsed code

## Testing

Run the included test suite:
```scheme
> (load "tests.scm")
```

The test file demonstrates all supported features with working examples.

## Technical Details

### Implementation
- **Language**: Pure C (no external dependencies)
- **Memory**: Dynamic allocation with cleanup

### Supported Scheme Features
- Lambda functions and closures
- Recursive function calls
- Proper list handling
- String parsing with escape sequences
- Multiple number types (int, float, rational)
- File loading and execution

## File Structure

The main interpreter file contains:
- Token handling (`TokenList`, `tokenize_input`)
- Data types (`data` struct with union for different types)
- Environment management (`Env`, `lookup`, `add_elements_to_environment`)
- Parser (`parse_func`, `parse`)
- Evaluator (`eval` function)
- Built-in functions (`cons_builtin`, `car_builtin`, etc.)