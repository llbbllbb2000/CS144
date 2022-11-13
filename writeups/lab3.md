Lab 3 Writeup
=============

My name: llbbllbb2000

Implementation Challenges:
1. It costed several hours to understand what I really need to do, like how the tick function works(it is used by TCPConnection, not TCPSender, So I don't need to worry about it)
2. about the function make_segment, first I passed the wrong parameters (_outstanding_bytes - window_size), which is always a negetive number, and since the datatype is size_t, it will become a very large number and cause a lot problems.
3. The initial _window_size is 1, not 0
4. about the constant number MAX_PAYLOAD_SIZE, it is just a limit about the payload(i.e. the strings), not about the whole segment
