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
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return {}; }

void TCPSender::fill_window() {
    if (stream_in().buffer_empty()) {
        return;
    }
    // Even if window size is 0, we still need to send a single byte to provoke the receiver into sending a new ack where it reveals that more space has opened up.
    auto _size = max(_window_size, 1);
    while (_size != 0 && !stream_in().buffer_empty()) {
        TCPSegment Seg;
        std::string s = stream_in().read(min(_size, TCPConfig::MAX_PAYLOAD_SIZE - _has_not_syn));
        Seg.header().syn = _has_not_syn;
        _has_not_syn = false;
        Seg.header().seqno = next_seqno();
        Seg.payload()(s);
       
        // If seg is not full, we can just set the fin to be true
        if (stream_in().buffer_empty() && Seg.length_in_sequence_space() < TCPConfig::MAX_PAYLOAD_SIZE) {
           Seg.header().fin = true; 
        }
        
        segments_out().push(Seg);
        _next_seqno += Seg.length_in_seqence_space();
        _size -= Seg.length_in_sequence_space();
        
        // in this case, we need to push an empty segment whose fin is true;
        if (stream_in().buffer_empty() && !Seg.header().fin) {
            TCPSegment temp;
            temp.header().fin = true;
            temp.header().seqno = next_seqno();
            segments_out().push(temp);
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    _ackno = max(_ackno, unwrap(ackno, _isn, next_seqno_absolute()));
    _window_size = window_size; 
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) { 
}

unsigned int TCPSender::consecutive_retransmissions() const { return {}; }

void TCPSender::send_empty_segment() {}
