/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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

#include "CPAppCommandDelegate.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/EventLogging.h>
#include <app/clusters/general-diagnostics-server/CodegenIntegration.h>
#include <app/clusters/software-diagnostics-server/software-fault-listener.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

CPAppCommandHandler * CPAppCommandHandler::FromJSON(const char * json)
{
    ChipLogProgress(NotSpecified, "===SHM %s()", __func__);
    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(json, value))
    {
        ChipLogError(NotSpecified,
                     "CommissioningProxy App: Error parsing JSON with error %s:", reader.getFormattedErrorMessages().c_str());
        return nullptr;
    }

    if (value.empty() || !value.isObject())
    {
        ChipLogError(NotSpecified, "CommissioningProxy App: Invalid JSON command received");
        return nullptr;
    }

    if (!value.isMember("Name") || !value["Name"].isString())
    {
        ChipLogError(NotSpecified, "CommissioningProxy App: Invalid JSON command received: command name is missing");
        return nullptr;
    }

    return Platform::New<CPAppCommandHandler>(std::move(value));
}

void CPAppCommandHandler::HandleCommand(intptr_t context)
{
    ChipLogProgress(NotSpecified, "===SHM %s()", __func__);
    auto * self      = reinterpret_cast<CPAppCommandHandler *>(context);
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
    else if (name == "UserIntentCommissioningStart")
    {
        TEMPORARY_RETURN_IGNORED Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    }
    else
    {
        ChipLogError(NotSpecified, "Unhandled command '%s': this should never happen", name.c_str());
        VerifyOrDie(false && "Named pipe command not supported, see log above.");
    }

exit:
    Platform::Delete(self);
}

bool CPAppCommandHandler::IsClusterPresentOnAnyEndpoint(ClusterId clusterId)
{
    ChipLogProgress(NotSpecified, "===SHM %s()", __func__);
    EnabledEndpointsWithServerCluster enabledEndpoints(clusterId);

    return (enabledEndpoints.begin() != enabledEndpoints.end());
}

void CPAppCommandHandler::OnRebootSignalHandler(BootReasonType bootReason)
{
    ChipLogProgress(NotSpecified, "===SHM %s()", __func__);
    if (ConfigurationMgr().StoreBootReason(static_cast<uint32_t>(bootReason)) == CHIP_NO_ERROR)
    {
        Server::GetInstance().GenerateShutDownEvent();
        TEMPORARY_RETURN_IGNORED PlatformMgr().ScheduleWork(
            [](intptr_t) { TEMPORARY_RETURN_IGNORED PlatformMgr().StopEventLoopTask(); });
    }
    else
    {
        ChipLogError(NotSpecified, "Failed to store boot reason:%d", static_cast<uint32_t>(bootReason));
    }
}

void CPAppCommandHandler::OnGeneralFaultEventHandler(uint32_t eventId)
{
    ChipLogProgress(NotSpecified, "===SHM %s()", __func__);

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
        Clusters::GeneralDiagnostics::GlobalNotifyHardwareFaultsDetect(previous, current);
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
        Clusters::GeneralDiagnostics::GlobalNotifyRadioFaultsDetect(previous, current);
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
        Clusters::GeneralDiagnostics::GlobalNotifyNetworkFaultsDetect(previous, current);
    }
    else
    {
        ChipLogError(NotSpecified, "Unknow event ID:%d", eventId);
    }
}

void CPAppCommandHandler::OnSoftwareFaultEventHandler(uint32_t eventId)
{
    ChipLogProgress(NotSpecified, "===SHM %s()", __func__);

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

    Clusters::SoftwareDiagnostics::SoftwareFaultListener::GlobalNotifySoftwareFaultDetect(softwareFault);
}

void CPAppCommandDelegate::OnEventCommandReceived(const char * json)
{
    ChipLogProgress(NotSpecified, "===SHM %s()", __func__);
    auto handler = CPAppCommandHandler::FromJSON(json);
    if (nullptr == handler)
    {
        ChipLogError(NotSpecified, "CommissioningProxy App: Unable to instantiate a command handler");
        return;
    }

    TEMPORARY_RETURN_IGNORED chip::DeviceLayer::PlatformMgr().ScheduleWork(CPAppCommandHandler::HandleCommand,
                                                                           reinterpret_cast<intptr_t>(handler));
}
