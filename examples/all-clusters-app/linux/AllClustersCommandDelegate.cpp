/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "AllClustersCommandDelegate.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/EventLogging.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/occupancy-sensor-server/occupancy-sensor-server.h>
#include <app/clusters/smoke-co-alarm-server/smoke-co-alarm-server.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-server.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <platform/PlatformManager.h>

#include "ButtonEventsSimulator.h"
#include <air-quality-instance.h>
#include <dishwasher-mode.h>
#include <laundry-washer-mode.h>
#include <operational-state-delegate-impl.h>
#include <oven-modes.h>
#include <oven-operational-state-delegate.h>
#include <rvc-modes.h>

#include <memory>
#include <string>
#include <utility>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

namespace {

std::unique_ptr<ButtonEventsSimulator> sButtonSimulatorInstance{ nullptr };

bool HasNumericField(Json::Value & jsonValue, const std::string & field)
{
    return jsonValue.isMember(field) && jsonValue[field].isNumeric();
}

uint8_t GetNumberOfSwitchPositions(EndpointId endpointId)
{
    // TODO: Move to using public API of cluster.
    uint8_t numPositions = 0;

    // On failure, the numPositions won't be changed, so 0 returned.
    (void) Switch::Attributes::NumberOfPositions::Get(endpointId, &numPositions);

    return numPositions;
}

/**
 * Named pipe handler for simulated long press
 *
 * Usage example:
 *   echo '{"Name": "SimulateLongPress", "EndpointId": 3, "ButtonId": 1, "LongPressDelayMillis": 800,
 * "LongPressDurationMillis": 1000}' > /tmp/chip_all_clusters_fifo_1146610
 *
 * JSON Arguments:
 *   - "Name": Must be "SimulateLongPress"
 *   - "EndpointId": ID of endpoint having a switch cluster
 *   - "ButtonId": switch position in the switch cluster for "down" button (not idle)
 *   - "LongPressDelayMillis": Time in milliseconds before the LongPress
 *   - "LongPressDurationMillis": Total duration in milliseconds from start of the press to LongRelease
 *   - "FeatureMap": The feature map to simulate
 *
 * @param jsonValue - JSON payload from named pipe
 */
void HandleSimulateLongPress(Json::Value & jsonValue)
{
    if (sButtonSimulatorInstance != nullptr)
    {
        ChipLogError(NotSpecified, "Button simulation already in progress! Ignoring request.");
        return;
    }

    bool hasEndpointId              = HasNumericField(jsonValue, "EndpointId");
    bool hasButtonId                = HasNumericField(jsonValue, "ButtonId");
    bool hasLongPressDelayMillis    = HasNumericField(jsonValue, "LongPressDelayMillis");
    bool hasLongPressDurationMillis = HasNumericField(jsonValue, "LongPressDurationMillis");
    bool hasFeatureMap              = HasNumericField(jsonValue, "FeatureMap");
    if (!hasEndpointId || !hasButtonId || !hasLongPressDelayMillis || !hasLongPressDurationMillis || !hasFeatureMap)
    {
        std::string inputJson = jsonValue.toStyledString();
        ChipLogError(NotSpecified,
                     "Missing or invalid value for one of EndpointId, ButtonId, LongPressDelayMillis, LongPressDurationMillis or "
                     "FeatureMap in %s",
                     inputJson.c_str());
        return;
    }

    EndpointId endpointId = static_cast<EndpointId>(jsonValue["EndpointId"].asUInt());
    uint8_t buttonId      = static_cast<uint8_t>(jsonValue["ButtonId"].asUInt());

    uint8_t numPositions = GetNumberOfSwitchPositions(endpointId);
    if (buttonId >= numPositions)
    {
        std::string inputJson = jsonValue.toStyledString();
        ChipLogError(NotSpecified, "Invalid ButtonId (out of range) in %s", inputJson.c_str());
        return;
    }

    System::Clock::Milliseconds32 longPressDelayMillis{ static_cast<unsigned>(jsonValue["LongPressDelayMillis"].asUInt()) };
    System::Clock::Milliseconds32 longPressDurationMillis{ static_cast<unsigned>(jsonValue["LongPressDurationMillis"].asUInt()) };
    uint32_t featureMap  = static_cast<uint32_t>(jsonValue["FeatureMap"].asUInt());
    auto buttonSimulator = std::make_unique<ButtonEventsSimulator>();

    bool success = buttonSimulator->SetMode(ButtonEventsSimulator::Mode::kModeLongPress)
                       .SetLongPressDelayMillis(longPressDelayMillis)
                       .SetLongPressDurationMillis(longPressDurationMillis)
                       .SetIdleButtonId(0)
                       .SetPressedButtonId(buttonId)
                       .SetEndpointId(endpointId)
                       .SetFeatureMap(featureMap)
                       .Execute([]() { sButtonSimulatorInstance.reset(); });

    if (!success)
    {
        ChipLogError(NotSpecified, "Failed to start execution of button simulator!");
        return;
    }

    sButtonSimulatorInstance = std::move(buttonSimulator);
}

/**
 * Named pipe handler for simulated multi-press.
 *
 * Usage example:
 *   echo '{"Name": "SimulateMultiPress", "EndpointId": 3, "ButtonId": 1, "MultiPressPressedTimeMillis": 100,
 * "MultiPressReleasedTimeMillis": 350, "MultiPressNumPresses": 2, "FeatureMap": 58}' > /tmp/chip_all_clusters_fifo_1146610
 *
 * JSON Arguments:
 *   - "Name": Must be "SimulateActionSwitchMultiPress"
 *   - "EndpointId": ID of endpoint having a switch cluster
 *   - "ButtonId": switch position in the switch cluster for "down" button (not idle)
 *   - "MultiPressPressedTimeMillis": Pressed time in milliseconds for each press
 *   - "MultiPressReleasedTimeMillis": Released time in milliseconds after each press
 *   - "MultiPressNumPresses": Number of presses to simulate
 *   - "FeatureMap": The feature map to simulate
 *   - "MultiPressMax": max number of presses (from attribute).
 *
 * @param jsonValue - JSON payload from named pipe
 */
void HandleSimulateMultiPress(Json::Value & jsonValue)
{
    if (sButtonSimulatorInstance != nullptr)
    {
        ChipLogError(NotSpecified, "Button simulation already in progress! Ignoring request.");
        return;
    }

    bool hasEndpointId                   = HasNumericField(jsonValue, "EndpointId");
    bool hasButtonId                     = HasNumericField(jsonValue, "ButtonId");
    bool hasMultiPressPressedTimeMillis  = HasNumericField(jsonValue, "MultiPressPressedTimeMillis");
    bool hasMultiPressReleasedTimeMillis = HasNumericField(jsonValue, "MultiPressReleasedTimeMillis");
    bool hasMultiPressNumPresses         = HasNumericField(jsonValue, "MultiPressNumPresses");
    bool hasFeatureMap                   = HasNumericField(jsonValue, "FeatureMap");
    bool hasMultiPressMax                = HasNumericField(jsonValue, "MultiPressMax");
    if (!hasEndpointId || !hasButtonId || !hasMultiPressPressedTimeMillis || !hasMultiPressReleasedTimeMillis ||
        !hasMultiPressNumPresses || !hasFeatureMap || !hasMultiPressMax)
    {
        std::string inputJson = jsonValue.toStyledString();
        ChipLogError(NotSpecified,
                     "Missing or invalid value for one of EndpointId, ButtonId, MultiPressPressedTimeMillis, "
                     "MultiPressReleasedTimeMillis, MultiPressNumPresses, FeatureMap or MultiPressMax in %s",
                     inputJson.c_str());
        return;
    }

    EndpointId endpointId = static_cast<EndpointId>(jsonValue["EndpointId"].asUInt());
    uint8_t buttonId      = static_cast<uint8_t>(jsonValue["ButtonId"].asUInt());

    uint8_t numPositions = GetNumberOfSwitchPositions(endpointId);
    if (buttonId >= numPositions)
    {
        std::string inputJson = jsonValue.toStyledString();
        ChipLogError(NotSpecified, "Invalid ButtonId (out of range) in %s", inputJson.c_str());
        return;
    }

    System::Clock::Milliseconds32 multiPressPressedTimeMillis{ static_cast<unsigned>(
        jsonValue["MultiPressPressedTimeMillis"].asUInt()) };
    System::Clock::Milliseconds32 multiPressReleasedTimeMillis{ static_cast<unsigned>(
        jsonValue["MultiPressReleasedTimeMillis"].asUInt()) };
    uint8_t multiPressNumPresses = static_cast<uint8_t>(jsonValue["MultiPressNumPresses"].asUInt());
    uint32_t featureMap          = static_cast<uint32_t>(jsonValue["FeatureMap"].asUInt());
    uint8_t multiPressMax        = static_cast<uint8_t>(jsonValue["MultiPressMax"].asUInt());
    auto buttonSimulator         = std::make_unique<ButtonEventsSimulator>();

    bool success = buttonSimulator->SetMode(ButtonEventsSimulator::Mode::kModeMultiPress)
                       .SetMultiPressPressedTimeMillis(multiPressPressedTimeMillis)
                       .SetMultiPressReleasedTimeMillis(multiPressReleasedTimeMillis)
                       .SetMultiPressNumPresses(multiPressNumPresses)
                       .SetIdleButtonId(0)
                       .SetPressedButtonId(buttonId)
                       .SetEndpointId(endpointId)
                       .SetFeatureMap(featureMap)
                       .SetMultiPressMax(multiPressMax)
                       .Execute([]() { sButtonSimulatorInstance.reset(); });

    if (!success)
    {
        ChipLogError(NotSpecified, "Failed to start execution of button simulator!");
        return;
    }

    sButtonSimulatorInstance = std::move(buttonSimulator);
}

/**
 * Named pipe handler for simulating a latched switch movement.
 *
 * Usage example:
 *   echo '{"Name": "SimulateLatchPosition", "EndpointId": 3, "PositionId": 1}' > /tmp/chip_all_clusters_fifo_1146610
 *
 * JSON Arguments:
 *   - "Name": Must be "SimulateLatchPosition"
 *   - "EndpointId": ID of endpoint having a switch cluster
 *   - "PositionId": switch position for new CurrentPosition to set in switch cluster
 *
 * @param jsonValue - JSON payload from named pipe
 */

void HandleSimulateLatchPosition(Json::Value & jsonValue)
{
    bool hasEndpointId = HasNumericField(jsonValue, "EndpointId");
    bool hasPositionId = HasNumericField(jsonValue, "PositionId");

    if (!hasEndpointId || !hasPositionId)
    {
        std::string inputJson = jsonValue.toStyledString();
        ChipLogError(NotSpecified, "Missing or invalid value for one of EndpointId, PositionId in %s", inputJson.c_str());
        return;
    }

    EndpointId endpointId = static_cast<EndpointId>(jsonValue["EndpointId"].asUInt());
    uint8_t positionId    = static_cast<uint8_t>(jsonValue["PositionId"].asUInt());

    uint8_t numPositions = GetNumberOfSwitchPositions(endpointId);
    if (positionId >= numPositions)
    {
        std::string inputJson = jsonValue.toStyledString();
        ChipLogError(NotSpecified, "Invalid PositionId (out of range) in %s", inputJson.c_str());
        return;
    }

    uint8_t previousPositionId                 = 0;
    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Get(endpointId, &previousPositionId);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to get CurrentPosition attribute"));

    if (positionId != previousPositionId)
    {
        status = Switch::Attributes::CurrentPosition::Set(endpointId, positionId);
        VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                       ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
        ChipLogDetail(NotSpecified, "The latching switch is moved to a new position: %u", static_cast<unsigned>(positionId));

        Clusters::SwitchServer::Instance().OnSwitchLatch(endpointId, positionId);
    }
    else
    {
        ChipLogDetail(NotSpecified, "Not moving latching switch to a new position, already at %u",
                      static_cast<unsigned>(positionId));
    }
}

/**
 * Named pipe handler for simulating switch is idle
 *
 * Usage example:
 *   echo '{"Name": "SimulateSwitchIdle", "EndpointId": 3}' > /tmp/chip_all_clusters_fifo_1146610
 *
 * JSON Arguments:
 *   - "Name": Must be "SimulateSwitchIdle"
 *   - "EndpointId": ID of endpoint having a switch cluster
 *
 * @param jsonValue - JSON payload from named pipe
 */

void HandleSimulateSwitchIdle(Json::Value & jsonValue)
{
    bool hasEndpointId = HasNumericField(jsonValue, "EndpointId");

    if (!hasEndpointId)
    {
        std::string inputJson = jsonValue.toStyledString();
        ChipLogError(NotSpecified, "Missing or invalid value for one of EndpointId in %s", inputJson.c_str());
        return;
    }

    EndpointId endpointId = static_cast<EndpointId>(jsonValue["EndpointId"].asUInt());
    (void) Switch::Attributes::CurrentPosition::Set(endpointId, 0);
}

void EmitOccupancyChangedEvent(EndpointId endpointId, uint8_t occupancyValue)
{
    Clusters::OccupancySensing::Events::OccupancyChanged::Type event{};
    event.occupancy         = static_cast<BitMask<Clusters::OccupancySensing::OccupancyBitmap>>(occupancyValue);
    EventNumber eventNumber = 0;

    CHIP_ERROR err = LogEvent(event, endpointId, eventNumber);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to log OccupancyChanged event: %" CHIP_ERROR_FORMAT, err.Format());
    }
    else
    {
        ChipLogProgress(NotSpecified, "Logged OccupancyChanged(occupancy=%u) on Endpoint %u", static_cast<unsigned>(occupancyValue),
                        static_cast<unsigned>(endpointId));
    }
}

} // namespace

AllClustersAppCommandHandler * AllClustersAppCommandHandler::FromJSON(const char * json)
{
    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(json, value))
    {
        ChipLogError(NotSpecified,
                     "AllClusters App: Error parsing JSON with error %s:", reader.getFormattedErrorMessages().c_str());
        return nullptr;
    }

    if (value.empty() || !value.isObject())
    {
        ChipLogError(NotSpecified, "AllClusters App: Invalid JSON command received");
        return nullptr;
    }

    if (!value.isMember("Name") || !value["Name"].isString())
    {
        ChipLogError(NotSpecified, "AllClusters App: Invalid JSON command received: command name is missing");
        return nullptr;
    }

    return Platform::New<AllClustersAppCommandHandler>(std::move(value));
}

void AllClustersAppCommandHandler::HandleCommand(intptr_t context)
{
    auto * self      = reinterpret_cast<AllClustersAppCommandHandler *>(context);
    std::string name = self->mJsonValue["Name"].asString();

    VerifyOrExit(!self->mJsonValue.empty(), ChipLogError(NotSpecified, "Invalid JSON event command received"));

    if (name == "SoftwareFault")
    {
        self->OnSoftwareFaultEventHandler(Clusters::SoftwareDiagnostics::Events::SoftwareFault::Id);
    }
    else if (name == "HardwareFaultChange")
    {
        self->OnGeneralFaultEventHandler(Clusters::GeneralDiagnostics::Events::HardwareFaultChange::Id);
    }
    else if (name == "RadioFaultChange")
    {
        self->OnGeneralFaultEventHandler(Clusters::GeneralDiagnostics::Events::RadioFaultChange::Id);
    }
    else if (name == "NetworkFaultChange")
    {
        self->OnGeneralFaultEventHandler(Clusters::GeneralDiagnostics::Events::NetworkFaultChange::Id);
    }
    else if (name == "SwitchLatched")
    {
        uint8_t newPosition = static_cast<uint8_t>(self->mJsonValue["NewPosition"].asUInt());
        self->OnSwitchLatchedHandler(newPosition);
    }
    else if (name == "InitialPress")
    {
        uint8_t newPosition = static_cast<uint8_t>(self->mJsonValue["NewPosition"].asUInt());
        self->OnSwitchInitialPressedHandler(newPosition);
    }
    else if (name == "LongPress")
    {
        uint8_t newPosition = static_cast<uint8_t>(self->mJsonValue["NewPosition"].asUInt());
        self->OnSwitchLongPressedHandler(newPosition);
    }
    else if (name == "ShortRelease")
    {
        uint8_t previousPosition = static_cast<uint8_t>(self->mJsonValue["PreviousPosition"].asUInt());
        self->OnSwitchShortReleasedHandler(previousPosition);
    }
    else if (name == "LongRelease")
    {
        uint8_t previousPosition = static_cast<uint8_t>(self->mJsonValue["PreviousPosition"].asUInt());
        self->OnSwitchLongReleasedHandler(previousPosition);
    }
    else if (name == "MultiPressOngoing")
    {
        uint8_t newPosition = static_cast<uint8_t>(self->mJsonValue["NewPosition"].asUInt());
        uint8_t count       = static_cast<uint8_t>(self->mJsonValue["CurrentNumberOfPressesCounted"].asUInt());
        self->OnSwitchMultiPressOngoingHandler(newPosition, count);
    }
    else if (name == "MultiPressComplete")
    {
        uint8_t previousPosition = static_cast<uint8_t>(self->mJsonValue["PreviousPosition"].asUInt());
        uint8_t count            = static_cast<uint8_t>(self->mJsonValue["TotalNumberOfPressesCounted"].asUInt());
        self->OnSwitchMultiPressCompleteHandler(previousPosition, count);
    }
    else if (name == "PowerOnReboot")
    {
        self->OnRebootSignalHandler(BootReasonType::kPowerOnReboot);
    }
    else if (name == "BrownOutReset")
    {
        self->OnRebootSignalHandler(BootReasonType::kBrownOutReset);
    }
    else if (name == "SoftwareWatchdogReset")
    {
        self->OnRebootSignalHandler(BootReasonType::kSoftwareWatchdogReset);
    }
    else if (name == "HardwareWatchdogReset")
    {
        self->OnRebootSignalHandler(BootReasonType::kHardwareWatchdogReset);
    }
    else if (name == "SoftwareUpdateCompleted")
    {
        self->OnRebootSignalHandler(BootReasonType::kSoftwareUpdateCompleted);
    }
    else if (name == "SoftwareReset")
    {
        self->OnRebootSignalHandler(BootReasonType::kSoftwareReset);
    }
    else if (name == "ModeChange")
    {
        using chip::app::DataModel::MakeNullable;
        std::string device   = self->mJsonValue["Device"].asString();
        std::string type     = self->mJsonValue["Type"].asString();
        Json::Value jsonMode = self->mJsonValue["Mode"];
        DataModel::Nullable<uint8_t> mode;
        if (!jsonMode.isNull())
        {
            mode = MakeNullable(static_cast<uint8_t>(jsonMode.asUInt()));
        }
        else
        {
            mode.SetNull();
        }
        self->OnModeChangeHandler(device, type, mode);
    }
    else if (name == "SetAirQuality")
    {
        Json::Value jsonAirQualityEnum = self->mJsonValue["NewValue"];

        if (jsonAirQualityEnum.isNull())
        {
            ChipLogError(NotSpecified, "The SetAirQuality command requires the NewValue key.");
        }
        else
        {
            self->OnAirQualityChange(static_cast<uint32_t>(jsonAirQualityEnum.asUInt()));
        }
    }
    else if (name == "OperationalStateChange")
    {
        std::string device    = self->mJsonValue["Device"].asString();
        std::string operation = self->mJsonValue["Operation"].asString();
        self->OnOperationalStateChange(device, operation, self->mJsonValue["Param"]);
    }
    else if (name == "SimulateLongPress")
    {
        HandleSimulateLongPress(self->mJsonValue);
    }
    else if (name == "SimulateMultiPress")
    {
        HandleSimulateMultiPress(self->mJsonValue);
    }
    else if (name == "SimulateLatchPosition")
    {
        HandleSimulateLatchPosition(self->mJsonValue);
    }
    else if (name == "SimulateSwitchIdle")
    {
        HandleSimulateSwitchIdle(self->mJsonValue);
    }
    else if (name == "SetOccupancy")
    {
        uint8_t occupancy     = static_cast<uint8_t>(self->mJsonValue["Occupancy"].asUInt());
        EndpointId endpointId = static_cast<EndpointId>(self->mJsonValue["EndpointId"].asUInt());

        if (1 == occupancy || 0 == occupancy)
        {
            self->HandleSetOccupancyChange(endpointId, occupancy);
        }
        else
        {
            ChipLogError(NotSpecified, "Invalid Occupancy state to set.");
        }
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command '%s': this hould never happen", name.c_str());
        VerifyOrDie(false && "Named pipe command not supported, see log above.");
    }

exit:
    Platform::Delete(self);
}

bool AllClustersAppCommandHandler::IsClusterPresentOnAnyEndpoint(ClusterId clusterId)
{
    EnabledEndpointsWithServerCluster enabledEndpoints(clusterId);

    return (enabledEndpoints.begin() != enabledEndpoints.end());
}

void AllClustersAppCommandHandler::OnRebootSignalHandler(BootReasonType bootReason)
{
    if (ConfigurationMgr().StoreBootReason(static_cast<uint32_t>(bootReason)) == CHIP_NO_ERROR)
    {
        Server::GetInstance().GenerateShutDownEvent();
        PlatformMgr().ScheduleWork([](intptr_t) { PlatformMgr().StopEventLoopTask(); });
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to store boot reason:%d", static_cast<uint32_t>(bootReason));
    }
}

void AllClustersAppCommandHandler::OnGeneralFaultEventHandler(uint32_t eventId)
{
    if (!IsClusterPresentOnAnyEndpoint(Clusters::GeneralDiagnostics::Id))
        return;

    if (eventId == Clusters::GeneralDiagnostics::Events::HardwareFaultChange::Id)
    {
        GeneralFaults<kMaxHardwareFaults> previous;
        GeneralFaults<kMaxHardwareFaults> current;

        using GeneralDiagnostics::HardwareFaultEnum;

        // On Linux Simulation, set following hardware faults statically.
        ReturnOnFailure(previous.add(to_underlying(HardwareFaultEnum::kRadio)));
        ReturnOnFailure(previous.add(to_underlying(HardwareFaultEnum::kPowerSource)));

        ReturnOnFailure(current.add(to_underlying(HardwareFaultEnum::kRadio)));
        ReturnOnFailure(current.add(to_underlying(HardwareFaultEnum::kSensor)));
        ReturnOnFailure(current.add(to_underlying(HardwareFaultEnum::kPowerSource)));
        ReturnOnFailure(current.add(to_underlying(HardwareFaultEnum::kUserInterfaceFault)));
        Clusters::GeneralDiagnosticsServer::Instance().OnHardwareFaultsDetect(previous, current);
    }
    else if (eventId == Clusters::GeneralDiagnostics::Events::RadioFaultChange::Id)
    {
        GeneralFaults<kMaxRadioFaults> previous;
        GeneralFaults<kMaxRadioFaults> current;

        // On Linux Simulation, set following radio faults statically.
        ReturnOnFailure(previous.add(to_underlying(GeneralDiagnostics::RadioFaultEnum::kWiFiFault)));
        ReturnOnFailure(previous.add(to_underlying(GeneralDiagnostics::RadioFaultEnum::kThreadFault)));

        ReturnOnFailure(current.add(to_underlying(GeneralDiagnostics::RadioFaultEnum::kWiFiFault)));
        ReturnOnFailure(current.add(to_underlying(GeneralDiagnostics::RadioFaultEnum::kCellularFault)));
        ReturnOnFailure(current.add(to_underlying(GeneralDiagnostics::RadioFaultEnum::kThreadFault)));
        ReturnOnFailure(current.add(to_underlying(GeneralDiagnostics::RadioFaultEnum::kNFCFault)));
        Clusters::GeneralDiagnosticsServer::Instance().OnRadioFaultsDetect(previous, current);
    }
    else if (eventId == Clusters::GeneralDiagnostics::Events::NetworkFaultChange::Id)
    {
        GeneralFaults<kMaxNetworkFaults> previous;
        GeneralFaults<kMaxNetworkFaults> current;

        // On Linux Simulation, set following radio faults statically.
        ReturnOnFailure(previous.add(to_underlying(Clusters::GeneralDiagnostics::NetworkFaultEnum::kHardwareFailure)));
        ReturnOnFailure(previous.add(to_underlying(Clusters::GeneralDiagnostics::NetworkFaultEnum::kNetworkJammed)));

        ReturnOnFailure(current.add(to_underlying(Clusters::GeneralDiagnostics::NetworkFaultEnum::kHardwareFailure)));
        ReturnOnFailure(current.add(to_underlying(Clusters::GeneralDiagnostics::NetworkFaultEnum::kNetworkJammed)));
        ReturnOnFailure(current.add(to_underlying(Clusters::GeneralDiagnostics::NetworkFaultEnum::kConnectionFailed)));
        Clusters::GeneralDiagnosticsServer::Instance().OnNetworkFaultsDetect(previous, current);
    }
    else
    {
        ChipLogError(NotSpecified, "Unknow event ID:%d", eventId);
    }
}

void AllClustersAppCommandHandler::OnSoftwareFaultEventHandler(uint32_t eventId)
{
    VerifyOrReturn(eventId == Clusters::SoftwareDiagnostics::Events::SoftwareFault::Id,
                   ChipLogError(NotSpecified, "Unknown software fault event received"));

    if (!IsClusterPresentOnAnyEndpoint(Clusters::SoftwareDiagnostics::Id))
        return;

    Clusters::SoftwareDiagnostics::Events::SoftwareFault::Type softwareFault;
    char threadName[kMaxThreadNameLength + 1];

    softwareFault.id = static_cast<uint64_t>(getpid());
    Platform::CopyString(threadName, std::to_string(softwareFault.id).c_str());

    softwareFault.name.SetValue(CharSpan::fromCharString(threadName));

    std::time_t result = std::time(nullptr);
    // Using size of 50 as it is double the expected 25 characters "Www Mmm dd hh:mm:ss yyyy\n".
    char timeChar[50];
    if (std::strftime(timeChar, sizeof(timeChar), "%c", std::localtime(&result)))
    {
        softwareFault.faultRecording.SetValue(ByteSpan(Uint8::from_const_char(timeChar), strlen(timeChar)));
    }

    Clusters::SoftwareDiagnosticsServer::Instance().OnSoftwareFaultDetect(softwareFault);
}

void AllClustersAppCommandHandler::OnSwitchLatchedHandler(uint8_t newPosition)
{
    EndpointId endpoint = 1;

    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The latching switch is moved to a new position:%d", newPosition);

    Clusters::SwitchServer::Instance().OnSwitchLatch(endpoint, newPosition);
}

void AllClustersAppCommandHandler::OnSwitchInitialPressedHandler(uint8_t newPosition)
{
    EndpointId endpoint = 1;

    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The new position when the momentary switch starts to be pressed:%d", newPosition);

    Clusters::SwitchServer::Instance().OnInitialPress(endpoint, newPosition);
}

void AllClustersAppCommandHandler::OnSwitchLongPressedHandler(uint8_t newPosition)
{
    EndpointId endpoint = 1;

    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The new position when the momentary switch has been pressed for a long time:%d", newPosition);

    Clusters::SwitchServer::Instance().OnLongPress(endpoint, newPosition);

    // Long press to trigger smokeco self-test
    SmokeCoAlarmServer::Instance().RequestSelfTest(endpoint);
}

void AllClustersAppCommandHandler::OnSwitchShortReleasedHandler(uint8_t previousPosition)
{
    EndpointId endpoint = 1;

    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, 0);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to reset CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The the previous value of the CurrentPosition when the momentary switch has been released:%d",
                  previousPosition);

    Clusters::SwitchServer::Instance().OnShortRelease(endpoint, previousPosition);
}

void AllClustersAppCommandHandler::OnSwitchLongReleasedHandler(uint8_t previousPosition)
{
    EndpointId endpoint = 1;

    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, 0);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to reset CurrentPosition attribute"));
    ChipLogDetail(NotSpecified,
                  "The the previous value of the CurrentPosition when the momentary switch has been released after having been "
                  "pressed for a long time:%d",
                  previousPosition);

    Clusters::SwitchServer::Instance().OnLongRelease(endpoint, previousPosition);
}

void AllClustersAppCommandHandler::OnSwitchMultiPressOngoingHandler(uint8_t newPosition, uint8_t count)
{
    EndpointId endpoint = 1;

    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The new position when the momentary switch has been pressed in a multi-press sequence:%d",
                  newPosition);
    ChipLogDetail(NotSpecified, "%d times the momentary switch has been pressed", count);

    Clusters::SwitchServer::Instance().OnMultiPressOngoing(endpoint, newPosition, count);
}

void AllClustersAppCommandHandler::OnSwitchMultiPressCompleteHandler(uint8_t previousPosition, uint8_t count)
{
    EndpointId endpoint = 1;

    Protocols::InteractionModel::Status status = Switch::Attributes::CurrentPosition::Set(endpoint, 0);
    VerifyOrReturn(Protocols::InteractionModel::Status::Success == status,
                   ChipLogError(NotSpecified, "Failed to reset CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The previous position when the momentary switch has been pressed in a multi-press sequence:%d",
                  previousPosition);
    ChipLogDetail(NotSpecified, "%d times the momentary switch has been pressed", count);

    Clusters::SwitchServer::Instance().OnMultiPressComplete(endpoint, previousPosition, count);
}

void AllClustersAppCommandHandler::OnModeChangeHandler(std::string device, std::string type, DataModel::Nullable<uint8_t> mode)
{
    ModeBase::Instance * modeInstance = nullptr;
    if (device == "DishWasher")
    {
        modeInstance = DishwasherMode::Instance();
    }
    else if (device == "LaundryWasher")
    {
        modeInstance = LaundryWasherMode::Instance();
    }
    else if (device == "RvcClean")
    {
        modeInstance = RvcCleanMode::Instance();
    }
    else if (device == "RvcRun")
    {
        modeInstance = RvcRunMode::Instance();
    }
    else
    {
        ChipLogDetail(NotSpecified, "Invalid device type : %s", device.c_str());
        return;
    }

    if (type == "Current")
    {
        if (mode.IsNull())
        {
            ChipLogDetail(NotSpecified, "Invalid value : null");
            return;
        }
        modeInstance->UpdateCurrentMode(mode.Value());
    }
    else if (type == "StartUp")
    {
        modeInstance->UpdateStartUpMode(mode);
    }
    else if (type == "On")
    {
        modeInstance->UpdateOnMode(mode);
    }
    else
    {
        ChipLogDetail(NotSpecified, "Invalid mode type : %s", type.c_str());
        return;
    }
}

void AllClustersAppCommandHandler::OnOperationalStateChange(std::string device, std::string operation, Json::Value param)
{
    if (device == "Generic")
    {
        OnGenericOperationalStateChange(device, operation, param);
    }
    else if (device == "Oven")
    {
        OnOvenOperationalStateChange(device, operation, param);
    }
    else
    {
        ChipLogDetail(NotSpecified, "Invalid device type : %s", device.c_str());
        return;
    }
}

void AllClustersAppCommandHandler::OnGenericOperationalStateChange(std::string device, std::string operation, Json::Value param)
{
    OperationalState::Instance * operationalStateInstance                 = OperationalState::GetOperationalStateInstance();
    OperationalState::OperationalStateDelegate * operationalStateDelegate = OperationalState::GetOperationalStateDelegate();
    OperationalState::GenericOperationalError noError(to_underlying(OperationalState::ErrorStateEnum::kNoError));
    OperationalState::OperationalStateEnum state =
        static_cast<OperationalState::OperationalStateEnum>(operationalStateInstance->GetCurrentOperationalState());
    if (operation == "Start")
    {
        operationalStateDelegate->HandleStartStateCallback(noError);
    }
    else if (operation == "Resume")
    {
        operationalStateDelegate->HandleResumeStateCallback(noError);
    }
    else if (operation == "Pause")
    {
        operationalStateDelegate->HandlePauseStateCallback(noError);
    }
    else if (operation == "Stop" && state == OperationalState::OperationalStateEnum::kRunning)
    {
        operationalStateDelegate->HandleStopStateCallback(noError);
    }
    else if (operation == "OnFault")
    {
        uint8_t event_id = to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation);
        if (!param.isNull())
        {
            event_id = to_underlying(static_cast<OperationalState::ErrorStateEnum>(param.asUInt()));
        }

        OperationalState::GenericOperationalError err(event_id);
        operationalStateInstance->OnOperationalErrorDetected(err);
    }
    else
    {
        ChipLogDetail(NotSpecified, "Invalid operation : %s", operation.c_str());
        return;
    }
}

void AllClustersAppCommandHandler::OnOvenOperationalStateChange(std::string device, std::string operation, Json::Value param)
{
    OperationalState::Instance * operationalStateInstance = OvenCavityOperationalState::GetOperationalStateInstance();
    if (operation == "Start" || operation == "Resume")
    {
        operationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning));
    }
    else if (operation == "Pause")
    {
        operationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused));
    }
    else if (operation == "Stop")
    {
        operationalStateInstance->SetOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped));
    }
    else if (operation == "OnFault")
    {

        uint8_t event_id = to_underlying(OperationalState::ErrorStateEnum::kUnableToCompleteOperation);
        if (!param.isNull())
        {
            event_id = to_underlying(static_cast<OperationalState::ErrorStateEnum>(param.asUInt()));
        }

        OperationalState::GenericOperationalError err(event_id);
        operationalStateInstance->OnOperationalErrorDetected(err);
    }
    else
    {
        ChipLogDetail(NotSpecified, "Invalid operation : %s", operation.c_str());
        return;
    }
}

void AllClustersAppCommandHandler::OnAirQualityChange(uint32_t aNewValue)
{
    AirQuality::Instance * airQualityInstance = AirQuality::GetInstance();
    Protocols::InteractionModel::Status status =
        airQualityInstance->UpdateAirQuality(static_cast<AirQuality::AirQualityEnum>(aNewValue));

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogDetail(NotSpecified, "Invalid value: %u", aNewValue);
    }
}

void AllClustersAppCommandHandler::HandleSetOccupancyChange(EndpointId endpointId, uint8_t newOccupancyValue)
{
    BitMask<chip::app::Clusters::OccupancySensing::OccupancyBitmap> currentOccupancy;
    Protocols::InteractionModel::Status status = OccupancySensing::Attributes::Occupancy::Get(endpointId, &currentOccupancy);

    if (static_cast<BitMask<chip::app::Clusters::OccupancySensing::OccupancyBitmap>>(newOccupancyValue) == currentOccupancy)
    {
        ChipLogDetail(NotSpecified, "Skipping setting occupancy changed due to same value.");
        return;
    }

    status = OccupancySensing::Attributes::Occupancy::Set(endpointId, newOccupancyValue);
    ChipLogDetail(NotSpecified, "Set Occupancy attribute to %u", newOccupancyValue);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogDetail(NotSpecified, "Invalid value/endpoint to set.");
        return;
    }

    EmitOccupancyChangedEvent(endpointId, newOccupancyValue);

    if (1 == newOccupancyValue)
    {
        uint16_t * holdTime = chip::app::Clusters::OccupancySensing::GetHoldTimeForEndpoint(endpointId);
        if (holdTime != nullptr)
        {
            CHIP_ERROR err = chip::DeviceLayer::SystemLayer().StartTimer(
                chip::System::Clock::Seconds16(*holdTime), AllClustersAppCommandHandler::OccupancyPresentTimerHandler,
                reinterpret_cast<void *>(static_cast<uintptr_t>(endpointId)));
            ChipLogDetail(NotSpecified, "Start HoldTime timer");
            if (CHIP_NO_ERROR != err)
            {
                ChipLogError(NotSpecified, "Failed to start HoldTime timer.");
            }
        }
    }
}

void AllClustersAppCommandHandler::OccupancyPresentTimerHandler(System::Layer * systemLayer, void * appState)
{
    EndpointId endpointId = static_cast<EndpointId>(reinterpret_cast<uintptr_t>(appState));
    chip::BitMask<Clusters::OccupancySensing::OccupancyBitmap> currentOccupancy;

    Protocols::InteractionModel::Status status = OccupancySensing::Attributes::Occupancy::Get(endpointId, &currentOccupancy);
    VerifyOrDie(status == Protocols::InteractionModel::Status::Success);

    uint8_t clearValue = 0;
    if (!currentOccupancy.Has(Clusters::OccupancySensing::OccupancyBitmap::kOccupied))
    {
        return;
    }

    status = OccupancySensing::Attributes::Occupancy::Set(endpointId, clearValue);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogDetail(NotSpecified, "Failed to set occupancy state.");
    }
    else
    {
        ChipLogDetail(NotSpecified, "Set Occupancy attribute to clear");
        EmitOccupancyChangedEvent(endpointId, clearValue);
    }
}

void AllClustersCommandDelegate::OnEventCommandReceived(const char * json)
{
    auto handler = AllClustersAppCommandHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "AllClusters App: Unable to instantiate a command handler");
        return;
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(AllClustersAppCommandHandler::HandleCommand, reinterpret_cast<intptr_t>(handler));
}
