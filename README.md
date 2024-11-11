# Winzig-Calc

A simple interpreter that can perform simple calculations. 

Still in development.

## Note

This project is still in development, so there may be a lot of bugs.

As this is a personal practice project, I may not be able to provide support in time.

Feel free to pull request or issue.

## How to use

You can simply use `winzig_ez_main(argc, argv)` in `winzig_calc.c` as a repl.

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

## Known Problems

1. evil special judgement in **parser**.
2. only 1 number type: long double supported
3. a load of bugs hiding in the code. See if you are lucky enough to find one.


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
