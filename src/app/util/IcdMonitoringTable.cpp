#include "IcdMonitoringTable.h"
#include <lib/support/CodeUtils.h>

namespace chip {

CHIP_ERROR IcdMonitoringEntry::UpdateKey(StorageKeyName & skey)
{
    VerifyOrReturnError(kUndefinedFabricIndex != this->fabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
    skey = DefaultStorageKeyAllocator::IcdManagementTableEntry(this->fabricIndex, index);
    return CHIP_NO_ERROR;
}

CHIP_ERROR IcdMonitoringEntry::Serialize(TLV::TLVWriter & writer) const
{
    return this->EncodeForWrite(writer, TLV::AnonymousTag());
}

CHIP_ERROR IcdMonitoringEntry::Deserialize(TLV::TLVReader & reader)
{
    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    return this->Decode(reader);
}

void IcdMonitoringEntry::Clear()
{
    this->checkInNodeID    = kUndefinedNodeId;
    this->monitoredSubject = kUndefinedNodeId;
    this->key              = ByteSpan();
}

CHIP_ERROR IcdMonitoringTable::Get(uint16_t index, IcdMonitoringEntry & entry) const
{
    entry.fabricIndex = this->mFabric;
    entry.index       = index;
    ReturnErrorOnFailure(entry.Load(this->mStorage));
    entry.fabricIndex = this->mFabric;
    return CHIP_NO_ERROR;
}

CHIP_ERROR IcdMonitoringTable::Find(NodeId id, IcdMonitoringEntry & entry)
{
    uint16_t index = 0;
    while (index < this->Limit())
    {
        ReturnErrorOnFailure(this->Get(index++, entry));
        if (id == entry.checkInNodeID)
        {
            return CHIP_NO_ERROR;
        }
    }
    entry.index = index;
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR IcdMonitoringTable::Set(uint16_t index, const IcdMonitoringEntry & entry)
{
    VerifyOrReturnError(index < this->Limit(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(kUndefinedNodeId != entry.checkInNodeID, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(kUndefinedNodeId != entry.monitoredSubject, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(entry.key.size() == IcdMonitoringEntry::kKeyMaxSize, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(nullptr != entry.key.data(), CHIP_ERROR_INVALID_ARGUMENT);
    IcdMonitoringEntry e(this->mFabric, index);
    e.checkInNodeID    = entry.checkInNodeID;
    e.monitoredSubject = entry.monitoredSubject;
    e.key              = entry.key;
    e.index            = index;
    return e.Save(this->mStorage);
}

CHIP_ERROR IcdMonitoringTable::Remove(uint16_t index)
{
    IcdMonitoringEntry entry(this->mFabric, index);

    // Shift remaining entries down one position
    while (CHIP_NO_ERROR == this->Get(static_cast<uint16_t>(index + 1), entry))
    {
        // WARNING: The key data is held in the entry's serializing buffer
        IcdMonitoringEntry copy = entry;
        this->Set(index++, copy);
    }

    // Remove last entry
    entry.fabricIndex = this->mFabric;
    entry.index       = index;
    return entry.Delete(this->mStorage);
}

CHIP_ERROR IcdMonitoringTable::RemoveAll()
{
    IcdMonitoringEntry entry(this->mFabric);
    uint16_t index = 0;
    while (index < this->Limit())
    {
        CHIP_ERROR err = this->Get(index++, entry);
        if (CHIP_ERROR_NOT_FOUND == err)
        {
            break;
        }
        ReturnErrorOnFailure(err);
        entry.fabricIndex = this->mFabric;
        entry.Delete(this->mStorage);
    }
    return CHIP_NO_ERROR;
}

uint16_t IcdMonitoringTable::Limit() const
{
    return mLimit;
}

} // namespace chip
