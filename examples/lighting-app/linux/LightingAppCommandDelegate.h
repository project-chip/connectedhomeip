/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "NamedPipeCommands.h"

#include <json/json.h>
#include <platform/DiagnosticDataProvider.h>

class LightingAppCommandHandler
{
public:
    static LightingAppCommandHandler * FromJSON(const char * json);

    static void HandleCommand(intptr_t context);

    LightingAppCommandHandler(Json::Value && jasonValue) : mJsonValue(std::move(jasonValue)) {}

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

class LightingAppCommandDelegate : public NamedPipeCommandDelegate
{
public:
    void OnEventCommandReceived(const char * json) override;
};
