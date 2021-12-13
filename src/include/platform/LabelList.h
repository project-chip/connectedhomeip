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
 *          This is a list of string tuples. Each entry is a LabelStruct.
 */

#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {

static constexpr size_t kMaxFixedLabels = 10;
static constexpr size_t kMaxUserLabels  = 10;

template <size_t N>
class LabelList
{
public:
    /* The iterator */
    class Iterator
    {
    public:
        Iterator(const LabelList<N> * LabelList, int index);
        app::Clusters::FixedLabel::Structs::LabelStruct::Type operator*() const;
        Iterator & operator++();
        bool operator!=(const Iterator & other) const;

    private:
        const LabelList<N> * mLabelListPtr;
        int mIndex = -1;
    };

public:
    LabelList() = default;
    ~LabelList() { mSize = 0; }

    CHIP_ERROR add(const app::Clusters::FixedLabel::Structs::LabelStruct::Type & label);

    size_t size() const;
    const app::Clusters::FixedLabel::Structs::LabelStruct::Type & operator[](int index) const;

    Iterator begin() const;
    Iterator end() const;

private:
    app::Clusters::FixedLabel::Structs::LabelStruct::Type mList[N];
    int mSize = 0;
};

/*
 * LabelList methods
 **/
template <size_t N>
inline CHIP_ERROR LabelList<N>::add(const app::Clusters::FixedLabel::Structs::LabelStruct::Type & label)
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

template <size_t N>
inline size_t LabelList<N>::size() const
{
    return static_cast<size_t>(mSize);
}

template <size_t N>
inline const app::Clusters::FixedLabel::Structs::LabelStruct::Type & LabelList<N>::operator[](int index) const
{
    VerifyOrDie(index < mSize);
    return mList[index];
}

template <size_t N>
inline typename LabelList<N>::Iterator LabelList<N>::begin() const
{
    return LabelList<N>::Iterator{ this, 0 };
}

template <size_t N>
inline typename LabelList<N>::Iterator LabelList<N>::end() const
{
    return LabelList<N>::Iterator{ this, mSize };
}

/*
 * Iterator methods
 **/
template <size_t N>
inline LabelList<N>::Iterator::Iterator(const LabelList<N> * pLabelList, int index) : mLabelListPtr(pLabelList), mIndex(index)
{}

template <size_t N>
inline app::Clusters::FixedLabel::Structs::LabelStruct::Type LabelList<N>::Iterator::operator*() const
{
    return mLabelListPtr->operator[](mIndex);
}

template <size_t N>
inline typename LabelList<N>::Iterator & LabelList<N>::Iterator::operator++()
{
    ++mIndex;
    return *this;
}

template <size_t N>
inline bool LabelList<N>::Iterator::operator!=(const LabelList<N>::Iterator & other) const
{
    return mIndex != other.mIndex;
}

} // namespace DeviceLayer
} // namespace chip
