/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <AppMain.h>
#include <air-purifier-manager.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/af.h>
#include <lib/support/logging/CHIPLogging.h>

#define AIR_PURIFIER_ENDPOINT 1
#define AIR_QUALITY_SENSOR_ENDPOINT 2
#define RELATIVE_HUMIDITY_SENSOR_ENDPOINT 3
#define TEMPERATURE_SENSOR_ENDPOINT 4

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

static AirPurifierManager * sAirPurifierManager = nullptr;

// TODO: Make a generic version of this as part of the AirPurifierManager interface, then offload this to there in common code, so
// we can keep this slim for each impl
void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    if ((attributePath.mClusterId == FanControl::Id) && (attributePath.mAttributeId == FanControl::Attributes::PercentSetting::Id))
    {
        DataModel::Nullable<Percent> percentSetting = static_cast<DataModel::Nullable<uint8_t>>(*value);
        if (percentSetting.IsNull())
        {
            sAirPurifierManager->PercentSettingChangedCallback(0);
        }
        else
        {
            sAirPurifierManager->PercentSettingChangedCallback(percentSetting.Value());
        }
    }
    else if ((attributePath.mClusterId == FanControl::Id) &&
             (attributePath.mAttributeId == FanControl::Attributes::SpeedSetting::Id))
    {
        DataModel::Nullable<uint8_t> speedSetting = static_cast<DataModel::Nullable<uint8_t>>(*value);
        if (speedSetting.IsNull())
        {
            sAirPurifierManager->SpeedSettingChangedCallback(0);
        }
        else
        {
            sAirPurifierManager->SpeedSettingChangedCallback(speedSetting.Value());
        }
    }
}

void ApplicationInit()
{
    SetParentEndpointForEndpoint(AIR_QUALITY_SENSOR_ENDPOINT, AIR_PURIFIER_ENDPOINT);
    SetParentEndpointForEndpoint(TEMPERATURE_SENSOR_ENDPOINT, AIR_PURIFIER_ENDPOINT);
    SetParentEndpointForEndpoint(RELATIVE_HUMIDITY_SENSOR_ENDPOINT, AIR_PURIFIER_ENDPOINT);

    sAirPurifierManager =
        new AirPurifierManager(EndpointId(AIR_PURIFIER_ENDPOINT), EndpointId(AIR_QUALITY_SENSOR_ENDPOINT),
                               EndpointId(TEMPERATURE_SENSOR_ENDPOINT), EndpointId(RELATIVE_HUMIDITY_SENSOR_ENDPOINT));
}

void ApplicationShutdown()
{
    ChipLogDetail(NotSpecified, "Air Purifier: ApplicationInit()");
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();
    return 0;
}