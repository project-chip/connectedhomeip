/*
 *
 * SPDX-FileCopyrightText: 2021-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <app/util/af-types.h>
#include <common/CHIPDeviceManager.h>
#include <common/CommonDeviceCallbacks.h>
#include <platform/CHIPDeviceLayer.h>

class AppDeviceCallbacks : public CommonDeviceCallbacks
{
public:
    virtual void PostAttributeChangeCallback(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                             uint8_t type, uint16_t size, uint8_t * value);

private:
    void OnIdentifyPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
    void OnOnOffPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
    void OnLevelControlAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
#if CONFIG_LED_TYPE_RMT
    void OnColorControlAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId, uint8_t * value);
#endif
};

class AppDeviceCallbacksDelegate : public DeviceCallbacksDelegate
{
public:
    void OnIPv4ConnectivityEstablished() override;
    void OnIPv4ConnectivityLost() override;
};
