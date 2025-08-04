/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "TermsAndConditionsManager.h"

#include "DefaultTermsAndConditionsProvider.h"

static chip::app::TermsAndConditionsManager sTermsAndConditionsManager;
static chip::app::DefaultTermsAndConditionsProvider sTermsAndConditionsProviderInstance;
static chip::app::DefaultTermsAndConditionsStorageDelegate sTermsAndConditionsStorageDelegateInstance;

chip::app::TermsAndConditionsManager * chip::app::TermsAndConditionsManager::GetInstance()
{
    return &sTermsAndConditionsManager;
}

CHIP_ERROR chip::app::TermsAndConditionsManager::Init(chip::PersistentStorageDelegate * inPersistentStorageDelegate,
                                                      const Optional<TermsAndConditions> & inRequiredTermsAndConditions)
{
    ReturnErrorOnFailure(sTermsAndConditionsStorageDelegateInstance.Init(inPersistentStorageDelegate));
    ReturnErrorOnFailure(
        sTermsAndConditionsProviderInstance.Init(&sTermsAndConditionsStorageDelegateInstance, inRequiredTermsAndConditions));

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::TermsAndConditionsManager::CommitAcceptance()
{
    return sTermsAndConditionsProviderInstance.CommitAcceptance();
}

CHIP_ERROR chip::app::TermsAndConditionsManager::GetAcceptance(Optional<TermsAndConditions> & outTermsAndConditions) const
{
    return sTermsAndConditionsProviderInstance.GetAcceptance(outTermsAndConditions);
}

CHIP_ERROR chip::app::TermsAndConditionsManager::GetAcknowledgementsRequired(bool & outAcknowledgementsRequired) const
{
    return sTermsAndConditionsProviderInstance.GetAcknowledgementsRequired(outAcknowledgementsRequired);
}

CHIP_ERROR chip::app::TermsAndConditionsManager::GetRequirements(Optional<TermsAndConditions> & outTermsAndConditions) const
{
    return sTermsAndConditionsProviderInstance.GetRequirements(outTermsAndConditions);
}

CHIP_ERROR chip::app::TermsAndConditionsManager::GetUpdateAcceptanceDeadline(Optional<uint32_t> & outUpdateAcceptanceDeadline) const
{
    return sTermsAndConditionsProviderInstance.GetUpdateAcceptanceDeadline(outUpdateAcceptanceDeadline);
}

CHIP_ERROR chip::app::TermsAndConditionsManager::ResetAcceptance()
{
    return sTermsAndConditionsProviderInstance.ResetAcceptance();
}

CHIP_ERROR chip::app::TermsAndConditionsManager::RevertAcceptance()
{
    return sTermsAndConditionsProviderInstance.RevertAcceptance();
}

CHIP_ERROR chip::app::TermsAndConditionsManager::SetAcceptance(const Optional<TermsAndConditions> & inTermsAndConditions)
{
    return sTermsAndConditionsProviderInstance.SetAcceptance(inTermsAndConditions);
}
