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
#include <app/tests/suites/commands/commissioner/CommissionerCommands.h>
#include <app/tests/suites/commands/delay/DelayCommands.h>
#include <app/tests/suites/commands/discovery/DiscoveryCommands.h>
#include <app/tests/suites/commands/interaction_model/InteractionModel.h>
#include <app/tests/suites/commands/log/LogCommands.h>
#include <app/tests/suites/commands/system/SystemCommands.h>
#include <app/tests/suites/include/ConstraintsChecker.h>
#include <app/tests/suites/include/PICSChecker.h>
#include <app/tests/suites/include/TestRunner.h>
#include <app/tests/suites/include/ValueChecker.h>

constexpr uint16_t kTimeoutInSeconds = 90;

class TestCommand : public TestRunner,
                    public CHIPCommand,
                    public ValueChecker,
                    public ConstraintsChecker,
                    public PICSChecker,
                    public LogCommands,
                    public CommissionerCommands,
                    public DiscoveryCommands,
                    public SystemCommands,
                    public DelayCommands,
                    public InteractionModel
{
public:
    TestCommand(const char * commandName, uint16_t testsCount, CredentialIssuerCommands * credsIssuerConfig) :
        TestRunner(commandName, testsCount), CHIPCommand(commandName, credsIssuerConfig),
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        AddArgument("continueOnFailure", 0, 1, &mContinueOnFailure,
                    "Boolean indicating if the test runner should continue execution if a test fails. Default to false.");
        AddArgument("delayInMs", 0, UINT64_MAX, &mDelayInMs);
        AddArgument("PICS", &mPICSFilePath);
    }

    ~TestCommand(){};

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;

protected:
    /////////// DelayCommands Interface /////////
    CHIP_ERROR WaitForCommissionee(const char * identity,
                                   const chip::app::Clusters::DelayCommands::Commands::WaitForCommissionee::Type & value) override;
    void OnWaitForMs() override { NextTest(); };

    /////////// Interaction Model Interface /////////
    chip::DeviceProxy * GetDevice(const char * identity) override { return mDevices[identity].get(); }
    void OnResponse(const chip::app::StatusIB & status, chip::TLV::TLVReader * data) override{};

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err) override;

    chip::Controller::DeviceCommissioner & GetCommissioner(const char * identity) override
    {
        return CHIPCommand::GetCommissioner(identity);
    };

    static void ExitAsync(intptr_t context);
    void Exit(std::string message, CHIP_ERROR err = CHIP_ERROR_INTERNAL) override;

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    bool IsUnsupported(const chip::app::StatusIB & status)
    {
        return status.mStatus == chip::Protocols::InteractionModel::Status::UnsupportedAttribute ||
            status.mStatus == chip::Protocols::InteractionModel::Status::UnsupportedCommand;
    }

    chip::Optional<char *> mPICSFilePath;
    chip::Optional<uint16_t> mTimeout;
    std::map<std::string, std::unique_ptr<chip::OperationalDeviceProxy>> mDevices;

    // When set to false, prevents interaction model events from affecting the current test status.
    // This flag exists because if an error happens while processing a response the allocated
    // command client/sender (ReadClient/WriteClient/CommandSender) can not be deallocated
    // as it still used by the stack afterward. So a task is scheduled to run to close the
    // test suite as soon as possible, and pending events are ignored in between.
    bool mContinueProcessing = true;

    // When set to true, the test runner continue to run after a test failure.
    chip::Optional<bool> mContinueOnFailure;
    std::vector<std::string> mErrorMessages;
};
