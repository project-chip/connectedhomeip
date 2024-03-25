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
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/smoke-co-alarm-server/smoke-co-alarm-server.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-server.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <app/util/att-storage.h>
#include <platform/PlatformManager.h>

#include <air-quality-instance.h>
#include <dishwasher-mode.h>
#include <laundry-washer-mode.h>
#include <operational-state-delegate-impl.h>
#include <oven-modes.h>
#include <oven-operational-state-delegate.h>
#include <rvc-modes.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

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
    else
    {
        ChipLogError(NotSpecified, "Unhandled command: Should never happens");
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
    OperationalState::Instance * operationalStateInstance = nullptr;
    if (device == "Generic")
    {
        operationalStateInstance = OperationalState::GetOperationalStateInstance();
    }
    else if (device == "Oven")
    {
        operationalStateInstance = OvenCavityOperationalState::GetOperationalStateInstance();
    }
    else
    {
        ChipLogDetail(NotSpecified, "Invalid device type : %s", device.c_str());
        return;
    }

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
