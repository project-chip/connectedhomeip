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
        dest->mTextEntryies = static_cast<TextEntry *>(chip::Platform::MemoryCalloc(source.mTextEntrySize, sizeof(TextEntry)));
        VerifyOrExit(dest->mTextEntryies != nullptr, error = CHIP_ERROR_NO_MEMORY);
        dest->mTextEntrySize = source.mTextEntrySize;

        for (size_t i = 0; i < source.mTextEntrySize; i++)
        {
            TextEntry & entry = dest->mTextEntryies[i];
            size_t keySize    = strnlen(source.mTextEntryies[i].mKey, kMdnsTextMaxSize);

            VerifyOrExit(keySize < kMdnsTextMaxSize, error = CHIP_ERROR_INVALID_ARGUMENT);
            entry.mKey = static_cast<char *>(chip::Platform::MemoryAlloc(keySize + 1));
            VerifyOrExit(entry.mKey != nullptr, error = CHIP_ERROR_NO_MEMORY);
            strncpy(const_cast<char *>(entry.mKey), source.mTextEntryies[i].mKey, keySize + 1);
            entry.mData = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(source.mTextEntryies[i].mDataSize));
            VerifyOrExit(entry.mData != nullptr, error = CHIP_ERROR_NO_MEMORY);
            entry.mDataSize = source.mTextEntryies[i].mDataSize;
            memcpy(const_cast<uint8_t *>(entry.mData), source.mTextEntryies[i].mData, entry.mDataSize);
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

ServicePool::Entry::Entry() : mNodeId(0), mFabricId(0), mHasValue(false), mHasPending(false)
{
    mService.mTextEntryies  = nullptr;
    mService.mTextEntrySize = 0;
}

ServicePool::Entry::Entry(Entry && rhs) : mNodeId(0), mFabricId(0)
{
    MoveFrom(std::move(rhs));
}

ServicePool::Entry & ServicePool::Entry::operator=(Entry && rhs)
{
    return MoveFrom(std::move(rhs));
}

ServicePool::Entry & ServicePool::Entry::MoveFrom(Entry && rhs)
{
    mService                          = rhs.mService;
    const_cast<uint64_t &>(mNodeId)   = rhs.mNodeId;
    const_cast<uint64_t &>(mFabricId) = rhs.mFabricId;
    mHasValue                         = rhs.mHasValue;
    mHasPending                       = rhs.mHasPending;

    rhs.mHasValue   = false;
    rhs.mHasPending = false;

    return *this;
}

CHIP_ERROR ServicePool::Entry::Emplace(const MdnsService & service, uint64_t nodeId, uint64_t fabricId)
{
    CHIP_ERROR error;

    Clear();
    mService                          = service;
    const_cast<uint64_t &>(mNodeId)   = nodeId;
    const_cast<uint64_t &>(mFabricId) = fabricId;
    mService.mTextEntryies            = nullptr;
    error                             = CopyTextEntries(&mService, service);

    if (error == CHIP_NO_ERROR)
    {
        mHasValue   = true;
        mHasPending = false;
    }
    return error;
}

CHIP_ERROR ServicePool::Entry::Emplace(MdnsService && service, uint64_t nodeId, uint64_t fabricId)
{
    Clear();
    mService                          = service;
    const_cast<uint64_t &>(mNodeId)   = nodeId;
    const_cast<uint64_t &>(mFabricId) = fabricId;
    service.mTextEntryies             = nullptr;
    service.mTextEntrySize            = 0;
    mHasValue                         = true;
    mHasPending                       = false;

    return CHIP_NO_ERROR;
}

void ServicePool::Entry::Clear()
{
    if (mHasValue)
    {
        FreeTextEntries(&mService);
    }
    mHasValue   = false;
    mHasPending = false;
}

ServicePool::Entry::~Entry()
{
    Clear();
}

ServicePool::Entry & ServicePool::FindAvailableSlot(size_t hashValue)
{
    size_t i   = hashValue;
    bool found = false;

    do
    {
        if (!mEntries[i].mHasValue)
        {
            found = true;
            break;
        }

        i++;
        i %= kServicePoolCapacity;
    } while (i != hashValue);

    if (!found)
    {
        i = hashValue;
    }

    return mEntries[i];
}

CHIP_ERROR ServicePool::AddService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service)
{
    return FindAvailableSlot(nodeId % kServicePoolCapacity).Emplace(service, nodeId, fabricId);
}

CHIP_ERROR ServicePool::AddService(uint64_t nodeId, uint64_t fabricId, MdnsService && service)
{

    return FindAvailableSlot(nodeId % kServicePoolCapacity).Emplace(std::move(service), nodeId, fabricId);
}

CHIP_ERROR ServicePool::RemoveService(uint64_t nodeId, uint64_t fabricId)
{
    Entry * entry = FindService(nodeId, fabricId);

    if (!entry)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    entry->Clear();
    // Because of hash map open addressing, we still need to mark it as "Has pending" to make the search continue
    entry->mHasPending = true;
    mLazyDeleteCount++;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ServicePool::UpdateService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service)
{
    Entry * entry = FindService(nodeId, fabricId);

    if (!entry)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return entry->Emplace(service, nodeId, fabricId);
}

ServicePool::Entry * ServicePool::FindService(uint64_t nodeId, uint64_t fabricId)
{
    size_t hashValue = nodeId % kServicePoolCapacity;
    size_t i         = hashValue;

    do
    {
        if (!mEntries[i].mHasValue && !mEntries[i].mHasPending)
        {
            break;
        }
        if (mEntries[i].mHasValue && mEntries[i].mNodeId == nodeId && mEntries[i].mFabricId == fabricId)
        {
            return &mEntries[i];
        }
        i++;
        i %= kServicePoolCapacity;
    } while (i != hashValue);

    return nullptr;
}

void ServicePool::Clear()
{
    for (Entry & entry : mEntries)
    {
        entry.Clear();
    }
    mLazyDeleteCount = 0;
}

void ServicePool::ReHash()
{
    Entry copyEntries[kServicePoolCapacity];

    for (size_t i = 0; i < kServicePoolCapacity; i++)
    {
        copyEntries[i] = std::move(mEntries[i]);
    }
    Clear();

    for (Entry & copyEntry : copyEntries)
    {
        if (copyEntry.mHasValue)
        {
            printf("Add node id %lu\n", copyEntry.mNodeId);
            AddService(copyEntry.mNodeId, copyEntry.mFabricId, std::move(copyEntry.mService));
        }
    }
}

} // namespace Mdns
} // namespace chip
