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
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.cpp>

#include <thread>

#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <malloc.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>

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
        ConfigurationMgrImpl().StoreBootReasons(EMBER_ZCL_BOOT_REASON_TYPE_SOFTWARE_RESET);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGHUP:
        ConfigurationMgrImpl().StoreBootReasons(EMBER_ZCL_BOOT_REASON_TYPE_BROWN_OUT_RESET);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGTERM:
        ConfigurationMgrImpl().StoreBootReasons(EMBER_ZCL_BOOT_REASON_TYPE_POWER_ON_REBOOT);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGUSR1:
        ConfigurationMgrImpl().StoreBootReasons(EMBER_ZCL_BOOT_REASON_TYPE_HARDWARE_WATCHDOG_RESET);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGUSR2:
        ConfigurationMgrImpl().StoreBootReasons(EMBER_ZCL_BOOT_REASON_TYPE_SOFTWARE_WATCHDOG_RESET);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    case SIGTSTP:
        ConfigurationMgrImpl().StoreBootReasons(EMBER_ZCL_BOOT_REASON_TYPE_SOFTWARE_UPDATE_COMPLETED);
        err = CHIP_ERROR_REBOOT_SIGNAL_RECEIVED;
        break;
    default:
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        PlatformMgr().Shutdown();
        exit(EXIT_FAILURE);
    }
    else
    {
        ChipLogDetail(DeviceLayer, "Ignore signal %d", signum);
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
    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    mStartTimeMilliseconds = System::SystemClock().GetMonotonicMilliseconds();

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    uint64_t upTime = 0;

    if (_GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalOperationalHours = 0;

        if (ConfigurationMgrImpl().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgrImpl().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
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

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    struct mallinfo mallocInfo = mallinfo();

    // Get the current amount of heap memory, in bytes, that are not being utilized
    // by the current running program.
    currentHeapFree = mallocInfo.fordblks;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    struct mallinfo mallocInfo = mallinfo();

    // Get the current amount of heap memory, in bytes, that are being used by
    // the current running program.
    currentHeapUsed = mallocInfo.uordblks;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    struct mallinfo mallocInfo = mallinfo();

    // The usecase of this function is embedded devices,on which we would need to intercept
    // malloc/calloc/free and then record the maximum amount of heap memory,in bytes, that
    // has been used by the Node.
    // On Linux, since it uses virtual memory, whereby a page of memory could be copied to
    // the hard disk, called swap space, and free up that page of memory. So it is impossible
    // to know accurately peak physical memory it use. We just return the current heap memory
    // being used by the current running program.
    currentHeapHighWatermark = mallocInfo.uordblks;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_GetRebootCount(uint16_t & rebootCount)
{
    uint32_t count = 0;

    CHIP_ERROR err = ConfigurationMgrImpl().GetRebootCount(count);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(count <= UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        rebootCount = static_cast<uint16_t>(count);
    }

    return err;
}

CHIP_ERROR PlatformManagerImpl::_GetUpTime(uint64_t & upTime)
{
    uint64_t currentTimeMilliseconds = System::SystemClock().GetMonotonicMilliseconds();

    if (currentTimeMilliseconds >= mStartTimeMilliseconds)
    {
        upTime = (currentTimeMilliseconds - mStartTimeMilliseconds) / 1000;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR PlatformManagerImpl::_GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    uint64_t upTime = 0;

    if (_GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalHours = 0;
        if (ConfigurationMgrImpl().GetTotalOperationalHours(totalHours) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(upTime / 3600 <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
            totalOperationalHours = totalHours + static_cast<uint32_t>(upTime / 3600);
        }
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR PlatformManagerImpl::_GetBootReasons(uint8_t & bootReasons)
{
    uint32_t reason = 0;

    CHIP_ERROR err = ConfigurationMgrImpl().GetBootReasons(reason);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReasons = static_cast<uint8_t>(reason);
    }

    return err;
}

#if CHIP_WITH_GIO
GDBusConnection * PlatformManagerImpl::GetGDBusConnection()
{
    return this->mpGDBusConnection.get();
}
#endif

} // namespace DeviceLayer
} // namespace chip
