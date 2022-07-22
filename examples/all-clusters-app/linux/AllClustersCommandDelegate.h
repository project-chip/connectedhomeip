/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "NamedPipeCommands.h"

#include <platform/DiagnosticDataProvider.h>

class AllClustersCommandDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * command) override;

private:
    std::string mCurrentCommand;

    static void HandleEventCommand(intptr_t context);

    bool IsClusterPresentOnAnyEndpoint(chip::ClusterId clusterId);

    /**
     * Should be called when a reason that caused the device to start-up has been set.
     */
    void OnRebootSignalHandler(chip::DeviceLayer::BootReasonType bootReason);

    /**
     * Should be called when a general fault takes place on the Node.
     */
    void OnGeneralFaultEventHandler(uint32_t eventId);

    /**
     * Should be called when a software fault takes place on the Node.
     */
    void OnSoftwareFaultEventHandler(uint32_t eventId);

    /**
     * Should be called when a switch operation takes place on the Node.
     */
    void OnSwitchEventHandler(uint32_t eventId);
};
