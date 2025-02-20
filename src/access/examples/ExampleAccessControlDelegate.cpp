/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "ExampleAccessControlDelegate.h"

#include <lib/core/CHIPConfig.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <type_traits>

namespace {

using chip::ClusterId;
using chip::DeviceTypeId;
using chip::EndpointId;
using chip::FabricIndex;
using chip::NodeId;

using chip::kUndefinedNodeId;

using chip::Access::AccessControl;
using chip::Access::AuthMode;
using chip::Access::Privilege;
using chip::Access::RequestPath;
using chip::Access::SubjectDescriptor;

using Entry         = chip::Access::AccessControl::Entry;
using EntryIterator = chip::Access::AccessControl::EntryIterator;
using Target        = Entry::Target;

// Pool sizes
constexpr int kEntryStoragePoolSize          = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_STORAGE_POOL_SIZE;
constexpr int kEntryDelegatePoolSize         = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_DELEGATE_POOL_SIZE;
constexpr int kEntryIteratorDelegatePoolSize = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_ITERATOR_DELEGATE_POOL_SIZE;

/*
              +---+   +---+   +---+   +---+
              | 1 |   | 2 |   | A |   | C |             ENTRIES
              +---+   +---+   +---+   +---+
                 |      |       |       |
                 |      +-+   +-+       |
                 +----+   |   |   +-----+
                      |   |   |   |
                      v   v   v   v
                    +---+---+---+---+
                    | 1 | 2 | A | C |                   ENTRY DELEGATE POOL
                    +---+---+---+---+
                      |   |   |   |
          +-----------+   |   |   +-----------+
          |   +-----------+   +-------+       |
          |   |                       |       |
          v   v                       v       v
    +---+---+---+---+               +---+---+---+---+
    | 0 | 1 | 2 | X |               | A | X | C | X |   ACL ENTRY STORAGE & POOL
    +---+---+---+---+               +---+---+---+---+
      ^       ^
      |       |
      |       +-----------+
      +---------------+   |
                      |   |
                    +---+---+---+---+
                    | 0 | 2 | X | X |                   ENTRY ITERATOR DELEGATE POOL
                    +---+---+---+---+
                      ^   ^
                      |   |
                      |   +-------+
                      |           |
                    +---+       +---+
                    | 0 |       | 2 |                   ENTRY ITERATORS
                    +---+       +---+
*/

class SubjectStorage
{
public:
    bool IsEmpty() const { return mNode == kUndefinedNodeId; }

    void Clear() { mNode = kUndefinedNodeId; }

    CHIP_ERROR Get(NodeId & node) const
    {
        if (!IsEmpty())
        {
            node = mNode;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR Set(NodeId node)
    {
        if (!IsEmpty())
        {
            if (IsValid(node))
            {
                mNode = node;
                return CHIP_NO_ERROR;
            }
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR Add(NodeId node)
    {
        if (IsValid(node))
        {
            mNode = node;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

private:
    static bool IsValid(NodeId node) { return node != kUndefinedNodeId; }

    static_assert(sizeof(NodeId) == 8, "Expecting 8 byte node ID");

    NodeId mNode;
};

class TargetStorage
{
public:
    bool IsEmpty() const { return mCluster == kClusterEmpty && mDeviceType == kDeviceTypeEmpty; }

    void Clear()
    {
        mCluster    = kClusterEmpty;
        mDeviceType = kDeviceTypeEmpty;
    }

    CHIP_ERROR Get(Target & target) const
    {
        if (!IsEmpty())
        {
            Decode(target);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR Set(const Target & target)
    {
        if (!IsEmpty())
        {
            if (IsValid(target))
            {
                Encode(target);
                return CHIP_NO_ERROR;
            }
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR Add(const Target & target)
    {
        if (IsValid(target))
        {
            Encode(target);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

private:
    // TODO: eventually this functionality should live where the type itself is defined
    static bool IsValidCluster(ClusterId cluster)
    {
        const auto id     = cluster & kClusterIdMask;
        const auto vendor = cluster & kClusterVendorMask;
        return ((id <= kClusterIdMaxStd) || (kClusterIdMinMs <= id && id <= kClusterIdMaxMs)) && (vendor <= kClusterVendorMax);
    }

    // TODO: eventually this functionality should live where the type itself is defined
    static constexpr bool IsValidEndpoint(EndpointId endpoint) { return true; }

    // TODO: eventually this functionality should live where the type itself is defined
    static bool IsValidDeviceType(DeviceTypeId deviceType)
    {
        const auto id     = deviceType & kDeviceTypeIdMask;
        const auto vendor = deviceType & kDeviceTypeVendorMask;
        return (id <= kDeviceTypeIdMax) && (vendor <= kDeviceTypeVendorMax);
    }

    // TODO: eventually this functionality should live where the type itself is defined
    static bool IsValid(const Target & target)
    {
        constexpr Target::Flags kNotAll     = Target::kEndpoint | Target::kDeviceType;
        constexpr Target::Flags kAtLeastOne = kNotAll | Target::kCluster;
        constexpr Target::Flags kNone       = ~kAtLeastOne;
        return ((target.flags & kNone) == 0) && ((target.flags & kAtLeastOne) != 0) && ((target.flags & kNotAll) != kNotAll) &&
            !((target.flags & Target::kCluster) && !IsValidCluster(target.cluster)) &&
            !((target.flags & Target::kEndpoint) && !IsValidEndpoint(target.endpoint)) &&
            !((target.flags & Target::kDeviceType) && !IsValidDeviceType(target.deviceType));
    }

    void Decode(Target & target) const
    {
        auto & flags      = target.flags;
        auto & cluster    = target.cluster;
        auto & endpoint   = target.endpoint;
        auto & deviceType = target.deviceType;
        flags             = 0;
        if (mCluster != kClusterEmpty)
        {
            cluster = mCluster;
            flags |= Target::kCluster;
        }
        if (mDeviceType != kDeviceTypeEmpty)
        {
            if ((mDeviceType & kDeviceTypeIdMask) == kEndpointMagic)
            {
                endpoint = static_cast<EndpointId>(mDeviceType >> kEndpointShift);
                flags |= Target::kEndpoint;
            }
            else
            {
                deviceType = mDeviceType;
                flags |= Target::kDeviceType;
            }
        }
    }

    void Encode(const Target & target)
    {
        const auto flags      = target.flags;
        const auto cluster    = target.cluster;
        const auto endpoint   = target.endpoint;
        const auto deviceType = target.deviceType;
        if (flags & Target::kCluster)
        {
            mCluster = cluster;
        }
        else
        {
            mCluster = kClusterEmpty;
        }
        if (flags & Target::kEndpoint)
        {
            mDeviceType = (static_cast<DeviceTypeId>(endpoint) << kEndpointShift) | kEndpointMagic;
        }
        else if (flags & Target::kDeviceType)
        {
            mDeviceType = deviceType;
        }
        else
        {
            mDeviceType = kDeviceTypeEmpty;
        }
    }

    static_assert(sizeof(ClusterId) == 4, "Expecting 4 byte cluster ID");
    static_assert(sizeof(EndpointId) == 2, "Expecting 2 byte endpoint ID");
    static_assert(sizeof(DeviceTypeId) == 4, "Expecting 4 byte device type ID");

    // TODO: some (not all) of these values should live where the type itself is defined

    // (mCluster == kClusterEmpty) --> mCluster contains no cluster
    static constexpr ClusterId kClusterEmpty = 0xFFFFFFFF;

    // (mCluster & kClusterIdMask) --> cluster id portion
    static constexpr ClusterId kClusterIdMask = 0x0000FFFF;

    // ((mCluster & kClusterIdMask) < kClusterIdMaxStd) --> invalid
    static constexpr ClusterId kClusterIdMaxStd = 0x00007FFF;

    // ((mCluster & kClusterIdMask) < kClusterIdMinMs) --> invalid
    static constexpr ClusterId kClusterIdMinMs = 0x0000FC00;

    // ((mCluster & kClusterIdMask) < kClusterIdMaxMs) --> invalid
    static constexpr ClusterId kClusterIdMaxMs = 0x0000FFFE;

    // (mCluster & kClusterVendorMask) --> cluster vendor portion
    static constexpr ClusterId kClusterVendorMask = 0xFFFF0000;

    // ((mCluster & kClusterVendorMask) > kClusterVendorMax) --> invalid
    static constexpr ClusterId kClusterVendorMax = 0xFFFE0000;

    // (mDeviceType == kDeviceTypeEmpty) --> mDeviceType contains neither endpoint nor device type
    static constexpr DeviceTypeId kDeviceTypeEmpty = 0xFFFFFFFF;

    // (mDeviceType & kDeviceTypeIdMask) --> device type id portion
    static constexpr DeviceTypeId kDeviceTypeIdMask = 0x0000FFFF;

    // ((mDeviceType & kDeviceTypeIdMask) < kDeviceTypeIdMax) --> invalid
    static constexpr DeviceTypeId kDeviceTypeIdMax = 0x0000BFFF;

    // (mDeviceType & kDeviceTypeVendorMask) --> device type vendor portion
    static constexpr DeviceTypeId kDeviceTypeVendorMask = 0xFFFF0000;

    // ((mDeviceType & kDeviceTypeVendorMask) > kDeviceTypeVendorMax) --> invalid
    static constexpr DeviceTypeId kDeviceTypeVendorMax = 0xFFFE0000;

    // ((mDeviceType & kDeviceTypeIdMask) == kEndpointMagic) --> mDeviceType contains endpoint
    static constexpr DeviceTypeId kEndpointMagic = 0x0000EEEE;

    // (mDeviceType >> kEndpointShift) --> extract endpoint from mDeviceType
    static constexpr int kEndpointShift = 16;

    ClusterId mCluster;
    DeviceTypeId mDeviceType;
};

class EntryStorage
{
public:
    // ACL support
    static constexpr size_t kNumberOfFabrics  = CHIP_CONFIG_MAX_FABRICS;
    static constexpr size_t kEntriesPerFabric = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC;
    static EntryStorage acl[kNumberOfFabrics * kEntriesPerFabric];

    // Find the next unused entry storage in the access control list, if one exists.
    static EntryStorage * FindUnusedInAcl()
    {
        for (auto & storage : acl)
        {
            if (!storage.InUse())
            {
                return &storage;
            }
        }
        return nullptr;
    }

    // Find the specified used entry storage in the access control list, if it exists.
    static EntryStorage * FindUsedInAcl(size_t index, const FabricIndex * fabricIndex)
    {
        if (fabricIndex != nullptr)
        {
            ConvertIndex(index, *fabricIndex, ConvertDirection::kRelativeToAbsolute);
        }
        if (index < MATTER_ARRAY_SIZE(acl))
        {
            auto * storage = acl + index;
            if (storage->InUse())
            {
                return storage;
            }
        }
        return nullptr;
    }

    // Pool support
    static EntryStorage pool[kEntryStoragePoolSize];

    // Find an unused entry storage in the pool, if one is available.
    // The candidate is preferred if provided and it is in the pool,
    // regardless of whether it is already in use.
    static EntryStorage * Find(EntryStorage * candidate)
    {
        if (candidate && candidate->InPool())
        {
            return candidate;
        }
        for (auto & storage : pool)
        {
            if (!storage.InUse())
            {
                return &storage;
            }
        }
        return nullptr;
    }

    bool InPool() const
    {
        constexpr auto * end = pool + MATTER_ARRAY_SIZE(pool);
        return pool <= this && this < end;
    }

    EntryStorage() = default;

    void Init()
    {
        if (!mInUse)
        {
            Clear();
            mInUse = true;
        }
    }

    bool InUse() const { return mInUse; }

    void Release()
    {
        if (InPool())
        {
            mInUse = false;
        }
    }

    void Clear()
    {
        mInUse       = false;
        mFabricIndex = chip::kUndefinedFabricIndex;
        mAuthMode    = AuthMode::kPase;
        mPrivilege   = Privilege::kView;
        for (auto & subject : mSubjects)
        {
            subject.Clear();
        }
        for (auto & target : mTargets)
        {
            target.Clear();
        }
    }

    enum class ConvertDirection
    {
        kAbsoluteToRelative,
        kRelativeToAbsolute
    };

    // Entries have a position in the access control list, denoted by an "absolute" index.
    // Because entries are scoped to a fabric, a "fabric relative" index can be inferred.
    //
    // For example: suppose there are 8 entries for fabrics A, B, and C (as fabric indexes 1, 2, 3).
    //
    //        0   1   2   3   4   5   6   7     ABSOLUTE INDEX
    //      +---+---+---+---+---+---+---+---+
    //      | A0| A1| B0| A2| B1| B2| C0| C1|   FABRIC RELATIVE INDEX
    //      +---+---+---+---+---+---+---+---+
    //
    // While the entry at (absolute) index 2 is the third entry, it is the first entry scoped to
    // fabric B. So relative to fabric index 2, the entry is at (relative) index 0.
    //
    // The opposite is true: the second entry scoped to fabric B, at (relative) index 1, is the
    // fifth entry overall, at (absolute) index 4.
    //
    // Not all conversions are possible. For example, absolute index 3 is not scoped to fabric B, so
    // attempting to convert it to be relative to fabric index 2 will fail. Likewise, fabric B does
    // not contain a fourth entry, so attempting to convert index 3 (relative to fabric index 2) to
    // an absolute index will also fail. Such failures are denoted by use of an index that is one
    // past the end of the access control list. (So in this example, failure produces index 8.)
    static void ConvertIndex(size_t & index, const FabricIndex fabricIndex, ConvertDirection direction)
    {
        size_t absoluteIndex = 0;
        size_t relativeIndex = 0;
        size_t & fromIndex   = (direction == ConvertDirection::kAbsoluteToRelative) ? absoluteIndex : relativeIndex;
        size_t & toIndex     = (direction == ConvertDirection::kAbsoluteToRelative) ? relativeIndex : absoluteIndex;
        bool found           = false;
        for (const auto & storage : acl)
        {
            if (!storage.InUse())
            {
                break;
            }
            if (storage.mFabricIndex == fabricIndex)
            {
                if (index == fromIndex)
                {
                    found = true;
                    break;
                }
                relativeIndex++;
            }
            absoluteIndex++;
        }
        index = found ? toIndex : MATTER_ARRAY_SIZE(acl);
    }

    static constexpr size_t kMaxSubjects = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY;
    static constexpr size_t kMaxTargets  = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY;

    bool mInUse;
    FabricIndex mFabricIndex;
    AuthMode mAuthMode;
    Privilege mPrivilege;
    SubjectStorage mSubjects[kMaxSubjects];
    TargetStorage mTargets[kMaxTargets];
};

class EntryDelegate : public Entry::Delegate
{
public:
    // Pool support
    static EntryDelegate pool[kEntryDelegatePoolSize];

    // Find an unused entry delegate in the pool, if one is available.
    // The candidate is preferred if it is in the pool, regardless of whether
    // it is already in use.
    static EntryDelegate * Find(Entry::Delegate & candidate)
    {
        if (InPool(candidate))
        {
            return &static_cast<EntryDelegate &>(candidate);
        }
        for (auto & delegate : pool)
        {
            if (!delegate.InUse())
            {
                return &delegate;
            }
        }
        return nullptr;
    }

    static bool InPool(const Entry::Delegate & delegate)
    {
        constexpr auto * end = pool + MATTER_ARRAY_SIZE(pool);
        return pool <= &delegate && &delegate < end;
    }

    void Release() override
    {
        mStorage->Release();
        mStorage = nullptr;
    }

    CHIP_ERROR GetAuthMode(AuthMode & authMode) const override
    {
        authMode = mStorage->mAuthMode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetFabricIndex(FabricIndex & fabricIndex) const override
    {
        fabricIndex = mStorage->mFabricIndex;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetPrivilege(Privilege & privilege) const override
    {
        privilege = mStorage->mPrivilege;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetAuthMode(AuthMode authMode) override
    {
        ReturnErrorOnFailure(EnsureStorageInPool());
        mStorage->mAuthMode = authMode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetFabricIndex(FabricIndex fabricIndex) override
    {
        ReturnErrorOnFailure(EnsureStorageInPool());
        mStorage->mFabricIndex = fabricIndex;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetPrivilege(Privilege privilege) override
    {
        ReturnErrorOnFailure(EnsureStorageInPool());
        mStorage->mPrivilege = privilege;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSubjectCount(size_t & count) const override
    {
        count = 0;
        for (const auto & subject : mStorage->mSubjects)
        {
            if (subject.IsEmpty())
            {
                break;
            }
            count++;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSubject(size_t index, NodeId & subject) const override
    {
        if (index < EntryStorage::kMaxSubjects)
        {
            return mStorage->mSubjects[index].Get(subject);
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR SetSubject(size_t index, NodeId subject) override
    {
        ReturnErrorOnFailure(EnsureStorageInPool());
        if (index < EntryStorage::kMaxSubjects)
        {
            return mStorage->mSubjects[index].Set(subject);
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR AddSubject(size_t * index, NodeId subject) override
    {
        ReturnErrorOnFailure(EnsureStorageInPool());
        size_t count = 0;
        GetSubjectCount(count);
        if (count < EntryStorage::kMaxSubjects)
        {
            CHIP_ERROR err = mStorage->mSubjects[count].Add(subject);
            if (err == CHIP_NO_ERROR && index != nullptr)
            {
                *index = count;
            }
            return err;
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    CHIP_ERROR RemoveSubject(size_t index) override
    {
        ReturnErrorOnFailure(EnsureStorageInPool());
        size_t count = 0;
        GetSubjectCount(count);
        if (index < count)
        {
            // The storage at the specified index will be deleted by copying any subsequent storage
            // over it, ideally also including one unused storage past the ones in use. If all
            // storage was in use, this isn't possible, so the final storage is manually cleared.
            auto * dest      = mStorage->mSubjects + index;
            const auto * src = dest + 1;
            const auto n     = std::min(count, EntryStorage::kMaxSubjects - 1) - index;
            memmove(dest, src, n * sizeof(*dest));
            if (count == EntryStorage::kMaxSubjects)
            {
                mStorage->mSubjects[EntryStorage::kMaxSubjects - 1].Clear();
            }
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR GetTargetCount(size_t & count) const override
    {
        count = 0;
        for (const auto & target : mStorage->mTargets)
        {
            if (target.IsEmpty())
            {
                break;
            }
            count++;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetTarget(size_t index, Target & target) const override
    {
        if (index < EntryStorage::kMaxTargets)
        {
            return mStorage->mTargets[index].Get(target);
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR SetTarget(size_t index, const Target & target) override
    {
        ReturnErrorOnFailure(EnsureStorageInPool());
        if (index < EntryStorage::kMaxTargets)
        {
            return mStorage->mTargets[index].Set(target);
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR AddTarget(size_t * index, const Target & target) override
    {
        ReturnErrorOnFailure(EnsureStorageInPool());
        size_t count = 0;
        GetTargetCount(count);
        if (count < EntryStorage::kMaxTargets)
        {
            CHIP_ERROR err = mStorage->mTargets[count].Add(target);
            if (err == CHIP_NO_ERROR && index != nullptr)
            {
                *index = count;
            }
            return err;
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    CHIP_ERROR RemoveTarget(size_t index) override
    {
        ReturnErrorOnFailure(EnsureStorageInPool());
        size_t count = 0;
        GetTargetCount(count);
        if (index < count)
        {
            // The storage at the specified index will be deleted by copying any subsequent storage
            // over it, ideally also including one unused storage past the ones in use. If all
            // storage was in use, this isn't possible, so the final storage is manually cleared.
            auto * dest      = mStorage->mTargets + index;
            const auto * src = dest + 1;
            const auto n     = std::min(count, EntryStorage::kMaxTargets - 1) - index;
            memmove(dest, src, n * sizeof(*dest));
            if (count == EntryStorage::kMaxTargets)
            {
                mStorage->mTargets[EntryStorage::kMaxTargets - 1].Clear();
            }
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_SENTINEL;
    }

    void Init(Entry & entry, EntryStorage & storage)
    {
        entry.SetDelegate(*this);
        storage.Init();
        mEntry   = &entry;
        mStorage = &storage;
    }

    bool InUse() const { return mStorage != nullptr; }

    const EntryStorage * GetStorage() const { return mStorage; }

    EntryStorage * GetStorage() { return mStorage; }

    // A storage is about to be deleted. If this delegate was
    // using it, make a best effort to copy it to the pool.
    void FixBeforeDelete(EntryStorage & storage)
    {
        if (mStorage == &storage)
        {
            // Best effort, OK if it fails.
            EnsureStorageInPool();
        }
    }

    // A storage was deleted, and others shuffled into its place.
    // Fix this delegate (if necessary) to ensure it's using the
    // correct storage.
    void FixAfterDelete(EntryStorage & storage)
    {
        constexpr auto & acl = EntryStorage::acl;
        constexpr auto * end = acl + MATTER_ARRAY_SIZE(acl);
        if (mStorage == &storage)
        {
            mEntry->ResetDelegate();
        }
        else if (&storage < mStorage && mStorage < end)
        {
            mStorage--;
        }
    }

    // Ensure the delegate is using storage from the pool (not the access control list),
    // by copying (from the access control list to the pool) if necessary.
    CHIP_ERROR EnsureStorageInPool()
    {
        if (mStorage->InPool())
        {
            return CHIP_NO_ERROR;
        }
        if (auto * storage = EntryStorage::Find(nullptr))
        {
            *storage = *mStorage;
            mStorage = storage;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

private:
    Entry * mEntry          = nullptr;
    EntryStorage * mStorage = nullptr;
};

class EntryIteratorDelegate : public EntryIterator::Delegate
{
public:
    // Pool support
    static EntryIteratorDelegate pool[kEntryIteratorDelegatePoolSize];

    // Find an unused entry iterator delegate in the pool, if one is available.
    // The candidate is preferred if it is in the pool, regardless of whether
    // it is already in use.
    static EntryIteratorDelegate * Find(EntryIterator::Delegate & candidate)
    {
        if (InPool(candidate))
        {
            return static_cast<EntryIteratorDelegate *>(&candidate);
        }
        for (auto & delegate : pool)
        {
            if (!delegate.InUse())
            {
                return &delegate;
            }
        }
        return nullptr;
    }

    static bool InPool(const EntryIterator::Delegate & delegate)
    {
        constexpr auto * end = pool + MATTER_ARRAY_SIZE(pool);
        return pool <= &delegate && &delegate < end;
    }

    void Release() override { mInUse = false; }

    CHIP_ERROR Next(Entry & entry) override
    {
        constexpr auto & acl = EntryStorage::acl;
        constexpr auto * end = acl + MATTER_ARRAY_SIZE(acl);
        while (true)
        {
            if (mStorage == nullptr)
            {
                // Start at beginning of access control list...
                mStorage = acl;
            }
            else if (mStorage < end)
            {
                // ...and continue iterating entries...
                mStorage++;
            }
            if (mStorage == end || !mStorage->InUse())
            {
                // ...but only used ones...
                mStorage = end;
                break;
            }
            if (mFabricFiltered && mStorage->mFabricIndex != mFabricIndex)
            {
                // ...skipping those that aren't scoped to a specified fabric...
                continue;
            }
            if (auto * delegate = EntryDelegate::Find(entry.GetDelegate()))
            {
                // ...returning any next entry via a delegate.
                delegate->Init(entry, *mStorage);
                return CHIP_NO_ERROR;
            }
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        return CHIP_ERROR_SENTINEL;
    }

    void Init(EntryIterator & iterator, const FabricIndex * fabricIndex)
    {
        iterator.SetDelegate(*this);
        mInUse          = true;
        mFabricFiltered = fabricIndex != nullptr;
        if (mFabricFiltered)
        {
            mFabricIndex = *fabricIndex;
        }
        mStorage = nullptr;
    }

    bool InUse() const { return mInUse; }

    // A storage was deleted, and others shuffled into its place.
    // Fix this delegate (if necessary) to ensure it's using the
    // correct storage.
    void FixAfterDelete(EntryStorage & storage)
    {
        constexpr auto & acl = EntryStorage::acl;
        constexpr auto * end = acl + MATTER_ARRAY_SIZE(acl);
        if (&storage <= mStorage && mStorage < end)
        {
            if (mStorage == acl)
            {
                mStorage = nullptr;
            }
            else
            {
                mStorage--;
            }
        }
    }

private:
    bool mInUse = false;
    bool mFabricFiltered;
    FabricIndex mFabricIndex;
    EntryStorage * mStorage;
};

CHIP_ERROR CopyViaInterface(const Entry & entry, EntryStorage & storage)
{
#if CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FLEXIBLE_COPY_SUPPORT
    // NOTE: uses sizeof(EntryStorage) on stack as a temporary and is only necessary if using this
    //       file with other Entry::Delegate implementations that are not EntryDelegate in this file
    EntryStorage temp;
    temp.Clear();

    FabricIndex fabricIndex = kUndefinedFabricIndex;
    ReturnErrorOnFailure(entry.GetFabricIndex(fabricIndex));
    temp.mFabricIndex = fabricIndex;

    AuthMode authMode = AuthMode::kNone;
    ReturnErrorOnFailure(entry.GetAuthMode(authMode));
    temp.mAuthMode = authMode;

    Privilege privilege = Privilege::kView;
    ReturnErrorOnFailure(entry.GetPrivilege(privilege));
    temp.mPrivilege = privilege;

    size_t subjectCount = 0;
    ReturnErrorOnFailure(entry.GetSubjectCount(subjectCount));
    VerifyOrReturnError(subjectCount <= EntryStorage::kMaxSubjects, CHIP_ERROR_BUFFER_TOO_SMALL);
    for (size_t i = 0; i < subjectCount; ++i)
    {
        NodeId subject = kUndefinedNodeId;
        ReturnErrorOnFailure(entry.GetSubject(i, subject));
        temp.mSubjects[i].Add(subject);
    }

    size_t targetCount = 0;
    ReturnErrorOnFailure(entry.GetTargetCount(targetCount));
    VerifyOrReturnError(targetCount <= EntryStorage::kMaxTargets, CHIP_ERROR_BUFFER_TOO_SMALL);
    for (size_t i = 0; i < targetCount; ++i)
    {
        Target target;
        ReturnErrorOnFailure(entry.GetTarget(i, target));
        temp.mTargets[i].Add(target);
    }

    temp.mInUse = true;
    storage     = temp;
    return CHIP_NO_ERROR;
#else
    // NOTE: save space by not implementing function
    VerifyOrDie(false);
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR Copy(const Entry & entry, EntryStorage & storage)
{
#if CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FAST_COPY_SUPPORT
    auto & delegate = entry.GetDelegate();
    if (EntryDelegate::InPool(delegate))
    {
        // NOTE: if an entry's delegate is in the pool, it must be an EntryDelegate,
        //       which must have a storage that is in use, which can be copied as POD
        storage = *static_cast<const EntryDelegate &>(delegate).GetStorage();
        return CHIP_NO_ERROR;
    }
#endif
    return CopyViaInterface(entry, storage);
}

class AccessControlDelegate : public AccessControl::Delegate
{
public:
    CHIP_ERROR Init() override
    {
        ChipLogProgress(DataManagement, "Examples::AccessControlDelegate::Init");
        for (auto & storage : EntryStorage::acl)
        {
            storage.Clear();
        }
        return CHIP_NO_ERROR;
    }

    void Finish() override { ChipLogProgress(DataManagement, "Examples::AccessControlDelegate::Finish"); }

    CHIP_ERROR GetMaxEntriesPerFabric(size_t & value) const override
    {
        value = EntryStorage::kEntriesPerFabric;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetMaxSubjectsPerEntry(size_t & value) const override
    {
        value = EntryStorage::kMaxSubjects;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetMaxTargetsPerEntry(size_t & value) const override
    {
        value = EntryStorage::kMaxTargets;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetMaxEntryCount(size_t & value) const override
    {
        value = MATTER_ARRAY_SIZE(EntryStorage::acl);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetEntryCount(FabricIndex fabric, size_t & value) const override
    {
        value = 0;
        for (const auto & storage : EntryStorage::acl)
        {
            if (!storage.InUse())
            {
                break;
            }
            if (storage.mFabricIndex == fabric)
            {
                value++;
            }
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetEntryCount(size_t & value) const override
    {
        value = 0;
        for (const auto & storage : EntryStorage::acl)
        {
            if (!storage.InUse())
            {
                break;
            }
            value++;
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR PrepareEntry(Entry & entry) override
    {
        if (auto * delegate = EntryDelegate::Find(entry.GetDelegate()))
        {
            if (auto * storage = EntryStorage::Find(delegate->GetStorage()))
            {
                delegate->Init(entry, *storage);
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    CHIP_ERROR CreateEntry(size_t * index, const Entry & entry, FabricIndex * fabricIndex) override
    {
        if (auto * storage = EntryStorage::FindUnusedInAcl())
        {
            CHIP_ERROR err = Copy(entry, *storage);
            if (err == CHIP_NO_ERROR)
            {
                if (fabricIndex != nullptr)
                {
                    *fabricIndex = storage->mFabricIndex;
                }
                if (index != nullptr)
                {
                    *index = size_t(storage - EntryStorage::acl);
                    if (fabricIndex != nullptr)
                    {
                        EntryStorage::ConvertIndex(*index, *fabricIndex, EntryStorage::ConvertDirection::kAbsoluteToRelative);
                    }
                }
            }
            return err;
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    CHIP_ERROR ReadEntry(size_t index, Entry & entry, const FabricIndex * fabricIndex) const override
    {
        if (auto * storage = EntryStorage::FindUsedInAcl(index, fabricIndex))
        {
            if (auto * delegate = EntryDelegate::Find(entry.GetDelegate()))
            {
                delegate->Init(entry, *storage);
                return CHIP_NO_ERROR;
            }
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR UpdateEntry(size_t index, const Entry & entry, const FabricIndex * fabricIndex) override
    {
        if (auto * storage = EntryStorage::FindUsedInAcl(index, fabricIndex))
        {
            return Copy(entry, *storage);
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR DeleteEntry(size_t index, const FabricIndex * fabricIndex) override
    {
        if (auto * storage = EntryStorage::FindUsedInAcl(index, fabricIndex))
        {
            // Best effort attempt to preserve any outstanding delegates...
            for (auto & delegate : EntryDelegate::pool)
            {
                delegate.FixBeforeDelete(*storage);
            }

            // ...then go through the access control list starting at the deleted storage...
            constexpr auto & acl = EntryStorage::acl;
            constexpr auto * end = acl + MATTER_ARRAY_SIZE(acl);
            for (auto * next = storage + 1; storage < end; ++storage, ++next)
            {
                // ...copying over each storage with its next one...
                if (next < end && next->InUse())
                {
                    *storage = *next;
                }
                else
                {
                    // ...clearing the last previously used one...
                    storage->Clear();
                    break;
                }
            }

            // ...then fix up all the delegates so they still use the proper storage.
            storage = acl + index;
            for (auto & delegate : EntryDelegate::pool)
            {
                delegate.FixAfterDelete(*storage);
            }
            for (auto & delegate : EntryIteratorDelegate::pool)
            {
                delegate.FixAfterDelete(*storage);
            }

            return CHIP_NO_ERROR;
        }

        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR Entries(EntryIterator & iterator, const FabricIndex * fabricIndex) const override
    {
        if (auto * delegate = EntryIteratorDelegate::Find(iterator.GetDelegate()))
        {
            delegate->Init(iterator, fabricIndex);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                     Privilege requestPrivilege) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

static_assert(std::is_pod<SubjectStorage>(), "Storage type must be POD");
static_assert(std::is_pod<TargetStorage>(), "Storage type must be POD");
static_assert(std::is_pod<EntryStorage>(), "Storage type must be POD");

EntryStorage EntryStorage::acl[];
EntryStorage EntryStorage::pool[];
EntryDelegate EntryDelegate::pool[];
EntryIteratorDelegate EntryIteratorDelegate::pool[];

} // namespace

namespace chip {
namespace Access {
namespace Examples {

AccessControl::Delegate * GetAccessControlDelegate()
{
    static AccessControlDelegate accessControlDelegate;
    return &accessControlDelegate;
}

} // namespace Examples
} // namespace Access
} // namespace chip
