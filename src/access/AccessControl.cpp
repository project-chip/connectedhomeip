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

#include "Config.h"
#include "DataProvider.h"

namespace {

chip::access::Config::DataProvider dataProviderInstance;
chip::access::AccessControl accessControlInstance(dataProviderInstance);

} // namespace

namespace chip {
namespace access {

AccessControl * AccessControl::mInstance = &accessControlInstance;

CHIP_ERROR AccessControl::Init()
{
    ChipLogDetail(DataManagement, "access control: initializing");
    return mDataProvider.Init();
}

void AccessControl::Finish()
{
    ChipLogDetail(DataManagement, "access control: finishing");
    mDataProvider.Finish();
}

CHIP_ERROR AccessControl::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege privilege)
{
    CHIP_ERROR err = CHIP_ERROR_ACCESS_DENIED;

    EntryIterator* iterator = mDataProvider.Entries(subjectDescriptor.fabricIndex);
#if 0
    ReturnErrorCodeIf(iterator == nullptr, CHIP_ERROR_INTERNAL);
#else
    // TODO: until we have an actual implementation, allow access
    ReturnErrorCodeIf(iterator == nullptr, CHIP_NO_ERROR);
#endif

    while (iterator->HasNext())
    {
        ChipLogDetail(DataManagement, "Checking entry");
        auto & entry = iterator->Next();

        if (!entry.MatchesPrivilege(privilege))
            continue;
        ChipLogDetail(DataManagement, "  --> matched privilege");
        if (!entry.MatchesAuthMode(subjectDescriptor.authMode))
            continue;
        ChipLogDetail(DataManagement, "  --> matched authmode");
        // TODO: check CATs (subject1, subject2)
        if (!entry.MatchesSubject(subjectDescriptor.subject))
            continue;
        ChipLogDetail(DataManagement, "  --> matched subject");
        if (!entry.MatchesTarget(requestPath.endpoint, requestPath.cluster))
            continue;
        ChipLogDetail(DataManagement, "  --> matched target");

        err = CHIP_NO_ERROR;
        break;
    }

    iterator->Release();
    return err;
}

} // namespace access
} // namespace chip
