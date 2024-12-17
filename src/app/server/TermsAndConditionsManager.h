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

#include <lib/core/CHIPPersistentStorageDelegate.h>

#include "TermsAndConditionsProvider.h"

namespace chip {
namespace app {

class TermsAndConditionsManager : public TermsAndConditionsProvider
{
public:
    static TermsAndConditionsManager * GetInstance();
    CHIP_ERROR Init(PersistentStorageDelegate * inPersistentStorageDelegate,
                    const Optional<TermsAndConditions> & inRequiredTermsAndConditions);
    CHIP_ERROR CommitAcceptance();
    CHIP_ERROR GetAcceptance(Optional<TermsAndConditions> & outTermsAndConditions) const;
    CHIP_ERROR GetAcknowledgementsRequired(bool & outAcknowledgementsRequired) const;
    CHIP_ERROR GetRequirements(Optional<TermsAndConditions> & outTermsAndConditions) const;
    CHIP_ERROR GetUpdateAcceptanceDeadline(Optional<uint32_t> & outUpdateAcceptanceDeadline) const;
    CHIP_ERROR ResetAcceptance();
    CHIP_ERROR RevertAcceptance();
    CHIP_ERROR SetAcceptance(const Optional<TermsAndConditions> & inTermsAndConditions);
};

} // namespace app
} // namespace chip
