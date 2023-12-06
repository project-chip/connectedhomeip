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
#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/support/Span.h>

#include <cstdint>

namespace chip {
namespace Credentials {

struct VendorReservedElement
{
    uint16_t vendorId;
    uint16_t profileNum;
    uint32_t tagNum;
    ByteSpan vendorReservedData;
};

// extract elements out of the device attestation bytespan
class DeviceAttestationVendorReservedDeconstructor
{

public:
    DeviceAttestationVendorReservedDeconstructor() {}

    // Read TLV until first profile tag
    CHIP_ERROR PrepareToReadVendorReservedElements(const ByteSpan & attestationElements, size_t count)
    {
        mIsInitialized         = false;
        mIsDone                = false;
        mNumVendorReservedData = count;
        mAttestationData       = attestationElements;

        mTlvReader.Init(mAttestationData);
        ReturnErrorOnFailure(mTlvReader.Next(containerType, TLV::AnonymousTag()));
        ReturnErrorOnFailure(mTlvReader.EnterContainer(containerType));

        // position to first ProfileTag
        while (true)
        {
            CHIP_ERROR err = mTlvReader.Next();
            if (err == CHIP_END_OF_TLV)
            {
                mIsDone = true;
                break;
            }

            ReturnErrorOnFailure(err);

            TLV::Tag tag = mTlvReader.GetTag();
            if (!TLV::IsContextTag(tag))
                break;
        }
        // positioned to first non-context tag (vendor reserved data)
        mIsInitialized  = true;
        mIsAtFirstToken = true;
        return CHIP_NO_ERROR;
    }

    size_t GetNumberOfElements() const { return mNumVendorReservedData; }

    /**
     *  @brief Return next VendorReserved element. PrepareToReadVendorReservedElements must be called first.
     *
     *  @param[out] element  Next vendor Reserved element
     *
     *  @returns   CHIP_NO_ERROR on success
     *             CHIP_ERROR_INCORRECT_STATE if PrepareToReadVendorReservedElements hasn't been called first
     *             CHIP_ERROR_UNEXPECTED_TLV_ELEMENT if we reach non-profile-specific tags or vendorId is zero
     *             CHIP_END_OF_TLV if not further entries are present
     */
    CHIP_ERROR GetNextVendorReservedElement(struct VendorReservedElement & element)
    {
        VerifyOrReturnError(mIsInitialized, CHIP_ERROR_UNINITIALIZED);
        if (mIsDone)
        {
            return CHIP_END_OF_TLV;
        }

        if (mIsAtFirstToken)
        {
            // Already had a Next() done for us by PrepareToReadVendorReservedElements
            // so we don't Next() since we should be pointing at a vendor-reserved.
            mIsAtFirstToken = false;
        }
        else
        {
            CHIP_ERROR error = mTlvReader.Next();
            if (error == CHIP_END_OF_TLV)
            {
                mIsDone = true;
            }
            ReturnErrorOnFailure(error);
        }

        TLV::Tag tag = mTlvReader.GetTag();
        if (!TLV::IsProfileTag(tag))
        {
            return CHIP_ERROR_UNEXPECTED_TLV_ELEMENT;
        }

        // tag is profile tag
        element.vendorId   = TLV::VendorIdFromTag(tag);
        element.profileNum = TLV::ProfileNumFromTag(tag);
        element.tagNum     = TLV::TagNumFromTag(tag);

        ReturnErrorOnFailure(mTlvReader.GetByteView(element.vendorReservedData));

        return CHIP_NO_ERROR;
    }

private:
    size_t mNumVendorReservedData; // number of VendorReserved entries (could be 0)
    ByteSpan mAttestationData;
    bool mIsInitialized  = false;
    bool mIsAtFirstToken = false;
    bool mIsDone         = false;
    TLV::ContiguousBufferTLVReader mTlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;
};

class DeviceAttestationVendorReservedConstructor
{
public:
    DeviceAttestationVendorReservedConstructor(struct VendorReservedElement * array, size_t size) : mElements(array), mMaxSize(size)
    {}

    typedef const struct VendorReservedElement * const_iterator;

    const_iterator Next()
    {
        VerifyOrReturnError(mCurrentIndex < mNumEntriesUsed, nullptr);
        return &mElements[mCurrentIndex++];
    }

    const_iterator cbegin()
    {
        // sort the array in place and return the head element.
        do_sorting();
        mCurrentIndex = 0;
        return mElements;
    }

    CHIP_ERROR addVendorReservedElement(uint16_t vendorId, uint16_t profileNum, uint32_t tagNum, ByteSpan span)
    {
        if (mNumEntriesUsed == mMaxSize)
            return CHIP_ERROR_NO_MEMORY;

        mElements[mNumEntriesUsed].tagNum             = tagNum;
        mElements[mNumEntriesUsed].profileNum         = profileNum;
        mElements[mNumEntriesUsed].vendorId           = vendorId;
        mElements[mNumEntriesUsed].vendorReservedData = span;
        mNumEntriesUsed++;
        return CHIP_NO_ERROR;
    }

    size_t GetNumberOfElements() const { return mNumEntriesUsed; }

private:
    /*
     * Sort according to A.2.4 in the spec.
     * Mark all sorted entries by setting used flag.
     * Order is head to tail, sorted by next
     * Executed when entries are about to be read
     */
    void do_sorting()
    {
        size_t starting = 0;

        while (starting < mNumEntriesUsed)
        {
            uint32_t minVendor = UINT32_MAX;

            // find lowest vendorId
            size_t i;
            for (i = starting; i < mNumEntriesUsed; i++)
            {
                if (mElements[i].vendorId < minVendor)
                {
                    minVendor = mElements[i].vendorId;
                }
            }

            uint32_t minProfile = UINT32_MAX;
            // find lowest ProfileNum
            for (i = starting; i < mNumEntriesUsed; i++)
            {
                if (mElements[i].vendorId == minVendor)
                {
                    if (mElements[i].profileNum < minProfile)
                        minProfile = mElements[i].profileNum;
                }
            }

            // first lowest tagNum for this vendorId/profileNum
            uint64_t minTagNum = UINT64_MAX;
            size_t lowestIndex = SIZE_MAX;
            for (i = starting; i < mNumEntriesUsed; i++)
            {
                if (mElements[i].vendorId == minVendor && mElements[i].profileNum == minProfile)
                {
                    if (mElements[i].tagNum < minTagNum)
                    {
                        minTagNum   = mElements[i].tagNum;
                        lowestIndex = i;
                    }
                }
            }

            // lowestIndex is the element to move into elements[starting].
            if (lowestIndex != starting)
            {
                //
                VendorReservedElement tmpElement;

                tmpElement             = mElements[starting];
                mElements[starting]    = mElements[lowestIndex];
                mElements[lowestIndex] = tmpElement;
            }
            starting++;
        }
    }

    VendorReservedElement * mElements;
    size_t mMaxSize;            // size of elements array
    size_t mNumEntriesUsed = 0; // elements used
    size_t mCurrentIndex;       // iterating from [0...maxSize -1]
};

} // namespace Credentials
} // namespace chip
