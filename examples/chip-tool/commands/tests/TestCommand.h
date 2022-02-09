/*
 *   Copyright (c) 2021 Project CHIP Authors
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

#include "../common/CHIPCommand.h"
#include <app/tests/suites/commands/discovery/DiscoveryCommands.h>
#include <app/tests/suites/commands/log/LogCommands.h>
#include <app/tests/suites/commands/system/SystemCommands.h>
#include <app/tests/suites/include/ConstraintsChecker.h>
#include <app/tests/suites/include/PICSChecker.h>
#include <app/tests/suites/include/ValueChecker.h>
#include <lib/support/UnitTestUtils.h>
#include <zap-generated/tests/CHIPClustersTest.h>

constexpr uint16_t kTimeoutInSeconds = 90;

class TestCommand : public CHIPCommand,
                    public ValueChecker,
                    public ConstraintsChecker,
                    public PICSChecker,
                    public LogCommands,
                    public DiscoveryCommands,
                    public SystemCommands
{
public:
    TestCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand(commandName, credsIssuerConfig), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("delayInMs", 0, UINT64_MAX, &mDelayInMs);
        AddArgument("PICS", &mPICSFilePath);
    }

    ~TestCommand(){};

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(kTimeoutInSeconds); }

    virtual void NextTest() = 0;

    /////////// GlobalCommands Interface /////////
    CHIP_ERROR Wait(chip::System::Clock::Timeout ms);
    CHIP_ERROR WaitForMs(uint16_t ms) { return Wait(chip::System::Clock::Milliseconds32(ms)); }
    CHIP_ERROR WaitForCommissionee();

protected:
    std::map<std::string, ChipDevice *> mDevices;
    chip::NodeId mNodeId;

    static void OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device);
    static void OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error);
    static void OnWaitForMsFn(chip::System::Layer * systemLayer, void * context);

    CHIP_ERROR ContinueOnChipMainThread() override { return WaitForMs(0); };

    void Exit(std::string message) override;
    void ThrowFailureResponse();
    void ThrowSuccessResponse();

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    void Wait()
    {
        if (mDelayInMs.HasValue())
        {
            chip::test_utils::SleepMillis(mDelayInMs.Value());
        }
    };
    chip::Optional<uint64_t> mDelayInMs;
    chip::Optional<char *> mPICSFilePath;
    chip::Optional<chip::EndpointId> mEndpointId;
    chip::Optional<uint16_t> mTimeout;
};
