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

#include <cstdint>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/Span.h>

namespace chip {
namespace Credentials {

struct VendorReservedElement
{
    uint16_t vendorId;
    uint16_t profileNum;
    uint32_t tagNum;
    ByteSpan vendorReservedData;
    struct VendorReservedElement * next; // for linking sorted construction list
    bool used;                           // when sorting, this element is already used
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

    CHIP_ERROR Next()
    {
        ReturnErrorOnFailure(tlvReader.Next());
        return CHIP_NO_ERROR;
    }

    size_t numElements() { return numVendorReservedData; }

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

    typedef struct VendorReservedElement * iterator;
    typedef const struct VendorReservedElement * const_iterator;

    iterator Next()
    {
        current = current->next;
        return current;
    }
    iterator begin()
    {
#if 1
        do_sorting(); // when beginning to iterator, make a linked list and return the head element
        current = head;
        return current;
#else
        if (used > 0)
            return &elements[0];
        else
            return nullptr;
#endif
    }
#if 0
    iterator end()
    {
        if (used > 0)
            return &elements[used];
        else
            return nullptr;
    }
#endif

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
        head = nullptr;
        tail = nullptr;

        if (used == 0)
        {
            return;
        }

        size_t i;

        for (i = 0; i < used; i++)
        {
            elements[i].used = false;
            elements[i].next = nullptr;
        }

        size_t left                                = used;
        struct VendorReservedElement * lastElement = nullptr;
        while (left)
        {
            uint32_t minVendor = UINT32_MAX;

            // find lowest vendor
            for (i = 0; i < used; i++)
            {
                if (!elements[i].used)
                {
                    if (elements[i].vendorId < minVendor)
                        minVendor = elements[i].vendorId;
                }
            }

            uint32_t minProfile = UINT32_MAX;

            // find lowest profileNum
            for (i = 0; i < used; i++)
            {
                if (!elements[i].used)
                {
                    if (elements[i].vendorId == minVendor)
                    {
                        if (elements[i].profileNum < minProfile)
                            minProfile = elements[i].profileNum;
                    }
                }
            }

            uint64_t minTagNum                           = ~0;
            struct VendorReservedElement * lowestElement = nullptr;

            // find lowest tagNum
            for (i = 0; i < used; i++)
            {
                if (!elements[i].used)
                {
                    if (elements[i].tagNum < minTagNum)
                    {
                        minTagNum     = elements[i].tagNum;
                        lowestElement = &elements[i];
                    }
                }
            }

            // lowest element is the next element in list
            if (!head)
            {
                head = lowestElement;
                tail = lowestElement;
            }
            else if (lastElement)
            {

                lastElement->next = lowestElement;
            }
            lowestElement->used = true;
            lastElement         = lowestElement;
            left--;
        }
    }

    VendorReservedElement * elements;
    size_t maxSize;                  // size of elements array
    size_t used = 0;                 // elements used
    VendorReservedElement * current; // element being looked at
    VendorReservedElement * head;    // after threading
    VendorReservedElement * tail;    // after threading
};

// allocate space for DeviceAttestationVendorReserved on the stack
// caller should know how many elements are needed ahead of time
#define CREATE_VENDOR_RESERVED(name, size)                                                                                         \
    struct VendorReservedElement _vendorReservedArray[size];                                                                       \
    DeviceAttestationVendorReservedConstructor name(_vendorReservedArray, size);

} // namespace Credentials
} // namespace chip
