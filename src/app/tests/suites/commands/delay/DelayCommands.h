/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayer.h>

#include <app-common/zap-generated/tests/simulated-cluster-objects.h>

class DelayCommands
{
public:
    DelayCommands(){};
    virtual ~DelayCommands(){};

    virtual CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err) = 0;
    virtual void OnWaitForMs()                                  = 0;

    virtual CHIP_ERROR WaitForCommissionee(const char * identity,
                                           const chip::app::Clusters::DelayCommands::Commands::WaitForCommissionee::Type & value)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    };
    virtual CHIP_ERROR WaitForCommissioning(const char * identity,
                                            const chip::app::Clusters::DelayCommands::Commands::WaitForCommissioning::Type & value)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    };
    CHIP_ERROR WaitForMs(const char * identity, const chip::app::Clusters::DelayCommands::Commands::WaitForMs::Type & value);
    // Wait for any message specified by value.message for the application specified by value.registerKey
    // If the message is never seen, a timeout would occur
    CHIP_ERROR WaitForMessage(const char * identity,
                              const chip::app::Clusters::DelayCommands::Commands::WaitForMessage::Type & value);

    // Busy-wait for a given duration in milliseconds
    CHIP_ERROR BusyWaitFor(chip::System::Clock::Milliseconds32 durationInMs);

private:
    static void OnWaitForMsFn(chip::System::Layer * systemLayer, void * context);
    CHIP_ERROR RunInternal(const char * command);
};
