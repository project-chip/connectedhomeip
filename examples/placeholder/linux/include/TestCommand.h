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

#include <app/tests/suites/pics/PICSBooleanExpressionParser.h>
#include <app/tests/suites/pics/PICSBooleanReader.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>

constexpr const char kIdentityAlpha[] = "";
constexpr const char kIdentityBeta[]  = "";
constexpr const char kIdentityGamma[] = "";

class TestCommand
{
public:
    TestCommand(const char * commandName) : mCommandPath(0, 0, 0), mAttributePath(0, 0, 0) {}
    virtual ~TestCommand() {}

    virtual void NextTest() = 0;
    void Wait() {}
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

    CHIP_ERROR Log(const char * message)
    {
        ChipLogProgress(chipTool, "%s", message);
        NextTest();
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR UserPrompt(const char * message)
    {
        ChipLogProgress(chipTool, "USER_PROMPT: %s", message);
        NextTest();
        return CHIP_NO_ERROR;
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
            chip::DeviceLayer::PlatformMgr().RemoveEventHandler(OnPlatformEvent, arg);

            TestCommand * command = reinterpret_cast<TestCommand *>(arg);
            command->isRunning    = true;
            command->NextTest();
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

    bool ShouldSkip(const char * expression)
    {
        // If there is no PICS configuration file, considers that nothing should be skipped.
        if (!PICS.HasValue())
        {
            return false;
        }

        std::map<std::string, bool> pics(PICS.Value());
        bool shouldSkip = !PICSBooleanExpressionParser::Eval(expression, pics);
        if (shouldSkip)
        {
            ChipLogProgress(chipTool, " **** Skipping: %s == false\n", expression);
            NextTest();
        }
        return shouldSkip;
    }

    chip::Optional<std::map<std::string, bool>> PICS;

    std::atomic_bool isRunning{ true };

protected:
    chip::app::ConcreteCommandPath mCommandPath;
    chip::app::ConcreteAttributePath mAttributePath;
    chip::Optional<chip::EndpointId> mEndpointId;
    void SetIdentity(const char * name){};
};
