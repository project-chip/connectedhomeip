/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#pragma once

#include <access/AccessControl.h>

namespace chip {
namespace Access {

class EntryIteratorDelegate: public AccessControl::EntryIterator::Delegate
{
public:
    EntryIteratorDelegate(AccessControl::EntryIterator::Delegate *first, AccessControl::EntryIterator::Delegate *second):
        mFirst(first), mSecond(second) {}

    EntryIteratorDelegate(const Delegate &)             = delete;
    EntryIteratorDelegate & operator=(const Delegate &) = delete;
    virtual ~EntryIteratorDelegate() = default;

    virtual void Release();
    virtual CHIP_ERROR Next(AccessControl::Entry & entry);

    CHIP_ERROR mError = CHIP_NO_ERROR;
    AccessControl::EntryIterator::Delegate *mFirst = nullptr;
    AccessControl::EntryIterator::Delegate *mSecond = nullptr;
};

class ExtendedAccessControlDelegate : public AccessControl::Delegate
{
public:
    ExtendedAccessControlDelegate() = default;

    ExtendedAccessControlDelegate(const Delegate &)             = delete;
    ExtendedAccessControlDelegate & operator=(const Delegate &) = delete;
    virtual ~ExtendedAccessControlDelegate() = default;

    void Release() override;
    using AccessControl::Delegate::Init;
    CHIP_ERROR Init(AccessControl::Delegate *primary, AccessControl::Delegate *seconday);
    virtual CHIP_ERROR Init() override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    virtual void Finish() override;

    // Capabilities
    CHIP_ERROR GetMaxEntriesPerFabric(size_t & value) const override;
    CHIP_ERROR GetMaxSubjectsPerEntry(size_t & value) const override;
    CHIP_ERROR GetMaxTargetsPerEntry(size_t & value) const override;
    CHIP_ERROR GetMaxEntryCount(size_t & value) const override;

    // Actualities
    CHIP_ERROR GetEntryCount(FabricIndex fabric, size_t & value) const override;
    CHIP_ERROR GetEntryCount(size_t & value) const override;

    // CRUD
    CHIP_ERROR PrepareEntry(AccessControl::Entry & entry) override;
    CHIP_ERROR CreateEntry(size_t * index, const AccessControl::Entry & entry, FabricIndex * fabricIndex) override;
    CHIP_ERROR ReadEntry(size_t index, AccessControl::Entry & entry, const FabricIndex * fabricIndex) const override;
    CHIP_ERROR UpdateEntry(size_t index, const AccessControl::Entry & entry, const FabricIndex * fabricIndex) override;
    CHIP_ERROR DeleteEntry(size_t index, const FabricIndex * fabricIndex) override;

    // Iteration
    CHIP_ERROR Entries(AccessControl::EntryIterator & iterator, const FabricIndex * fabricIndex) const override;

    CHIP_ERROR Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege requestPrivilege) override;
    
private:
    AccessControl::Delegate *mPrimary = nullptr;
    AccessControl::Delegate *mSecondary = nullptr;
};
    
} // namespace Access
} // namespace chip
