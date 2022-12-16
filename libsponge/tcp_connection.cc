#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _received_tick_time; }

// fill the segments with ACK flag, ackno and window size
TCPSegment TCPConnection::make_segments(TCPSegment seg) {
    auto ack = _receiver.ackno();
    if (ack.has_value()) {
        seg.header().ack = true;
        seg.header().ackno = ack.value();
    }

    seg.header().win = _receiver.window_size() <= numeric_limits<uint16_t>::max() ? _receiver.window_size() : numeric_limits<uint16_t>::max();

    return seg;
}

void TCPConnection::send_segments() {
    while (!_sender.segments_out().empty()) {
        TCPSegment seg = make_segments(_sender.segments_out().front());
        _sender.segments_out().pop();
        segments_out().push(seg);
    }
}

void TCPConnection::send_RST() {
    _sender.send_empty_segment();
    TCPSegment seg = make_segments(_sender.segments_out().front());
    _sender.segments_out().pop();
    seg.header().rst = true;
    segments_out().push(seg);

    unclean_shutdown();
}

void TCPConnection::unclean_shutdown() {
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _active = false;
}

void TCPConnection::segment_received(const TCPSegment &seg) { 
    // means TCPConnection received a new segment
    // the tick time should be restarted
    _received_tick_time = 0;

    // unclear shutdown
    if (seg.header().rst) {
        unclean_shutdown();
        // kill the connection permanently
        return;
    }

    _receiver.segment_received(seg);

    // maybe the flag ack in segment is true
    // but actually the receiver hasn't received the syn yet
    // so we need to avoid this bug
    if (!_receiver.ackno().has_value()) {
        return;
    }

    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }

    _sender.fill_window();

    if (_receiver.ackno().has_value() 
        && seg.length_in_sequence_space() == 0
        && seg.header().seqno == _receiver.ackno().value() - 1) {
        _sender.send_empty_segment();    
    }

    // if segment occupied any sequence number,
    // make sure that at least one segment is sent in reply
    if (seg.length_in_sequence_space() > 0 && _sender.segments_out().empty()) {
        _sender.send_empty_segment();
    }

    send_segments();

    if (_receiver.stream_out().eof() && !_sender.stream_in().eof()) {
        _linger_after_streams_finish = false;
    }
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    size_t len = _sender.stream_in().write(data);
    _sender.fill_window();
    send_segments();
    return len;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { 
    _sender.tick(ms_since_last_tick);

    // need to send a RST segment
    // and must be in front of send_segments()
    // because if the time is out, we just need to send RST
    // and there is no need to send other segments
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        send_RST();
        return;
    }

    // the sender may send a segment again since the time is out
    send_segments();

    _received_tick_time += ms_since_last_tick;
    // Option_A or Option_B
    if (_receiver.stream_out().eof()  //Pre 1
        && _sender.stream_in().eof()  //Pre 2
        && _sender.bytes_in_flight() == 0  //Pre 3
        && (!_linger_after_streams_finish || _received_tick_time >= 10 * _cfg.rt_timeout)) {
        _active = false;
    } 
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    send_segments();
}

void TCPConnection::connect() {
    if (_sender.next_seqno_absolute() > 0) {
        return ;
    }
    _sender.fill_window();
    send_segments();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            send_RST();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
