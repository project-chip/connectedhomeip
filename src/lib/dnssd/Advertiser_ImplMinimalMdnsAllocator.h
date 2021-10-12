/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <inttypes.h>

#include <lib/core/CHIPError.h>
#include <lib/dnssd/minimal_mdns/core/FlatAllocatedQName.h>
#include <lib/dnssd/minimal_mdns/core/QName.h>
#include <lib/dnssd/minimal_mdns/responders/IP.h>
#include <lib/dnssd/minimal_mdns/responders/Ptr.h>
#include <lib/dnssd/minimal_mdns/responders/QueryResponder.h>
#include <lib/dnssd/minimal_mdns/responders/RecordResponder.h>
#include <lib/dnssd/minimal_mdns/responders/Srv.h>
#include <lib/dnssd/minimal_mdns/responders/Txt.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace Dnssd {

template <size_t kMaxRecords>
class QueryResponderAllocator
{
public:
    QueryResponderAllocator()
    {
        for (auto & responder : mAllocatedResponders)
        {
            responder = nullptr;
        }
        for (auto & name : mAllocatedQNameParts)
        {
            name = nullptr;
        }
    }
    ~QueryResponderAllocator() { Clear(); }

    /// Appends another responder to the internal replies.
    template <typename ResponderType, typename... Args>
    mdns::Minimal::QueryResponderSettings AddResponder(Args &&... args)
    {
        return AddAllocatedResponder(chip::Platform::New<ResponderType>(std::forward<Args>(args)...));
    }

    template <typename... Args>
    mdns::Minimal::FullQName AllocateQName(Args &&... names)
    {
        void * storage = AllocateQNameSpace(mdns::Minimal::FlatAllocatedQName::RequiredStorageSize(std::forward<Args>(names)...));
        if (storage == nullptr)
        {
            return mdns::Minimal::FullQName();
        }
        return mdns::Minimal::FlatAllocatedQName::Build(storage, std::forward<Args>(names)...);
    }

    mdns::Minimal::FullQName AllocateQNameFromArray(char const * const * names, size_t num)
    {
        void * storage = AllocateQNameSpace(mdns::Minimal::FlatAllocatedQName::RequiredStorageSizeFromArray(names, num));
        if (storage == nullptr)
        {
            return mdns::Minimal::FullQName();
        }
        return mdns::Minimal::FlatAllocatedQName::BuildFromArray(storage, names, num);
    }

    /// Sets the query responder to a blank state and frees up any
    /// allocated memory.
    void Clear()
    {
        // Init clears all responders, so that data can be freed
        mQueryResponder.Init();

        // Free all allocated data
        for (auto & responder : mAllocatedResponders)
        {
            if (responder != nullptr)
            {
                chip::Platform::Delete(responder);
                responder = nullptr;
            }
        }

        for (auto & name : mAllocatedQNameParts)
        {
            if (name != nullptr)
            {
                chip::Platform::MemoryFree(name);
                name = nullptr;
            }
        }
    }
    mdns::Minimal::QueryResponder<kMaxRecords + 1> * GetQueryResponder() { return &mQueryResponder; }
    const mdns::Minimal::RecordResponder * GetResponder(const mdns::Minimal::QType & qtype,
                                                        const mdns::Minimal::FullQName & qname) const
    {
        for (auto & responder : mAllocatedResponders)
        {
            if (responder != nullptr && responder->GetQType() == qtype && responder->GetQName() == qname)
            {
                return responder;
            }
        }
        return nullptr;
    }
    bool IsEmpty() const
    {
        for (auto & responder : mAllocatedResponders)
        {
            if (responder != nullptr)
            {
                return false;
            }
        }
        for (auto & name : mAllocatedQNameParts)
        {
            if (name != nullptr)
            {
                return false;
            }
        }
        return true;
    }

protected:
    // For testing.
    size_t GetMaxAllocatedQNames() { return kMaxAllocatedQNameData; }
    void * GetQNamePart(size_t idx) { return mAllocatedQNameParts[idx]; }
    mdns::Minimal::RecordResponder * GetRecordResponder(size_t idx) { return mAllocatedResponders[idx]; }

private:
    static constexpr size_t kMaxAllocatedQNameData = 32;
    // dynamically allocated items
    mdns::Minimal::RecordResponder * mAllocatedResponders[kMaxRecords];
    void * mAllocatedQNameParts[kMaxAllocatedQNameData];
    // The QueryResponder needs 1 extra space to hold the record for itself.
    mdns::Minimal::QueryResponder<kMaxRecords + 1> mQueryResponder;

    mdns::Minimal::QueryResponderSettings AddAllocatedResponder(mdns::Minimal::RecordResponder * newResponder)
    {
        if (newResponder == nullptr)
        {
            ChipLogError(Discovery, "Responder memory allocation failed");
            return mdns::Minimal::QueryResponderSettings(); // failed
        }

        for (auto & responder : mAllocatedResponders)
        {
            if (responder != nullptr)
            {
                continue;
            }

            responder = newResponder;
            return mQueryResponder.AddResponder(responder);
        }

        Platform::Delete(newResponder);
        ChipLogError(Discovery, "Failed to find free slot for adding a responder");
        return mdns::Minimal::QueryResponderSettings();
    }

    void * AllocateQNameSpace(size_t size)
    {
        for (auto & name : mAllocatedQNameParts)
        {
            if (name != nullptr)
            {
                continue;
            }

            name = chip::Platform::MemoryAlloc(size);
            if (name == nullptr)
            {
                ChipLogError(Discovery, "QName memory allocation failed");
            }
            return name;
        }
        ChipLogError(Discovery, "Failed to find free slot for adding a qname");
        return nullptr;
    }
};

} // namespace Dnssd
} // namespace chip
