AST

type: statement, function call, operator, literal, identifier, block,

## Operators:

* calculator:   +, -, *, /, ^ ( this is pow )
* assignment:   =, +=, -=, *=, /=
* logical:      &, |
* comparison:   ==, !=, <, <=, >, >=
* unary:        +, -, ! ( maybe not be implemented ) :(   1 - x as 'not x'? add 'not' as a built-in
*
* priority:     (, )
* block:        {, }

## Known grammar:

* Define a function: fn <Function_Name>(<args...>){}; // TODO may not be implemented

## Known keywords:

* if, else, while, return

finished tokenizer, parser, and interpreter.

## Problems Analysis:

1. evil special judgement in **parser**.
    1. add a **lexer**, recognize the operators and keywords, and mark them with Enums,
       which will facilitate the **parser** to judge.
2. only 1 number type: long double supported
    1. improve data structure to save the number type
    2. saving more memory: declaration grammar
    3. improve the **tokenizer** to recognize integer, then **parse** it.
3. only 1 variable type: long double.
    1. plan more types: boolean, string, etc.
    2. add true and false keyword to support boolean
    3. pure string type and "" syntax need **tokenizer** to recognize
4. didn't support user function
   1. add function grammar to **parser**
   2. recognize built-in function and user function
   3. add ',' to support multiple arguments
5. internal error enum
   1. add an error enum for better semantic

## Plan:

(write something not mentioned in the above)

1. add a **linter** to optimize the code style (possibly operators in advance, 
   leaving the tokenizer only to split ' ' and recognize keywords)
2. pre-collect all declaration to save memory, improve speed, and easily report 'NotDefined' error
3. provide a **runner** struct everywhere to save all errors and easily report them
4. arrayed types
   1. array literal [,] syntax and array access [] syntax
   2. slice, range, and other iterator methods
   3. for-in loop
5. constant optimize
   1. when **parser** found all sub node is Literal, call interpreter to calculate them.