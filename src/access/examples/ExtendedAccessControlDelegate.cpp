#include "ExtendedAccessControlDelegate.h"
#include <access/AccessControl.h>
#include <access/AuthMode.h>
#include <lib/support/PersistentArray.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Pool.h>

// using namespace chip::Access;
// using Entry         = AccessControl::Entry;
// using EntryIterator = AccessControl::EntryIterator;
// using Target        = AccessControl::Entry::Target;

namespace chip {
namespace Access {

static constexpr size_t kIteratorsMax = 2;
static ObjectPool<EntryIteratorDelegate, kIteratorsMax> sEntryIterators;

void EntryIteratorDelegate::Release() {
    if(mFirst) {
        mFirst->Release();
    }
    if(mSecond) {
        mSecond->Release();
    }
    sEntryIterators.ReleaseObject(this);
}

CHIP_ERROR EntryIteratorDelegate::Next(AccessControl::Entry & entry) {
    VerifyOrReturnError(nullptr != mFirst, CHIP_ERROR_INCORRECT_STATE);
    if(CHIP_NO_ERROR == mError) {
        mError = mFirst->Next(entry);
    }
    if(mSecond && (CHIP_ERROR_SENTINEL == mError)) {
        return mSecond->Next(entry);
    }
    return mError;
}


void ExtendedAccessControlDelegate::Release() {}

CHIP_ERROR ExtendedAccessControlDelegate::Init(AccessControl::Delegate *primary, AccessControl::Delegate *secondary)
{
    VerifyOrReturnError((nullptr != primary) && (nullptr != secondary), CHIP_ERROR_INCORRECT_STATE);
    // Primary
    mPrimary = primary;
    ReturnErrorOnFailure(mPrimary->Init());
    // Secondary
    mSecondary = secondary;
    return mSecondary->Init();
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
    VerifyOrReturnError(nullptr != mPrimary || nullptr != mSecondary, CHIP_ERROR_INCORRECT_STATE);
    ChipLogDetail(DeviceLayer, "~~~ ExtendedAccessControlDelegate::Entries: #%p/%p\n", mPrimary, mSecondary);
    // Primary
    AccessControl::EntryIterator first;
    ReturnErrorOnFailure(mPrimary->Entries(first, fabricIndex)); // TODO: OPTIONAL fabricIndex
    AccessControl::EntryIterator::Delegate *it1 = &first.GetDelegate();
    // Secondary
    AccessControl::EntryIterator second;
    ReturnErrorOnFailure(mSecondary->Entries(second, fabricIndex)); 
    AccessControl::EntryIterator::Delegate *it2 = &second.GetDelegate();
    // Delegate
    AccessControl::EntryIterator::Delegate *delegate = sEntryIterators.CreateObject(it1, it2);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_NO_MEMORY);
    iterator.SetDelegate(*delegate);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtendedAccessControlDelegate::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege requestPrivilege) {
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Access
} // namespace chip
