Lab 2 Writeup
=============

My name: llbbllbb2000

This lab took me about 10 hours to do. 

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
[]

Implementation Challenges:
1. About unwrap: At first, I counted offset by "(n - isn) + ...". But (n - isn) here is automatically become uint64_t since the offset is uint64_t. So I have to use static_cast<uint32_t>(n - isn) to make sure the result should be a uint32_t number so that it won't be wrapped up to a uint64_t
2. For TCP receiver: size_t is uint64_t when the computer is 64bits, and I was struggling what size_t is and whether I can use functions that return size_t. And for the function segment_received, I forgot to consider a special situation : a segment with syn = 1 and fin = 1, but a empty string. So my first parameter of unwarp is upwarp(seqno + syn, isn, ...) - 1, instead of just upwarp(seqno - 1, isn ...).

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
