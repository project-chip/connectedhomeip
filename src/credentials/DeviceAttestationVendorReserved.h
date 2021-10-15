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

private:
    bool used;                    // set to true when we have valid data
    size_t numVendorReservedData; // number of VendorReserved entries (could be 0)
    ByteSpan AttestationData;
    bool initialized;
    TLV::ContiguousBufferTLVReader tlvReader;
    TLV::TLVType containerType = TLV::kTLVType_Structure;

public:
    DeviceAttestationVendorReservedDeconstructor() : used(false), initialized(false) {}

    void SaveAttestationElements(size_t count, ByteSpan attestationElements)
    {
        used                  = true;
        numVendorReservedData = count;
        if (count > 0)
        {
            AttestationData = attestationElements;
        }
    }

    // read TLV until first profile tag
    CHIP_ERROR Init()
    {
        if (!used)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        tlvReader.Init(AttestationData);
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
        initialized = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Next() { return tlvReader.Next(); }

    size_t GetNumberOfElements() { return numVendorReservedData; }

    CHIP_ERROR GetNextVendorReservedElement(struct VendorReservedElement & element)
    {
        if (used && !initialized)
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
};

class DeviceAttestationVendorReservedConstructor
{
public:
    DeviceAttestationVendorReservedConstructor(struct VendorReservedElement * array, size_t size) :
        elements(array), maxSize(size), used(0)
    {}

    typedef const struct VendorReservedElement * const_iterator;

    const_iterator Next()
    {
        if (current + 1 == used)
        {
            return nullptr;
        }
        return &elements[++current];
    }

    const_iterator cbegin()
    {
        do_sorting(); // when beginning to iterator, make a linked list and return the head element
        current = 0;
        return elements;
    }

    CHIP_ERROR addVendorReservedElement(uint16_t vendorId, uint16_t profileNum, uint32_t tagNum, ByteSpan span)
    {
        if (used == maxSize)
            return CHIP_ERROR_NO_MEMORY;

        elements[used].tagNum             = tagNum;
        elements[used].profileNum         = profileNum;
        elements[used].vendorId           = vendorId;
        elements[used].vendorReservedData = span;
        used++;
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

        while (starting < used)
        {
            uint32_t minVendor = UINT32_MAX;

            // find lowest vendorId
            size_t i;
            for (i = starting; i < used; i++)
            {
                if (elements[i].vendorId < minVendor)
                {
                    minVendor = elements[i].vendorId;
                }
            }

            uint32_t minProfile = UINT32_MAX;
            // find lowest ProfileNum
            for (i = starting; i < used; i++)
            {
                if (elements[i].vendorId == minVendor)
                {
                    if (elements[i].profileNum < minProfile)
                        minProfile = elements[i].profileNum;
                }
            }

            // first lowest tagNum for this vendorId/profileNum
            uint64_t minTagNum = UINT64_MAX;
            size_t lowestIndex;
            for (i = starting; i < used; i++)
            {
                if (elements[i].vendorId == minVendor && elements[i].profileNum == minProfile)
                {
                    if (elements[i].tagNum < minTagNum)
                    {
                        minTagNum   = elements[i].tagNum;
                        lowestIndex = i;
                    }
                }
            }

            // lowestIndex is the element to move into elements[starting].
            if (lowestIndex != starting)
            {
                //
                VendorReservedElement tmpElement;

                tmpElement            = elements[starting];
                elements[starting]    = elements[lowestIndex];
                elements[lowestIndex] = tmpElement;
            }
            starting++;
        }
    }

    VendorReservedElement * elements;
    size_t maxSize;  // size of elements array
    size_t used = 0; // elements used
    size_t current;  // iterating from [0...maxSize -1]
};

// allocate space for DeviceAttestationVendorReserved on the stack
// caller should know how many elements are needed ahead of time
#define CREATE_VENDOR_RESERVED(name, size)                                                                                         \
    struct VendorReservedElement _vendorReservedArray[size];                                                                       \
    DeviceAttestationVendorReservedConstructor name(_vendorReservedArray, size);

} // namespace Credentials
} // namespace chip
