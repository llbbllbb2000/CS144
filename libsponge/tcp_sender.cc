#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

//template <typename... Targs>
//void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _RTO{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _outstanding_bytes; }

TCPSegment TCPSender::make_segment(size_t max_len) {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    
    // MAX_PAYLOAD_SIZE means the length of string(payload), not the whole segment size
    auto len = max_len - _has_not_syn < TCPConfig::MAX_PAYLOAD_SIZE ? max_len - _has_not_syn: TCPConfig::MAX_PAYLOAD_SIZE;
    
    seg.payload() = Buffer(stream_in().read(len));

    if (_has_not_syn) {
        seg.header().syn = true;
        _has_not_syn = false;
    }
     
    // when there are still some spaces, set the header's fin to be true
    if (stream_in().eof() && max_len > seg.length_in_sequence_space()) {
        seg.header().fin = true;
        _has_fin = true;
    }

    return seg;
}

void TCPSender::fill_window() {
    // if the window_size is zero, just assumes the size is 1
    auto window_size = _window_size ? _window_size : 1;
    while (window_size > _outstanding_bytes) {
        if (_has_fin) {
            break;
        }

        // remenber there must be "windows_size - _outstanding_bytes"!!!!
        auto seg = make_segment(window_size - _outstanding_bytes);
       
        // means this seg is an empty segment (because the bytestream has not read data yet, and the process is not finished)
        if (!seg.length_in_sequence_space()) {
            break;
        }

        if (!_outstanding_bytes) {
            _RTO = _initial_retransmission_timeout;
            _tick_time = 0;
        }

        _outstanding_segments.push(seg);
        segments_out().push(seg);

        _next_seqno += seg.length_in_sequence_space();
        _outstanding_bytes += seg.length_in_sequence_space();
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) { 
    auto abs_ackno = unwrap(ackno, _isn, next_seqno_absolute());
    // ignore the impossible situation
    if (abs_ackno > next_seqno_absolute()) {
        return;
    }
    bool flag = false; // whether this ackno means new data or not
    _window_size = window_size;

    while (!_outstanding_segments.empty()) {
        auto temp = unwrap(_outstanding_segments.front().header().seqno, _isn, abs_ackno) + _outstanding_segments.front().length_in_sequence_space();
        if (temp > abs_ackno) {
            break;
        }
        _outstanding_bytes -= _outstanding_segments.front().length_in_sequence_space();
        _outstanding_segments.pop();
        flag = true;
    }

    if (flag) {
        _consecutive_retransmissions = 0;
        _RTO = _initial_retransmission_timeout;
        _tick_time = 0;
    }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _tick_time += ms_since_last_tick;
    if (_tick_time < _RTO || _outstanding_segments.empty()) {
        return ;
    }

    ++_consecutive_retransmissions;
    segments_out().push(_outstanding_segments.front());

    _tick_time = 0;
    if (_window_size) {
       // double the value of RTO
        _RTO <<= 1;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    segments_out().push(seg);
}
