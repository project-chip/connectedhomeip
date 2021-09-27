/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app/data-model/Decode.h>
#include <app/data-model/Encode.h>
#include <lib/core/CHIPTLV.h>

namespace chip {
namespace app {
namespace DataModel {

/*
 * @brief
 *
 * This class provides an iteratable decoder of list items within TLV payloads
 * such that no memory has to be provided ahead of time to store the entirety of the decoded
 * list contents.
 *
 */
template <typename T>
class DecodableList
{
public:
    /*
     * @brief
     *
     * This call stores a TLV reader positioned on the list this class is to manage.
     *
     * Specifically, the passed-in TLV reader should be pointing into the list just after
     * having called `OpenContainer` on the list element.
     */
    void SetReader(const TLV::TLVReader & reader) { mReader = reader; }

    class Iterator
    {
    public:
        /*
         * Initialize the iterator with a reference to a reader.
         *
         * This reader should be pointing into the list just after
         * having called `OpenContainer` on the list element.
         */
        Iterator(const TLV::TLVReader & reader)
        {
            mStatus = CHIP_NO_ERROR;
            mReader.Init(reader);
        }

        /*
         * Increments the iterator to point to the next list element
         * if a valid one exists, and decodes the list element into
         * the internal value storage.
         *
         * If an element does exist and was successfully decoded, this
         * shall return true.
         *
         * Otherwise, if the end of list is reached, this call shall return
         * false.
         *
         * If an error was encountered at any point during the iteration or decode,
         * this shall return false as well. The caller is expected to invoke GetStatus()
         * to retrieve the status of the operation.
         *
         */
        bool Next()
        {
            if (mStatus == CHIP_NO_ERROR)
            {
                mStatus = mReader.Next();
            }

            if (mStatus == CHIP_NO_ERROR)
            {
                mStatus = Decode(mReader, mValue);
            }

            return (mStatus == CHIP_NO_ERROR);
        }

        /*
         * Retrieves a reference to the decoded value, if one
         * was decoded on a previous call to Next().
         */
        const T & GetValue() const { return mValue; }

        /*
         * Returns the result of all previous operations on this iterator.
         *
         * Notably, if the end-of-list was encountered in a previous call to Next,
         * the status returned shall be CHIP_NO_ERROR.
         *
         */
        CHIP_ERROR GetStatus() const
        {
            if (mStatus == CHIP_END_OF_TLV)
            {
                return CHIP_NO_ERROR;
            }
            else
            {
                return mStatus;
            }
        }

    private:
        T mValue;
        CHIP_ERROR mStatus;
        TLV::TLVReader mReader;
    };

    Iterator begin() const { return Iterator(mReader); }

private:
    TLV::TLVReader mReader;
};

template <typename X>
CHIP_ERROR Decode(TLV::TLVReader & reader, DecodableList<X> & x)
{
    TLV::TLVType type;

    ReturnErrorOnFailure(reader.EnterContainer(type));
    x.SetReader(reader);
    ReturnErrorOnFailure(reader.ExitContainer(type));

    return CHIP_NO_ERROR;
}

} // namespace DataModel
} // namespace app
} // namespace chip
