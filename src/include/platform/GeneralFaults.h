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
 *          General faults could be detected by the Node.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {

static constexpr size_t kMaxHardwareFaults = 11;
static constexpr size_t kMaxRadioFaults    = 7;
static constexpr size_t kMaxNetworkFaults  = 4;

template <size_t N>
class GeneralFaults
{
public:
    /* The iterator */
    class Iterator
    {
    public:
        Iterator(const GeneralFaults<N> * GeneralFaults, int index);
        uint8_t operator*() const;
        Iterator & operator++();
        bool operator!=(const Iterator & other) const;

    private:
        const GeneralFaults<N> * mGeneralFaultsPtr;
        int mIndex = -1;
    };

    GeneralFaults() = default;
    ~GeneralFaults() { mSize = 0; }

    CHIP_ERROR add(const uint8_t value);

    const uint8_t * data() const { return mData; }
    size_t size() const;
    uint8_t operator[](int index) const;

    Iterator begin() const;
    Iterator end() const;

private:
    uint8_t mData[N];
    int mSize = 0;
};

/*
 * GeneralFaults methods
 **/
template <size_t N>
inline CHIP_ERROR GeneralFaults<N>::add(const uint8_t value)
{
    if (mSize == N)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // add the new element
    mData[mSize] = value;
    ++mSize;
    return CHIP_NO_ERROR;
}

template <size_t N>
inline size_t GeneralFaults<N>::size() const
{
    return static_cast<size_t>(mSize);
}

template <size_t N>
inline uint8_t GeneralFaults<N>::operator[](int index) const
{
    VerifyOrDie(index < mSize);
    return mData[index];
}

template <size_t N>
inline typename GeneralFaults<N>::Iterator GeneralFaults<N>::begin() const
{
    return GeneralFaults<N>::Iterator{ this, 0 };
}

template <size_t N>
inline typename GeneralFaults<N>::Iterator GeneralFaults<N>::end() const
{
    return GeneralFaults<N>::Iterator{ this, mSize };
}

/*
 * Iterator methods
 **/
template <size_t N>
inline GeneralFaults<N>::Iterator::Iterator(const GeneralFaults<N> * pGeneralFaults, int index) :
    mGeneralFaultsPtr(pGeneralFaults), mIndex(index)
{}

template <size_t N>
inline uint8_t GeneralFaults<N>::Iterator::operator*() const
{
    return mGeneralFaultsPtr->operator[](mIndex);
}

template <size_t N>
inline typename GeneralFaults<N>::Iterator & GeneralFaults<N>::Iterator::operator++()
{
    ++mIndex;
    return *this;
}

template <size_t N>
inline bool GeneralFaults<N>::Iterator::operator!=(const GeneralFaults<N>::Iterator & other) const
{
    return mIndex != other.mIndex;
}

} // namespace DeviceLayer
} // namespace chip
