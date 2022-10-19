#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

//template <typename... Targs>
//void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _capacity(capacity), _total_written(0), _total_pop(0), _buffer() {}

size_t ByteStream::write(const string &data) {
    /*if (_end) {
        set_error(); 
        return 0;
    }*/

    size_t len = 0;
    for (auto &s : data) {
        if (_buffer.size() == _capacity) { break; }
        _buffer.push_back(s);
        ++len;
    }

    _total_written += len;
    return len;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    return string(_buffer.begin(), _buffer.begin() + min(len, _buffer.size())); 
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    size_t _len = min(len, _buffer.size());
    _total_pop += len;

    while(_len--) {
        _buffer.pop_front();
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t _len = min(len, _buffer.size());
    string result(_buffer.begin(), _buffer.begin() + _len);
    _total_pop += len;

    while (_len--) {
        _buffer.pop_front();
    }
    return result;
}

void ByteStream::end_input() { _end = true; }

bool ByteStream::input_ended() const { return _end; }

size_t ByteStream::buffer_size() const { return _buffer.size(); }

bool ByteStream::buffer_empty() const { return _buffer.empty(); }

bool ByteStream::eof() const { return _end && _buffer.empty(); }

size_t ByteStream::bytes_written() const { return _total_written; }

size_t ByteStream::bytes_read() const { return _total_pop; }

size_t ByteStream::remaining_capacity() const { return _capacity - _buffer.size(); }
