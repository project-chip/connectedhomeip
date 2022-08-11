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

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-server.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <platform/PlatformManager.h>

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
    if (ConfigurationMgr().StoreBootReason(static_cast<uint32_t>(bootReason)) != CHIP_NO_ERROR)
    {
        Server::GetInstance().DispatchShutDownAndStopEventLoop();
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

#if CHIP_CONFIG_TEST
        // On Linux Simulation, set following hardware faults statically.
        ReturnOnFailure(previous.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_RADIO));
        ReturnOnFailure(previous.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_POWER_SOURCE));

        ReturnOnFailure(current.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_RADIO));
        ReturnOnFailure(current.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_SENSOR));
        ReturnOnFailure(current.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_POWER_SOURCE));
        ReturnOnFailure(current.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_USER_INTERFACE_FAULT));
#endif
        Clusters::GeneralDiagnosticsServer::Instance().OnHardwareFaultsDetect(previous, current);
    }
    else if (eventId == Clusters::GeneralDiagnostics::Events::RadioFaultChange::Id)
    {
        GeneralFaults<kMaxRadioFaults> previous;
        GeneralFaults<kMaxRadioFaults> current;

#if CHIP_CONFIG_TEST
        // On Linux Simulation, set following radio faults statically.
        ReturnOnFailure(previous.add(EMBER_ZCL_RADIO_FAULT_TYPE_WI_FI_FAULT));
        ReturnOnFailure(previous.add(EMBER_ZCL_RADIO_FAULT_TYPE_THREAD_FAULT));

        ReturnOnFailure(current.add(EMBER_ZCL_RADIO_FAULT_TYPE_WI_FI_FAULT));
        ReturnOnFailure(current.add(EMBER_ZCL_RADIO_FAULT_TYPE_CELLULAR_FAULT));
        ReturnOnFailure(current.add(EMBER_ZCL_RADIO_FAULT_TYPE_THREAD_FAULT));
        ReturnOnFailure(current.add(EMBER_ZCL_RADIO_FAULT_TYPE_NFC_FAULT));
#endif
        Clusters::GeneralDiagnosticsServer::Instance().OnRadioFaultsDetect(previous, current);
    }
    else if (eventId == Clusters::GeneralDiagnostics::Events::NetworkFaultChange::Id)
    {
        GeneralFaults<kMaxNetworkFaults> previous;
        GeneralFaults<kMaxNetworkFaults> current;

#if CHIP_CONFIG_TEST
        // On Linux Simulation, set following radio faults statically.
        ReturnOnFailure(previous.add(EMBER_ZCL_NETWORK_FAULT_TYPE_HARDWARE_FAILURE));
        ReturnOnFailure(previous.add(EMBER_ZCL_NETWORK_FAULT_TYPE_NETWORK_JAMMED));

        ReturnOnFailure(current.add(EMBER_ZCL_NETWORK_FAULT_TYPE_HARDWARE_FAILURE));
        ReturnOnFailure(current.add(EMBER_ZCL_NETWORK_FAULT_TYPE_NETWORK_JAMMED));
        ReturnOnFailure(current.add(EMBER_ZCL_NETWORK_FAULT_TYPE_CONNECTION_FAILED));
#endif
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
    char * asctime     = std::asctime(std::localtime(&result));
    softwareFault.faultRecording.SetValue(ByteSpan(Uint8::from_const_char(asctime), strlen(asctime)));

    Clusters::SoftwareDiagnosticsServer::Instance().OnSoftwareFaultDetect(softwareFault);
}

void AllClustersAppCommandHandler::OnSwitchLatchedHandler(uint8_t newPosition)
{
    EndpointId endpoint = 1;

    EmberAfStatus status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The latching switch is moved to a new position:%d", newPosition);

    Clusters::SwitchServer::Instance().OnSwitchLatch(endpoint, newPosition);
}

void AllClustersAppCommandHandler::OnSwitchInitialPressedHandler(uint8_t newPosition)
{
    EndpointId endpoint = 1;

    EmberAfStatus status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The new position when the momentary switch starts to be pressed:%d", newPosition);

    Clusters::SwitchServer::Instance().OnInitialPress(endpoint, newPosition);
}

void AllClustersAppCommandHandler::OnSwitchLongPressedHandler(uint8_t newPosition)
{
    EndpointId endpoint = 1;

    EmberAfStatus status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The new position when the momentary switch has been pressed for a long time:%d", newPosition);

    Clusters::SwitchServer::Instance().OnLongPress(endpoint, newPosition);
}

void AllClustersAppCommandHandler::OnSwitchShortReleasedHandler(uint8_t previousPosition)
{
    EndpointId endpoint = 1;

    EmberAfStatus status = Switch::Attributes::CurrentPosition::Set(endpoint, 0);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to reset CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The the previous value of the CurrentPosition when the momentary switch has been released:%d",
                  previousPosition);

    Clusters::SwitchServer::Instance().OnShortRelease(endpoint, previousPosition);
}

void AllClustersAppCommandHandler::OnSwitchLongReleasedHandler(uint8_t previousPosition)
{
    EndpointId endpoint = 1;

    EmberAfStatus status = Switch::Attributes::CurrentPosition::Set(endpoint, 0);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to reset CurrentPosition attribute"));
    ChipLogDetail(NotSpecified,
                  "The the previous value of the CurrentPosition when the momentary switch has been released after having been "
                  "pressed for a long time:%d",
                  previousPosition);

    Clusters::SwitchServer::Instance().OnLongRelease(endpoint, previousPosition);
}

void AllClustersAppCommandHandler::OnSwitchMultiPressOngoingHandler(uint8_t newPosition, uint8_t count)
{
    EndpointId endpoint = 1;

    EmberAfStatus status = Switch::Attributes::CurrentPosition::Set(endpoint, newPosition);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to set CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The new position when the momentary switch has been pressed in a multi-press sequence:%d",
                  newPosition);
    ChipLogDetail(NotSpecified, "%d times the momentary switch has been pressed", count);

    Clusters::SwitchServer::Instance().OnMultiPressOngoing(endpoint, newPosition, count);
}

void AllClustersAppCommandHandler::OnSwitchMultiPressCompleteHandler(uint8_t previousPosition, uint8_t count)
{
    EndpointId endpoint = 1;

    EmberAfStatus status = Switch::Attributes::CurrentPosition::Set(endpoint, 0);
    VerifyOrReturn(EMBER_ZCL_STATUS_SUCCESS == status, ChipLogError(NotSpecified, "Failed to reset CurrentPosition attribute"));
    ChipLogDetail(NotSpecified, "The previous position when the momentary switch has been pressed in a multi-press sequence:%d",
                  previousPosition);
    ChipLogDetail(NotSpecified, "%d times the momentary switch has been pressed", count);

    Clusters::SwitchServer::Instance().OnMultiPressComplete(endpoint, previousPosition, count);
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
