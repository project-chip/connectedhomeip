#include "ServicePool.h"

#include <string.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/RandUtils.h>

using chip::Mdns::kMdnsTextMaxSize;
using chip::Mdns::MdnsService;
using chip::Mdns::TextEntry;

namespace {

void FreeTextEntries(MdnsService * service)
{
    if (service->mTextEntryies)
    {
        for (size_t i = 0; i < service->mTextEntrySize; i++)
        {
            TextEntry & entry = service->mTextEntryies[i];

            if (entry.mKey)
            {
                chip::Platform::MemoryFree(const_cast<char *>(entry.mKey));
            }
            if (entry.mData)
            {
                chip::Platform::MemoryFree(const_cast<uint8_t *>(entry.mData));
            }
        }
        chip::Platform::MemoryFree(service->mTextEntryies);
    }
}

CHIP_ERROR CopyTextEntries(MdnsService * dest, const MdnsService & source)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (source.mTextEntrySize)
    {
        dest->mTextEntrySize = source.mTextEntrySize;
        dest->mTextEntryies  = static_cast<TextEntry *>(chip::Platform::MemoryCalloc(source.mTextEntrySize, sizeof(TextEntry)));
        VerifyOrExit(dest->mTextEntryies != nullptr, error = CHIP_ERROR_NO_MEMORY);

        for (size_t i = 0; i < source.mTextEntrySize; i++)
        {
            TextEntry & entry = dest->mTextEntryies[i];

            entry.mKey = static_cast<char *>(chip::Platform::MemoryAlloc(strnlen(source.mTextEntryies[i].mKey, kMdnsTextMaxSize)));
            VerifyOrExit(entry.mKey != nullptr, error = CHIP_ERROR_NO_MEMORY);
            entry.mDataSize = source.mTextEntryies[i].mDataSize;
            entry.mData     = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(entry.mDataSize));
            VerifyOrExit(entry.mData != nullptr, error = CHIP_ERROR_NO_MEMORY);
        }
    }

exit:
    if (error != CHIP_NO_ERROR)
    {
        FreeTextEntries(dest);
    }

    return error;
}
} // namespace

namespace chip {
namespace Mdns {

ServicePool::ServicePool()
{
    Clear();
}

CHIP_ERROR ServicePool::AddService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service, bool deepCopy)
{
    size_t hashValue = nodeId % kServicePoolCapacity;
    size_t i         = hashValue;
    bool found       = false;
    CHIP_ERROR error = CHIP_NO_ERROR;

    do
    {
        if (mEntries[i].mLazyDelete || mEntries[i].mNodeId == kUndefinedNodeId)
        {
            found = true;
            break;
        }

        i++;
        i %= kServicePoolCapacity;
    } while (i != hashValue);

    if (!found)
    {
        // Remove one randomly
        i = GetRandU16() % kServicePoolCapacity;
    }

    if (mEntries[i].mLazyDelete)
    {
        mLazyDeleteCount--;
        mEntries[i].mLazyDelete = false;
    }
    mEntries[i].mService  = service;
    mEntries[i].mNodeId   = nodeId;
    mEntries[i].mFabricId = fabricId;
    if (deepCopy)
    {
        error = CopyTextEntries(&mEntries[i].mService, service);
        if (error != CHIP_NO_ERROR)
        {
            mEntries[i].mNodeId = kUndefinedNodeId;
        }
    }
    return error;
}

CHIP_ERROR ServicePool::RemoveService(uint64_t nodeId, uint64_t fabricId)
{
    size_t hashValue = nodeId % kServicePoolCapacity;
    size_t i         = hashValue;

    if (nodeId == kUndefinedNodeId)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    do
    {
        if (!mEntries[i].mLazyDelete && mEntries[i].mNodeId == kUndefinedNodeId)
        {
            break;
        }
        if (!mEntries[i].mLazyDelete && mEntries[i].mNodeId == nodeId && mEntries[i].mFabricId == fabricId)
        {
            mEntries[i].mLazyDelete = true;
            mLazyDeleteCount++;
            FreeTextEntries(&mEntries[i].mService);
        }
        i++;
        i %= kServicePoolCapacity;
    } while (i != hashValue);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ServicePool::UpdateService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service)
{
    MdnsService * foundService;

    if (!FindService(nodeId, fabricId, &foundService))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    FreeTextEntries(foundService);
    *foundService = service;
    return CopyTextEntries(foundService, service);
}

bool ServicePool::FindService(uint64_t nodeId, uint64_t fabricId, MdnsService ** service)
{
    size_t hashValue = nodeId % kServicePoolCapacity;
    size_t i         = hashValue;

    if (nodeId == kUndefinedNodeId)
    {
        *service = nullptr;
        return false;
    }

    do
    {
        if (!mEntries[i].mLazyDelete && mEntries[i].mNodeId == kUndefinedNodeId)
        {
            break;
        }
        if (!mEntries[i].mLazyDelete && mEntries[i].mNodeId == nodeId && mEntries[i].mFabricId == fabricId)
        {
            *service = &mEntries[i].mService;
            return true;
        }
        i++;
        i %= kServicePoolCapacity;
    } while (i != hashValue);

    *service = nullptr;
    return false;
}

void ServicePool::Clear()
{
    for (size_t i = 0; i < kServicePoolCapacity; i++)
    {
        mEntries[i].mNodeId     = kUndefinedNodeId;
        mEntries[i].mLazyDelete = false;
    }
    mLazyDeleteCount = 0;
}

void ServicePool::ReHash()
{
    ServicePoolEntry copyEntries[kServicePoolCapacity];

    memcpy(&copyEntries, mEntries, sizeof(mEntries));
    for (size_t i = 0; i < kServicePoolCapacity; i++)
    {
        if (!mEntries[i].mLazyDelete && mEntries[i].mNodeId != kUndefinedNodeId)
        {
        }
    }

    Clear();
    for (size_t i = 0; i < kServicePoolCapacity; i++)
    {
        if (!copyEntries[i].mLazyDelete && copyEntries[i].mNodeId != kUndefinedNodeId)
        {
        }
    }

    for (size_t i = 0; i < kServicePoolCapacity; i++)
    {
        if (!copyEntries[i].mLazyDelete && copyEntries[i].mNodeId != kUndefinedNodeId)
        {
            AddService(copyEntries[i].mNodeId, copyEntries[i].mFabricId, copyEntries[i].mService, /* deepCopy = */ false);
        }
    }
}

} // namespace Mdns
} // namespace chip
