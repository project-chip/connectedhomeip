/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <controller/CommissioningDelegate.h>
#include <controller/CurrentFabricRemover.h>
#include <controller/ICDRegistrationDelegate.h>

#include <commands/common/CredentialIssuerCommands.h>
#include <lib/support/Span.h>

class ChipToolICDRegistrationDelegate : public chip::Controller::ICDRegistrationDelegate
{
public:
    ChipToolICDRegistrationDelegate() : mCheckInNodeId(chip::kUndefinedNodeId){};

    void SetCheckInNodeId(chip::NodeId checkInNodeId);

    void GenerateSymmetricKey(chip::NodeId device,
                              chip::Callback::Callback<OnSymmetricKeyGenerationCompleted> * onCompletion) override;

    void OnRegistrationComplete(chip::NodeId device, uint32_t icdCounter) override;

private:
    chip::NodeId mCheckInNodeId;
};
