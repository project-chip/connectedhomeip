/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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
#include "AppTask.h"

#include "DeviceCallbacks.h"
#include "Globals.h"

#include <app/util/util.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

static const char * TAG = "presence-app-callbacks";

using namespace chip;
using namespace chip::Inet;
using namespace chip::System;
using namespace chip::app::Clusters;

void emberAfOccupancySensingClusterInitCallback(EndpointId endpoint)
{
    ESP_LOGI(TAG, "emberAfOccupancySensingClusterInitCallback");
    GetAppTask().UpdateOccupancySensorConfiguration();
}

void AppDeviceCallbacksDelegate::OnIPv4ConnectivityEstablished()
{
    wifiLED.Set(true);
}

void AppDeviceCallbacksDelegate::OnDnssdInitialized()
{
    GetAppTask().EnableStatusUpdates();
    wifiLED.Set(true);
}

void AppDeviceCallbacksDelegate::OnIPv4ConnectivityLost()
{
    GetAppTask().DisableStatusUpdates();
    wifiLED.Set(false);
}

