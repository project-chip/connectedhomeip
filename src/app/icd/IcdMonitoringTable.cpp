#include "IcdMonitoringTable.h"
#include <lib/support/CodeUtils.h>

namespace chip {

enum class Fields : uint8_t
{
    kCheckInNodeID = static_cast<uint8_t>(PersistentTags::kList) + 1,
    kMonitoredSubject,
    kKey,
};

CHIP_ERROR IcdMonitoringTable::UpdateKey(StorageKeyName & skey)
{
    VerifyOrReturnError(kUndefinedFabricIndex != this->mFabric, CHIP_ERROR_INVALID_FABRIC_INDEX);
    skey = DefaultStorageKeyAllocator::IcdManagementTableEntries(this->mFabric);
    return CHIP_NO_ERROR;
}

bool IcdMonitoringEntry::Compare(const IcdMonitoringEntry & other)
{
    return this->checkInNodeID == other.checkInNodeID;
}

CHIP_ERROR IcdMonitoringEntry::Copy(const IcdMonitoringEntry & other)
{
    VerifyOrReturnError(kUndefinedNodeId != other.checkInNodeID, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(kUndefinedNodeId != other.monitoredSubject, CHIP_ERROR_INVALID_ARGUMENT);
    checkInNodeID    = other.checkInNodeID;
    monitoredSubject = other.monitoredSubject;
    memcpy(key, other.key, kKeyMaxSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR IcdMonitoringEntry::Serialize(TLV::TLVWriter & writer) const
{
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Fields::kCheckInNodeID), this->checkInNodeID));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Fields::kMonitoredSubject), this->monitoredSubject));
    ReturnErrorOnFailure(writer.Put(TLV::ContextTag(Fields::kKey), ByteSpan(this->key, IcdMonitoringEntry::kKeyMaxSize)));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR IcdMonitoringEntry::Deserialize(TLV::TLVReader & reader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLV::TLVType outer;

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outer));
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        if (TLV::IsContextTag(reader.GetTag()))
        {
            switch (TLV::TagNumFromTag(reader.GetTag()))
            {
            case to_underlying(Fields::kCheckInNodeID):
                ReturnErrorOnFailure(reader.Get(this->checkInNodeID));
                break;
            case to_underlying(Fields::kMonitoredSubject):
                ReturnErrorOnFailure(reader.Get(this->monitoredSubject));
                break;
            case to_underlying(Fields::kKey): {
                ByteSpan span;
                ReturnErrorOnFailure(reader.Get(span));
                memcpy(this->key, span.data(), IcdMonitoringEntry::kKeyMaxSize);
                break;
            }
            default:
                break;
            }
        }
    }

    VerifyOrReturnError(err == CHIP_END_OF_TLV, err);
    ReturnErrorOnFailure(reader.ExitContainer(outer));
    return CHIP_NO_ERROR;
}

CHIP_ERROR IcdMonitoringTable::Find(NodeId id, IcdMonitoringEntry & entry)
{
    entry.checkInNodeID = id;
    return this->Get(entry);
}

} // namespace chip
