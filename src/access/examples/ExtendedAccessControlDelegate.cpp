#include "ExtendedAccessControlDelegate.h"
#include <access/AccessControl.h>
#include <access/AuthMode.h>
#include <lib/support/PersistentArray.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

// using namespace chip::Access;
// using Entry         = AccessControl::Entry;
// using EntryIterator = AccessControl::EntryIterator;
// using Target        = AccessControl::Entry::Target;

namespace chip {
namespace Access {


class EntryIteratorDelegate: public AccessControl::EntryIterator::Delegate
{
public:
    EntryIteratorDelegate(AccessControl::EntryIterator::Delegate *first, AccessControl::EntryIterator::Delegate *second):
        mFirst(first), mSecond(second) {}

    // EntryIteratorDelegate(AccessControl::EntryIterator &first, AccessControl::EntryIterator &second):
    //     mFirst(&first.GetDelegate()), mSecond(&second.GetDelegate()) {}

    EntryIteratorDelegate(const Delegate &)             = delete;
    EntryIteratorDelegate & operator=(const Delegate &) = delete;
    virtual ~EntryIteratorDelegate() = default;

    virtual void Release() {
        if(mFirst) {
            mFirst->Release();
        }
        if(mSecond) {
            mSecond->Release();
        }

        // WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
        // WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
        // TODO: REPLACE THIS OBJECT SUICIDE
        delete this;
        // WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
        // WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
    }

    virtual CHIP_ERROR Next(AccessControl::Entry & entry) {
        VerifyOrReturnError(nullptr != mFirst, CHIP_ERROR_INCORRECT_STATE);
        if(CHIP_NO_ERROR == mError) {
            mError = mFirst->Next(entry);
        }
        if(mSecond && (CHIP_ERROR_SENTINEL == mError)) {
            return mSecond->Next(entry);
        }
        return mError;
    }

    CHIP_ERROR mError = CHIP_NO_ERROR;
    AccessControl::EntryIterator::Delegate *mFirst = nullptr;
    AccessControl::EntryIterator::Delegate *mSecond = nullptr;
};


void ExtendedAccessControlDelegate::Release() {}

CHIP_ERROR ExtendedAccessControlDelegate::Init() {
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mPrimary->Init());
    if(mSecondary) {
        ReturnErrorOnFailure(mSecondary->Init());
    }
    return CHIP_NO_ERROR;
}

void ExtendedAccessControlDelegate::Finish() {
    if(mPrimary) {
        mPrimary->Finish();
    }
    if(mSecondary) {
        mSecondary->Finish();
    }
}

// Capabilities
CHIP_ERROR ExtendedAccessControlDelegate::GetMaxEntriesPerFabric(size_t & value) const
{
    ChipLogDetail(DeviceLayer, "~~~ ExtendedAccessControlDelegate::GetMaxEntriesPerFabric: #%p/%p\n", mPrimary, mSecondary);
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->GetMaxEntriesPerFabric(value);
}

CHIP_ERROR ExtendedAccessControlDelegate::GetMaxSubjectsPerEntry(size_t & value) const
{
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->GetMaxSubjectsPerEntry(value);
}

CHIP_ERROR ExtendedAccessControlDelegate::GetMaxTargetsPerEntry(size_t & value) const
{
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->GetMaxTargetsPerEntry(value);
}

CHIP_ERROR ExtendedAccessControlDelegate::GetMaxEntryCount(size_t & value) const
{
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->GetMaxEntryCount(value);
}

// Actualities
CHIP_ERROR ExtendedAccessControlDelegate::GetEntryCount(FabricIndex fabric, size_t & value) const
{
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->GetEntryCount(fabric, value);
}

CHIP_ERROR ExtendedAccessControlDelegate::GetEntryCount(size_t & value) const
{
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->GetEntryCount(value);
}

// Preparation
CHIP_ERROR ExtendedAccessControlDelegate::PrepareEntry(AccessControl::Entry & entry) {
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->PrepareEntry(entry);
}

// CRUD
CHIP_ERROR ExtendedAccessControlDelegate::CreateEntry(size_t * index, const AccessControl::Entry & entry, FabricIndex * fabricIndex) {
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->CreateEntry(index, entry, fabricIndex);
}

CHIP_ERROR ExtendedAccessControlDelegate::ReadEntry(size_t index, AccessControl::Entry & entry, const FabricIndex * fabricIndex) const {
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->ReadEntry(index, entry, fabricIndex);
}

CHIP_ERROR ExtendedAccessControlDelegate::UpdateEntry(size_t index, const AccessControl::Entry & entry, const FabricIndex * fabricIndex) {
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->UpdateEntry(index, entry, fabricIndex);
}

CHIP_ERROR ExtendedAccessControlDelegate::DeleteEntry(size_t index, const FabricIndex * fabricIndex) {
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);
    return mPrimary->DeleteEntry(index, fabricIndex);
}

// Iteration
CHIP_ERROR ExtendedAccessControlDelegate::Entries(AccessControl::EntryIterator & iterator, const FabricIndex * fabricIndex) const {
    VerifyOrReturnError(nullptr != mPrimary, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(DeviceLayer, "~~~ ExtendedAccessControlDelegate::Entries: #%p/%p\n", mPrimary, mSecondary);

    AccessControl::EntryIterator::Delegate *delegate = nullptr; // TODO: Use a pool
    AccessControl::EntryIterator first;
    ReturnErrorOnFailure(mPrimary->Entries(first, fabricIndex)); // TODO: OPTIONAL fabricIndex
    AccessControl::EntryIterator::Delegate *it1 = &first.GetDelegate();
    if(mSecondary) {
        AccessControl::EntryIterator second;
        ReturnErrorOnFailure(mSecondary->Entries(second, fabricIndex)); 
        AccessControl::EntryIterator::Delegate *it2 = &second.GetDelegate();
        delegate = new EntryIteratorDelegate(it1, it2);
    } else {
        delegate = new EntryIteratorDelegate(it1, nullptr);
    }
    iterator.SetDelegate(*delegate);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtendedAccessControlDelegate::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege requestPrivilege) {
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Access
} // namespace chip
