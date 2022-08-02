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

#include <json/json.h>
#include <platform/DiagnosticDataProvider.h>

class AllClustersAppCommandHandler
{
public:
    static AllClustersAppCommandHandler * FromJSON(const char * json);

    static void HandleCommand(intptr_t context);

    AllClustersAppCommandHandler(Json::Value && jasonValue) : mJsonValue(std::move(jasonValue)) {}

private:
    Json::Value mJsonValue;

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
     * Should be called when the latching switch is moved to a new position.
     */
    void OnSwitchLatchedHandler(uint8_t newPosition);

    /**
     * Should be called when the momentary switch starts to be pressed.
     */
    void OnSwitchInitialPressedHandler(uint8_t newPosition);

    /**
     * Should be called when the momentary switch has been pressed for a "long" time.
     */
    void OnSwitchLongPressedHandler(uint8_t newPosition);

    /**
     * Should be called when the momentary switch has been released.
     */
    void OnSwitchShortReleasedHandler(uint8_t previousPosition);

    /**
     * Should be called when the momentary switch has been released after having been pressed for a long time.
     */
    void OnSwitchLongReleasedHandler(uint8_t previousPosition);

    /**
     * Should be called to indicate how many times the momentary switch has been pressed in a multi-press
     * sequence, during that sequence.
     */
    void OnSwitchMultiPressOngoingHandler(uint8_t newPosition, uint8_t count);

    /**
     * Should be called to indicate how many times the momentary switch has been pressed in a multi-press
     * sequence, after it has been detected that the sequence has ended.
     */
    void OnSwitchMultiPressCompleteHandler(uint8_t previousPosition, uint8_t count);
};

class AllClustersCommandDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * json) override;
};
