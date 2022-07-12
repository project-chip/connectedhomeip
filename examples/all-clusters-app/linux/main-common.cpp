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

#include "main-common.h"
#include "event-command-handler.h"
#include "include/tv-callbacks.h"
#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/CommandHandler.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <lib/support/CHIPMem.h>
#include <new>
#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
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

static constexpr const size_t kChipEventCmdBufSize     = 80;
static constexpr const char kChipEventFifoPathPrefix[] = "/tmp/chip_all_cluster_fifo_";

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

namespace {
static LowPowerManager lowPowerManager;
pthread_t gChipEventCommandListener;
std::string gChipEventFifoPath;

void * EventCommandListenerTask(void * arg)
{
    char readbuf[kChipEventCmdBufSize];

    for (;;)
    {
        int fd = open(gChipEventFifoPath.c_str(), O_RDONLY);
        if (fd == -1)
        {
            ChipLogError(Zcl, "Failed to open Event FIFO");
            break;
        }

        ssize_t readBytes  = read(fd, readbuf, kChipEventCmdBufSize);
        readbuf[readBytes] = '\0';
        ChipLogProgress(Zcl, "Received payload: \"%s\" and length is %ld\n", readbuf, readBytes);

        // Process the received command request from event fifo
        ProcessCommandRequest(readbuf);

        close(fd);
    }

    return nullptr;
}

// TODO(#20664) REPL test will fail if signal SIGINT is not caught, temporarily keep following logic.

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

void SetupSignalHandlers()
{
    // sigaction is not used here because Tsan interceptors seems to
    // never dispatch the signals on darwin.
    signal(SIGINT, OnRebootSignalHandler);
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

    gChipEventFifoPath = kChipEventFifoPathPrefix + std::to_string(getpid());

    // Creating the named file(FIFO)
    if ((mkfifo(gChipEventFifoPath.c_str(), 0666) == -1) && (errno != EEXIST))
    {
        ChipLogError(Zcl, "Could not create Event FIFO");
        exit(1);
    }

    if (pthread_create(&gChipEventCommandListener, nullptr, EventCommandListenerTask, nullptr) != 0)
    {
        ChipLogError(Zcl, "Could not create EventCommandListenerTask");
    }
}

void ApplicationExit()
{
    if (pthread_cancel(gChipEventCommandListener) != 0)
    {
        ChipLogError(Zcl, "Failed to cancel EventCommandListenerTask");
        exit(1);
    }

    // Wait further for the thread to terminate if we had previously created it.
    if (pthread_join(gChipEventCommandListener, nullptr) != 0)
    {
        ChipLogError(Zcl, "Failed to terminate EventCommandListenerTask");
    }

    if (unlink(gChipEventFifoPath.c_str()) != 0)
    {
        ChipLogError(Zcl, "Failed to delete Event FIFO");
    }
}

void emberAfLowPowerClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: LowPower::SetDefaultDelegate");
    chip::app::Clusters::LowPower::SetDefaultDelegate(endpoint, &lowPowerManager);
}
