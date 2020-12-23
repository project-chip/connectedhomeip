#include "ServicePool.h"

#include <string.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/RandUtils.h>

using chip::Mdns::kMdnsTextMaxSize;
using chip::Mdns::MdnsService;
using chip::Mdns::TextEntry;

namespace chip {
namespace Mdns {

ServicePool::Entry::Entry() : mHasValue(false), mIsTombstone(false), mNodeId(0), mFabricId(0), mPort(0) {}

ServicePool::Entry::Entry(Entry && rhs)
{
    MoveFrom(std::move(rhs));
}

ServicePool::Entry & ServicePool::Entry::MoveFrom(Entry && rhs)
{
    mNodeId      = rhs.mNodeId;
    mFabricId    = rhs.mFabricId;
    mHasValue    = rhs.mHasValue;
    mIsTombstone = rhs.mIsTombstone;
    mAddress     = rhs.mAddress;
    mPort        = rhs.mPort;

    rhs.mHasValue    = false;
    rhs.mIsTombstone = false;

    return *this;
}

void ServicePool::Entry::Emplace(uint64_t nodeId, uint64_t fabricId, const Inet::IPAddress & address, uint16_t port)
{
    mAddress  = address;
    mPort     = port;
    mNodeId   = nodeId;
    mFabricId = fabricId;

    mHasValue    = true;
    mIsTombstone = false;
}

void ServicePool::Entry::Clear()
{
    mHasValue    = false;
    mIsTombstone = false;
}

ServicePool::Entry::~Entry()
{
    Clear();
}

ServicePool::Entry & ServicePool::FindAvailableSlot(uint64_t nodeId)
{
    size_t hashValue = nodeId % kServicePoolCapacity;
    size_t i         = hashValue;
    bool found       = false;

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

void ServicePool::AddService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service)
{
    FindAvailableSlot(nodeId).Emplace(nodeId, fabricId, service.mAddress.Value(), service.mPort);
}

void ServicePool::AddService(uint64_t nodeId, uint64_t fabricId, const Inet::IPAddress & address, uint16_t port)
{
    FindAvailableSlot(nodeId).Emplace(nodeId, fabricId, address, port);
}

CHIP_ERROR ServicePool::RemoveService(uint64_t nodeId, uint64_t fabricId)
{
    Entry * entry = FindServiceMutable(nodeId, fabricId);

    if (!entry)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    entry->Clear();
    // Because of hash map open addressing, we still need to mark it as tombstone to make the search continue
    entry->mIsTombstone = true;
    mLazyDeleteCount++;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ServicePool::UpdateService(uint64_t nodeId, uint64_t fabricId, const MdnsService & service)
{
    Entry * entry = FindServiceMutable(nodeId, fabricId);

    if (!entry)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    entry->Emplace(nodeId, fabricId, service.mAddress.Value(), service.mPort);
    return CHIP_NO_ERROR;
}

const ServicePool::Entry * ServicePool::FindService(uint64_t nodeId, uint64_t fabricId)
{
    return FindServiceMutable(nodeId, fabricId);
}

ServicePool::Entry * ServicePool::FindServiceMutable(uint64_t nodeId, uint64_t fabricId)
{
    size_t hashValue = nodeId % kServicePoolCapacity;
    size_t i         = hashValue;

    do
    {
        if (!mEntries[i].mHasValue && !mEntries[i].mIsTombstone)
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
    EntryList copyEntries(std::move(mEntries));

    mLazyDeleteCount = 0;
    for (Entry & copyEntry : copyEntries)
    {
        if (copyEntry.mHasValue)
        {
            AddService(copyEntry.GetNodeId(), copyEntry.GetFabricId(), copyEntry.GetAddress(), copyEntry.GetPort());
        }
    }
}

} // namespace Mdns
} // namespace chip
