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

#include "SubDevice.h"
#include "SubDeviceManager.h"
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/reporting/reporting.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/ZclString.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

#ifdef __cplusplus
extern "C" {
#endif

// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGED_NODE 0x0013
// (taken from lo-devices.xml)
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100

// (taken from chip-devices.xml)
#define DEVICE_TYPE_ROOT_NODE 0x0016
// (taken from chip-devices.xml)
#define DEVICE_TYPE_BRIDGE 0x000e

// Device Version for dynamic endpoints:
#define DEVICE_VERSION_DEFAULT 1

/* REVISION definitions:
 */

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)
#define ZCL_BRIDGED_DEVICE_BASIC_CLUSTER_REVISION (1u)
#define ZCL_FIXED_LABEL_CLUSTER_REVISION (1u)
#define ZCL_ON_OFF_CLUSTER_REVISION (4u)

int AddDeviceEndpoint(SubDevice * dev, EmberAfEndpointType * ep, const Span<const EmberAfDeviceType> & deviceTypeList,
                      const Span<DataVersion> & dataVersionStorage, chip::EndpointId parentEndpointId);
CHIP_ERROR RemoveDeviceEndpoint(SubDevice * dev);
Protocols::InteractionModel::Status HandleReadBridgedDeviceBasicAttribute(SubDevice * dev, chip::AttributeId attributeId,
                                                                          uint8_t * buffer, uint16_t maxReadLength);
void HandleDeviceStatusChanged(SubDevice * dev, SubDevice::Changed_t itemChangedMask);
void Init_Bridge_Endpoint();
#ifdef __cplusplus
}
#endif
