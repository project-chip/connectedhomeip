/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef __Ring_h__
#define __Ring_h__

#include <assert.h>
#include <stdint.h>
#include <string.h>

class Ring
{
    uint8_t * _buffer;
    uint16_t _itemSize;
    uint16_t _itemCount;
    uint16_t _readIndex;
    uint16_t _writeIndex;

public:
    typedef uint16_t item_size_t;

    Ring(uint16_t itemSize, uint16_t itemCount)
    {
        assert(!((itemCount - 1) & itemCount)); // ERROR: Ring item count must be power of two.

        _readIndex  = 0;
        _writeIndex = 0;
        _itemSize   = itemSize;
        _itemCount  = itemCount;
        _buffer     = new uint8_t[itemSize * itemCount];
    }

    ~Ring()
    {
        if (_buffer)
        {
            delete[] _buffer;
        }
    }

    uint16_t item_size() { return _itemSize; }

    uint16_t max_size() { return _itemCount; }

    uint16_t size() { return _writeIndex - _readIndex; }

    bool empty() { return _readIndex == _writeIndex; }

    bool full() { return size() == _itemCount; }

    void push_back(void * item)
    {
        assert(!full());
        memcpy(&_buffer[mask(_writeIndex) * _itemSize], item, _itemSize);
        _writeIndex = inc(_writeIndex);
    }

    void pop_front()
    {
        assert(!empty());
        _readIndex = inc(_readIndex);
    }

    void * front()
    {
        void * item = (void *) &_buffer[mask(_readIndex) * _itemSize];
        return (empty()) ? NULL : item;
    }

private:
    unsigned mask(unsigned index) { return index & (_itemCount - 1); }
    unsigned inc(unsigned index) { return index + 1; }
};

#endif // __Ring_h__
