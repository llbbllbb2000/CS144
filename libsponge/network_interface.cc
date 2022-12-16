#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();
    EthernetFrame eframe{};

    if (_cache.count(next_hop_ip) != 0  // If the destination Ethernet address is already known
        && _cache[next_hop_ip].first + 30000 >= _tick  // and is not outdated (30s)
        && _cache[next_hop_ip].second.has_value()) {  // has received the Ethernet Address
        eframe.payload() = dgram.serialize();
        eframe.header().dst = _cache[next_hop_ip].second.value();
        eframe.header().src = _ethernet_address;
        eframe.header().type = EthernetHeader::TYPE_IPv4;
        frames_out().push(eframe);
    } else { // If unknown
        if (_cache.count(next_hop_ip) == 0 || _cache[next_hop_ip].first + 5000 <= _tick) {  // and avoid flooding the network
            _cache[next_hop_ip].second.reset();  //Remember to reset the value
            _cache[next_hop_ip].first = _tick;   //Set the time
            ARPMessage arp{};
            arp.sender_ip_address = _ip_address.ipv4_numeric();
            arp.sender_ethernet_address = _ethernet_address;
            arp.target_ip_address = next_hop_ip;
            arp.opcode = ARPMessage::OPCODE_REQUEST;

            eframe.payload() = arp.serialize();
            eframe.header().dst = ETHERNET_BROADCAST;
            eframe.header().src = _ethernet_address;
            eframe.header().type = EthernetHeader::TYPE_ARP;
            frames_out().push(eframe);
        }
        _not_sent_dgrams[next_hop_ip].push({dgram, next_hop});
    }
}

//! \param[in] frame the incoming Ethernet frame
std::optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        if (frame.header().dst != _ethernet_address) {
            return {};
        }

        InternetDatagram dgram{};
        if (dgram.parse(frame.payload()) == ParseResult::NoError) {
            return dgram;
        }
    } else {
        ARPMessage arp{};
        if (arp.parse(frame.payload()) == ParseResult::NoError) {
            _cache[arp.sender_ip_address] = {_tick, arp.sender_ethernet_address};
            while (!_not_sent_dgrams[arp.sender_ip_address].empty()) {
                auto data = _not_sent_dgrams[arp.sender_ip_address].front();
                _not_sent_dgrams[arp.sender_ip_address].pop();

                send_datagram(data.first, data.second);
            }
            
            // if the request asking for out IP address
            if (arp.opcode == ARPMessage::OPCODE_REQUEST
                && arp.target_ip_address == _ip_address.ipv4_numeric()) {
               
                ARPMessage reply{};
                reply.sender_ip_address = _ip_address.ipv4_numeric();
                reply.sender_ethernet_address = _ethernet_address;
                reply.target_ip_address = arp.sender_ip_address;
                reply.target_ethernet_address = arp.sender_ethernet_address;
                reply.opcode = ARPMessage::OPCODE_REPLY;

                EthernetFrame eframe{};
                eframe.payload() = reply.serialize();
                eframe.header().dst = arp.sender_ethernet_address;
                eframe.header().src = _ethernet_address;
                eframe.header().type = EthernetHeader::TYPE_ARP;
                frames_out().push(eframe);
            }
        }
    }

    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) { 
    _tick += ms_since_last_tick;
}
