#include "ICDMonitoringTable.h"
#include <lib/support/CodeUtils.h>

namespace chip {

enum class Fields : uint8_t
{
    kCheckInNodeID    = 1,
    kMonitoredSubject = 2,
    kKey              = 3,
};

CHIP_ERROR ICDMonitoringEntry::UpdateKey(StorageKeyName & skey)
{
    VerifyOrReturnError(kUndefinedFabricIndex != this->fabricIndex, CHIP_ERROR_INVALID_FABRIC_INDEX);
    skey = DefaultStorageKeyAllocator::ICDManagementTableEntry(this->fabricIndex, index);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDMonitoringEntry::Serialize(TLV::TLVWriter & writer) const
{
    TLV::TLVType outer;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Fields::kCheckInNodeID), checkInNodeID));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Fields::kMonitoredSubject), monitoredSubject));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Fields::kKey), key));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDMonitoringEntry::Deserialize(TLV::TLVReader & reader)
{

    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVType outer;

    ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
    VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
    ReturnErrorOnFailure(reader.EnterContainer(outer));
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        if (TLV::IsContextTag(reader.GetTag()))
        {
            switch (TLV::TagNumFromTag(reader.GetTag()))
            {
            case to_underlying(Fields::kCheckInNodeID):
                ReturnErrorOnFailure(reader.Get(checkInNodeID));
                break;
            case to_underlying(Fields::kMonitoredSubject):
                ReturnErrorOnFailure(reader.Get(monitoredSubject));
                break;
            case to_underlying(Fields::kKey):
                ReturnErrorOnFailure(reader.Get(key));
                break;
            default:
                break;
            }
        }
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}

void ICDMonitoringEntry::Clear()
{
    this->checkInNodeID    = kUndefinedNodeId;
    this->monitoredSubject = kUndefinedNodeId;
    this->key              = ByteSpan();
}

CHIP_ERROR ICDMonitoringTable::Get(uint16_t index, ICDMonitoringEntry & entry) const
{
    entry.fabricIndex = this->mFabric;
    entry.index       = index;
    ReturnErrorOnFailure(entry.Load(this->mStorage));
    entry.fabricIndex = this->mFabric;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDMonitoringTable::Find(NodeId id, ICDMonitoringEntry & entry)
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

CHIP_ERROR ICDMonitoringTable::Set(uint16_t index, const ICDMonitoringEntry & entry)
{
    VerifyOrReturnError(index < this->Limit(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(kUndefinedNodeId != entry.checkInNodeID, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(kUndefinedNodeId != entry.monitoredSubject, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(entry.key.size() == ICDMonitoringEntry::kKeyMaxSize, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(nullptr != entry.key.data(), CHIP_ERROR_INVALID_ARGUMENT);
    ICDMonitoringEntry e(this->mFabric, index);
    e.checkInNodeID    = entry.checkInNodeID;
    e.monitoredSubject = entry.monitoredSubject;
    e.key              = entry.key;
    e.index            = index;
    return e.Save(this->mStorage);
}

CHIP_ERROR ICDMonitoringTable::Remove(uint16_t index)
{
    ICDMonitoringEntry entry(this->mFabric, index);

    // Shift remaining entries down one position
    while (CHIP_NO_ERROR == this->Get(static_cast<uint16_t>(index + 1), entry))
    {
        // WARNING: The key data is held in the entry's serializing buffer
        ICDMonitoringEntry copy = entry;
        this->Set(index++, copy);
    }

    // Remove last entry
    entry.fabricIndex = this->mFabric;
    entry.index       = index;
    return entry.Delete(this->mStorage);
}

CHIP_ERROR ICDMonitoringTable::RemoveAll()
{
    ICDMonitoringEntry entry(this->mFabric);
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

bool ICDMonitoringTable::IsEmpty()
{
    ICDMonitoringEntry entry(this->mFabric);
    return (this->Get(0, entry) == CHIP_ERROR_NOT_FOUND);
}

uint16_t ICDMonitoringTable::Limit() const
{
    return mLimit;
}

} // namespace chip
