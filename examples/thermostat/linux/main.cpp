/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppMain.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/thermostat-server/AttributeAccessorShim.h>
#include <app/clusters/thermostat-server/CodegenIntegration.h>
#include <app/clusters/thermostat-server/ThermostatCluster.h>
#include <app/reporting/reporting.h>
#include <fstream>
#include <lib/core/DataModelTypes.h>
#include <platform/CHIPDeviceLayer.h>

#include "thermostat-delegate-impl.h"
#include "thermostat-hold-delegate-impl.h"
#include "thermostat-mode-delegate-impl.h"
#include "thermostat-presets-delegate-impl.h"
#include "thermostat-schedules-delegate-impl.h"
#include "thermostat-sensors-delegate-impl.h"
#include "thermostat-setpoints-delegate-impl.h"
#include "thermostat-suggestions-delegate-impl.h"

using namespace chip;
using namespace chip::app;

void OnIdentifyStart(Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
}

void OnIdentifyStop(Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
}

void OnTriggerEffect(Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        break;

    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        break;

    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        break;

    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        break;

    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
}

static Identify gIdentify0 = {
    chip::EndpointId{ 0 }, OnIdentifyStart, OnIdentifyStop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnTriggerEffect,
};

static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, OnIdentifyStart, OnIdentifyStop, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnTriggerEffect,
};

constexpr EndpointId gThermostatEndpoint(1);

static Clusters::Thermostat::ThermostatDelegate gThermostatDelegate(gThermostatEndpoint);

static Clusters::Thermostat::ThermostatSetpointsDelegate gSetpointsDelegate(gThermostatEndpoint);

static Clusters::Thermostat::ThermostatHoldDelegate gHoldDelegate(gThermostatEndpoint);

static Clusters::Thermostat::ThermostatPresetsDelegate gPresetsDelegate(gThermostatEndpoint);

static Clusters::Thermostat::ThermostatSuggestionsDelegate gSuggestionsDelegate(gThermostatEndpoint, gPresetsDelegate);

static Clusters::Thermostat::ThermostatSchedulesDelegate gSchedulesDelegate(gThermostatEndpoint);

static Clusters::Thermostat::ThermostatSensorsDelegate gSensorsDelegate(gThermostatEndpoint);

using ThermostatClusterType = Clusters::Thermostat::ThermostatCluster<
    Clusters::Thermostat::ThermostatDelegate, Clusters::Thermostat::ThermostatSetpointsDelegate,
    Clusters::Thermostat::ThermostatHoldDelegate, Clusters::Thermostat::ThermostatPresetsDelegate,
    Clusters::Thermostat::ThermostatSuggestionsDelegate, Clusters::Thermostat::ThermostatSchedulesDelegate,
    Clusters::Thermostat::ThermostatSensorsDelegate>;

namespace {

constexpr char kTemperatureFile[] = "/tmp/thermostat-temperature";

void UpdateSimulatedTemperature(chip::System::Layer * layer, void *)
{
    std::ifstream file(kTemperatureFile);
    int temperature = 0;

    if (file >> temperature)
    {
        bool changed = false;

        auto status = gThermostatDelegate.SetLocalTemperature(
            chip::app::DataModel::MakeNullable<int16_t>(static_cast<int16_t>(temperature)), changed);

        if (status == chip::Protocols::InteractionModel::Status::Success && changed)
        {
            ::MatterReportingAttributeChangeCallback(gThermostatEndpoint, chip::app::Clusters::Thermostat::Id,
                                                     chip::app::Clusters::Thermostat::Attributes::LocalTemperature::Id);

            ChipLogProgress(Zcl, "Simulated temperature updated: %d", temperature);
        }
    }

    layer->StartTimer(chip::System::Clock::Seconds32(1), UpdateSimulatedTemperature, nullptr);
}
} // namespace

void ApplicationInit()
{
    ChipLogProgress(Zcl, "Thermostat application init");

    Clusters::Thermostat::ServerInit<ThermostatClusterType>(gThermostatEndpoint, gThermostatDelegate, gSetpointsDelegate,
                                                            gHoldDelegate, gPresetsDelegate, gSuggestionsDelegate,
                                                            gSchedulesDelegate, gSensorsDelegate);

    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(1), UpdateSimulatedTemperature, nullptr);
}

void ApplicationShutdown()
{
    chip::app::Clusters::Thermostat::ServerShutdown<ThermostatClusterType>(gThermostatEndpoint,
                                                                           MatterClusterShutdownType::kClusterShutdown);
}

int main(int argc, char * argv[])
{
    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

    ChipLinuxAppMainLoop();

    return 0;
}
