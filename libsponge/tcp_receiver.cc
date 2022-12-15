#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

//template <typename... Targs>
//void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (seg.header().syn == 1) {
        syn = true;
        isn = seg.header().seqno;
    }

    if (syn) { // the initial sequence number has been set
        _reassembler.push_substring(seg.payload().copy(), unwrap(seg.header().seqno + seg.header().syn, isn, stream_out().bytes_written()) - 1, seg.header().fin);
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (syn) {
        return wrap(stream_out().bytes_written() + 1 + stream_out().input_ended(), isn);
    }
    return {}; 
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
