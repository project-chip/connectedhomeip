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

#include <system/SystemConfig.h>

#include "DefaultEnhancedSetupFlowProvider.h"

#include <lib/core/CHIPConfig.h>
#include <lib/support/CodeUtils.h>

CHIP_ERROR chip::app::DefaultEnhancedSetupFlowProvider::Init(TermsAndConditionsProvider * const inTermsAndConditionsProvider)
{
    VerifyOrReturnError(nullptr != inTermsAndConditionsProvider, CHIP_ERROR_INVALID_ARGUMENT);

    mTermsAndConditionsProvider = inTermsAndConditionsProvider;

    return CHIP_NO_ERROR;
}

CHIP_ERROR
chip::app::DefaultEnhancedSetupFlowProvider::HasTermsAndConditionsRequiredAcknowledgementsBeenAccepted(bool & outAccepted) const
{
    uint16_t requiredAcknowledgements;
    uint16_t requiredAcknowledgementsVersion;
    uint16_t acceptedAcknowledgements;
    uint16_t acceptedAcknowledgementsVersion;

    VerifyOrReturnError(nullptr != mTermsAndConditionsProvider, CHIP_ERROR_UNINITIALIZED);
    ReturnErrorOnFailure(mTermsAndConditionsProvider->GetRequirements(requiredAcknowledgements, requiredAcknowledgementsVersion));

    if (0 == requiredAcknowledgements)
    {
        outAccepted = true;
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(mTermsAndConditionsProvider->GetAcceptance(acceptedAcknowledgements, acceptedAcknowledgementsVersion));

    outAccepted = ((requiredAcknowledgements & acceptedAcknowledgements) == requiredAcknowledgements);

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultEnhancedSetupFlowProvider::HasTermsAndConditionsRequiredAcknowledgementsVersionBeenAccepted(
    bool & outAccepted) const
{
    uint16_t requiredAcknowledgements;
    uint16_t requiredAcknowledgementsVersion;
    uint16_t acceptedAcknowledgements;
    uint16_t acceptedAcknowledgementsVersion;

    VerifyOrReturnError(nullptr != mTermsAndConditionsProvider, CHIP_ERROR_UNINITIALIZED);
    ReturnErrorOnFailure(mTermsAndConditionsProvider->GetRequirements(requiredAcknowledgements, requiredAcknowledgementsVersion));

    if (0 == requiredAcknowledgementsVersion)
    {
        outAccepted = true;
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(mTermsAndConditionsProvider->GetAcceptance(acceptedAcknowledgements, acceptedAcknowledgementsVersion));

    outAccepted = (acceptedAcknowledgementsVersion >= requiredAcknowledgementsVersion);

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultEnhancedSetupFlowProvider::GetTermsAndConditionsRequiredAcknowledgements(uint16_t & outValue) const
{
    uint16_t requiredAcknowledgements;
    uint16_t requiredAcknowledgementsVersion;

    VerifyOrReturnError(nullptr != mTermsAndConditionsProvider, CHIP_ERROR_UNINITIALIZED);
    ReturnErrorOnFailure(mTermsAndConditionsProvider->GetRequirements(requiredAcknowledgements, requiredAcknowledgementsVersion));

    outValue = requiredAcknowledgements;

    return CHIP_NO_ERROR;
}

CHIP_ERROR
chip::app::DefaultEnhancedSetupFlowProvider::GetTermsAndConditionsRequiredAcknowledgementsVersion(uint16_t & outValue) const
{
    uint16_t requiredAcknowledgements;
    uint16_t requiredAcknowledgementsVersion;

    VerifyOrReturnError(nullptr != mTermsAndConditionsProvider, CHIP_ERROR_UNINITIALIZED);
    ReturnErrorOnFailure(mTermsAndConditionsProvider->GetRequirements(requiredAcknowledgements, requiredAcknowledgementsVersion));

    outValue = requiredAcknowledgementsVersion;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultEnhancedSetupFlowProvider::GetTermsAndConditionsAcceptedAcknowledgements(uint16_t & outValue) const
{
    uint16_t acceptedAcknowledgements;
    uint16_t acceptedAcknowledgementsVersion;

    VerifyOrReturnError(nullptr != mTermsAndConditionsProvider, CHIP_ERROR_UNINITIALIZED);
    ReturnErrorOnFailure(mTermsAndConditionsProvider->GetAcceptance(acceptedAcknowledgements, acceptedAcknowledgementsVersion));

    outValue = acceptedAcknowledgements;

    return CHIP_NO_ERROR;
}

CHIP_ERROR
chip::app::DefaultEnhancedSetupFlowProvider::GetTermsAndConditionsAcceptedAcknowledgementsVersion(uint16_t & outValue) const
{
    uint16_t acceptedAcknowledgements;
    uint16_t acceptedAcknowledgementsVersion;

    VerifyOrReturnError(nullptr != mTermsAndConditionsProvider, CHIP_ERROR_UNINITIALIZED);
    ReturnErrorOnFailure(mTermsAndConditionsProvider->GetAcceptance(acceptedAcknowledgements, acceptedAcknowledgementsVersion));

    outValue = acceptedAcknowledgementsVersion;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultEnhancedSetupFlowProvider::SetTermsAndConditionsAcceptance(uint16_t inTCAcknowledgementsValue,
                                                                                        uint16_t inTCAcknowledgementsVersionValue)
{
    VerifyOrReturnError(nullptr != mTermsAndConditionsProvider, CHIP_ERROR_UNINITIALIZED);
    ReturnErrorOnFailure(mTermsAndConditionsProvider->SetAcceptance(inTCAcknowledgementsValue, inTCAcknowledgementsVersionValue));

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultEnhancedSetupFlowProvider::ClearTermsAndConditionsAcceptance()
{
    VerifyOrReturnError(nullptr != mTermsAndConditionsProvider, CHIP_ERROR_UNINITIALIZED);
    ReturnErrorOnFailure(mTermsAndConditionsProvider->ClearAcceptance());

    return CHIP_NO_ERROR;
}
