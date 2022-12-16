Lab 5 Writeup
=============

My name: llbbllbb2000

This lab took me about 5 hours to do.

Program Structure and Design of the NetworkInterface:
Hashmap(unordered map, pair and optional) 

Implementation Challenges:
1. Don't know what the destination should be if the next hop ethernet address is unknown. But actually there is a constant ethernet address ETHERNET_BROADCAST(ff:ff:ff:ff:ff:ff)
2. Don't know how to parse the payload to be InternetDatagram or ARPMessage. But actually we just need to use the function "parse" (even the payload is "BufferList" and the attribute in parse is "Buffer")
