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

#include "event-command-handler.h"
#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-server.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

namespace {

bool IsClusterPresentOnAnyEndpoint(ClusterId clusterId)
{
    bool retval = false;

    for (auto endpointId : EnabledEndpointsWithServerCluster(clusterId))
    {
        IgnoreUnusedVariable(endpointId);
        retval = true;
    }

    return retval;
}

/**
 * Should be called when a software fault takes place on the Node.
 */
void HandleSoftwareFaultEvent(uint32_t eventId)
{
    VerifyOrReturn(eventId == Clusters::SoftwareDiagnostics::Events::SoftwareFault::Id,
                   ChipLogError(DeviceLayer, "Unknown software fault event received"));

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

/**
 * Should be called when a general fault takes place on the Node.
 */
void HandleGeneralFaultEvent(uint32_t eventId)
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
        ChipLogError(DeviceLayer, "Unknow event ID:%d", eventId);
    }
}

/**
 * Should be called when a switch operation takes place on the Node.
 */
void HandleSwitchEvent(uint32_t eventId)
{
    EndpointId endpoint      = 1;
    uint8_t newPosition      = 20;
    uint8_t previousPosition = 10;
    uint8_t count            = 3;

    if (eventId == Clusters::Switch::Events::SwitchLatched::Id)
    {
        Clusters::SwitchServer::Instance().OnSwitchLatch(endpoint, newPosition);
    }
    else if (eventId == Clusters::Switch::Events::InitialPress::Id)
    {
        Clusters::SwitchServer::Instance().OnInitialPress(endpoint, newPosition);
    }
    else if (eventId == Clusters::Switch::Events::LongPress::Id)
    {
        Clusters::SwitchServer::Instance().OnLongPress(endpoint, newPosition);
    }
    else if (eventId == Clusters::Switch::Events::ShortRelease::Id)
    {
        Clusters::SwitchServer::Instance().OnShortRelease(endpoint, previousPosition);
    }
    else if (eventId == Clusters::Switch::Events::LongRelease::Id)
    {
        Clusters::SwitchServer::Instance().OnLongRelease(endpoint, previousPosition);
    }
    else if (eventId == Clusters::Switch::Events::MultiPressOngoing::Id)
    {
        Clusters::SwitchServer::Instance().OnMultiPressOngoing(endpoint, newPosition, count);
    }
    else if (eventId == Clusters::Switch::Events::MultiPressComplete::Id)
    {
        Clusters::SwitchServer::Instance().OnMultiPressComplete(endpoint, newPosition, count);
    }
    else
    {
        ChipLogError(DeviceLayer, "Unknow event ID:%d", eventId);
    }
}

void HandleRebootSignal(BootReasonType bootReason)
{
    if (ConfigurationMgr().StoreBootReason(static_cast<uint32_t>(bootReason)) != CHIP_NO_ERROR)
    {
        Server::GetInstance().DispatchShutDownAndStopEventLoop();
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to store boot reason:%d", static_cast<uint32_t>(bootReason));
    }
}

} // namespace

void ProcessCommandRequest(const char * payload)
{
    if (strncmp("SoftwareFault", payload, strlen("SoftwareFault")) == 0)
    {
        HandleSoftwareFaultEvent(Clusters::SoftwareDiagnostics::Events::SoftwareFault::Id);
    }
    else if (strncmp("HardwareFaultChange", payload, strlen("HardwareFaultChange")) == 0)
    {
        HandleGeneralFaultEvent(Clusters::GeneralDiagnostics::Events::HardwareFaultChange::Id);
    }
    else if (strncmp("RadioFaultChange", payload, strlen("RadioFaultChange")) == 0)
    {
        HandleGeneralFaultEvent(Clusters::GeneralDiagnostics::Events::RadioFaultChange::Id);
    }
    else if (strncmp("NetworkFaultChange", payload, strlen("NetworkFaultChange")) == 0)
    {
        HandleGeneralFaultEvent(Clusters::GeneralDiagnostics::Events::NetworkFaultChange::Id);
    }
    else if (strncmp("SwitchLatched", payload, strlen("SwitchLatched")) == 0)
    {
        HandleSwitchEvent(Clusters::Switch::Events::SwitchLatched::Id);
    }
    else if (strncmp("InitialPress", payload, strlen("InitialPress")) == 0)
    {
        HandleSwitchEvent(Clusters::Switch::Events::InitialPress::Id);
    }
    else if (strncmp("LongPress", payload, strlen("LongPress")) == 0)
    {
        HandleSwitchEvent(Clusters::Switch::Events::LongPress::Id);
    }
    else if (strncmp("ShortRelease", payload, strlen("ShortRelease")) == 0)
    {
        HandleSwitchEvent(Clusters::Switch::Events::ShortRelease::Id);
    }
    else if (strncmp("LongRelease", payload, strlen("LongRelease")) == 0)
    {
        HandleSwitchEvent(Clusters::Switch::Events::LongRelease::Id);
    }
    else if (strncmp("MultiPressOngoing", payload, strlen("MultiPressOngoing")) == 0)
    {
        HandleSwitchEvent(Clusters::Switch::Events::MultiPressOngoing::Id);
    }
    else if (strncmp("MultiPressComplete", payload, strlen("MultiPressComplete")) == 0)
    {
        HandleSwitchEvent(Clusters::Switch::Events::MultiPressComplete::Id);
    }
    else if (strncmp("PowerOnReboot", payload, strlen("PowerOnReboot")) == 0)
    {
        HandleRebootSignal(BootReasonType::kPowerOnReboot);
    }
    else if (strncmp("BrownOutReset", payload, strlen("BrownOutReset")) == 0)
    {
        HandleRebootSignal(BootReasonType::kBrownOutReset);
    }
    else if (strncmp("SoftwareWatchdogReset", payload, strlen("SoftwareWatchdogReset")) == 0)
    {
        HandleRebootSignal(BootReasonType::kSoftwareWatchdogReset);
    }
    else if (strncmp("HardwareWatchdogReset", payload, strlen("HardwareWatchdogReset")) == 0)
    {
        HandleRebootSignal(BootReasonType::kHardwareWatchdogReset);
    }
    else if (strncmp("SoftwareUpdateCompleted", payload, strlen("SoftwareUpdateCompleted")) == 0)
    {
        HandleRebootSignal(BootReasonType::kSoftwareUpdateCompleted);
    }
    else if (strncmp("SoftwareReset", payload, strlen("SoftwareReset")) == 0)
    {
        HandleRebootSignal(BootReasonType::kSoftwareReset);
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command: Should never happens");
    }
}
