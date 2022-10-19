Lab 1 Writeup
=============

My name: llbbllbb2000

I collaborated with: myself

This lab took me about 3 hours to do.

Program Structure and Design of the StreamReassembler:
two loop back vectors

Implementation Challenges:
First : the problem with eof. The data may be in eof, but the Bytestream has not yet read it because of the unordered sequence.
Second : the index may less than _next_index
Third : when the data's size is bigger than capacity: I forget to clean the cache and to sum up to _next_index

Remaining Bugs:
All pass

- Optional: I had unexpected difficulty with: the data structure I should use to cache strings
