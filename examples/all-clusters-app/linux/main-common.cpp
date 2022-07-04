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

#include "include/tv-callbacks.h"
#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/CommandHandler.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-server.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <lib/support/CHIPMem.h>
#include <new>
#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>
#include <transport/raw/PeerAddress.h>

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#include <platform/Darwin/NetworkCommissioningDriver.h>
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/Darwin/WiFi/NetworkCommissioningWiFiDriver.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

#if CHIP_DEVICE_LAYER_TARGET_LINUX
#include <platform/Linux/NetworkCommissioningDriver.h>
#endif // CHIP_DEVICE_LAYER_TARGET_LINUX

#include <Options.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

namespace {
static LowPowerManager lowPowerManager;

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
void HandleSoftwareFaultEvent(intptr_t arg)
{
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
void HandleGeneralFaultEvent(intptr_t arg)
{
    uint32_t eventId = static_cast<uint32_t>(arg);

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
void HandleSwitchEvent(intptr_t arg)
{
    uint32_t eventId = static_cast<uint32_t>(arg);

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

// when the shell is enabled, don't intercept signals since it prevents the user from
// using expected commands like CTRL-C to quit the application. (see issue #17845)
// We should stop using signals for those faults, and move to a different notification
// means, like a pipe. (see issue #19114)
#if !defined(ENABLE_CHIP_SHELL)
void OnRebootSignalHandler(int signum)
{
    ChipLogDetail(DeviceLayer, "Caught signal %d", signum);

    // The BootReason attribute SHALL indicate the reason for the Node’s most recent boot, the real usecase
    // for this attribute is embedded system. In Linux simulation, we use different signals to tell the current
    // running process to terminate with different reasons.
    BootReasonType bootReason = BootReasonType::kUnspecified;
    switch (signum)
    {
    case SIGVTALRM:
        bootReason = BootReasonType::kPowerOnReboot;
        break;
    case SIGALRM:
        bootReason = BootReasonType::kBrownOutReset;
        break;
    case SIGILL:
        bootReason = BootReasonType::kSoftwareWatchdogReset;
        break;
    case SIGTRAP:
        bootReason = BootReasonType::kHardwareWatchdogReset;
        break;
    case SIGIO:
        bootReason = BootReasonType::kSoftwareUpdateCompleted;
        break;
    case SIGINT:
        bootReason = BootReasonType::kSoftwareReset;
        break;
    default:
        IgnoreUnusedVariable(bootReason);
        ChipLogError(NotSpecified, "Unhandled signal: Should never happens");
        chipDie();
        break;
    }

    Server::GetInstance().DispatchShutDownAndStopEventLoop();
}

void OnSoftwareFaultSignalHandler(int signum)
{
    ChipLogDetail(DeviceLayer, "Caught signal %d", signum);

    VerifyOrDie(signum == SIGUSR1);
    PlatformMgr().ScheduleWork(HandleSoftwareFaultEvent);
}

void OnGeneralFaultSignalHandler(int signum)
{
    ChipLogDetail(DeviceLayer, "Caught signal %d", signum);

    uint32_t eventId;
    switch (signum)
    {
    case SIGUSR2:
        eventId = Clusters::GeneralDiagnostics::Events::HardwareFaultChange::Id;
        break;
    case SIGHUP:
        eventId = Clusters::GeneralDiagnostics::Events::RadioFaultChange::Id;
        break;
    case SIGTTIN:
        eventId = Clusters::GeneralDiagnostics::Events::NetworkFaultChange::Id;
        break;
    default:
        ChipLogError(NotSpecified, "Unhandled signal: Should never happens");
        chipDie();
        break;
    }

    PlatformMgr().ScheduleWork(HandleGeneralFaultEvent, static_cast<intptr_t>(eventId));
}

void OnSwitchSignalHandler(int signum)
{
    ChipLogDetail(DeviceLayer, "Caught signal %d", signum);

    uint32_t eventId;
    switch (signum)
    {
    case SIGTSTP:
        eventId = Clusters::Switch::Events::SwitchLatched::Id;
        break;
    case SIGSTOP:
        eventId = Clusters::Switch::Events::InitialPress::Id;
        break;
    case SIGTTOU:
        eventId = Clusters::Switch::Events::LongPress::Id;
        break;
    case SIGWINCH:
        eventId = Clusters::Switch::Events::ShortRelease::Id;
        break;
    case SIGQUIT:
        eventId = Clusters::Switch::Events::LongRelease::Id;
        break;
    case SIGFPE:
        eventId = Clusters::Switch::Events::MultiPressOngoing::Id;
        break;
    case SIGPIPE:
        eventId = Clusters::Switch::Events::MultiPressComplete::Id;
        break;
    default:
        ChipLogError(NotSpecified, "Unhandled signal: Should never happens");
        chipDie();
        break;
    }

    PlatformMgr().ScheduleWork(HandleSwitchEvent, static_cast<intptr_t>(eventId));
}
void SetupSignalHandlers()
{
    // sigaction is not used here because Tsan interceptors seems to
    // never dispatch the signals on darwin.
    signal(SIGALRM, OnRebootSignalHandler);
    signal(SIGVTALRM, OnRebootSignalHandler);
    signal(SIGILL, OnRebootSignalHandler);
    signal(SIGTRAP, OnRebootSignalHandler);
    signal(SIGTERM, OnRebootSignalHandler);
    signal(SIGIO, OnRebootSignalHandler);
    signal(SIGINT, OnRebootSignalHandler);
    signal(SIGUSR1, OnSoftwareFaultSignalHandler);
    signal(SIGUSR2, OnGeneralFaultSignalHandler);
    signal(SIGHUP, OnGeneralFaultSignalHandler);
    signal(SIGTTIN, OnGeneralFaultSignalHandler);
    signal(SIGTSTP, OnSwitchSignalHandler);
    signal(SIGSTOP, OnSwitchSignalHandler);
    signal(SIGTTOU, OnSwitchSignalHandler);
    signal(SIGWINCH, OnSwitchSignalHandler);
    signal(SIGQUIT, OnSwitchSignalHandler);
    signal(SIGFPE, OnSwitchSignalHandler);
    signal(SIGPIPE, OnSwitchSignalHandler);
}
#endif // !defined(ENABLE_CHIP_SHELL)

} // namespace

bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::CommandHandler * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

void OnIdentifyStart(::Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStart");
}

void OnIdentifyStop(::Identify *)
{
    ChipLogProgress(Zcl, "OnIdentifyStop");
}

void OnTriggerEffect(::Identify * identify)
{
    switch (identify->mCurrentEffectIdentifier)
    {
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BLINK");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_BREATHE");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_OKAY");
        break;
    case EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE:
        ChipLogProgress(Zcl, "EMBER_ZCL_IDENTIFY_EFFECT_IDENTIFIER_CHANNEL_CHANGE");
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }
}

static Identify gIdentify0 = {
    chip::EndpointId{ 0 }, OnIdentifyStart, OnIdentifyStop, EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED, OnTriggerEffect,
};

static Identify gIdentify1 = {
    chip::EndpointId{ 1 }, OnIdentifyStart, OnIdentifyStop, EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED, OnTriggerEffect,
};

// Network commissioning
namespace {
// This file is being used by platforms other than Linux, so we need this check to disable related features since we only
// implemented them on linux.
constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

#if CHIP_DEVICE_LAYER_TARGET_LINUX
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
NetworkCommissioning::LinuxThreadDriver sThreadDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
NetworkCommissioning::LinuxWiFiDriver sWiFiDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

NetworkCommissioning::LinuxEthernetDriver sEthernetDriver;
#endif // CHIP_DEVICE_LAYER_TARGET_LINUX

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
NetworkCommissioning::DarwinWiFiDriver sWiFiDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

NetworkCommissioning::DarwinEthernetDriver sEthernetDriver;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
Clusters::NetworkCommissioning::Instance sThreadNetworkCommissioningInstance(kNetworkCommissioningEndpointMain, &sThreadDriver);
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
Clusters::NetworkCommissioning::Instance sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointSecondary, &sWiFiDriver);
#endif

Clusters::NetworkCommissioning::Instance sEthernetNetworkCommissioningInstance(kNetworkCommissioningEndpointMain, &sEthernetDriver);
} // namespace

void ApplicationInit()
{
#if !defined(ENABLE_CHIP_SHELL)
    SetupSignalHandlers();
#endif // !defined(ENABLE_CHIP_SHELL)

    (void) kNetworkCommissioningEndpointMain;
    // Enable secondary endpoint only when we need it, this should be applied to all platforms.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);

    const bool kThreadEnabled = {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        LinuxDeviceOptions::GetInstance().mThread
#else
        false
#endif
    };

    const bool kWiFiEnabled = {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        LinuxDeviceOptions::GetInstance().mWiFi
#else
        false
#endif
    };

    if (kThreadEnabled && kWiFiEnabled)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        sThreadNetworkCommissioningInstance.Init();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        sWiFiNetworkCommissioningInstance.Init();
#endif
        // Only enable secondary endpoint for network commissioning cluster when both WiFi and Thread are enabled.
        emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, true);
    }
    else if (kThreadEnabled)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        sThreadNetworkCommissioningInstance.Init();
#endif
    }
    else if (kWiFiEnabled)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        // If we only enable WiFi on this device, "move" WiFi instance to main NetworkCommissioning cluster endpoint.
        sWiFiNetworkCommissioningInstance.~Instance();
        new (&sWiFiNetworkCommissioningInstance)
            Clusters::NetworkCommissioning::Instance(kNetworkCommissioningEndpointMain, &sWiFiDriver);
        sWiFiNetworkCommissioningInstance.Init();
#endif
    }
    else
    {
        sEthernetNetworkCommissioningInstance.Init();
    }
}

void emberAfLowPowerClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: LowPower::SetDefaultDelegate");
    chip::app::Clusters::LowPower::SetDefaultDelegate(endpoint, &lowPowerManager);
}
