# Winzig-Calc

A simple interpreter that can perform simple calculations. 

Still in development.

## Note

This project is still in development, so there may be a lot of bugs.

As this is a personal practice project, I may not be able to provide support in time.

Feel free to pull request or issue.

## How to use

You can simply use `winzig_ez_main(argc, argv)` in `winzig_calc.c` as a repl. Start it and input the allowed expression syntax directly.

Or you can initialize a `WinzigCalc` object and use `winzig_code` to evaluate a sourcecode.

you can also try separately use Tokenizer, Parser or Interpreter provided.

## Features

- [x] Only one data type: `long double`
- [x] lots of operators supported 
  * calculator:   +, -, *, /, ^ ( it's pow )
  * assignment:   =, +=, -=, *=, /=
  * logical:      &, | ( will return 0 or 1, no short-circuit or lazy evaluation )
  * comparison:   ==, !=, <, <=, >, >=
- [x] ( ) to control the priority
- [x] { } block control
- [x] if, else, while statement

## Syntax

### Statement

```
<expr>;
```

The semicolon is optional, but it is recommended. Unlike Rust, the semicolon will not discard the value of the expression.

### Assignment

```
<name> = <expr>;
```

Assign a value to a variable. The variable name can only be a combination of letters and numbers, and cannot start with a number.

Since there is only one data type, there is no declaration syntax.

Values are stored in a hash table by the hash of the variable name, and there is a small probability of collision. We pre-fill the hash table with qnan and do not check for undefined variables, so please be careful.

### Expression

```
<expr> ::= <term> | <term> <addop> <expr>
```

Note that we do not have unary operators.

### Control Flow

```
if (<expr:condition>) { <block:body> }
[else { <block:body> }]
```

Only single-layer if-else statements are supported, else is optional, and there is no else if, you can nest it yourself.

There is no switch-case statement.

Curly braces are optional for single-line blocks.

### Loop

```
while (<expr:condition>) { <block:body> }
```

There is only one loop statement, while, and no for or do-while.

Curly braces are optional for single-line blocks.

No break or continue statement yet.

### Predefined Functions

No user defined functions yet.

For convenience, all functions are unary functions, there are no multi-parameter functions or zero-parameter functions. 
Please pass one value or one anything when calling a zero-parameter function.

Following functions are predefined:

math functions: `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `sqrt`, `exp`, `log`, `log10`, `ceil`, `floor`, `round`. (who can't call a library?)

Our provided functions:

- `sign(x)` (sign function, return -1, 0, 1)
- `boolean(x)` (convert to boolean, return 0 or 1)
- `print(x)` (print a single number and newline, return the number of bytes printed)
- `rand(_)` (return a random number between 0 and 1)
- `input(_)` (return the input number, or input q to interrupt the entire program)
- `exit(_)` (exit the program)

## Known Problems

1. evil special judgement in **parser**.
2. only 1 number type: long double supported
3. a load of bugs hiding in the code. See if you are lucky enough to find one.
4. REPL does not support multi-line input well.

## Future Plan

These are also problems, somehow.

Maybe I will solve them when pigs fly. :)
- [ ] **linter**: optimize the code style (possibly mark operators in advance,
  leaving the tokenizer only to split ' ' and recognize keywords)
- [ ] **lexer**: recognize the operators and keywords , facilitate the **parser** to judge.
- [ ] pre-collect all declaration to save memory, improve speed, and easily report 'NotDefined' error
- [ ] improve data structure to save the variable type, add more types: boolean, string, etc.
- [ ] string literal
- [ ] function grammar
- [ ] provide a **runner** struct everywhere to save all errors and easily report them
- [ ] arrayed types
  1. array literal [,] syntax and array access [] syntax
  2. slice, range, and other iterator methods
  3. for-in loop
- [ ] constant optimize
  1. when **parser** found all sub node is Literal, call interpreter to calculate them.
