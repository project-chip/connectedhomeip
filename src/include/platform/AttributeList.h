/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *          This is a list of attribute. Each entry is an attribute of type T.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {

template <typename T, size_t N>
class AttributeList
{
public:
    /* The iterator */
    class Iterator
    {
    public:
        Iterator(const AttributeList<T, N> * AttributeList, int index);
        const T & operator*() const;
        Iterator & operator++();
        bool operator!=(const Iterator & other) const;

    private:
        const AttributeList<T, N> * mAttributeListPtr;
        int mIndex = -1;
    };

    AttributeList() = default;
    ~AttributeList() { mSize = 0; }

    CHIP_ERROR add(const T & label);

    size_t size() const;
    const T & operator[](int index) const;

    Iterator begin() const;
    Iterator end() const;

private:
    T mList[N];
    int mSize = 0;
};

/*
 * AttributeList methods
 **/
template <typename T, size_t N>
inline CHIP_ERROR AttributeList<T, N>::add(const T & label)
{
    if (mSize == N)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // add the new element
    mList[mSize] = label;
    ++mSize;
    return CHIP_NO_ERROR;
}

template <typename T, size_t N>
inline size_t AttributeList<T, N>::size() const
{
    return static_cast<size_t>(mSize);
}

template <typename T, size_t N>
inline const T & AttributeList<T, N>::operator[](int index) const
{
    VerifyOrDie(index < mSize);
    return mList[index];
}

template <typename T, size_t N>
inline typename AttributeList<T, N>::Iterator AttributeList<T, N>::begin() const
{
    return AttributeList<T, N>::Iterator{ this, 0 };
}

template <typename T, size_t N>
inline typename AttributeList<T, N>::Iterator AttributeList<T, N>::end() const
{
    return AttributeList<T, N>::Iterator{ this, mSize };
}

/*
 * Iterator methods
 **/
template <typename T, size_t N>
inline AttributeList<T, N>::Iterator::Iterator(const AttributeList<T, N> * pAttributeList, int index) :
    mAttributeListPtr(pAttributeList), mIndex(index)
{}

template <typename T, size_t N>
inline const T & AttributeList<T, N>::Iterator::operator*() const
{
    return mAttributeListPtr->operator[](mIndex);
}

template <typename T, size_t N>
inline typename AttributeList<T, N>::Iterator & AttributeList<T, N>::Iterator::operator++()
{
    ++mIndex;
    return *this;
}

template <typename T, size_t N>
inline bool AttributeList<T, N>::Iterator::operator!=(const AttributeList<T, N>::Iterator & other) const
{
    return mIndex != other.mIndex;
}

} // namespace DeviceLayer
} // namespace chip
