/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <atomic>

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>

#include <app/tests/suites/commands/delay/DelayCommands.h>
#include <app/tests/suites/commands/discovery/DiscoveryCommands.h>
#include <app/tests/suites/commands/log/LogCommands.h>
#include <app/tests/suites/include/ConstraintsChecker.h>
#include <app/tests/suites/include/PICSChecker.h>
#include <app/tests/suites/include/TestRunner.h>
#include <app/tests/suites/include/ValueChecker.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>

constexpr const char kIdentityAlpha[] = "";
constexpr const char kIdentityBeta[]  = "";
constexpr const char kIdentityGamma[] = "";

class TestCommand : public TestRunner,
                    public PICSChecker,
                    public LogCommands,
                    public DiscoveryCommands,
                    public DelayCommands,
                    public ValueChecker,
                    public ConstraintsChecker
{
public:
    TestCommand(const char * commandName, uint16_t testsCount) :
        TestRunner(commandName, testsCount), mCommandPath(0, 0, 0), mAttributePath(0, 0, 0)
    {}
    virtual ~TestCommand() {}

    void SetCommandExitStatus(CHIP_ERROR status)
    {
        chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
        exit(CHIP_NO_ERROR == status ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    template <typename T>
    size_t AddArgument(const char * name, chip::Optional<T> * value)
    {
        return 0;
    }

    template <typename T>
    size_t AddArgument(const char * name, int64_t min, uint64_t max, chip::Optional<T> * value)
    {
        return 0;
    }

    CHIP_ERROR ContinueOnChipMainThread(CHIP_ERROR err) override
    {
        if (CHIP_NO_ERROR == err)
        {
            NextTest();
        }
        else
        {
            Exit(chip::ErrorStr(err), err);
        }
        return CHIP_NO_ERROR;
    }

    void Exit(std::string message, CHIP_ERROR err) override
    {
        LogEnd(message, err);
        SetCommandExitStatus(err);
    }

    static void ScheduleNextTest(intptr_t context)
    {
        TestCommand * command = reinterpret_cast<TestCommand *>(context);
        command->isRunning    = true;
        command->NextTest();
        chip::DeviceLayer::PlatformMgr().RemoveEventHandler(OnPlatformEvent, context);
    }

    static void OnPlatformEvent(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
    {
        switch (event->Type)
        {
        case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
            ChipLogProgress(chipTool, "Commissioning complete");
            chip::DeviceLayer::PlatformMgr().ScheduleWork(ScheduleNextTest, arg);
            break;
        }
    }

    void CheckCommandPath(const chip::app::ConcreteCommandPath & commandPath)
    {
        if (commandPath == mCommandPath)
        {
            NextTest();
            return;
        }

        ChipLogError(chipTool, "CommandPath does not match");
        SetCommandExitStatus(CHIP_ERROR_INTERNAL);
    }

    void CheckAttributePath(const chip::app::ConcreteAttributePath & attributePath)
    {
        if (attributePath == mAttributePath)
        {
            NextTest();
            return;
        }

        ChipLogError(chipTool, "AttributePath does not match");
        return SetCommandExitStatus(CHIP_ERROR_INTERNAL);
    }

    void ClearAttributeAndCommandPaths()
    {
        mCommandPath   = chip::app::ConcreteCommandPath(0, 0, 0);
        mAttributePath = chip::app::ConcreteAttributePath(0, 0, 0);
    }

    std::atomic_bool isRunning{ true };

    CHIP_ERROR WaitAttribute(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId)
    {
        ClearAttributeAndCommandPaths();
        ChipLogError(chipTool, "[Endpoint: 0x%08x Cluster: %d, Attribute: %d]", endpointId, clusterId, attributeId);
        mAttributePath = chip::app::ConcreteAttributePath(endpointId, clusterId, attributeId);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR WaitCommand(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId)
    {
        ClearAttributeAndCommandPaths();
        ChipLogError(chipTool, "[Endpoint: 0x%08x Cluster: %d, Command: %d]", endpointId, clusterId, commandId);
        mCommandPath = chip::app::ConcreteCommandPath(endpointId, clusterId, commandId);
        return CHIP_NO_ERROR;
    }

protected:
    chip::app::ConcreteCommandPath mCommandPath;
    chip::app::ConcreteAttributePath mAttributePath;
    chip::Optional<chip::EndpointId> mEndpointId;
    void SetIdentity(const char * name){};

    /////////// DelayCommands Interface /////////
    void OnWaitForMs() override { NextTest(); }

    CHIP_ERROR WaitForCommissioning(const char * identity,
                                    const chip::app::Clusters::DelayCommands::Commands::WaitForCommissioning::Type & value) override
    {
        isRunning = false;
        return chip::DeviceLayer::PlatformMgr().AddEventHandler(OnPlatformEvent, reinterpret_cast<intptr_t>(this));
    }
};
