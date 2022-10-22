Lab 2 Writeup
=============

My name: llbbllbb2000

This lab took me about 10 hours to do. 

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[]

Implementation Challenges:
1. About unwrap: At first, I counted offset by "(n - isn) + ...". But (n - isn) here is automatically become uint64_t since the offset is uint64_t. So I have to use static_cast<uint32_t>(n - isn) to make sure the result should be a uint32_t number so that it won't be wrapped up to a uint64_t

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
