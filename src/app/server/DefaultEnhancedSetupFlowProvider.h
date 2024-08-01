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

#pragma once

#include <system/SystemConfig.h>

#include "EnhancedSetupFlowProvider.h"

#include <stdint.h>

#include <lib/core/CHIPError.h>

#include "TermsAndConditionsProvider.h"

namespace chip {
namespace app {
class DefaultEnhancedSetupFlowProvider : public EnhancedSetupFlowProvider
{
public:
    /**
     * @brief Initializes the EnhancedSetupFlowProvider.
     *
     * @param[in] inTermsAndConditionsProvider The terms and conditions provide dependency.
     */
    CHIP_ERROR Init(TermsAndConditionsProvider * const inTermsAndConditionsProvider);

    /**
     * @copydoc EnhancedSetupFlowProvider::HasTermsAndConditionsRequiredAcknowledgementsBeenAccepted
     */
    CHIP_ERROR HasTermsAndConditionsRequiredAcknowledgementsBeenAccepted(bool & outAccepted) const override;

    /**
     * @copydoc EnhancedSetupFlowProvider::HasTermsAndConditionsRequiredAcknowledgementsVersionBeenAccepted
     */
    CHIP_ERROR HasTermsAndConditionsRequiredAcknowledgementsVersionBeenAccepted(bool & outAccepted) const override;

    /**
     * @copydoc EnhancedSetupFlowProvider::GetTermsAndConditionsRequiredAcknowledgements
     */
    CHIP_ERROR GetTermsAndConditionsRequiredAcknowledgements(uint16_t & outValue) const override;

    /**
     * @copydoc EnhancedSetupFlowProvider::GetTermsAndConditionsRequiredAcknowledgementsVersion
     */
    CHIP_ERROR GetTermsAndConditionsRequiredAcknowledgementsVersion(uint16_t & outValue) const override;

    /**
     * @copydoc EnhancedSetupFlowProvider::GetTermsAndConditionsAcceptedAcknowledgements
     */
    CHIP_ERROR GetTermsAndConditionsAcceptedAcknowledgements(uint16_t & outValue) const override;

    /**
     * @copydoc EnhancedSetupFlowProvider::GetTermsAndConditionsAcceptedAcknowledgementsVersion
     */
    CHIP_ERROR GetTermsAndConditionsAcceptedAcknowledgementsVersion(uint16_t & outValue) const override;

    /**
     * @copydoc EnhancedSetupFlowProvider::SetTermsAndConditionsAcceptance
     */
    CHIP_ERROR SetTermsAndConditionsAcceptance(uint16_t aTCAcknowledgements, uint16_t inTCAcknowledgementsVersionValue) override;

    /**
     * @copydoc EnhancedSetupFlowProvider::ClearTermsAndConditionsAcceptance
     */
    CHIP_ERROR ClearTermsAndConditionsAcceptance() override;

private:
    TermsAndConditionsProvider * mTermsAndConditionsProvider; /**< TermsAndConditionsProvider instance. */
};

}; // namespace app
}; // namespace chip
