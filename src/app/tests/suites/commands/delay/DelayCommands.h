/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
