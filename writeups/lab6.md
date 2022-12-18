Lab 6 Writeup
=============

My name: llbbllbb2000

This lab took me about 3 hours to do.

Program Structure and Design of the Router:
Using hashmap to find the longest prefix IP address
The time complexity of searching the IP should be O(log 32), since IP has 32 bits

Implementation Challenges:
1. The most absurd and time-consuming problem is the operation "<<" & ">>".
This is the first time I find that if the variable "i" is 32 bits, then it can do the thing like "i << 32" or "i >> 32", because the result(i does not change) is not I expected(i = 0). I think this is a tricky place that I faced.
