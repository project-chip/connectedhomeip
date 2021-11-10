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

#include <cassert>
#include <cstdint>
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

using Entry         = chip::Access::AccessControl::Entry;
using EntryIterator = chip::Access::AccessControl::EntryIterator;
using Target        = Entry::Target;

// Pool sizes
constexpr int kEntryStoragePoolSize          = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_ENTRY_STORGAGE_POOL_SIZE;
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
        assert(IsEmpty());
        if (IsValid(node))
        {
            mNode = node;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

private:
    static bool IsValid(NodeId node) { return node != kUndefinedNodeId; }

private:
    static_assert(sizeof(NodeId) == 8, "Expecting 8 byte node ID");

private:
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
        assert(IsEmpty());
        if (IsValid(target))
        {
            Encode(target);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

private:
    static bool IsValidCluster(ClusterId cluster)
    {
        const auto & id     = cluster & kClusterIdMask;
        const auto & vendor = cluster & kClusterVendorMask;
        return ((kClusterIdMinStd <= id && id <= kClusterIdMaxStd) || (kClusterIdMinMs <= id && id <= kClusterIdMaxMs)) &&
            (kClusterVendorMin <= vendor && vendor <= kClusterVendorMax);
    }

    static constexpr bool IsValidEndpoint(EndpointId endpoint) { return true; }

    static bool IsValidDeviceType(DeviceTypeId deviceType)
    {
        const auto & id     = deviceType & kDeviceTypeIdMask;
        const auto & vendor = deviceType & kDeviceTypeVendorMask;
        return (kDeviceTypeIdMin <= id && id <= kDeviceTypeIdMax) &&
            (kDeviceTypeVendorMin <= vendor && vendor <= kDeviceTypeVendorMax);
    }

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

private:
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
        const auto & flags      = target.flags;
        const auto & cluster    = target.cluster;
        const auto & endpoint   = target.endpoint;
        const auto & deviceType = target.deviceType;
        assert(IsValid(target));
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

private:
    static_assert(sizeof(ClusterId) == 4, "Expecting 4 byte cluster ID");
    static_assert(sizeof(EndpointId) == 2, "Expecting 2 byte endpoint ID");
    static_assert(sizeof(DeviceTypeId) == 4, "Expecting 4 byte device type ID");

    // (mCluster == kClusterEmpty) --> mCluster contains no cluster
    static constexpr ClusterId kClusterEmpty = 0xFFFFFFFF;

    // (mCluster & kClusterIdMask) --> cluster id portion
    static constexpr ClusterId kClusterIdMask = 0x0000FFFF;

    // ((mCluster & kClusterIdMask) < kClusterIdMinStd) --> invalid
    static constexpr ClusterId kClusterIdMinStd = 0x00000000;

    // ((mCluster & kClusterIdMask) < kClusterIdMaxStd) --> invalid
    static constexpr ClusterId kClusterIdMaxStd = 0x00007FFF;

    // ((mCluster & kClusterIdMask) < kClusterIdMinMs) --> invalid
    static constexpr ClusterId kClusterIdMinMs = 0x0000FC00;

    // ((mCluster & kClusterIdMask) < kClusterIdMaxMs) --> invalid
    static constexpr ClusterId kClusterIdMaxMs = 0x0000FFFE;

    // (mCluster & kClusterVendorMask) --> cluster vendor portion
    static constexpr ClusterId kClusterVendorMask = 0xFFFF0000;

    // ((mCluster & kClusterVendorMask) < kClusterVendorMin) --> invalid
    static constexpr ClusterId kClusterVendorMin = 0x00000000;

    // ((mCluster & kClusterVendorMask) > kClusterVendorMax) --> invalid
    static constexpr ClusterId kClusterVendorMax = 0xFFFE0000;

    // (mDeviceType == kDeviceTypeEmpty) --> mDeviceType contains neither endpoint nor device type
    static constexpr DeviceTypeId kDeviceTypeEmpty = 0xFFFFFFFF;

    // (mDeviceType & kDeviceTypeIdMask) --> device type id portion
    static constexpr DeviceTypeId kDeviceTypeIdMask = 0x0000FFFF;

    // ((mDeviceType & kDeviceTypeIdMask) < kDeviceTypeIdMin) --> invalid
    static constexpr DeviceTypeId kDeviceTypeIdMin = 0x00000000;

    // ((mDeviceType & kDeviceTypeIdMask) < kDeviceTypeIdMax) --> invalid
    static constexpr DeviceTypeId kDeviceTypeIdMax = 0x0000BFFF;

    // (mDeviceType & kDeviceTypeVendorMask) --> device type vendor portion
    static constexpr DeviceTypeId kDeviceTypeVendorMask = 0xFFFF0000;

    // ((mDeviceType & kDeviceTypeVendorMask) < kDeviceTypeVendorMin) --> invalid
    static constexpr DeviceTypeId kDeviceTypeVendorMin = 0x00000000;

    // ((mDeviceType & kDeviceTypeVendorMask) > kDeviceTypeVendorMax) --> invalid
    static constexpr DeviceTypeId kDeviceTypeVendorMax = 0xFFFE0000;

    // ((mDeviceType & kDeviceTypeIdMask) == kEndpointMagic) --> mDeviceType contains endpoint
    static constexpr DeviceTypeId kEndpointMagic = 0x0000EEEE;

    // (mDeviceType >> kEndpointShift) --> extract endpoint from mDeviceType
    static constexpr int kEndpointShift = 16;

private:
    ClusterId mCluster;
    DeviceTypeId mDeviceType;
};

class EntryStorage
{
public:
    // ACL support
    static constexpr int kNumberOfFabrics  = 4; // TODO: get from config
    static constexpr int kEntriesPerFabric = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC;
    static EntryStorage acl[kNumberOfFabrics * kEntriesPerFabric];

    static EntryStorage * FindUnusedInAcl(size_t * index, const FabricIndex * fabricIndex)
    {
        for (auto & storage : acl)
        {
            if (!storage.InUse())
            {
                if (index != nullptr)
                {
                    if (fabricIndex != nullptr)
                    {
                        *index = 0;
                        for (auto & storage2 : acl)
                        {
                            if (&storage2 == &storage)
                            {
                                break;
                            }
                            else if (storage2.mFabricIndex == storage.mFabricIndex)
                            {
                                (*index)++;
                            }
                        }
                    }
                    else
                    {
                        *index = &storage - acl;
                    }
                }
                return &storage;
            }
        }
        return nullptr;
    }

    static EntryStorage * FindUsedInAcl(size_t index, const FabricIndex * fabricIndex)
    {
        index = FilterIndex(index, fabricIndex);
        if (index < ArraySize(acl))
        {
            auto storage = acl + index;
            if (storage->InUse())
            {
                return storage;
            }
        }
        return nullptr;
    }

public:
    // Pool support
    static EntryStorage pool[kEntryStoragePoolSize];

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
        constexpr auto end = pool + ArraySize(pool);
        return pool <= this && this < end;
    }

public:
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

private:
    static size_t FilterIndex(size_t index, const FabricIndex * fabricIndex)
    {
        if (fabricIndex != nullptr)
        {
            size_t unfiltered = 0;
            size_t filtered   = 0;
            for (auto & storage : acl)
            {
                if (storage.InUse())
                {
                    return ArraySize(acl);
                }
                if (storage.mFabricIndex == *fabricIndex)
                {
                    if (filtered == index)
                    {
                        break;
                    }
                    filtered++;
                }
                unfiltered++;
            }
            return unfiltered;
        }
        return index;
    }

public:
    static constexpr int kMaxSubjects = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY;
    static constexpr int kMaxTargets  = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY;

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
        constexpr auto end = pool + ArraySize(pool);
        return pool <= &delegate && &delegate < end;
    }

public:
    void Release() override
    {
        assert(mStorage != nullptr);
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
        ReturnErrorCodeIf(EnsureInPool(), CHIP_ERROR_BUFFER_TOO_SMALL);
        mStorage->mAuthMode = authMode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetFabricIndex(FabricIndex fabricIndex) override
    {
        ReturnErrorCodeIf(EnsureInPool(), CHIP_ERROR_BUFFER_TOO_SMALL);
        mStorage->mFabricIndex = fabricIndex;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetPrivilege(Privilege privilege) override
    {
        ReturnErrorCodeIf(EnsureInPool(), CHIP_ERROR_BUFFER_TOO_SMALL);
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
        ReturnErrorCodeIf(EnsureInPool(), CHIP_ERROR_BUFFER_TOO_SMALL);
        if (index < EntryStorage::kMaxSubjects)
        {
            return mStorage->mSubjects[index].Set(subject);
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR AddSubject(size_t * index, NodeId subject) override
    {
        ReturnErrorCodeIf(EnsureInPool(), CHIP_ERROR_BUFFER_TOO_SMALL);
        size_t count;
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
        ReturnErrorCodeIf(EnsureInPool(), CHIP_ERROR_BUFFER_TOO_SMALL);
        size_t count;
        GetSubjectCount(count);
        if (index < count)
        {
            const auto & dest = mStorage->mSubjects + index;
            const auto & src  = dest + 1;
            auto bytes        = (count - index - 1) * sizeof(*dest);
            memmove(dest, src, bytes);
            if (count == EntryStorage::kMaxSubjects)
            {
                mStorage->mSubjects[EntryStorage::kMaxSubjects - 1].Clear();
            }
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
        ReturnErrorCodeIf(EnsureInPool(), CHIP_ERROR_BUFFER_TOO_SMALL);
        if (index < EntryStorage::kMaxTargets)
        {
            return mStorage->mTargets[index].Set(target);
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR AddTarget(size_t * index, const Target & target) override
    {
        ReturnErrorCodeIf(EnsureInPool(), CHIP_ERROR_BUFFER_TOO_SMALL);
        size_t count;
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
        ReturnErrorCodeIf(EnsureInPool(), CHIP_ERROR_BUFFER_TOO_SMALL);
        size_t count;
        GetTargetCount(count);
        if (index < count)
        {
            const auto & dest = mStorage->mTargets + index;
            const auto & src  = dest + 1;
            auto bytes        = (count - index - 1) * sizeof(*dest);
            memmove(dest, src, bytes);
            if (count == EntryStorage::kMaxTargets)
            {
                mStorage->mTargets[EntryStorage::kMaxTargets - 1].Clear();
            }
        }
        return CHIP_ERROR_SENTINEL;
    }

public:
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

    void FixAfterDelete(EntryStorage & storage)
    {
        constexpr auto & acl = EntryStorage::acl;
        constexpr auto end   = acl + ArraySize(acl);
        if (mStorage == &storage)
        {
            mEntry->ResetDelegate();
        }
        else if (&storage < mStorage && mStorage < end)
        {
            mStorage--;
        }
    }

    bool EnsureInPool()
    {
        if (mStorage->InPool())
        {
            return false;
        }
        else if (auto storage = EntryStorage::Find(nullptr))
        {
            *storage = *mStorage;
            mStorage = storage;
            return false;
        }
        return true;
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
        constexpr auto end = pool + ArraySize(pool);
        return pool <= &delegate && &delegate < end;
    }

public:
    void Release() override { mInUse = false; }

    CHIP_ERROR Next(Entry & entry) override
    {
        constexpr auto & acl = EntryStorage::acl;
        constexpr auto end   = acl + ArraySize(acl);
        while (true)
        {
            if (mStorage == nullptr)
            {
                mStorage = acl;
            }
            else if (mStorage < end)
            {
                mStorage++;
            }
            if (mStorage == end || !mStorage->InUse())
            {
                mStorage = end;
                break;
            }
            if (mFabricFiltered && mStorage->mFabricIndex != mFabricIndex)
            {
                continue;
            }
            if (auto delegate = EntryDelegate::Find(entry.GetDelegate()))
            {
                delegate->Init(entry, *mStorage);
                return CHIP_NO_ERROR;
            }
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        return CHIP_ERROR_SENTINEL;
    }

public:
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

    void FixAfterDelete(EntryStorage & storage)
    {
        constexpr auto & acl = EntryStorage::acl;
        constexpr auto end   = acl + ArraySize(acl);
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
#if CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_FOREIGN_DELEGATE_SUPPORT
    // NOTE: function uses sizeof(EntryStorage) on stack as a temporary
    //       and is only necessary if using foreign delegates (i.e. not
    //       EntryDelegate from this file)
    EntryStorage temp;
    temp.Clear();

    FabricIndex fabricIndex;
    ReturnErrorOnFailure(entry.GetFabricIndex(fabricIndex));
    temp.mFabricIndex = fabricIndex;

    AuthMode authMode;
    ReturnErrorOnFailure(entry.GetAuthMode(authMode));
    temp.mAuthMode = authMode;

    Privilege privilege;
    ReturnErrorOnFailure(entry.GetPrivilege(privilege));
    temp.mPrivilege = privilege;

    size_t subjectCount;
    ReturnErrorOnFailure(entry.GetSubjectCount(subjectCount));
    ReturnErrorCodeIf(subjectCount > EntryStorage::kMaxSubjects, CHIP_ERROR_BUFFER_TOO_SMALL);
    for (size_t i = 0; i < subjectCount; ++i)
    {
        NodeId subject;
        ReturnErrorOnFailure(entry.GetSubject(i, subject));
        temp.mSubjects[i].Add(subject);
    }

    size_t targetCount;
    ReturnErrorOnFailure(entry.GetTargetCount(targetCount));
    ReturnErrorCodeIf(targetCount > EntryStorage::kMaxTargets, CHIP_ERROR_BUFFER_TOO_SMALL);
    for (size_t i = 0; i < targetCount; ++i)
    {
        Target target;
        ReturnErrorOnFailure(entry.GetTarget(i, target));
        temp.mTargets[i].Add(target);
    }

    storage = temp;
    return CHIP_NO_ERROR;
#else
    // NOTE: save space by not implementing function
    assert(false);
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR Copy(const Entry & entry, EntryStorage & storage)
{
    auto & delegate = entry.GetDelegate();
    if (EntryDelegate::InPool(delegate))
    {
        assert(static_cast<const EntryDelegate &>(delegate).GetStorage() != nullptr);
        storage = *static_cast<const EntryDelegate &>(delegate).GetStorage();
        return CHIP_NO_ERROR;
    }
    return CopyViaInterface(entry, storage);
}

class AccessControlDelegate : public AccessControl::Delegate
{
public:
    CHIP_ERROR Init() override
    {
        ChipLogDetail(DataManagement, "Examples::AccessControlDelegate::Init");
        CHIP_ERROR err = LoadFromFlash();
        if (err != CHIP_NO_ERROR)
        {
            for (auto & storage : EntryStorage::acl)
            {
                storage.Clear();
            }
        }
        return err;
    }

    CHIP_ERROR Finish() override
    {
        ChipLogDetail(DataManagement, "Examples::AccessControlDelegate::Finish");
        return SaveToFlash();
    }

    CHIP_ERROR GetMaxEntries(int & value) const override
    {
        value = ArraySize(EntryStorage::acl);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR PrepareEntry(Entry & entry) override
    {
        if (auto delegate = EntryDelegate::Find(entry.GetDelegate()))
        {
            if (auto storage = EntryStorage::Find(delegate->GetStorage()))
            {
                delegate->Init(entry, *storage);
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    CHIP_ERROR CreateEntry(size_t * index, const Entry & entry, const FabricIndex * fabricIndex) override
    {
        if (auto storage = EntryStorage::FindUnusedInAcl(index, fabricIndex))
        {
            return Copy(entry, *storage);
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    CHIP_ERROR ReadEntry(size_t index, Entry & entry, const FabricIndex * fabricIndex) const override
    {
        if (auto storage = EntryStorage::FindUsedInAcl(index, fabricIndex))
        {
            if (auto delegate = EntryDelegate::Find(entry.GetDelegate()))
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
        if (auto storage = EntryStorage::FindUsedInAcl(index, fabricIndex))
        {
            return Copy(entry, *storage);
        }
        return CHIP_ERROR_SENTINEL;
    }

    CHIP_ERROR DeleteEntry(size_t index, const FabricIndex * fabricIndex) override
    {
        if (auto storage = EntryStorage::FindUsedInAcl(index, fabricIndex))
        {
            constexpr auto & acl = EntryStorage::acl;
            constexpr auto end   = acl + ArraySize(acl);
            for (auto next = storage + 1; storage < end; ++storage, ++next)
            {
                if (next < end && next->InUse())
                {
                    *storage = *next;
                }
                else
                {
                    storage->Clear();
                    break;
                }
            }
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
        if (auto delegate = EntryIteratorDelegate::Find(iterator.GetDelegate()))
        {
            delegate->Init(iterator, fabricIndex);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

private:
    CHIP_ERROR LoadFromFlash() { return CHIP_NO_ERROR; }

    CHIP_ERROR SaveToFlash() { return CHIP_NO_ERROR; }
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

AccessControl::Delegate & GetAccessControlDelegate()
{
    static AccessControlDelegate accessControlDelegate;
    return accessControlDelegate;
}

} // namespace Examples
} // namespace Access
} // namespace chip
