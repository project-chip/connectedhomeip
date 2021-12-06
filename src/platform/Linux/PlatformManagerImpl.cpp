/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for Linux platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app-common/zap-generated/enums.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/DiagnosticDataProviderImpl.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.cpp>

#include <thread>

#include <arpa/inet.h>
#include <dirent.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>

using namespace ::chip::app::Clusters;

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

namespace {

void SignalHandler(int signum)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DeviceLayer, "Caught signal %d", signum);

    // The BootReason attribute SHALL indicate the reason for the Node’s most recent boot, the real usecase
    // for this attribute is embedded system. In Linux simulation, we use different signals to tell the current
    // running process to terminate with different reasons.
    switch (signum)
    {
    case SIGINT:
        ConfigurationMgr().StoreBootReason(EMBER_ZCL_BOOT_REASON_TYPE_SOFTWARE_RESET);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGHUP:
        ConfigurationMgr().StoreBootReason(EMBER_ZCL_BOOT_REASON_TYPE_BROWN_OUT_RESET);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGTERM:
        ConfigurationMgr().StoreBootReason(EMBER_ZCL_BOOT_REASON_TYPE_POWER_ON_REBOOT);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGUSR1:
        ConfigurationMgr().StoreBootReason(EMBER_ZCL_BOOT_REASON_TYPE_HARDWARE_WATCHDOG_RESET);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGUSR2:
        ConfigurationMgr().StoreBootReason(EMBER_ZCL_BOOT_REASON_TYPE_SOFTWARE_WATCHDOG_RESET);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGTSTP:
        ConfigurationMgr().StoreBootReason(EMBER_ZCL_BOOT_REASON_TYPE_SOFTWARE_UPDATE_COMPLETED);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGTRAP:
        PlatformMgrImpl().HandleSoftwareFault(SoftwareDiagnostics::Events::SoftwareFault::kEventId);
        break;
    case SIGILL:
        PlatformMgrImpl().HandleGeneralFault(GeneralDiagnostics::Events::HardwareFaultChange::kEventId);
        break;
    case SIGALRM:
        PlatformMgrImpl().HandleGeneralFault(GeneralDiagnostics::Events::RadioFaultChange::kEventId);
        break;
    case SIGVTALRM:
        PlatformMgrImpl().HandleGeneralFault(GeneralDiagnostics::Events::NetworkFaultChange::kEventId);
        break;
    default:
        break;
    }

    if (err == CHIP_ERROR_REBOOT_SIGNAL_RECEIVED)
    {
        PlatformMgr().Shutdown();
        exit(EXIT_FAILURE);
    }
}

#if CHIP_WITH_GIO
void GDBus_Thread()
{
    GMainLoop * loop = g_main_loop_new(nullptr, false);

    g_main_loop_run(loop);
    g_main_loop_unref(loop);
}
#endif
} // namespace

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
void PlatformManagerImpl::WiFIIPChangeListener()
{
    int sock;
    if ((sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to init netlink socket for ip addresses.");
        return;
    }

    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_IPV4_IFADDR;

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to bind netlink socket for ip addresses.");
        return;
    }

    ssize_t len;
    char buffer[4096];
    for (struct nlmsghdr * header = reinterpret_cast<struct nlmsghdr *>(buffer); (len = recv(sock, header, sizeof(buffer), 0)) > 0;)
    {
        for (struct nlmsghdr * messageHeader = header;
             (NLMSG_OK(messageHeader, static_cast<uint32_t>(len))) && (messageHeader->nlmsg_type != NLMSG_DONE);
             messageHeader = NLMSG_NEXT(messageHeader, len))
        {
            if (header->nlmsg_type == RTM_NEWADDR)
            {
                struct ifaddrmsg * addressMessage = (struct ifaddrmsg *) NLMSG_DATA(header);
                struct rtattr * routeInfo         = IFA_RTA(addressMessage);
                size_t rtl                        = IFA_PAYLOAD(header);

                for (; rtl && RTA_OK(routeInfo, rtl); routeInfo = RTA_NEXT(routeInfo, rtl))
                {
                    if (routeInfo->rta_type == IFA_LOCAL)
                    {
                        char name[IFNAMSIZ];
                        ChipDeviceEvent event;
                        if_indextoname(addressMessage->ifa_index, name);
                        if (strcmp(name, ConnectivityManagerImpl::GetWiFiIfName()) != 0)
                        {
                            continue;
                        }

                        event.Type                            = DeviceEventType::kInternetConnectivityChange;
                        event.InternetConnectivityChange.IPv4 = kConnectivity_Established;
                        event.InternetConnectivityChange.IPv6 = kConnectivity_NoChange;
                        inet_ntop(AF_INET, RTA_DATA(routeInfo), event.InternetConnectivityChange.address,
                                  sizeof(event.InternetConnectivityChange.address));

                        ChipLogDetail(DeviceLayer, "Got IP address on interface: %s IP: %s", name,
                                      event.InternetConnectivityChange.address);

                        CHIP_ERROR status = PlatformMgr().PostEvent(&event);
                        if (status != CHIP_NO_ERROR)
                        {
                            ChipLogDetail(DeviceLayer, "Failed to report IP address: %" CHIP_ERROR_FORMAT, status.Format());
                        }
                    }
                }
            }
        }
    }
}
#endif // #if CHIP_DEVICE_CONFIG_ENABLE_WIFI

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
    CHIP_ERROR err;
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = SignalHandler;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);

#if CHIP_WITH_GIO
    GError * error = nullptr;

    this->mpGDBusConnection = UniqueGDBusConnection(g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error));

    std::thread gdbusThread(GDBus_Thread);
    gdbusThread.detach();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    std::thread wifiIPThread(WiFIIPChangeListener);
    wifiIPThread.detach();
#endif

    // Initialize the configuration system.
    err = Internal::PosixConfig::Init();
    SuccessOrExit(err);
    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    mStartTime = System::SystemClock().GetMonotonicTimestamp();

    ScheduleWork(HandleDeviceRebooted, 0);

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    uint64_t upTime = 0;

    if (GetDiagnosticDataProvider().GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalOperationalHours = 0;

        if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get current uptime since the Node’s last reboot");
    }

    return Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_Shutdown();
}

void PlatformManagerImpl::HandleDeviceRebooted(intptr_t arg)
{
    GeneralDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetGeneralDiagnosticsDelegate();

    if (delegate != nullptr)
    {
        delegate->OnDeviceRebooted();
    }
}

void PlatformManagerImpl::HandleGeneralFault(uint32_t EventId)
{
    GeneralDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetGeneralDiagnosticsDelegate();

    if (delegate == nullptr)
    {
        ChipLogError(DeviceLayer, "No delegate registered to handle General Diagnostics event");
        return;
    }

    if (EventId == GeneralDiagnostics::Events::HardwareFaultChange::kEventId)
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
        delegate->OnHardwareFaultsDetected(previous, current);
    }
    else if (EventId == GeneralDiagnostics::Events::RadioFaultChange::kEventId)
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
        delegate->OnRadioFaultsDetected(previous, current);
    }
    else if (EventId == GeneralDiagnostics::Events::NetworkFaultChange::kEventId)
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
        delegate->OnNetworkFaultsDetected(previous, current);
    }
    else
    {
    }
}

void PlatformManagerImpl::HandleSoftwareFault(uint32_t EventId)
{
    SoftwareDiagnosticsDelegate * delegate = GetDiagnosticDataProvider().GetSoftwareDiagnosticsDelegate();

    if (delegate != nullptr)
    {
        SoftwareDiagnostics::Structs::SoftwareFault::Type softwareFault;
        char threadName[kMaxThreadNameLength + 1];

        softwareFault.id = gettid();
        strncpy(threadName, std::to_string(softwareFault.id).c_str(), kMaxThreadNameLength);
        threadName[kMaxThreadNameLength] = '\0';
        softwareFault.name               = CharSpan(threadName, strlen(threadName));
        softwareFault.faultRecording     = ByteSpan(Uint8::from_const_char("FaultRecording"), strlen("FaultRecording"));

        delegate->OnSoftwareFaultDetected(softwareFault);
    }
}

#if CHIP_WITH_GIO
GDBusConnection * PlatformManagerImpl::GetGDBusConnection()
{
    return this->mpGDBusConnection.get();
}
#endif

} // namespace DeviceLayer
} // namespace chip
