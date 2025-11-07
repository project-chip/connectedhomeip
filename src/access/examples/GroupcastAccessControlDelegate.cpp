    #include "GroupcastAccessControlDelegate.h"
#include <access/AccessControl.h>
#include <access/AuthMode.h>
#include <lib/support/PersistentArray.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <algorithm>

using namespace chip::Access;
using Entry         = AccessControl::Entry;
using EntryIterator = AccessControl::EntryIterator;
using Target        = AccessControl::Entry::Target;

namespace chip {
namespace Access {
namespace Groupcast {
namespace {

constexpr size_t kNumberOfFabrics  = CHIP_CONFIG_MAX_FABRICS;
constexpr size_t kEntriesPerFabric = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC;
constexpr uint16_t kMaxSubjects = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY;
constexpr uint16_t kMaxTargets  = CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY;
constexpr size_t kPersistentBufferMax = 256;


struct EntryStorage {

    bool operator==(const EntryStorage & other) const
    {
        return fabric_index != other.fabric_index;
    }

    // EntryStorage& operator=(const EntryStorage &x)
    // {
    //     if(this != &x) {
    //         fabric_index = x.fabric_index;
    //         auth_mode = x.auth_mode;
    //         privilege = x.privilege;
    //         subject_count = std::min(x.subject_count, kMaxSubjects);
    //         memcpy(subjects, x.subjects, subject_count * sizeof(NodeId));
    //         target_count = std::min(x.target_count, kMaxTargets);
    //         memcpy(targets, x.targets, target_count * sizeof(Target));
    //     } 
    //     return *this;
    // }

    FabricIndex fabric_index;
    AuthMode auth_mode;
    Privilege privilege;
    uint16_t subject_count = 0;
    NodeId subjects[kMaxSubjects];
    uint16_t target_count = 0;
    Target targets[kMaxTargets];
    // TARGET
    // Flags flags(4)
    // ClusterId cluster(4)
    // EndpointId endpoint(2)
    // DeviceTypeId deviceType(4)
}; 
// f(1) a(1) p(1) sc(2) n(8)*4 tc(2) T[ f(4) c(4) e(2) d(4) PADDING() ]*3
// f(1) a(1) p(1) sc(2) n(8)*4 tc(2) T[16+]*3
// 7 + 32 + 48 + PADDING(3) = 90

struct Tags {
    static constexpr TLV::Tag AuthMode() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag Privilege() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag Subject() { return TLV::ContextTag(10); }
    static constexpr TLV::Tag SubjectCount() { return TLV::ContextTag(11); }
    static constexpr TLV::Tag SubjectList() { return TLV::ContextTag(12); }
    static constexpr TLV::Tag TargetCount() { return TLV::ContextTag(21); }
    static constexpr TLV::Tag TargetList() { return TLV::ContextTag(22); }
    static constexpr TLV::Tag TargetFlags() { return TLV::ContextTag(23); }
    static constexpr TLV::Tag TargetCluster() { return TLV::ContextTag(24); }
    static constexpr TLV::Tag TargetEndpoint() { return TLV::ContextTag(25); }
    static constexpr TLV::Tag TargetDevice() { return TLV::ContextTag(26); }
};

struct EntryList : public PersistentArray<kNumberOfFabrics, kPersistentBufferMax, EntryStorage>
{
    EntryList() :
        PersistentArray<CHIP_CONFIG_MAX_FABRICS, kPersistentBufferMax, EntryStorage>(nullptr), mFabric(kUndefinedFabricIndex)
    {}

    EntryList(FabricIndex fabric, PersistentStorageDelegate * storage) :
        PersistentArray<CHIP_CONFIG_MAX_FABRICS, kPersistentBufferMax, EntryStorage>(storage), mFabric(fabric)
    {}

    CHIP_ERROR Init(FabricIndex fabric, PersistentStorageDelegate * storage)
    {
        mFabric = fabric;
        mStorage = storage;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR UpdateKey(StorageKeyName & key) const override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != mFabric, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::GroupcastAccess(mFabric);
        return CHIP_NO_ERROR;
    }

    void ClearEntry(EntryStorage & entry) override
    {
        entry.fabric_index = kUndefinedFabricIndex;
        entry.auth_mode = static_cast<AuthMode>(0);
        entry.privilege = static_cast<Privilege>(0);
        entry.subject_count = 0;
        memset(entry.subjects, 0x00, sizeof(entry.subjects));
        entry.target_count = 0;
        for(size_t i=0; i < kMaxTargets; i++) {
            auto &t = entry.targets[i];
            t.flags                        = 0;
            t.cluster = kInvalidClusterId;
            t.endpoint = kInvalidEndpointId;
            t.deviceType = 0;
        }
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer, const EntryStorage & entry) const override
    {
        // AuthMode
        ReturnErrorOnFailure(writer.Put(Tags::AuthMode(), static_cast<uint8_t>(entry.auth_mode)));
        // Privilege
        ReturnErrorOnFailure(writer.Put(Tags::Privilege(), static_cast<uint8_t>(entry.privilege)));
        // Subjects
        ReturnErrorOnFailure(writer.Put(Tags::SubjectCount(), static_cast<uint16_t>(entry.subject_count)));
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(writer.StartContainer(Tags::SubjectList(), TLV::kTLVType_Array, array));
            for (size_t i = 0; i < entry.subject_count; ++i)
            {
                ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, item));
                // Endpoint
                ReturnErrorOnFailure(writer.Put(Tags::Subject(), entry.subjects[i]));
                ReturnErrorOnFailure(writer.EndContainer(item));
            }
            ReturnErrorOnFailure(writer.EndContainer(array));
        }
        // Targets
        ReturnErrorOnFailure(writer.Put(Tags::TargetCount(), static_cast<uint16_t>(entry.target_count)));
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(writer.StartContainer(Tags::TargetList(), TLV::kTLVType_Array, array));
            for (size_t i = 0; i < entry.target_count; ++i)
            {
                const Target &t = entry.targets[i];
                ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, item));
                // Target
                ReturnErrorOnFailure(writer.Put(Tags::TargetFlags(), static_cast<uint8_t>(t.flags)));
                ReturnErrorOnFailure(writer.Put(Tags::TargetCluster(), t.cluster));
                ReturnErrorOnFailure(writer.Put(Tags::TargetEndpoint(), t.endpoint));
                ReturnErrorOnFailure(writer.Put(Tags::TargetDevice(), t.deviceType));
                ReturnErrorOnFailure(writer.EndContainer(item));
            }
            ReturnErrorOnFailure(writer.EndContainer(array));
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader, EntryStorage & entry) override
    {
        // Fabric
        entry.fabric_index = mFabric;
        // AuthMode
        ReturnErrorOnFailure(reader.Next(Tags::AuthMode()));
        ReturnErrorOnFailure(reader.Get(entry.auth_mode));
        // Privilege
        ReturnErrorOnFailure(reader.Next(Tags::Privilege()));
        ReturnErrorOnFailure(reader.Get(entry.privilege));
        // Subjects
        ReturnErrorOnFailure(reader.Next(Tags::SubjectCount()));
        ReturnErrorOnFailure(reader.Get(entry.subject_count));
        ReturnErrorOnFailure(reader.Next(Tags::SubjectList()));
        VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_INTERNAL);
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(reader.EnterContainer(array));
            for (size_t i = 0; i < entry.subject_count; ++i)
            {
                ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
                VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);
                ReturnErrorOnFailure(reader.EnterContainer(item));
                // Endpoint
                ReturnErrorOnFailure(reader.Next(Tags::Subject()));
                ReturnErrorOnFailure(reader.Get(entry.subjects[i]));
                ReturnErrorOnFailure(reader.ExitContainer(item));
            }
            ReturnErrorOnFailure(reader.ExitContainer(array));
        }
        // Targets
        ReturnErrorOnFailure(reader.Next(Tags::TargetCount()));
        ReturnErrorOnFailure(reader.Get(entry.target_count));
        ReturnErrorOnFailure(reader.Next(Tags::TargetList()));
        VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_INTERNAL);
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(reader.EnterContainer(array));
            for (size_t i = 0; i < entry.target_count; ++i)
            {
                // Target
                Target &t = entry.targets[i];
                uint8_t flags;
                ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
                VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);
                ReturnErrorOnFailure(reader.EnterContainer(item));
                // Flags
                ReturnErrorOnFailure(reader.Next(Tags::TargetFlags()));
                ReturnErrorOnFailure(reader.Get(flags));
                t.flags = flags;
                // Cluster
                ReturnErrorOnFailure(reader.Next(Tags::TargetCluster()));
                ReturnErrorOnFailure(reader.Get(t.cluster));
                // Endpoint
                ReturnErrorOnFailure(reader.Next(Tags::TargetEndpoint()));
                ReturnErrorOnFailure(reader.Get(t.endpoint));
                // Device type
                ReturnErrorOnFailure(reader.Next(Tags::TargetDevice()));
                ReturnErrorOnFailure(reader.Get(t.deviceType));

                ReturnErrorOnFailure(reader.ExitContainer(item));
            }
            ReturnErrorOnFailure(reader.ExitContainer(array));
        }
        return CHIP_NO_ERROR;
    }

    FabricIndex mFabric;
};


class EntryDelegate: public AccessControl::Entry::Delegate
{
public:
    EntryDelegate() = default;
    EntryDelegate(const Delegate &)             = delete;
    EntryDelegate & operator=(const Delegate &) = delete;
    ~EntryDelegate() = default;
    void Release() override {}

    // Simple getters
    CHIP_ERROR GetAuthMode(AuthMode & authMode) const override {
        authMode = mEntry.auth_mode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetFabricIndex(FabricIndex & fabricIndex) const override {
        fabricIndex = mEntry.fabric_index;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetPrivilege(Privilege & privilege) const override {
        privilege = mEntry.privilege;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetAuthMode(AuthMode authMode) override {
        mEntry.auth_mode = authMode;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetFabricIndex(FabricIndex fabricIndex) override {
        mEntry.fabric_index = fabricIndex;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetPrivilege(Privilege privilege) override {
        mEntry.privilege = privilege;
        return CHIP_NO_ERROR;
    }

    // Subjects
    CHIP_ERROR GetSubjectCount(size_t & count) const override {
        count = mEntry.subject_count;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetSubject(size_t index, NodeId & subject) const override {
        VerifyOrReturnError(index < kMaxSubjects, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(index < mEntry.subject_count, CHIP_ERROR_INVALID_ARGUMENT);
        subject = mEntry.subjects[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetSubject(size_t index, NodeId subject) override {
        VerifyOrReturnError(index < kMaxSubjects, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(index < mEntry.subject_count, CHIP_ERROR_INVALID_ARGUMENT);
        mEntry.subjects[index] = subject;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AddSubject(size_t * index, NodeId subject) override {
        VerifyOrReturnError(mEntry.subject_count < kMaxSubjects, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(nullptr != index, CHIP_ERROR_INVALID_ARGUMENT);
        *index = mEntry.subject_count;
        mEntry.subjects[*index] = subject;
        mEntry.subject_count++;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveSubject(size_t index) override {
        VerifyOrReturnError(index < kMaxSubjects, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(index < mEntry.subject_count, CHIP_ERROR_INVALID_ARGUMENT);
        for (size_t i = index; i + 1 < mEntry.subject_count; ++i) {
            mEntry.subjects[i] = mEntry.subjects[i + 1];
        }
        mEntry.subject_count--;
        return CHIP_NO_ERROR;
    }

    // Targets
    CHIP_ERROR GetTargetCount(size_t & count) const override {
        count = mEntry.target_count;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetTarget(size_t index, Target & target) const override {
        VerifyOrReturnError(index < kMaxTargets, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(index < mEntry.target_count, CHIP_ERROR_INVALID_ARGUMENT);
        target = mEntry.targets[index];
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetTarget(size_t index, const Target & target) override {
        VerifyOrReturnError(index < kMaxTargets, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(index < mEntry.target_count, CHIP_ERROR_INVALID_ARGUMENT);
        mEntry.targets[index] = target;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AddTarget(size_t * index, const Target & target) override {
        VerifyOrReturnError(mEntry.target_count < kMaxTargets, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(nullptr != index, CHIP_ERROR_INVALID_ARGUMENT);
        *index = mEntry.target_count;
        mEntry.targets[*index] = target;
        mEntry.target_count++;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveTarget(size_t index) override {
        VerifyOrReturnError(index < kMaxTargets, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(index < mEntry.target_count, CHIP_ERROR_INVALID_ARGUMENT);
        for (size_t i = index; i + 1 < mEntry.target_count; ++i) {
            mEntry.targets[i] = mEntry.targets[i + 1];
        }
        mEntry.target_count--;
        return CHIP_NO_ERROR;
    }
    EntryStorage mEntry;
};


static EntryDelegate mEntryDelegate;
static EntryList sList;

class EntryIteratorDelegate: public EntryIterator::Delegate
{
public:
    EntryIteratorDelegate() = default;

    EntryIteratorDelegate(FabricIndex fabric, PersistentStorageDelegate * storage) {
        sList.Init(fabric, storage);
    }
        

    EntryIteratorDelegate(const Delegate &)             = delete;
    EntryIteratorDelegate & operator=(const Delegate &) = delete;

    virtual ~EntryIteratorDelegate() = default;

    CHIP_ERROR Init(PersistentStorageDelegate * storage, FabricIndex fabric)
    {
        mFabric = fabric;
        mIndex = 0;
        sList.Init(fabric, storage);
        sList.Load();
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR Next(Entry & entry) override
    {
        if(mIndex < sList.Count()) {
            ReturnErrorOnFailure(sList.Get(mIndex++, mDelegate.mEntry));
            entry.SetDelegate(mDelegate);
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_SENTINEL;
    }

    void Release() override {}

    FabricIndex mFabric;
    EntryDelegate mDelegate;
    EntryList sList;
    size_t mIndex = 0;
};

static EntryIteratorDelegate mIteratorDelegate;


class AccessControlDelegate : public AccessControl::Delegate
{
public:
    AccessControlDelegate() = default;

    AccessControlDelegate(const Delegate &)             = delete;
    AccessControlDelegate & operator=(const Delegate &) = delete;
    virtual ~AccessControlDelegate() = default;

    virtual void Release() override {}

    CHIP_ERROR Init() override
    {
        return CHIP_NO_ERROR;
    }
    virtual void Finish() override {}

    void SetPersistentStorage(PersistentStorageDelegate *storage)
    {
        mStorage = storage;
    }

    // Capabilities
    CHIP_ERROR GetMaxEntriesPerFabric(size_t & value) const override
    {
        value = kEntriesPerFabric;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetMaxSubjectsPerEntry(size_t & value) const override
    {
        value = kMaxSubjects;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetMaxTargetsPerEntry(size_t & value) const override
    {
        value = kMaxTargets;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetMaxEntryCount(size_t & value) const override
    {
        value = kNumberOfFabrics * kEntriesPerFabric;
        return CHIP_NO_ERROR;
    }

    // Actualities
    CHIP_ERROR GetEntryCount(FabricIndex fabric, size_t & value) const override
    {
        value = 0;
        VerifyOrReturnError(nullptr != mStorage, CHIP_ERROR_INCORRECT_STATE);
        EntryList list(fabric, mStorage); 
        ReturnErrorOnFailure(list.Load());
        value = list.Count();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetEntryCount(size_t & value) const override
    {
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        FabricIndex fabric_index = 1;
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        return GetEntryCount(fabric_index, value);
        // return CHIP_NO_ERROR;
    }

    // Preparation
    CHIP_ERROR PrepareEntry(Entry & entry) override
    {
        entry.SetDelegate(mEntryDelegate);
        return CHIP_NO_ERROR;
    }

    // CRUD
    CHIP_ERROR CreateEntry(size_t * index, const Entry & entry, FabricIndex * fabricIndex) override
    {
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        FabricIndex fabric_index = fabricIndex ? *fabricIndex : 1;
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        sList.Init(fabric_index, mStorage);
        sList.Load();
        if(index) *index = sList.Count();
        return sList.Add(mEntryDelegate.mEntry);
    }

    CHIP_ERROR ReadEntry(size_t index, Entry & entry, const FabricIndex * fabricIndex) const override
    {
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        FabricIndex fabric_index = fabricIndex ? *fabricIndex : 1;
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        sList.Init(fabric_index, mStorage);
        return sList.Get(index, mEntryDelegate.mEntry);
    }

    CHIP_ERROR UpdateEntry(size_t index, const Entry & entry, const FabricIndex * fabricIndex) override
    {
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        FabricIndex fabric_index = fabricIndex ? *fabricIndex : 1;
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        sList.Init(fabric_index, mStorage);
        return sList.Set(index, mEntryDelegate.mEntry);
    }

    CHIP_ERROR DeleteEntry(size_t index, const FabricIndex * fabricIndex) override
    {
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        FabricIndex fabric_index = fabricIndex ? *fabricIndex : 1;
        // FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME
        sList.Init(fabric_index, mStorage);
        return sList.Remove(index);
    }

    // Iteration
    CHIP_ERROR Entries(EntryIterator & iterator, const FabricIndex * fabricIndex) const override
    {
        ReturnErrorOnFailure(mIteratorDelegate.Init(mStorage, fabricIndex ? *fabricIndex : 0));
        iterator.SetDelegate(mIteratorDelegate);
        return CHIP_NO_ERROR;
    }

private:
    PersistentStorageDelegate *mStorage = nullptr;
};

} // namespace

AccessControl::Delegate * GetAccessControlDelegate(PersistentStorageDelegate *storage) {
    static AccessControlDelegate sDelegate;
    if(storage) {
        sDelegate.SetPersistentStorage(storage);
    }
    return &sDelegate;
}

} // namespace Groupcast
} // namespace Access
} // namespace chip
