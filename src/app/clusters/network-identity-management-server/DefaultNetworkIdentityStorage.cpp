/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/network-identity-management-server/DefaultNetworkIdentityStorage.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/PopCount.h>
#include <lib/support/logging/CHIPLogging.h>

#include <algorithm>

// Storage Architecture
// ====================
//
// This storage implementation is built on top of PersistentStorageDelegate. All mutating
// NetworkIdentityStorage operations provided by this implementation are atomic, as long
// as the underlying PersistentStorageDelegate implements Sync{Set,Delete}KeyValue
// atomically. This includes operations that mutate multiple underlying K/V records.
//
// The strategy used to achieve this is to have each operation centered around an anchor
// record that achieves durability for the entire operation in a single K/V write.
// Generally this is the singleton table index record of the relevant table, which will be
// updated AFTER writing the detail record during an addition, but BEFORE deleting the
// detail record during removal. This makes the relevant operations atomic both under
// failures as well as after a crash. In the latter case, state will be cleaned up on the
// next startup (incomplete import) or lazily when required (dirty client counts). Note
// that in both failure and crash scenarios it is possible for stale records to be left
// behind ("leaked"). The rollback logic after a failure always restores in-memory state
// and makes a best-effort attempt to delete newly created records, but generally avoids
// overwriting existing records.
//
// ## Network Administrator Shared Secret (NASS)
//
// A singleton TLV record (g/nim/nass) stores the current NASS.
//
// * g/nim/nass
//   [1] createdTimestamp  uint32   Matter Epoch Seconds
//   [2] secretHandle      bytes    raw HkdfKeyHandle buffer
//
// ## Network Identity (NI) Table
//
// The primary key for this table is the Network Identity Index number (range 1 - 0xFFFE).
// This table has a to-many relationship with the Client Table: Each client record
// references the NI against which that client most recently successfully authenticated.
// Conversely, the count of clients referencing a particular NI is tracked in this table.
//
// The table index TLV record (g/nim/ni) contains the NI Index number for all NIs in the
// table, as well as a small amount of meta-data (type, client count), so that important
// operations like looking up the "current" NI for a particular type, or updating the
// client counts when the NI referenced by a client changes can be performed solely
// using the index record. Note that the "current" flag is not stored explicitly: The
// last (i.e. most recently appended) entry in the table index record with a given type
// is considered the current NI for that type.
//
// Detail TLV records (g/nim/n/<index>) contain the bulk of the information for each NI,
// including the identity (certificate) itself, the 20 byte identifier, and the opaque
// keypair handle.
//
// The table index is the source of truth for whether a Network Identity "exists", so it is
// written last during import and deleted first during identity removal; stale / orphaned
// detail records may be left behind in scenarios where errors occur during rollback. In
// the case of an import, the pending{Count,Timestamp} fields are written temporarily along
// with the newly imported identities. They are only considered successfully written if the
// pendingTimestamp matches the current NASS.
//
//  * g/nim/ni                          table index record
//    [1] nextIndex         uint16      next index to try when allocating
//    [2] entries
//      [1] index           uint16      NI Index number
//      [2] type            uint8       type (e.g. kEcdsa)
//      [3] clientCount     uint16      number of clients referencing this NI
//    [3] pendingCount      uint16      optional; number of pending entries
//    [4] pendingTimestamp  uint32      only if pendingCount is present; timestamp of pending NASS
//
// * g/nim/n/<index>                    detail record
//    [1] identifier        bytes       20-byte key identifier
//    [2] createdTimestamp  uint32      timestamp (Matter epoch seconds)
//    [3] compactIdentity   bytes       serialized Compact Network Identity certificate
//    [4] keypairHandle     bytes       opaque keypair handle bytes
//
// ## Client Table
//
// The primary key for this table is the Client Index number (range 1 - 2047).
// Each client record references the NI against which that client most recently
// successfully authenticated (or 0 if it has never authenticated).
//
// The table index record (g/nim/ci) contains an allocation bitmap that tracks which
// indices are in use. A separate set of identifier-to-index mapping records
// (g/nim/@<base85-identifier>) provides O(1) lookup by client identifier without
// scanning the table.
//
// The table index is the source of truth for whether a client "exists", so it is written
// last when adding a client and deleted first during removal; stale / orphaned detail or
// mapping records may be left behind in scenarios where errors occur during rollback.
//
// * g/nim/ci                           table index record
//   [1] nextIndex        uint16        next index to try when allocating
//   [2] allocBitmap      bytes         (256 bytes, bit i = index i is allocated)
//
// * g/nim/c/<index>                    detail record
//   [1] identifier       bytes         20-byte client identifier
//   [2] compactIdentity  bytes         client identity certificate (compact, max 140)
//   [3] niIndex          uint16        NI index, or 0xFFFF (= kNullNetworkIdentityIndex, never authenticated)
//
// * g/nim/@<base85-identifier>         client identifier -> index mapping (raw uint16)

namespace chip::app::Clusters {

// TLV tags and serialized size estimates for all record types.
// Defined as a nested struct to access private members of DefaultNetworkIdentityStorage.
struct DefaultNetworkIdentityStorage::TLVConstants
{
    // NASS singleton record (g/nim/nass)
    static constexpr TLV::Tag kNASS_CreatedTimestamp = TLV::ContextTag(1);
    static constexpr TLV::Tag kNASS_SecretHandle     = TLV::ContextTag(2);
    static constexpr size_t kNASS_Size               = TLV::EstimateStructOverhead( //
        sizeof(uint32_t),                                             // createdTimestamp
        size_t(Crypto::HkdfKeyHandle::Size()));                       // secretHandle

    // NI table index record (g/nim/ni)
    static constexpr TLV::Tag kNIIndex_NextIndex        = TLV::ContextTag(1);
    static constexpr TLV::Tag kNIIndex_Entries          = TLV::ContextTag(2);
    static constexpr TLV::Tag kNIIndex_PendingCount     = TLV::ContextTag(3);
    static constexpr TLV::Tag kNIIndex_PendingTimestamp = TLV::ContextTag(4);
    static constexpr TLV::Tag kNIIndexEntry_Index       = TLV::ContextTag(1);
    static constexpr TLV::Tag kNIIndexEntry_Type        = TLV::ContextTag(2);
    static constexpr TLV::Tag kNIIndexEntry_ClientCount = TLV::ContextTag(3);
    static constexpr size_t kNIIndexEntry_Size          = TLV::EstimateStructOverhead( //
        sizeof(uint16_t), sizeof(uint8_t), sizeof(uint16_t));
    static constexpr size_t kNIIndex_Size               = TLV::EstimateStructOverhead(         //
        sizeof(uint16_t),                                                        // nextIndex
        TLV::EstimateStructOverhead(kNIIndexEntry_Size * kMaxNetworkIdentities), // entries array
        sizeof(uint16_t),                                                        // pendingCount (optional)
        sizeof(uint32_t));                                                       // pendingTimestamp (optional)

    // NI detail record (g/nim/n/<index>)
    static constexpr TLV::Tag kNIDetail_Identifier       = TLV::ContextTag(1);
    static constexpr TLV::Tag kNIDetail_CreatedTimestamp = TLV::ContextTag(2);
    static constexpr TLV::Tag kNIDetail_CompactIdentity  = TLV::ContextTag(3);
    static constexpr TLV::Tag kNIDetail_KeypairHandle    = TLV::ContextTag(4);
    static constexpr size_t kNIDetail_Size               = TLV::EstimateStructOverhead( //
        Credentials::kKeyIdentifierLength,                                // identifier
        sizeof(uint32_t),                                                 // createdTimestamp
        Credentials::kMaxCHIPCompactNetworkIdentityLength,                // compactIdentity
        kMaxKeypairHandleBytesLength);                                    // keypairHandle

    // Client table index record (g/nim/ci)
    static constexpr TLV::Tag kClientIndex_NextIndex   = TLV::ContextTag(1);
    static constexpr TLV::Tag kClientIndex_AllocBitmap = TLV::ContextTag(2);
    static constexpr size_t kClientIndex_Size          = TLV::EstimateStructOverhead( //
        sizeof(uint16_t),                                                    // nextIndex
        sizeof(mClientAllocBitmap));                                         // allocBitmap

    // Client detail record (g/nim/c/<index>)
    static constexpr TLV::Tag kClientDetail_Identifier      = TLV::ContextTag(1);
    static constexpr TLV::Tag kClientDetail_CompactIdentity = TLV::ContextTag(2);
    static constexpr TLV::Tag kClientDetail_NIIndex         = TLV::ContextTag(3);
    static constexpr size_t kClientDetail_Size              = TLV::EstimateStructOverhead( //
        Credentials::kKeyIdentifierLength,                                    // identifier
        Credentials::kMaxCHIPCompactNetworkIdentityLength,                    // compactIdentity
        sizeof(uint16_t));                                                    // niIndex
};

DefaultNetworkIdentityStorage::DefaultNetworkIdentityStorage(PersistentStorageDelegate & storage, uint16_t maxClients) :
    mStorage(storage), mMaxClients(std::min(maxClients, ClientIndexRange::kCardinality))
{}

/////  NASS Storage ///////////////////////////////////////////////////////////////////

CHIP_ERROR DefaultNetworkIdentityStorage::StoreNASS(const NetworkAdministratorSecretInfo & secretInfo)
{
    Crypto::SensitiveDataBuffer<TLVConstants::kNASS_Size> buf; // secretHandle may contain raw secret
    TLV::TLVWriter writer;
    writer.Init(buf.Bytes(), buf.Capacity());

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kNASS_CreatedTimestamp, secretInfo.createdTimestamp.count()));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kNASS_SecretHandle, secretInfo.secretHandle.OpaqueBytes()));
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementAdministratorSecret();
    return mStorage.SyncSetKeyValue(key.KeyName(), buf.Bytes(), static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR DefaultNetworkIdentityStorage::LoadNASS(NetworkAdministratorSecretInfo & outSecretInfo)
{
    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementAdministratorSecret();
    Crypto::SensitiveDataBuffer<TLVConstants::kNASS_Size> buf; // secretHandle may contain raw secret
    uint16_t size  = static_cast<uint16_t>(buf.Capacity());
    CHIP_ERROR err = mStorage.SyncGetKeyValue(key.KeyName(), buf.Bytes(), size);
    VerifyOrReturnError(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_ERROR_NOT_FOUND);
    ReturnErrorOnFailure(err);

    TLV::TLVReader reader;
    TLV::TLVType outerType;
    reader.Init(buf.Bytes(), size);
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    ReturnErrorOnFailure(reader.Next(TLVConstants::kNASS_CreatedTimestamp));
    uint32_t timestamp;
    ReturnErrorOnFailure(reader.Get(timestamp));
    outSecretInfo.createdTimestamp = System::Clock::Seconds32(timestamp);

    ReturnErrorOnFailure(reader.Next(TLVConstants::kNASS_SecretHandle));
    constexpr size_t secretHandleSize = decltype(outSecretInfo.secretHandle)::Size();
    VerifyOrReturnError(reader.GetLength() == secretHandleSize, CHIP_ERROR_INVALID_TLV_ELEMENT);
    ReturnErrorOnFailure(reader.GetBytes(outSecretInfo.secretHandle.OpaqueBytes().data(), secretHandleSize));

    return reader.ExitContainer(outerType);
}

CHIP_ERROR DefaultNetworkIdentityStorage::RetrieveNetworkAdministratorSecret(NetworkAdministratorSecretInfo & outSecretInfo)
{
    CHIP_ERROR err = LoadNASS(outSecretInfo);
    SuccessOrLog(err.NoErrorIf(CHIP_ERROR_NOT_FOUND), Zcl, "Failed to load NASS record");
    return err;
}

CHIP_ERROR DefaultNetworkIdentityStorage::RemoveNetworkAdministratorSecret()
{
    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementAdministratorSecret();
    CHIP_ERROR err     = mStorage.SyncDeleteKeyValue(key.KeyName());
    VerifyOrReturnError(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_ERROR_NOT_FOUND);
    ReturnErrorAndLogOnFailure(err, Zcl, "Failed to delete NASS record");
    return err;
}

/////  Network Identity Storage ///////////////////////////////////////////////////////

uint16_t DefaultNetworkIdentityStorage::AllocateNIIndex()
{
    // Successful allocation / loop termination is guaranteed because the caller has already
    // checked that we have less than kMaxNetworkIdentities identities, and kMaxNetworkIdentities
    // is guaranteed to be <= NetworkIdentityIndexRange::kCardinality.
    using Range = NetworkIdentityIndexRange;
    for (auto index = Range::Clamp(mNINextIndex);; index = Range::Increment(index))
    {
        if (FindNIIndexEntry(index) == nullptr)
        {
            mNINextIndex = Range::Increment(index);
            return index;
        }
    }
}

CHIP_ERROR DefaultNetworkIdentityStorage::StoreNITableIndex(const StoreNITableIndexOptions & options)
{
    VerifyOrDie(mNINextIndex != kTableIndexNotLoaded); // must be loaded / initialized

    uint8_t buf[TLVConstants::kNIIndex_Size];
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kNIIndex_NextIndex, mNINextIndex));

    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLVConstants::kNIIndex_Entries, TLV::kTLVType_Array, arrayType));
    for (size_t i = 0; i < mNICount; i++)
    {
        const auto & entry = mNITableIndex[i];
        if (entry.index == options.indexToSkip)
        {
            continue;
        }

        TLV::TLVType entryType;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, entryType));
        ReturnErrorOnFailure(writer.Put(TLVConstants::kNIIndexEntry_Index, entry.index));
        ReturnErrorOnFailure(writer.Put(TLVConstants::kNIIndexEntry_Type, entry.type));
        ReturnErrorOnFailure(writer.Put(TLVConstants::kNIIndexEntry_ClientCount, //
                                        options.dirtyClientCounts ? kClientCountDirty : entry.clientCount));
        ReturnErrorOnFailure(writer.EndContainer(entryType));
    }
    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    if (options.pendingImportCount > 0)
    {
        ReturnErrorOnFailure(writer.Put(TLVConstants::kNIIndex_PendingCount, options.pendingImportCount));
        ReturnErrorOnFailure(writer.Put(TLVConstants::kNIIndex_PendingTimestamp, options.pendingImportTimestamp.count()));
    }

    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex();
    return mStorage.SyncSetKeyValue(key.KeyName(), buf, static_cast<uint16_t>(writer.GetLengthWritten()));
}

void DefaultNetworkIdentityStorage::LoadNITableIndexIfNeeded()
{
    VerifyOrReturn(mNINextIndex == kTableIndexNotLoaded);

    CHIP_ERROR err;
    TLV::TLVReader reader;

    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex();
    uint8_t buf[TLVConstants::kNIIndex_Size];
    uint16_t size = sizeof(buf);
    SuccessOrExit(err = mStorage.SyncGetKeyValue(key.KeyName(), buf, size));

    reader.Init(buf, size);
    TLV::TLVType outerType;
    SuccessOrExit(err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    SuccessOrExit(err = reader.EnterContainer(outerType));

    SuccessOrExit(err = reader.Next(TLVConstants::kNIIndex_NextIndex));
    decltype(mNINextIndex) nextIndex;
    SuccessOrExit(err = reader.Get(nextIndex));
    mNINextIndex = NetworkIdentityIndexRange::Clamp(nextIndex);

    TLV::TLVType arrayType;
    SuccessOrExit(err = reader.Next(TLV::kTLVType_Array, TLVConstants::kNIIndex_Entries));
    SuccessOrExit(err = reader.EnterContainer(arrayType));
    mNICount = 0;
    while ((err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag())) == CHIP_NO_ERROR)
    {
        VerifyOrExit(mNICount < kMaxNetworkIdentities, err = CHIP_ERROR_INTERNAL);
        auto & entry = mNITableIndex[mNICount++];
        TLV::TLVType entryType;
        SuccessOrExit(err = reader.EnterContainer(entryType));
        SuccessOrExit(err = reader.Next(TLVConstants::kNIIndexEntry_Index));
        SuccessOrExit(err = reader.Get(entry.index));
        SuccessOrExit(err = reader.Next(TLVConstants::kNIIndexEntry_Type));
        SuccessOrExit(err = reader.Get(entry.type));
        SuccessOrExit(err = reader.Next(TLVConstants::kNIIndexEntry_ClientCount));
        SuccessOrExit(err = reader.Get(entry.clientCount));
        SuccessOrExit(err = reader.ExitContainer(entryType));
    }
    SuccessOrExit(err.NoErrorIf(CHIP_ERROR_END_OF_TLV));
    SuccessOrExit(err = reader.ExitContainer(arrayType));

    // Check for optional pending{Count,Timestamp}. If present, a previous
    // StoreNetworkAdministratorSecretAndDerivedIdentities() wrote new identities and
    // update the NI table index, but may not have completed the NASS write.
    if (reader.Next(TLVConstants::kNIIndex_PendingCount) == CHIP_NO_ERROR)
    {
        uint16_t pendingCount;
        SuccessOrExit(err = reader.Get(pendingCount));
        VerifyOrExit(pendingCount <= mNICount, err = CHIP_ERROR_INTERNAL);
        uint32_t pendingTimestamp;
        SuccessOrExit(err = reader.Next(TLVConstants::kNIIndex_PendingTimestamp));
        SuccessOrExit(err = reader.Get(pendingTimestamp));

        // Compare the pendingTimestamp to the NASS timestamp. If they match the import
        // completed, otherwise we have to remove the indicated number of identities.
        // In either case we re-write the table index without the pending marker.
        NetworkAdministratorSecretInfo nassInfo;
        SuccessOrExitAction((err = LoadNASS(nassInfo)).NoErrorIf(CHIP_ERROR_NOT_FOUND),
                            ChipLogFailure(err, Zcl, "Failed to load NASS record during Network Identity table recovery"));
        if (!(err == CHIP_NO_ERROR && nassInfo.createdTimestamp == System::Clock::Seconds32(pendingTimestamp)))
        {
            ChipLogProgress(Zcl, "Rolling back %" PRIu16 " Network Identity entries from incomplete import", pendingCount);
            for (size_t i = 0; i < pendingCount; i++)
            {
                DeleteNetworkIdentityDetail(mNITableIndex[--mNICount].index, /* ignoreNotFound */ true);
            }
        }

        // Rewrite the index without the pending marker (and without the rolled-back entries, if any)
        SuccessOrLog(StoreNITableIndex({}), Zcl, "Failed to clear pending import marker from Network Identity table index");
    }

    SuccessOrExit(err = reader.ExitContainer(outerType));
    return;

exit:
    mNICount     = 0;
    mNINextIndex = NetworkIdentityIndexRange::kMin;
    if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogFailure(err, Zcl, "Failed to load Network Identity table index");
    }
}

CHIP_ERROR DefaultNetworkIdentityStorage::RebuildNIClientCounts()
{
    VerifyOrDie(mNINextIndex != kTableIndexNotLoaded); // must be loaded / initialized

    ChipLogProgress(Zcl, "Rebuilding dirty Network Identity client counts");
    for (size_t i = 0; i < mNICount; i++)
    {
        mNITableIndex[i].clientCount = 0;
    }

    // Scan all client records and tally them based on their networkIdentityIndex
    LoadClientTableIndexIfNeeded();
    CHIP_ERROR err;
    for (uint16_t clientIndex = ClientIndexRange::kMin; clientIndex <= ClientIndexRange::kMax; clientIndex++)
    {
        if (!IsClientIndexAllocated(clientIndex))
        {
            continue;
        }

        ClientEntry clientEntry;
        SuccessOrExit((err = LoadClient(clientIndex, clientEntry, ClientFlags::kPopulateNetworkIdentityIndex, MutableByteSpan())));
        if (clientEntry.networkIdentityIndex == kNullNetworkIdentityIndex)
        {
            continue;
        }

        NetworkIdentityIndexEntry * niEntry = FindClientNIIndexEntry(clientEntry);
        if (niEntry != nullptr)
        {
            niEntry->clientCount++;
        }
    }

    // Our in-memory counts are now accurate, so we can proceed even if persisting them
    // fails: At worst, we will rebuild the counts again after the next restart.
    SuccessOrLog(StoreNITableIndex({}), Zcl, "Failed to persist rebuilt Network Identity client counts");
    return CHIP_NO_ERROR;

exit:
    // If we failed to load any client records, counts won't be accurate. Re-dirty them.
    for (size_t i = 0; i < mNICount; i++)
    {
        mNITableIndex[i].clientCount = kClientCountDirty;
    }
    return err;
}

DefaultNetworkIdentityStorage::NetworkIdentityIndexEntry * DefaultNetworkIdentityStorage::FindNIIndexEntry(uint16_t index)
{
    for (size_t i = 0; i < mNICount; i++)
    {
        auto & entry = mNITableIndex[i];
        if (entry.index == index)
        {
            return &entry;
        }
    }
    return nullptr;
}

DefaultNetworkIdentityStorage::NetworkIdentityIndexEntry *
DefaultNetworkIdentityStorage::FindCurrentNIIndexEntry(NetworkIdentityManagement::IdentityTypeEnum type)
{
    // Iterate in reverse order (most recent last) to find the NI that is current
    for (size_t i = mNICount; i-- > 0;)
    {
        auto & entry = mNITableIndex[i];
        if (entry.type == type)
        {
            return &entry;
        }
    }
    return nullptr;
}

DefaultNetworkIdentityStorage::NetworkIdentityIndexEntry *
DefaultNetworkIdentityStorage::FindClientNIIndexEntry(ClientEntry const & client)
{
    VerifyOrReturnValue(client.networkIdentityIndex != kNullNetworkIdentityIndex, nullptr);
    auto * entry = FindNIIndexEntry(client.networkIdentityIndex);
    if (entry == nullptr)
    {
        ChipLogError(Zcl, "Network Client record %" PRIu16 " references invalid Network Identity %" PRIu16, client.index,
                     client.networkIdentityIndex);
    }
    return entry;
}

CHIP_ERROR DefaultNetworkIdentityStorage::StoreNetworkIdentityDetail(uint16_t index, const NetworkIdentityInfo & info,
                                                                     System::Clock::Seconds32 createdTimestamp)
{
    Crypto::SensitiveDataBuffer<TLVConstants::kNIDetail_Size> buf; // keypairHandle may contain raw key material
    TLV::TLVWriter writer;
    writer.Init(buf.Bytes(), buf.Capacity());

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kNIDetail_Identifier, ByteSpan(info.identifier)));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kNIDetail_CreatedTimestamp, createdTimestamp.count()));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kNIDetail_CompactIdentity, info.compactIdentity));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kNIDetail_KeypairHandle, info.keypairHandle));
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentity(index);
    return mStorage.SyncSetKeyValue(key.KeyName(), buf.Bytes(), static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR DefaultNetworkIdentityStorage::LoadNetworkIdentity(const NetworkIdentityIndexEntry & meta,
                                                              NetworkIdentityEntry & outEntry, BitFlags<NetworkIdentityFlags> flags,
                                                              MutableByteSpan buffer)
{
    outEntry = {};

    // Copy fields that are embedded in the table index entry
    outEntry.index   = meta.index;
    outEntry.type    = meta.type;
    outEntry.current = (FindCurrentNIIndexEntry(meta.type) == &meta);

    // Client count is stored in the index but may need rebuilding
    if (flags.Has(NetworkIdentityFlags::kPopulateClientCount))
    {
        if (meta.clientCount == kClientCountDirty)
        {
            ReturnErrorAndLogOnFailure(RebuildNIClientCounts(), Zcl, "Failed to rebuild Network Identity client counts");
        }
        outEntry.clientCount = meta.clientCount;
    }

    // Remaining fields require loading the detail record
    VerifyOrReturnValue(flags.HasAny(NetworkIdentityFlags::kPopulateIdentifier,       //
                                     NetworkIdentityFlags::kPopulateCreatedTimestamp, //
                                     NetworkIdentityFlags::kPopulateCompactIdentity,  //
                                     NetworkIdentityFlags::kPopulateKeypairHandle),
                        CHIP_NO_ERROR);

    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentity(meta.index);
    Crypto::SensitiveDataBuffer<TLVConstants::kNIDetail_Size> buf;
    uint16_t size  = static_cast<uint16_t>(buf.Capacity());
    CHIP_ERROR err = mStorage.SyncGetKeyValue(key.KeyName(), buf.Bytes(), size);
    VerifyOrReturnError(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(err);

    TLV::TLVReader reader;
    reader.Init(buf.Bytes(), size);

    // Helper for allocating and reading a value into the caller-provided buffer
    size_t bufferOffset    = 0;
    auto ReadByteSpanField = [&](ByteSpan & value, bool populate) {
        value = ByteSpan();
        if (populate)
        {
            auto valueSize = reader.GetLength();
            VerifyOrReturnError(bufferOffset + valueSize <= buffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
            auto valueBuf = buffer.data() + bufferOffset;
            ReturnErrorOnFailure(reader.GetBytes(valueBuf, valueSize));
            value = ByteSpan(valueBuf, valueSize);
            bufferOffset += valueSize;
        }
        return CHIP_NO_ERROR;
    };

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    ReturnErrorOnFailure(reader.Next(TLVConstants::kNIDetail_Identifier));
    if (flags.Has(NetworkIdentityFlags::kPopulateIdentifier))
    {
        constexpr auto identifierSize = decltype(FixedSpan(outEntry.identifier))::size();
        VerifyOrReturnError(reader.GetLength() == identifierSize, CHIP_ERROR_INVALID_TLV_ELEMENT);
        ReturnErrorOnFailure(reader.GetBytes(outEntry.identifier.data(), identifierSize));
    }

    ReturnErrorOnFailure(reader.Next(TLVConstants::kNIDetail_CreatedTimestamp));
    if (flags.Has(NetworkIdentityFlags::kPopulateCreatedTimestamp))
    {
        uint32_t timestamp;
        ReturnErrorOnFailure(reader.Get(timestamp));
        outEntry.createdTimestamp = System::Clock::Seconds32(timestamp);
    }

    ReturnErrorOnFailure(reader.Next(TLVConstants::kNIDetail_CompactIdentity));
    ReturnErrorOnFailure(ReadByteSpanField(outEntry.compactIdentity, flags.Has(NetworkIdentityFlags::kPopulateCompactIdentity)));

    ReturnErrorOnFailure(reader.Next(TLVConstants::kNIDetail_KeypairHandle));
    ReturnErrorOnFailure(ReadByteSpanField(outEntry.keypairHandle, flags.Has(NetworkIdentityFlags::kPopulateKeypairHandle)));
    return reader.ExitContainer(outerType);
}

void DefaultNetworkIdentityStorage::DeleteNetworkIdentityDetail(uint16_t index, bool ignoreNotFound)
{
    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentity(index);
    SuccessOrLog(mStorage.SyncDeleteKeyValue(key.KeyName())
                     .NoErrorIf(ignoreNotFound ? CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND : CHIP_NO_ERROR),
                 Zcl, "Failed to delete Network Identity record %" PRIu16 ", stale record left in place", index);
}

NetworkIdentityStorage::NetworkIdentityIterator *
DefaultNetworkIdentityStorage::IterateNetworkIdentities(BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer)
{
    VerifyOrReturnValue(buffer.size() >= NetworkIdentityBufferSize(flags), nullptr);
    LoadNITableIndexIfNeeded();
    return mNIIterators.CreateObject(*this, flags, buffer);
}

CHIP_ERROR DefaultNetworkIdentityStorage::FindNetworkIdentity(uint16_t index, NetworkIdentityEntry & outEntry,
                                                              BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer)
{
    LoadNITableIndexIfNeeded();
    for (size_t i = 0; i < mNICount; i++)
    {
        if (mNITableIndex[i].index == index)
        {
            ReturnErrorAndLogOnFailure(LoadNetworkIdentity(mNITableIndex[i], outEntry, flags, buffer), Zcl,
                                       "Failed to load Network Identity record %" PRIu16, index);
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR DefaultNetworkIdentityStorage::FindNetworkIdentity(const Credentials::CertificateKeyId & identifier,
                                                              NetworkIdentityEntry & outEntry, BitFlags<NetworkIdentityFlags> flags,
                                                              MutableByteSpan buffer)
{
    // There is no shortcut for directly finding the index given the identifier
    return FindNetworkIdentityByIterating(identifier, outEntry, flags, buffer);
}

CHIP_ERROR DefaultNetworkIdentityStorage::FindCurrentNetworkIdentity(NetworkIdentityManagement::IdentityTypeEnum type,
                                                                     NetworkIdentityEntry & outEntry,
                                                                     BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer)
{
    LoadNITableIndexIfNeeded();
    const auto * meta = FindCurrentNIIndexEntry(type);
    VerifyOrReturnError(meta != nullptr, CHIP_ERROR_NOT_FOUND);
    ReturnErrorAndLogOnFailure(LoadNetworkIdentity(*meta, outEntry, flags, buffer), Zcl,
                               "Failed to load current Network Identity record %" PRIu16 " for type %u", meta->index,
                               static_cast<unsigned>(type));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultNetworkIdentityStorage::StoreNetworkAdministratorSecretAndDerivedIdentities(
    const NetworkAdministratorSecretInfo & nassInfo, Span<const NetworkIdentityInfo * const> identities, Span<uint16_t> outIndices)
{
    LoadNITableIndexIfNeeded();
    VerifyOrReturnError(identities.size() == outIndices.size(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mNICount + identities.size() <= kMaxNetworkIdentities, CHIP_ERROR_NO_MEMORY);

    // Persist NI detail records and then the table index
    CHIP_ERROR err;
    auto const savedNextIndex = mNINextIndex;
    auto const savedNICount   = mNICount;
    if (!identities.empty())
    {
        for (size_t i = 0; i < identities.size(); i++)
        {
            auto & identity = *identities[i];
            auto & outIndex = outIndices[i];

            outIndex = AllocateNIIndex(); // must call before adding the new index entry
            SuccessOrExitAction(err = StoreNetworkIdentityDetail(outIndex, identity, nassInfo.createdTimestamp),
                                ChipLogFailure(err, Zcl, "Failed to store new Network Identity record %" PRIu16, outIndex));

            auto & meta      = mNITableIndex[mNICount++];
            meta.index       = outIndex;
            meta.type        = identity.type;
            meta.clientCount = 0;
        }

        // Add a pending import marker to the table index referencing the new NASS timestamp;
        // it will be removed after we persist the NASS. If we crash part-way through, the
        // pending operation will be completed or rolled back by LoadNITableIndexIfNeeded().
        SuccessOrExitAction(err = StoreNITableIndex({
                                .pendingImportCount     = static_cast<uint16_t>(identities.size()),
                                .pendingImportTimestamp = nassInfo.createdTimestamp,
                            }),
                            ChipLogFailure(err, Zcl, "Failed to store pending Network Identity table index"));
    }

    // Store the NASS last. This makes the whole "transaction" durable.
    SuccessOrExitAction(err = StoreNASS(nassInfo), ChipLogFailure(err, Zcl, "Failed to store NASS record"));

    // Write the table index again to remove the pending marker.
    SuccessOrLog(StoreNITableIndex({}), Zcl, "Failed to finalize Network Identity table index, pending marker left in place");
    return CHIP_NO_ERROR;

exit:
    if (!identities.empty())
    {
        // Restore in-memory table index
        // We don't attempt to re-write the table index record -- see discussion of rollbacks in
        // Storage Architecture above. The stale pending marker will be cleaned up on next startup
        // by LoadNITableIndexIfNeeded(), or by an intervening successful write.
        auto newNICount = mNICount;
        mNICount        = savedNICount;
        mNINextIndex    = savedNextIndex;

        // Delete NI detail records that were written.
        for (auto i = mNICount; i < newNICount; i++)
        {
            DeleteNetworkIdentityDetail(mNITableIndex[i].index);
        }
    }
    return err;
}

CHIP_ERROR DefaultNetworkIdentityStorage::RemoveNetworkIdentity(uint16_t index)
{
    LoadNITableIndexIfNeeded();
    auto * entry = FindNIIndexEntry(index);
    VerifyOrReturnError(entry != nullptr, CHIP_ERROR_NOT_FOUND);

    // A current identity cannot be removed.
    VerifyOrReturnError(FindCurrentNIIndexEntry(entry->type) != entry, CHIP_ERROR_INCORRECT_STATE);

    // An identity referenced by clients cannot be removed. Ensure client count is accurate before checking.
    if (entry->clientCount == kClientCountDirty)
    {
        ReturnErrorOnFailure(RebuildNIClientCounts());
    }
    VerifyOrReturnError(entry->clientCount == 0, CHIP_ERROR_INCORRECT_STATE);

    // Persist the table index, omitting the identity being removed. This makes the removal durable.
    ReturnErrorAndLogOnFailure(StoreNITableIndex({ .indexToSkip = index }), Zcl,
                               "Failed to persist Network Identity table index for identity removal");

    // Remove from the in-memory index by shifting down, preserving the ordering of the remaining
    // entries -- this matters because "current" status is implied by being last in the list.
    std::copy(entry + 1, mNITableIndex + mNICount--, entry);

    // Delete the detail record (best-effort)
    DeleteNetworkIdentityDetail(index);
    return CHIP_NO_ERROR;
}

DefaultNetworkIdentityStorage::NetworkIdentityIteratorImpl::NetworkIdentityIteratorImpl(DefaultNetworkIdentityStorage & storage,
                                                                                        BitFlags<NetworkIdentityFlags> flags,
                                                                                        MutableByteSpan buffer) :
    mStorage(storage),
    mFlags(flags), mBuffer(buffer)
{}

size_t DefaultNetworkIdentityStorage::NetworkIdentityIteratorImpl::Count()
{
    return mStorage.mNICount;
}

bool DefaultNetworkIdentityStorage::NetworkIdentityIteratorImpl::Next(NetworkIdentityEntry & outEntry)
{
    while (mNext < mStorage.mNICount)
    {
        const auto & meta = mStorage.mNITableIndex[mNext++];
        CHIP_ERROR err    = mStorage.LoadNetworkIdentity(meta, outEntry, mFlags, mBuffer);
        VerifyOrDie(err != CHIP_ERROR_BUFFER_TOO_SMALL); // buffer size was validated in IterateNetworkIdentities
        if (err != CHIP_NO_ERROR)
        {
            // Count() will be incorrect -- There is no good way to deal with data corruption in CommonIterator.
            ChipLogFailure(err, Zcl, "Skipping corrupted Network Identity record %" PRIu16, meta.index);
            continue;
        }
        return true;
    }
    return false;
}

void DefaultNetworkIdentityStorage::NetworkIdentityIteratorImpl::Release()
{
    mStorage.mNIIterators.ReleaseObject(this);
}

/////  Client Storage ///////////////////////////////////////////////////////////////////

bool DefaultNetworkIdentityStorage::IsClientIndexAllocated(uint16_t index) const
{
    VerifyOrReturnValue(index <= ClientIndexRange::kMax, false);
    auto & word = mClientAllocBitmap[index / kBitmapWordBits]; // little endian
    auto mask   = BitmapWord{ 1 } << (index % kBitmapWordBits);
    return (word & mask) != 0;
}

void DefaultNetworkIdentityStorage::SetClientIndexAllocated(uint16_t index, bool allocated)
{
    VerifyOrDie(ClientIndexRange::kMin <= index && index <= ClientIndexRange::kMax);
    auto & word = mClientAllocBitmap[index / kBitmapWordBits]; // little endian
    auto mask   = BitmapWord{ 1 } << (index % kBitmapWordBits);
    word        = (allocated) ? word | mask : word & ~mask;
}

uint16_t DefaultNetworkIdentityStorage::AllocateClientIndex()
{
    // Successful allocation / loop termination is guaranteed because the caller has already
    // checked that we have less than mMaxClients clients, and mMaxClients is clamped to
    // ClientIndexRange::Cardinality() by the constructor.
    using Range = ClientIndexRange;
    for (auto index = Range::Clamp(mClientNextIndex);; index = Range::Increment(index))
    {
        if (!IsClientIndexAllocated(index))
        {
            mClientNextIndex = Range::Increment(index);
            return index;
        }
    }
}

CHIP_ERROR DefaultNetworkIdentityStorage::StoreClientTableIndex()
{
    VerifyOrDie(mClientNextIndex != kTableIndexNotLoaded);

    uint8_t buf[TLVConstants::kClientIndex_Size];
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kClientIndex_NextIndex, mClientNextIndex));

    // Convert bitmap to little-endian byte order for storage
    decltype(mClientAllocBitmap) bitmapLE;
    for (size_t i = 0; i < std::size(mClientAllocBitmap); i++)
    {
        bitmapLE[i] = Encoding::LittleEndian::HostSwap(mClientAllocBitmap[i]);
    }
    ReturnErrorOnFailure(writer.PutBytes(TLVConstants::kClientIndex_AllocBitmap, //
                                         reinterpret_cast<const uint8_t *>(bitmapLE), sizeof(bitmapLE)));
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementClientIndex();
    return mStorage.SyncSetKeyValue(key.KeyName(), buf, static_cast<uint16_t>(writer.GetLengthWritten()));
}

void DefaultNetworkIdentityStorage::LoadClientTableIndexIfNeeded()
{
    VerifyOrReturn(mClientNextIndex == kTableIndexNotLoaded);

    CHIP_ERROR err;
    TLV::TLVReader reader;

    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementClientIndex();
    uint8_t buf[TLVConstants::kClientIndex_Size];
    uint16_t size = sizeof(buf);
    SuccessOrExit(err = mStorage.SyncGetKeyValue(key.KeyName(), buf, size));

    reader.Init(buf, size);
    TLV::TLVType outerType;
    SuccessOrExit(err = reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    SuccessOrExit(err = reader.EnterContainer(outerType));

    SuccessOrExit(err = reader.Next(TLVConstants::kClientIndex_NextIndex));
    decltype(mClientNextIndex) nextIndex;
    SuccessOrExit(err = reader.Get(nextIndex));
    mClientNextIndex = ClientIndexRange::Clamp(nextIndex);

    // The AllocBitmap is effectively a single 2048 bit value in little endian order,
    // and the word size used to access it in memory is completely arbitrary. The
    // choice of uint32_t (instead of e.g. uint8_t) will make the popcount operations
    // below (and potentially reading / writing individual bits) slightly more efficient.
    SuccessOrExit(err = reader.Next(TLVConstants::kClientIndex_AllocBitmap));
    VerifyOrExit(reader.GetLength() == sizeof(mClientAllocBitmap), err = CHIP_ERROR_INVALID_TLV_ELEMENT);
    SuccessOrExit(err = reader.GetBytes(reinterpret_cast<uint8_t *>(mClientAllocBitmap), sizeof(mClientAllocBitmap)));
    mClientCount = 0;
    for (auto & word : mClientAllocBitmap)
    {
        word = Encoding::LittleEndian::HostSwap(word);
        mClientCount += PopCount(word); // out-of-range bits (e.g. bit 0) are never set
    }

    SuccessOrExit(err = reader.ExitContainer(outerType));
    return;

exit:
    memset(mClientAllocBitmap, 0, sizeof(mClientAllocBitmap));
    mClientNextIndex = ClientIndexRange::kMin;
    mClientCount     = 0;
    if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogFailure(err, Zcl, "Failed to load Client table index");
    }
}

CHIP_ERROR DefaultNetworkIdentityStorage::StoreClientDetail(uint16_t index, const ClientInfo & info, uint16_t networkIdentityIndex)
{
    uint8_t buf[TLVConstants::kClientDetail_Size];
    TLV::TLVWriter writer;
    writer.Init(buf);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerType));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kClientDetail_Identifier, ByteSpan(info.identifier)));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kClientDetail_CompactIdentity, info.compactIdentity));
    ReturnErrorOnFailure(writer.Put(TLVConstants::kClientDetail_NIIndex, networkIdentityIndex));
    ReturnErrorOnFailure(writer.EndContainer(outerType));
    ReturnErrorOnFailure(writer.Finalize());

    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementClient(index);
    return mStorage.SyncSetKeyValue(key.KeyName(), buf, static_cast<uint16_t>(writer.GetLengthWritten()));
}

CHIP_ERROR DefaultNetworkIdentityStorage::LoadClient(uint16_t index, ClientEntry & outEntry, BitFlags<ClientFlags> flags,
                                                     MutableByteSpan buffer)
{
    outEntry = {};

    outEntry.index = index;
    VerifyOrReturnValue(flags.HasAny(), CHIP_NO_ERROR); // nothing further to do

    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementClient(index);
    uint8_t buf[TLVConstants::kClientDetail_Size];
    uint16_t size  = sizeof(buf);
    CHIP_ERROR err = mStorage.SyncGetKeyValue(key.KeyName(), buf, size);
    VerifyOrReturnError(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(err);

    TLV::TLVReader reader;
    reader.Init(buf, size);

    TLV::TLVType outerType;
    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerType));

    ReturnErrorOnFailure(reader.Next(TLVConstants::kClientDetail_Identifier));
    if (flags.Has(ClientFlags::kPopulateIdentifier))
    {
        constexpr size_t identifierSize = decltype(FixedSpan(outEntry.identifier))::size();
        VerifyOrReturnError(reader.GetLength() == identifierSize, CHIP_ERROR_INVALID_TLV_ELEMENT);
        ReturnErrorOnFailure(reader.GetBytes(outEntry.identifier.data(), identifierSize));
    }

    ReturnErrorOnFailure(reader.Next(TLVConstants::kClientDetail_CompactIdentity));
    if (flags.Has(ClientFlags::kPopulateCompactIdentity))
    {
        auto identitySize = reader.GetLength();
        VerifyOrReturnError(identitySize <= buffer.size(), CHIP_ERROR_BUFFER_TOO_SMALL);
        ReturnErrorOnFailure(reader.GetBytes(buffer.data(), identitySize));
        outEntry.compactIdentity = ByteSpan(buffer.data(), identitySize);
    }

    ReturnErrorOnFailure(reader.Next(TLVConstants::kClientDetail_NIIndex));
    if (flags.Has(ClientFlags::kPopulateNetworkIdentityIndex))
    {
        ReturnErrorOnFailure(reader.Get(outEntry.networkIdentityIndex));
    }

    return reader.ExitContainer(outerType);
}

void DefaultNetworkIdentityStorage::DeleteClientDetail(uint16_t index)
{
    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementClient(index);
    SuccessOrLog(mStorage.SyncDeleteKeyValue(key.KeyName()), Zcl,
                 "Failed to delete Network Client record %" PRIu16 ", stale record left in place", index);
}

CHIP_ERROR DefaultNetworkIdentityStorage::StoreClientIdentifierMapping(const Credentials::CertificateKeyId & identifier,
                                                                       uint16_t clientIndex)
{
    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementClientIdentifierMapping(identifier);
    uint8_t value[sizeof(uint16_t)];
    Encoding::LittleEndian::Put16(value, clientIndex);
    return mStorage.SyncSetKeyValue(key.KeyName(), value, sizeof(value));
}

CHIP_ERROR DefaultNetworkIdentityStorage::LookupClientIdentifierMapping(const Credentials::CertificateKeyId & identifier,
                                                                        uint16_t & outClientIndex)
{
    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementClientIdentifierMapping(identifier);
    uint8_t value[sizeof(uint16_t)];
    uint16_t size  = sizeof(value);
    CHIP_ERROR err = mStorage.SyncGetKeyValue(key.KeyName(), value, size);
    VerifyOrReturnError(err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_ERROR_NOT_FOUND);
    ReturnErrorOnFailure(err);
    VerifyOrReturnError(size == sizeof(uint16_t), CHIP_ERROR_INTERNAL);
    outClientIndex = Encoding::LittleEndian::Get16(value);
    return CHIP_NO_ERROR;
}

void DefaultNetworkIdentityStorage::DeleteClientIdentifierMapping(const Credentials::CertificateKeyId & identifier)
{
    StorageKeyName key = DefaultStorageKeyAllocator::NetworkIdentityManagementClientIdentifierMapping(identifier);
    SuccessOrLog(mStorage.SyncDeleteKeyValue(key.KeyName()), Zcl, "Failed to delete Network Client Identifier mapping");
}

NetworkIdentityStorage::ClientIterator * DefaultNetworkIdentityStorage::IterateClients(BitFlags<ClientFlags> flags,
                                                                                       MutableByteSpan buffer)
{
    VerifyOrReturnValue(buffer.size() >= ClientBufferSize(flags), nullptr);
    LoadClientTableIndexIfNeeded();
    return mClientIterators.CreateObject(*this, flags, buffer);
}

CHIP_ERROR DefaultNetworkIdentityStorage::FindClient(uint16_t index, ClientEntry & outEntry, BitFlags<ClientFlags> flags,
                                                     MutableByteSpan buffer)
{
    LoadClientTableIndexIfNeeded();
    VerifyOrReturnError(IsClientIndexAllocated(index), CHIP_ERROR_NOT_FOUND);
    ReturnErrorAndLogOnFailure(LoadClient(index, outEntry, flags, buffer), //
                               Zcl, "Failed to load Network Client record %" PRIu16, index);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultNetworkIdentityStorage::FindClient(const Credentials::CertificateKeyId & identifier, ClientEntry & outEntry,
                                                     BitFlags<ClientFlags> flags, MutableByteSpan buffer)
{
    LoadClientTableIndexIfNeeded();
    uint16_t clientIndex;
    ReturnErrorOnFailure(LookupClientIdentifierMapping(identifier, clientIndex));
    if (!IsClientIndexAllocated(clientIndex))
    {
        ChipLogProgress(Zcl, "Deleting stale Network Client Identifier mapping for unallocated index %" PRIu16, clientIndex);
        DeleteClientIdentifierMapping(identifier);
        return CHIP_ERROR_NOT_FOUND;
    }

    // Always load the identifier so we can verify the mapping is not stale. A stale mapping
    // can occur if a crash happened after a client was removed (allocation bitmap persisted)
    // but before the mapping record was cleaned up, and the index has since been re-allocated
    // to a different client after wraparound.
    bool callerWantsIdentifier = flags.Has(ClientFlags::kPopulateIdentifier);
    flags.Set(ClientFlags::kPopulateIdentifier);
    ReturnErrorAndLogOnFailure(LoadClient(clientIndex, outEntry, flags, buffer), Zcl,
                               "Failed to load Network Client record %" PRIu16, clientIndex);
    if (!identifier.data_equal(outEntry.identifier))
    {
        ChipLogProgress(Zcl, "Deleting stale (mismatched) Network Client Identifier mapping for index %" PRIu16, clientIndex);
        DeleteClientIdentifierMapping(identifier);
        return CHIP_ERROR_NOT_FOUND;
    }
    if (!callerWantsIdentifier)
    {
        outEntry.identifier = {}; // avoid Hyrum's law
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultNetworkIdentityStorage::AddClient(const ClientInfo & info, uint16_t & outClientIndex)
{
    LoadClientTableIndexIfNeeded();
    VerifyOrReturnError(mClientCount < mMaxClients, CHIP_ERROR_NO_MEMORY);

    CHIP_ERROR err;
    auto const savedNextIndex = mClientNextIndex;
    uint16_t index            = AllocateClientIndex();
    bool wroteDetail          = false;
    bool wroteMapping         = false;

    // Write the detail and mapping records first. They are not the source of truth for existence of the client.
    SuccessOrExitAction(err = StoreClientDetail(index, info, kNullNetworkIdentityIndex),
                        ChipLogFailure(err, Zcl, "Failed to persist Network Client Detail record"));
    wroteDetail = true;

    SuccessOrExitAction(err = StoreClientIdentifierMapping(info.identifier, index),
                        ChipLogFailure(err, Zcl, "Failed to persist Network Client Identifier mapping record"));
    wroteMapping = true;

    // Persisting the table index makes the client durable
    SetClientIndexAllocated(index, true);
    SuccessOrExitAction(err = StoreClientTableIndex(), ChipLogFailure(err, Zcl, "Failed to persist Network Client table index"));

    outClientIndex = index;
    mClientCount++;
    return CHIP_NO_ERROR;

exit:
    // Rollback
    mClientNextIndex = savedNextIndex;
    SetClientIndexAllocated(index, false);

    if (wroteDetail)
    {
        DeleteClientDetail(index);
    }
    if (wroteMapping)
    {
        DeleteClientIdentifierMapping(info.identifier);
    }

    return err;
}

CHIP_ERROR DefaultNetworkIdentityStorage::RemoveClient(uint16_t clientIndex, uint16_t & outNetworkIdentityIndex)
{
    LoadClientTableIndexIfNeeded();
    LoadNITableIndexIfNeeded();
    VerifyOrReturnError(IsClientIndexAllocated(clientIndex), CHIP_ERROR_NOT_FOUND);

    // Load the detail record to get the identifier and NI reference
    ClientEntry entry;
    ReturnErrorAndLogOnFailure(LoadClient(clientIndex, entry,
                                          { ClientFlags::kPopulateIdentifier, ClientFlags::kPopulateNetworkIdentityIndex },
                                          MutableByteSpan()),
                               Zcl, "Failed to load Network Client record %" PRIu16 " for removal", clientIndex);

    auto * niEntry          = FindClientNIIndexEntry(entry);
    outNetworkIdentityIndex = (niEntry != nullptr) ? niEntry->index : kNullNetworkIdentityIndex;

    // Generally we need to update the clientCount on the NI, but if it is already dirty
    // we'll leave it like that; the lazy rebuild will recalculate it when it's requested.
    bool updateClientCount = niEntry != nullptr && niEntry->clientCount != kClientCountDirty && niEntry->clientCount > 0;

    // Mark NI client counts as dirty in case we crash or fail part-way through.
    if (updateClientCount)
    {
        ReturnErrorAndLogOnFailure(StoreNITableIndex({ .dirtyClientCounts = true }), Zcl,
                                   "Failed to mark Network Identity client counts dirty before client removal");
    }

    // Update the allocation bitmap and persist the table index. This makes the removal durable.
    SetClientIndexAllocated(clientIndex, false);
    CHIP_ERROR err = StoreClientTableIndex();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogFailure(err, Zcl, "Failed to persist Client table index for client removal");
        SetClientIndexAllocated(clientIndex, true); // rollback
        return err;
    }
    mClientCount--;

    // Update the in-memory NI client count and persist (best effort)
    if (updateClientCount)
    {
        niEntry->clientCount--;
        SuccessOrLog(StoreNITableIndex({}), Zcl, "Failed to persist Network Identity table index after client removal");
    }

    // Delete detail record and identifier mapping (best effort)
    DeleteClientDetail(clientIndex);
    DeleteClientIdentifierMapping(entry.identifier);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultNetworkIdentityStorage::UpdateClientNetworkIdentityIndex(uint16_t clientIndex, uint16_t newNetworkIdentityIndex,
                                                                           uint16_t & outOldNetworkIdentityIndex)
{
    LoadClientTableIndexIfNeeded();
    outOldNetworkIdentityIndex = 0; // invalid
    VerifyOrReturnError(IsClientIndexAllocated(clientIndex), CHIP_ERROR_NOT_FOUND);

    LoadNITableIndexIfNeeded();
    NetworkIdentityIndexEntry * newNIEntry = nullptr;
    if (newNetworkIdentityIndex != kNullNetworkIdentityIndex)
    {
        newNIEntry = FindNIIndexEntry(newNetworkIdentityIndex);
        VerifyOrReturnError(newNIEntry != nullptr, CHIP_ERROR_NOT_FOUND);
    }

    // Load the full detail record so we can re-persist it with the updated NI index
    ClientEntry entry;
    uint8_t identityBuf[Credentials::kMaxCHIPCompactNetworkIdentityLength];
    ReturnErrorOnFailure(LoadClient(clientIndex, entry, ClientFlags::kPopulateAll, MutableByteSpan(identityBuf)));
    outOldNetworkIdentityIndex = entry.networkIdentityIndex;
    VerifyOrReturnValue(entry.networkIdentityIndex != newNetworkIdentityIndex, CHIP_NO_ERROR); // nothing to do

    // Generally we need to update the clientCount on the old and new NI, but if they're
    // dirty we won't touch them; the lazy rebuild will recalculate them when requested.
    auto * oldNIEntry   = FindNIIndexEntry(entry.networkIdentityIndex);
    bool updateOldCount = oldNIEntry != nullptr && oldNIEntry->clientCount != kClientCountDirty && oldNIEntry->clientCount > 0;
    bool updateNewCount = newNIEntry != nullptr && newNIEntry->clientCount != kClientCountDirty;

    // Mark NI client counts as dirty in case we crash or fail part-way through.
    if (updateOldCount || updateNewCount)
    {
        ReturnErrorAndLogOnFailure(StoreNITableIndex({ .dirtyClientCounts = true }), Zcl,
                                   "Failed to mark Network Identity client counts dirty before client update");
    }

    // Persist the client detail record with the new NI index
    ReturnErrorAndLogOnFailure(StoreClientDetail(clientIndex, entry, newNetworkIdentityIndex), Zcl,
                               "Failed to update Network Client record %" PRIu16, clientIndex);

    // Update the in-memory NI client counts
    if (updateOldCount)
    {
        oldNIEntry->clientCount--;
    }
    if (updateNewCount)
    {
        newNIEntry->clientCount++;
    }

    // Persist update client counts (best effort)
    if (updateOldCount || updateNewCount)
    {
        SuccessOrLog(StoreNITableIndex({}), Zcl, "Failed to persist Network Identity table index after client update");
    }
    return CHIP_NO_ERROR;
}

/////  ClientIterator //////////////////////////////////////////////////////////////////

DefaultNetworkIdentityStorage::ClientIteratorImpl::ClientIteratorImpl(DefaultNetworkIdentityStorage & storage,
                                                                      BitFlags<ClientFlags> flags, MutableByteSpan buffer) :
    mStorage(storage),
    mFlags(flags), mBuffer(buffer)
{}

size_t DefaultNetworkIdentityStorage::ClientIteratorImpl::Count()
{
    return mStorage.mClientCount;
}

bool DefaultNetworkIdentityStorage::ClientIteratorImpl::Next(ClientEntry & outEntry)
{
    using Range = ClientIndexRange;
    while (mNextIndex <= Range::kMax)
    {
        uint16_t index = mNextIndex++;
        if (!mStorage.IsClientIndexAllocated(index))
        {
            continue;
        }
        CHIP_ERROR err = mStorage.LoadClient(index, outEntry, mFlags, mBuffer);
        if (err != CHIP_NO_ERROR)
        {
            // Count() will be incorrect -- There is no good way to deal with data corruption in CommonIterator.
            ChipLogFailure(err, Zcl, "Skipping corrupted Network Client record %" PRIu16, index);
            continue;
        }
        return true;
    }
    return false;
}

void DefaultNetworkIdentityStorage::ClientIteratorImpl::Release()
{
    mStorage.mClientIterators.ReleaseObject(this);
}

} // namespace chip::app::Clusters
