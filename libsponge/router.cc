#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    // Your code here.
    uint64_t ind = route_prefix >> (32 - prefix_length); // the information of prefix
    // if prefix_length = 0, there is no need to mark its prefix length
    if (prefix_length != 0) {
        ind |= 1ull << (prefix_length + 31); // the information of prefix_length
    }
    _route_map[ind] = {next_hop, interface_num};
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    // Your code here.
    if (dgram.header().ttl <= 1) return;
    --dgram.header().ttl;

    const auto dgram_dst = dgram.header().dst;
    
    // why N starts from 33? Because N is uint8_t, so when N = 0, N - 1 is not -1
    // instead, it will be 255 (1111 1111(2))
    // So in order to avoid this bug, I start the variable N from 33 to 1
    for (uint8_t N = 33; N >= 1; --N) {
        // get the prefix
        /*
        Somethine wrong with these codes.
        For a uint32_t "i", if I use "i << 33" or "i << 31", the result will be fine
        But if using "i << 32", i will not change!!!! This is a weird problem!

        cerr << (dgram_dst >> (32 - (N - 1))) << ",";
        uint64_t ind = dgram_dst >> (32 - (N - 1));
        cerr << ind << ", ";
        */

        // N = 1 means prefix_length = 0
        // So there is no need to add the information of prefix_length
        
        uint64_t ind = N == 1 ? 0 : (dgram_dst >> (32 - (N - 1))) | (1ull << (31 + (N-1)));

        // matches the IP address
        if (_route_map.count(ind)) {
            auto &pr = _route_map[ind];
            if (pr.first.has_value()) { // has next hop address
                interface(pr.second).send_datagram(dgram, pr.first.value());
            } else {
                interface(pr.second).send_datagram(dgram, Address::from_ipv4_numeric(dgram_dst));
            }

            break;
        }

        ind ^= 1ull << (31 + (N - 1));
    }
    /* 
    Not finished
    the original version (using map, and the worst time complexity is O(n))
    for (auto it = _route_map.begin(); it != _route_map.end(); ++it) {
        auto &key = it -> first;

        for (auto &tu : it -> second) {
            prefix = numeric_limits<uint32_t>::max() << (32 - key);
            prefix &= dgram.header().dst & std::get<0>(tu);
            if (prefix == std::get<0>(tu)) {
                if (std::get<1>(tu).has_value()) {
                    interface(std::get<2>(tu)).sent_datagram(dgram, std::get<1>(tu));
                    return;
                } else {
                    Address dst;
                    dst.from_ipv4_numeric(dgram.header().dst);

                    interface(std::get<2>(tu)).send_datagram(dgram, dst);
                }
            }
        }
    }*/
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
