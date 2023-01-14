# Multi-Threaded-Producer-Consumer-Pipeline
A multi-threaded program that creates 4 threads to process input using the producer-consumer approach 

The program creates 4 threads to process input from STDIN as follows:

* Thread 1, called the Input Thread, reads in lines of characters from the standard input.
* Thread 2, called the Line Separator Thread, replaces every line separator in the input by a space.
* Thread, 3 called the Plus Sign thread, replaces every pair of plus signs, i.e., "++", by a "^".
* Thread 4, called the Output Thread, write this processed data to standard output as lines of exactly 80 characters.

---
### How to compile:
```
gcc -std=c99 -o line_processor line_processor.c -lpthread -lm
```
