/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file DeviceCallbacks.h
 *
 * Implementations for the DeviceManager callbacks for this application
 *
 **/

#pragma once

#include <BindingHandler.h>
#include <common/CHIPDeviceManager.h>
#include <common/CommonDeviceCallbacks.h>

class AppDeviceCallbacks : public CommonDeviceCallbacks
{
public:
    virtual void PostAttributeChangeCallback(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                             uint8_t mask, uint8_t type, uint16_t size, uint8_t * value);

private:
    void OnIdentifyPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
    void OnOffSwitchConfigurationAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t type,
                                                         uint8_t * value, uint16_t size);
};

class AppDeviceCallbacksDelegate : public DeviceCallbacksDelegate
{
public:
    void OnIPv4ConnectivityEstablished(void) override {}
    void OnIPv4ConnectivityLost(void) override {}
    void OnDnssdInitialized(void) override { InitBindingHandler(); }
};
