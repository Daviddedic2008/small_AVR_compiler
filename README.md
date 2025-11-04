# This compiler is slightly unfinished
It is a compiler for the Arduino Uno R3.
I chose this board as I have memorized AVR asm and know how to work with the Atmega mcu inside of it.

I wanted to attempt to write a compiler in cpp, with no yak or lex. 
I finished a simple tokenizer(that does not handle syntax errors) and a parser that recursively parses the vector of tokens into an orderly AST.
I also made a simple emulator to "run" the results.
I started(but never finished) the actual assembly generator. The framework is largely there, I just didn't have the heart to bring it to a close.

