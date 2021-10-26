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

#include "AccessControl.h"

namespace {

using chip::FabricIndex;
using namespace chip::Access;

// Avoid GetAccessControl returning nullptr before SetAccessControl is called.
class UnimplementedDataProvider : public AccessControlDataProvider
{
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    void Finish() override {}

    EntryIterator * Entries() const override { return nullptr; }

    EntryIterator * Entries(FabricIndex fabricIndex) const override { return nullptr; }
};

// Avoid GetAccessControl returning nullptr before SetAccessControl is called.
UnimplementedDataProvider gUnimplementedDataProvider;
AccessControl gUnimplementedAccessControl(gUnimplementedDataProvider);

AccessControl * gAccessControl = &gUnimplementedAccessControl;

} // namespace

namespace chip {
namespace Access {

CHIP_ERROR AccessControl::Init()
{
    ChipLogDetail(DataManagement, "access control: initializing");
    // ...
    return CHIP_NO_ERROR;
}

void AccessControl::Finish()
{
    ChipLogDetail(DataManagement, "access control: finishing");
    // ...
}

CHIP_ERROR AccessControl::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege privilege)
{
    CHIP_ERROR err = CHIP_ERROR_ACCESS_DENIED;

    EntryIterator * iterator = mDataProvider.Entries(subjectDescriptor.fabricIndex);
    // TODO: check error (but can't until we have an implementation)
#if 0
    ReturnErrorCodeIf(iterator == nullptr, CHIP_ERROR_INTERNAL);
#else
    ReturnErrorCodeIf(iterator == nullptr, CHIP_NO_ERROR);
#endif

    // TODO: a few more cases (PASE commissioning, CASE Authenticated Tags, etc.)

    while (auto entry = iterator->Next())
    {
        ChipLogDetail(DataManagement, "Checking entry");

        if (!entry->MatchesPrivilege(privilege))
            continue;
        ChipLogDetail(DataManagement, "  --> matched privilege");
        if (!entry->MatchesAuthMode(subjectDescriptor.authMode))
            continue;
        ChipLogDetail(DataManagement, "  --> matched authmode");
        if (!entry->MatchesSubject(subjectDescriptor.subject))
            continue;
        ChipLogDetail(DataManagement, "  --> matched subject");
        if (!entry->MatchesTarget(requestPath.endpoint, requestPath.cluster))
            continue;
        ChipLogDetail(DataManagement, "  --> matched target");

        err = CHIP_NO_ERROR;
        break;
    }

    iterator->Release();
    return err;
}

AccessControl * GetAccessControl()
{
    return gAccessControl;
}

void SetAccessControl(AccessControl * accessControl)
{
    if (accessControl != nullptr)
    {
        gAccessControl = accessControl;
    }
}

} // namespace Access
} // namespace chip
