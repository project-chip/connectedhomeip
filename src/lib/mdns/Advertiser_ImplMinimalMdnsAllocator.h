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
#include <inttypes.h>

#include <core/CHIPError.h>
#include <mdns/minimal/core/FlatAllocatedQName.h>
#include <mdns/minimal/core/QName.h>
#include <mdns/minimal/responders/IP.h>
#include <mdns/minimal/responders/Ptr.h>
#include <mdns/minimal/responders/QueryResponder.h>
#include <mdns/minimal/responders/RecordResponder.h>
#include <mdns/minimal/responders/Srv.h>
#include <mdns/minimal/responders/Txt.h>
#include <support/CHIPMem.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Mdns {

template <size_t kMaxRecords>
class QueryResponderAllocator
{
public:
    QueryResponderAllocator()
    {
        for (size_t i = 0; i < kMaxRecords; i++)
        {
            mAllocatedResponders[i] = nullptr;
        }
        for (size_t i = 0; i < kMaxAllocatedQNameData; i++)
        {
            mAllocatedQNameParts[i] = nullptr;
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
        for (size_t i = 0; i < kMaxRecords; i++)
        {
            if (mAllocatedResponders[i] != nullptr)
            {
                chip::Platform::Delete(mAllocatedResponders[i]);
                mAllocatedResponders[i] = nullptr;
            }
        }

        for (size_t i = 0; i < kMaxAllocatedQNameData; i++)
        {
            if (mAllocatedQNameParts[i] != nullptr)
            {
                chip::Platform::MemoryFree(mAllocatedQNameParts[i]);
                mAllocatedQNameParts[i] = nullptr;
            }
        }
    }
    mdns::Minimal::QueryResponder<kMaxRecords + 1> * GetQueryResponder() { return &mQueryResponder; }

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

    mdns::Minimal::QueryResponderSettings AddAllocatedResponder(mdns::Minimal::RecordResponder * responder)
    {
        if (responder == nullptr)
        {
            ChipLogError(Discovery, "Responder memory allocation failed");
            return mdns::Minimal::QueryResponderSettings(); // failed
        }

        for (size_t i = 0; i < kMaxRecords; i++)
        {
            if (mAllocatedResponders[i] != nullptr)
            {
                continue;
            }

            mAllocatedResponders[i] = responder;
            return mQueryResponder.AddResponder(mAllocatedResponders[i]);
        }

        Platform::Delete(responder);
        ChipLogError(Discovery, "Failed to find free slot for adding a responder");
        return mdns::Minimal::QueryResponderSettings();
    }

    void * AllocateQNameSpace(size_t size)
    {
        for (size_t i = 0; i < kMaxAllocatedQNameData; i++)
        {
            if (mAllocatedQNameParts[i] != nullptr)
            {
                continue;
            }

            mAllocatedQNameParts[i] = chip::Platform::MemoryAlloc(size);
            if (mAllocatedQNameParts[i] == nullptr)
            {
                ChipLogError(Discovery, "QName memory allocation failed");
            }
            return mAllocatedQNameParts[i];
        }
        ChipLogError(Discovery, "Failed to find free slot for adding a qname");
        return nullptr;
    }
};

} // namespace Mdns
} // namespace chip
