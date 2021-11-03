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

#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>

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

    CHIP_ERROR Log(const char * message)
    {
        ChipLogProgress(chipTool, "%s", message);
        NextTest();
        return CHIP_NO_ERROR;
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

protected:
    chip::app::ConcreteCommandPath mCommandPath;
    chip::app::ConcreteAttributePath mAttributePath;
};
