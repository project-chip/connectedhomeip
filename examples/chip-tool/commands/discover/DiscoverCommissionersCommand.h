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

class DiscoverCommissionersCommand : public Command
{
public:
    DiscoverCommissionersCommand() : Command("commissioners") {}
    CHIP_ERROR Run() override;
    uint16_t GetWaitDurationInSeconds() const override { return 3; }
    void Shutdown() override;

private:
    chip::Controller::CommissionableNodeController mCommissionableNodeController;
};
