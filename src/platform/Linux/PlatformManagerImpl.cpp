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
#include <app-common/zap-generated/ids/Events.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DeviceControlServer.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/Linux/DeviceInstanceInfoProviderImpl.h>
#include <platform/Linux/DiagnosticDataProviderImpl.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.ipp>

#include <thread>

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

using namespace ::chip::app::Clusters;

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

namespace {

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
                        if (if_indextoname(addressMessage->ifa_index, name) == nullptr)
                        {
                            ChipLogError(DeviceLayer, "Error %d when getting the interface name at index: %d", errno,
                                         addressMessage->ifa_index);
                            continue;
                        }

                        if (ConnectivityManagerImpl::GetWiFiIfName() == nullptr)
                        {
                            ChipLogDetail(DeviceLayer, "No wifi interface name. Ignoring IP update event.");
                            continue;
                        }

                        if (strcmp(name, ConnectivityManagerImpl::GetWiFiIfName()) != 0)
                        {
                            continue;
                        }

                        char ipStrBuf[chip::Inet::IPAddress::kMaxStringLength] = { 0 };
                        inet_ntop(AF_INET, RTA_DATA(routeInfo), ipStrBuf, sizeof(ipStrBuf));
                        ChipLogDetail(DeviceLayer, "Got IP address on interface: %s IP: %s", name, ipStrBuf);

                        ChipDeviceEvent event;
                        event.Type                            = DeviceEventType::kInternetConnectivityChange;
                        event.InternetConnectivityChange.IPv4 = kConnectivity_Established;
                        event.InternetConnectivityChange.IPv6 = kConnectivity_NoChange;

                        if (!chip::Inet::IPAddress::FromString(ipStrBuf, event.InternetConnectivityChange.ipAddress))
                        {
                            ChipLogDetail(DeviceLayer, "Failed to report IP address - ip address parsing failed");
                            continue;
                        }

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
    ReturnErrorOnFailure(Internal::PosixConfig::Init());

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    ReturnErrorOnFailure(Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_InitChipStack());

    // Now set up our device instance info provider.  We couldn't do that
    // earlier, because the generic implementation sets a generic one.
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());

    mStartTime = System::SystemClock().GetMonotonicTimestamp();

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_Shutdown()
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

    Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_Shutdown();
}

#if CHIP_WITH_GIO
GDBusConnection * PlatformManagerImpl::GetGDBusConnection()
{
    return this->mpGDBusConnection.get();
}
#endif

} // namespace DeviceLayer
} // namespace chip
