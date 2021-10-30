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
#include <lib/core/CHIPTLV.h>
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

    void SaveAttestationElements(size_t count, ByteSpan attestationElements)
    {
        mIsUsed                = true;
        mNumVendorReservedData = count;
        if (count > 0)
        {
            mAttestationData = attestationElements;
        }
    }

    // read TLV until first profile tag
    CHIP_ERROR Init()
    {
        if (!mIsUsed)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        tlvReader.Init(mAttestationData);
        ReturnErrorOnFailure(tlvReader.Next(containerType, TLV::AnonymousTag));
        ReturnErrorOnFailure(tlvReader.EnterContainer(containerType));

        // position to first ProfileTag
        while (true)
        {
            ReturnErrorOnFailure(tlvReader.Next());
            if (!TLV::IsProfileTag(tlvReader.GetTag()))
                break;
        }
        // positioned to first context tag (vendor reserved data)
        mIsInitialized = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Next() { return tlvReader.Next(); }

    size_t GetNumberOfElements() { return mNumVendorReservedData; }

    CHIP_ERROR GetNextVendorReservedElement(struct VendorReservedElement & element)
    {
        if (mIsUsed && !mIsInitialized)
        {
            ReturnErrorOnFailure(Init());
        }

        while (1)
        {
            uint64_t tag = tlvReader.GetTag();
            if (!TLV::IsProfileTag(tag))
            {
                ReturnErrorOnFailure(Next());
                continue;
            }
            // tag is profile tag
            element.vendorId   = TLV::VendorIdFromTag(tag);
            element.profileNum = TLV::ProfileNumFromTag(tag);
            element.tagNum     = TLV::TagNumFromTag(tag);

            return tlvReader.GetByteView(element.vendorReservedData);
        }
    }

private:
    bool mIsUsed = false;          // set to true when we have valid data
    size_t mNumVendorReservedData; // number of VendorReserved entries (could be 0)
    ByteSpan mAttestationData;
    bool mIsInitialized = false;
    TLV::ContiguousBufferTLVReader tlvReader;
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
        if (mCurrent + 1 == mIsUsed)
        {
            return nullptr;
        }
        return &mElements[++mCurrent];
    }

    const_iterator cbegin()
    {
        do_sorting(); // when beginning to iterator, make a linked list and return the head element
        mCurrent = 0;
        return mElements;
    }

    CHIP_ERROR addVendorReservedElement(uint16_t vendorId, uint16_t profileNum, uint32_t tagNum, ByteSpan span)
    {
        if (mIsUsed == mMaxSize)
            return CHIP_ERROR_NO_MEMORY;

        mElements[mIsUsed].tagNum             = tagNum;
        mElements[mIsUsed].profileNum         = profileNum;
        mElements[mIsUsed].vendorId           = vendorId;
        mElements[mIsUsed].vendorReservedData = span;
        mIsUsed++;
        return CHIP_NO_ERROR;
    }

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

        while (starting < mIsUsed)
        {
            uint32_t minVendor = UINT32_MAX;

            // find lowest vendorId
            size_t i;
            for (i = starting; i < mIsUsed; i++)
            {
                if (mElements[i].vendorId < minVendor)
                {
                    minVendor = mElements[i].vendorId;
                }
            }

            uint32_t minProfile = UINT32_MAX;
            // find lowest ProfileNum
            for (i = starting; i < mIsUsed; i++)
            {
                if (mElements[i].vendorId == minVendor)
                {
                    if (mElements[i].profileNum < minProfile)
                        minProfile = mElements[i].profileNum;
                }
            }

            // first lowest tagNum for this vendorId/profileNum
            uint64_t minTagNum = UINT64_MAX;
            size_t lowestIndex;
            for (i = starting; i < mIsUsed; i++)
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
    size_t mMaxSize;    // size of elements array
    size_t mIsUsed = 0; // elements used
    size_t mCurrent;    // iterating from [0...maxSize -1]
};

// allocate space for DeviceAttestationVendorReserved on the stack
// caller should know how many elements are needed ahead of time
#define CREATE_VENDOR_RESERVED(name, size)                                                                                         \
    struct VendorReservedElement _vendorReservedArray[size];                                                                       \
    DeviceAttestationVendorReservedConstructor name(_vendorReservedArray, size);

} // namespace Credentials
} // namespace chip
