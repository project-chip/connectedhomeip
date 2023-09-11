/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "app-common/zap-generated/ids/Attributes.h"
#include "app-common/zap-generated/ids/Clusters.h"
#include "app-common/zap-generated/ids/Commands.h"
#include "lib/core/CHIPError.h"
#include <app/CommandSender.h>
#include <app/clusters/bindings/BindingManager.h>
#include <controller/InvokeInteraction.h>
#include <platform/CHIPDeviceLayer.h>

class BindingHandler
{
public:
    static void Init();
    static void ThermostatNotifyFunction(intptr_t);
    static void PrintBindingTable();
    static bool IsGroupBound();

    struct BindingData
    {
        chip::EndpointId EndpointId;
        chip::CommandId CommandId;
        chip::ClusterId ClusterId;
        chip::AttributeId AttributeId;
        uint8_t Value;
        bool IsGroup{ false };
    };

    static void SubscribeHumidityAttribute(chip::AttributeId attributeId, chip::DeviceProxy * peer_device, void * context);

private:
    static void ReadTemperatureAttribute(chip::AttributeId, const EmberBindingTableEntry &, chip::DeviceProxy *, void *);
    static void ReadHumidityAttribute(chip::AttributeId, const EmberBindingTableEntry &, chip::DeviceProxy *, void *);
    static void ThermostatChangedHandler(const EmberBindingTableEntry &, chip::OperationalDeviceProxy *, void *);
    static void ThermostatContextReleaseHandler(void *);
    static void InitInternal(intptr_t);
};
