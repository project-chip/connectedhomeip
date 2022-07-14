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
using namespace chip::DeviceLayer;

void AllClustersCommandDelegate::OnEventCommandReceived(const char * command)
{
    mCurrentCommand.assign(command);

    DeviceLayer::PlatformMgr().ScheduleWork(HandleEventCommand, reinterpret_cast<intptr_t>(this));
}

void AllClustersCommandDelegate::HandleEventCommand(intptr_t context)
{
    auto * self = reinterpret_cast<AllClustersCommandDelegate *>(context);

    if (self->mCurrentCommand == "SoftwareFault")
    {
        self->OnSoftwareFaultEventHandler(Clusters::SoftwareDiagnostics::Events::SoftwareFault::Id);
    }
    else if (self->mCurrentCommand == "HardwareFaultChange")
    {
        self->OnGeneralFaultEventHandler(Clusters::GeneralDiagnostics::Events::HardwareFaultChange::Id);
    }
    else if (self->mCurrentCommand == "RadioFaultChange")
    {
        self->OnGeneralFaultEventHandler(Clusters::GeneralDiagnostics::Events::RadioFaultChange::Id);
    }
    else if (self->mCurrentCommand == "NetworkFaultChange")
    {
        self->OnGeneralFaultEventHandler(Clusters::GeneralDiagnostics::Events::NetworkFaultChange::Id);
    }
    else if (self->mCurrentCommand == "SwitchLatched")
    {
        self->OnSwitchEventHandler(Clusters::Switch::Events::SwitchLatched::Id);
    }
    else if (self->mCurrentCommand == "InitialPress")
    {
        self->OnSwitchEventHandler(Clusters::Switch::Events::InitialPress::Id);
    }
    else if (self->mCurrentCommand == "LongPress")
    {
        self->OnSwitchEventHandler(Clusters::Switch::Events::LongPress::Id);
    }
    else if (self->mCurrentCommand == "ShortRelease")
    {
        self->OnSwitchEventHandler(Clusters::Switch::Events::ShortRelease::Id);
    }
    else if (self->mCurrentCommand == "LongRelease")
    {
        self->OnSwitchEventHandler(Clusters::Switch::Events::LongRelease::Id);
    }
    else if (self->mCurrentCommand == "MultiPressOngoing")
    {
        self->OnSwitchEventHandler(Clusters::Switch::Events::MultiPressOngoing::Id);
    }
    else if (self->mCurrentCommand == "MultiPressComplete")
    {
        self->OnSwitchEventHandler(Clusters::Switch::Events::MultiPressComplete::Id);
    }
    else if (self->mCurrentCommand == "PowerOnReboot")
    {
        self->OnRebootSignalHandler(BootReasonType::kPowerOnReboot);
    }
    else if (self->mCurrentCommand == "BrownOutReset")
    {
        self->OnRebootSignalHandler(BootReasonType::kBrownOutReset);
    }
    else if (self->mCurrentCommand == "SoftwareWatchdogReset")
    {
        self->OnRebootSignalHandler(BootReasonType::kSoftwareWatchdogReset);
    }
    else if (self->mCurrentCommand == "HardwareWatchdogReset")
    {
        self->OnRebootSignalHandler(BootReasonType::kHardwareWatchdogReset);
    }
    else if (self->mCurrentCommand == "SoftwareUpdateCompleted")
    {
        self->OnRebootSignalHandler(BootReasonType::kSoftwareUpdateCompleted);
    }
    else if (self->mCurrentCommand == "SoftwareReset")
    {
        self->OnRebootSignalHandler(BootReasonType::kSoftwareReset);
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command: Should never happens");
    }
}

bool AllClustersCommandDelegate::IsClusterPresentOnAnyEndpoint(ClusterId clusterId)
{
    EnabledEndpointsWithServerCluster enabledEndpoints(clusterId);

    return (enabledEndpoints.begin() != enabledEndpoints.end());
}

void AllClustersCommandDelegate::OnRebootSignalHandler(BootReasonType bootReason)
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

void AllClustersCommandDelegate::OnGeneralFaultEventHandler(uint32_t eventId)
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

void AllClustersCommandDelegate::OnSoftwareFaultEventHandler(uint32_t eventId)
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

void AllClustersCommandDelegate::OnSwitchEventHandler(uint32_t eventId)
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
        ChipLogError(NotSpecified, "Unknow event ID:%d", eventId);
    }
}
