/*
 *    Copyright (c) 2021-2024 Project CHIP Authors
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
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {

/// Maintains the internal state of list encoding
///
/// List encoding is generally assumed incremental and chunkable (i.e.
/// partial encoding is ok.). For this purpose the class maintains two
/// pieces of data:
///   - AllowPartialData tracks if partial encoding is acceptable in the
///     current encoding state (to be used for atomic/non-atomic list item writes)
///   - CurrentEncodingListIndex representing the list index that is next
///     to be encoded in the output. kInvalidListIndex means that a new list
///     encoding has been started.
class AttributeEncodeState
{
public:
    AttributeEncodeState() = default;

    /// Allows the encode state to be initialized from an OPTIONAL
    /// other encoding state
    ///
    /// if other is nullptr, this is the same as the default initializer.
    AttributeEncodeState(const AttributeEncodeState * other)
    {
        if (other != nullptr)
        {
            *this = *other;
        }
        else
        {
            mCurrentEncodingListIndex = kInvalidListIndex;
            mAllowPartialData         = false;
        }
    }

    bool AllowPartialData() const { return mAllowPartialData; }
    ListIndex CurrentEncodingListIndex() const { return mCurrentEncodingListIndex; }

    AttributeEncodeState & SetAllowPartialData(bool allow)
    {
        mAllowPartialData = allow;
        return *this;
    }

    AttributeEncodeState & SetCurrentEncodingListIndex(ListIndex idx)
    {
        mCurrentEncodingListIndex = idx;
        return *this;
    }

    void Reset()
    {
        mCurrentEncodingListIndex = kInvalidListIndex;
        mAllowPartialData         = false;
    }

private:
    /**
     * If set to kInvalidListIndex, indicates that we have not encoded any data for the list yet and
     * need to start by encoding an empty list before we start encoding any list items.
     *
     * When set to a valid ListIndex value, indicates the index of the next list item that needs to be
     * encoded (i.e. the count of items encoded so far).
     */
    ListIndex mCurrentEncodingListIndex = kInvalidListIndex;

    /**
     * When an attempt to encode an attribute returns an error, the buffer may contain tailing dirty data
     * (since the put was aborted).  The report engine normally rolls back the buffer to right before encoding
     * of the attribute started on errors.
     *
     * When chunking a list, EncodeListItem will atomically encode list items, ensuring that the
     * state of the buffer is valid to send (i.e. contains no trailing garbage), and return an error
     * if the list doesn't entirely fit.  In this situation, mAllowPartialData is set to communicate to the
     * report engine that it should not roll back the list items.
     *
     * TODO: There might be a better name for this variable.
     */
    bool mAllowPartialData = false;
};

} // namespace app
} // namespace chip
