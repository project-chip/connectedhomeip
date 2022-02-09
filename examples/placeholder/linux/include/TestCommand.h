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

#include <app/tests/suites/commands/discovery/DiscoveryCommands.h>
#include <app/tests/suites/commands/log/LogCommands.h>
#include <app/tests/suites/include/PICSChecker.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>

constexpr const char kIdentityAlpha[] = "";
constexpr const char kIdentityBeta[]  = "";
constexpr const char kIdentityGamma[] = "";

class TestCommand : public PICSChecker, public LogCommands, public DiscoveryCommands
{
public:
    TestCommand(const char * commandName) : mCommandPath(0, 0, 0), mAttributePath(0, 0, 0) {}
    virtual ~TestCommand() {}

    virtual void NextTest() = 0;
    CHIP_ERROR WaitMS(chip::System::Clock::Timeout ms)
    {
        return chip::DeviceLayer::SystemLayer().StartTimer(ms, OnWaitForMsFn, this);
    }
    CHIP_ERROR WaitForMs(uint16_t ms) { return WaitMS(chip::System::Clock::Milliseconds32(ms)); }
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

    CHIP_ERROR ContinueOnChipMainThread() override
    {
        NextTest();
        return CHIP_NO_ERROR;
    }

    void Exit(std::string message)
    {
        ChipLogError(chipTool, " ***** Test Failure: %s\n", message.c_str());
        SetCommandExitStatus(CHIP_ERROR_INTERNAL);
    }

    static void ScheduleNextTest(intptr_t context)
    {
        TestCommand * command = reinterpret_cast<TestCommand *>(context);
        command->isRunning    = true;
        command->NextTest();
        chip::DeviceLayer::PlatformMgr().RemoveEventHandler(OnPlatformEvent, context);
    }

    CHIP_ERROR WaitForCommissioning()
    {
        isRunning = false;
        return chip::DeviceLayer::PlatformMgr().AddEventHandler(OnPlatformEvent, reinterpret_cast<intptr_t>(this));
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
    static void OnWaitForMsFn(chip::System::Layer * systemLayer, void * context)
    {
        auto * command = static_cast<TestCommand *>(context);
        command->NextTest();
    }

    std::atomic_bool isRunning{ true };

protected:
    chip::app::ConcreteCommandPath mCommandPath;
    chip::app::ConcreteAttributePath mAttributePath;
    chip::Optional<chip::EndpointId> mEndpointId;
    void SetIdentity(const char * name){};
    void Wait(){};
};
