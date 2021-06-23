/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "../common/Command.h"
#include <controller/CHIPCommissionableNodeController.h>

/**
 *    @brief
 *      RequestCommissioningCommand, when issued by the user, will request a
 *      user selected Commissioner to begin commissioning the requesting device
 *      (i.e. a Commissionee). This will involve the Commissionee entering
 *      commissioning mode, displaying an onboarding payload to the user,
 *      initiating a User directed commissioning request and Advertising itself
 *      as a Commissionable Node over DNS-SD.
 */
class RequestCommissioningCommand : public Command
{
public:
    RequestCommissioningCommand() : Command("request-commissioning") {}
    CHIP_ERROR Run() override;
    uint16_t GetWaitDurationInSeconds() const override { return 3 * 60; }

private:
    chip::Controller::CommissionableNodeController mCommissionableNodeController;
};
