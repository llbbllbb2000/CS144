#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

//template <typename... Targs>
//void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _cache(capacity), _cache_check(capacity), _capacity(capacity), _cache_size(0), _next_index(0), _eof(false), _eof_index(0) {}

void StreamReassembler::cache_insert(const uint64_t index, const string &data) {
    uint64_t ind = index;
    uint64_t lim = _next_index + _capacity - _output.buffer_size();
    if (ind >= lim) return;
    ind %= _capacity;
    lim %= _capacity;

    for (auto &c : data) {
        if (ind == lim) break;
        if (!_cache_check[ind]) {
            _cache_check[ind] = true;
            _cache[ind] = c;
            ++_cache_size;
        }
        ++ind;
        if (ind == _capacity) {
            ind = 0;
        }
    }
}

void StreamReassembler::cache_clear(const uint64_t left, const uint64_t right) {
    uint64_t l = left % _capacity;
    uint64_t r = right % _capacity;
    while (l != r) {
        if (_cache_check[l]) {
            _cache_check[l] = false;
            --_cache_size;
        }

        ++l;
        if (l == _capacity) {
            l = 0;
        }
    }
}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if (eof) {
        if (_eof && index + data.size() != _eof_index) {
            _output.set_error();
        }
        _eof = true;
        _eof_index = index + data.size();
    }
    
    if (index > _next_index) {
        cache_insert(index, data);
    } else if(index < _next_index) {
        if (_next_index < index + data.size()) {  // the string has some parts that have not been read
            push_substring(data.substr(_next_index - index), _next_index, eof);
        }
    } else {
        if (_output.remaining_capacity() < data.size()) {
            cache_clear(index, index + _output.remaining_capacity());
            _next_index += _output.remaining_capacity();
            _output.write(data);
        } else {
            cache_clear(index, index + data.size());
            _output.write(data);
            _next_index += data.size();
            uint64_t i = _next_index % _capacity;
            string s;
            while (_cache_check[i]) {
                _cache_check[i] = false;
                s += _cache[i];
                --_cache_size;

                ++i;
                if (i == _capacity) {
                    i = 0;
                }
            }
            _output.write(s);
            _next_index += s.size();
        }
    }

    if (_eof && _next_index == _eof_index) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _cache_size; }

bool StreamReassembler::empty() const { return _cache_size == 0; }
