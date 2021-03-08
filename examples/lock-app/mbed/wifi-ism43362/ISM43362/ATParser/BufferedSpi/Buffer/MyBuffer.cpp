
/**
 * @file    Buffer.cpp
 * @brief   Software Buffer - Templated Ring Buffer for most data types
 * @author  sam grove
 * @version 1.0
 * @see
 *
 * Copyright (c) 2013
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MyBuffer.h"

template <class T>
MyBuffer<T>::MyBuffer(uint32_t size)
{
    _buf = new T [size];
    _size = size;
    clear();

    return;
}

template <class T>
MyBuffer<T>::~MyBuffer()
{
    delete [] _buf;

    return;
}

template <class T>
uint32_t MyBuffer<T>::getSize()
{
    return this->_size;
}

template <class T>
uint32_t MyBuffer<T>::getNbAvailable()
{
    if (_wloc >= _rloc) {
        return (_wloc - _rloc);
    } else {
        return (_size - _rloc + _wloc);
    }
}

template <class T>
void MyBuffer<T>::clear(void)
{
    _wloc = 0;
    _rloc = 0;
    memset(_buf, 0, _size);

    return;
}

template <class T>
uint32_t MyBuffer<T>::peek(char c)
{
    return 1;
}

// make the linker aware of some possible types
template class MyBuffer<uint8_t>;
template class MyBuffer<int8_t>;
template class MyBuffer<uint16_t>;
template class MyBuffer<int16_t>;
template class MyBuffer<uint32_t>;
template class MyBuffer<int32_t>;
template class MyBuffer<uint64_t>;
template class MyBuffer<int64_t>;
template class MyBuffer<char>;
template class MyBuffer<wchar_t>;
