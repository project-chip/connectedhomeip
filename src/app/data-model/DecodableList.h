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
#include <app/data-model/FabricScoped.h>
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
 */
template <typename T>
class DecodableList
{
public:
    DecodableList() { ClearReader(); }

    /*
     * @brief
     *
     * This call stores a TLV reader positioned on the list this class is to manage.
     *
     * Specifically, the passed-in reader should be pointing into the list just after
     * having called `OpenContainer` on the list element.
     */
    void SetReader(const TLV::TLVReader & reader) { mReader = reader; }

    /*
     * @brief
     *
     * This call clears the TLV reader managed by this class, so it can be reused.
     */
    void ClearReader() { mReader.Init(nullptr, 0); }

    /**
     * Make it fabric scoped when the internal field is fabric scoped.
     *
     * This function should not be implemented, since DecodableList won't be used for encode (so using it will cause link time
     * failure).
     */
    template <typename T0 = T, std::enable_if_t<DataModel::IsFabricScoped<T0>::value, bool> = true>
    bool MatchesFabricIndex(FabricIndex);

    class Iterator
    {
    public:
        /*
         * Initialize the iterator with a reference to a reader.
         *
         * This reader should be pointing into the list just after
         * having called `OpenContainer` on the list element, or should
         * have a `kTLVType_NotSpecified` container type if there is
         * no list.
         */
        Iterator(const TLV::TLVReader & reader, Optional<FabricIndex> fabricIndex) : mFabricIndex(fabricIndex)
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
         * Otherwise, if the end of list is reached, or there was no list,
         * this call shall return false.
         *
         * If an error was encountered at any point during the iteration or decode,
         * this shall return false as well. The caller is expected to invoke GetStatus()
         * to retrieve the status of the operation.
         */
        template <
            typename X = T,
            typename std::enable_if_t<
                std::is_same<decltype(DataModel::Decode(std::declval<TLV::TLVReader &>(), std::declval<X &>())), CHIP_ERROR>::value,
                X> * = nullptr>
        bool Next()
        {
            if (mReader.GetContainerType() == TLV::kTLVType_NotSpecified)
            {
                return false;
            }

            if (mStatus == CHIP_NO_ERROR)
            {
                mStatus = mReader.Next();
            }

            if (mStatus == CHIP_NO_ERROR)
            {
                mStatus = DataModel::Decode(mReader, mValue);
            }

            return (mStatus == CHIP_NO_ERROR);
        }

        template <typename X = T,
                  typename std::enable_if_t<
                      std::is_class<X>::value &&
                          std::is_same<decltype(DataModel::Decode(std::declval<TLV::TLVReader &>(), std::declval<X &>(),
                                                                  std::declval<Optional<FabricIndex> &>())),
                                       CHIP_ERROR>::value,
                      X> * = nullptr>
        bool Next()
        {
            if (mReader.GetContainerType() == TLV::kTLVType_NotSpecified)
            {
                return false;
            }

            if (mStatus == CHIP_NO_ERROR)
            {
                mStatus = mReader.Next();
            }

            if (mStatus == CHIP_NO_ERROR)
            {
                mStatus = DataModel::Decode(mReader, mValue, mFabricIndex);
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
        const Optional<FabricIndex> mFabricIndex;
    };

    Iterator begin() const { return Iterator(mReader, mFabricIndex); }

    /*
     * Compute the size of the list. This can fail if the TLV is malformed. If
     * this succeeds, that does not guarantee that the individual items can be
     * successfully decoded; consumers should check Iterator::GetStatus() when
     * actually decoding them. If there is no list then the size is considered
     * to be zero.
     */
    CHIP_ERROR ComputeSize(size_t * size) const
    {
        if (mReader.GetContainerType() == TLV::kTLVType_NotSpecified)
        {
            *size = 0;
            return CHIP_NO_ERROR;
        }
        else
        {
            return mReader.CountRemainingInContainer(size);
        }
    }

    template <typename T0 = T, std::enable_if_t<!DataModel::IsFabricScoped<T0>::value, bool> = true>
    CHIP_ERROR Decode(TLV::TLVReader & reader)
    {
        VerifyOrReturnError(reader.GetType() == TLV::kTLVType_Array, CHIP_ERROR_SCHEMA_MISMATCH);
        TLV::TLVType type;
        ReturnErrorOnFailure(reader.EnterContainer(type));
        SetReader(reader);
        ReturnErrorOnFailure(reader.ExitContainer(type));
        return CHIP_NO_ERROR;
    }

    template <typename T0 = T, std::enable_if_t<DataModel::IsFabricScoped<T0>::value, bool> = true>
    CHIP_ERROR Decode(TLV::TLVReader & reader, const Optional<FabricIndex> & overwriteFabricIndex)
    {
        VerifyOrReturnError(reader.GetType() == TLV::kTLVType_Array, CHIP_ERROR_SCHEMA_MISMATCH);
        TLV::TLVType type;
        ReturnErrorOnFailure(reader.EnterContainer(type));
        SetReader(reader);
        mFabricIndex = overwriteFabricIndex;
        ReturnErrorOnFailure(reader.ExitContainer(type));
        return CHIP_NO_ERROR;
    }

private:
    TLV::TLVReader mReader;
    chip::Optional<FabricIndex> mFabricIndex;
};

} // namespace DataModel
} // namespace app
} // namespace chip
