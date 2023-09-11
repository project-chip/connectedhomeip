/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
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

#include <binding-handler.h>
#include <common/CHIPDeviceManager.h>
#include <common/CommonDeviceCallbacks.h>

class AppDeviceCallbacks : public CommonDeviceCallbacks
{
public:
    virtual void PostAttributeChangeCallback(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                             uint8_t type, uint16_t size, uint8_t * value);

private:
    void OnOnOffPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
    void OnLevelControlAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
#if CONFIG_DEVICE_TYPE_ESP32_C3_DEVKITM
    void OnColorControlAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
#endif
    void OnIdentifyPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint16_t size,
                                               uint8_t * value);
    bool mEndpointOnOffState[2];
};

class AppDeviceCallbacksDelegate : public DeviceCallbacksDelegate
{
public:
    void OnIPv4ConnectivityEstablished(void) override;
    void OnIPv4ConnectivityLost(void) override;
    void OnDnssdInitialized(void) override { InitBindingHandlers(); }
};
