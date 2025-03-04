/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
 *    Copyright 2024 NXP
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

/**
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/
#include "DeviceCallbacks.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/Dnssd.h>
#include <app/util/attribute-storage.h>
#include <app/util/attribute-table.h>

#include <lib/support/CodeUtils.h>


using namespace ::chip;
using namespace chip::app;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters;

void CooksurfaceApp::DeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                                          uint8_t type, uint16_t size, uint8_t * value)
{
    ChipLogProgress(DeviceLayer,
                    "endpointId " ChipLogFormatMEI " clusterId " ChipLogFormatMEI " attribute ID: " ChipLogFormatMEI
                    " Type: %u Value: %u, length %u",
                    ChipLogValueMEI(endpointId), ChipLogValueMEI(clusterId), ChipLogValueMEI(attributeId), type, *value, size);
    switch (clusterId)
    {
    case Clusters::OnOff::Id:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;
    }
}

void CooksurfaceApp::DeviceCallbacks::OnOnOffPostAttributeChangeCallback(chip::EndpointId endpointId, chip::AttributeId attributeId,
                                                                 uint8_t * value)
{
    switch (attributeId)
    {
    case Clusters::OnOff::Attributes::OnOff::Id:
        break;

    default:;
    }
}

// This returns an instance of this class.
CooksurfaceApp::DeviceCallbacks & CooksurfaceApp::DeviceCallbacks::GetDefaultInstance()
{
    static CooksurfaceApp::DeviceCallbacks sDeviceCallbacks;
    return sDeviceCallbacks;
}

chip::DeviceManager::CHIPDeviceManagerCallbacks & chip::NXP::App::GetDeviceCallbacks()
{
    return CooksurfaceApp::DeviceCallbacks::GetDefaultInstance();
}
