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

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>

#include <mutex>

#include <app-common/zap-generated/ids/Events.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DeviceControlServer.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/Linux/DeviceInstanceInfoProviderImpl.h>
#include <platform/Linux/DiagnosticDataProviderImpl.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.ipp>

using namespace ::chip::app::Clusters;

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

namespace {

#if CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP
void * GLibMainLoopThread(void * userData)
{
    GMainLoop * loop       = static_cast<GMainLoop *>(userData);
    GMainContext * context = g_main_loop_get_context(loop);

    g_main_context_push_thread_default(context);
    g_main_loop_run(loop);

    return nullptr;
}
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI

gboolean WiFiIPChangeListener(GIOChannel * ch, GIOCondition /* condition */, void * /* userData */)
{

    char buffer[4096];
    auto * header = reinterpret_cast<struct nlmsghdr *>(buffer);
    ssize_t len;

    if ((len = recv(g_io_channel_unix_get_fd(ch), buffer, sizeof(buffer), 0)) == -1)
    {
        if (errno == EINTR || errno == EAGAIN)
            return G_SOURCE_CONTINUE;
        ChipLogError(DeviceLayer, "Error reading from netlink socket: %d", errno);
        return G_SOURCE_CONTINUE;
    }

    if (len > 0)
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
                        char name[Inet::InterfaceId::kMaxIfNameLength];
                        if (if_indextoname(addressMessage->ifa_index, name) == nullptr)
                        {
                            ChipLogError(DeviceLayer, "Error %d when getting the interface name at index: %d", errno,
                                         addressMessage->ifa_index);
                            continue;
                        }

                        if (ConnectivityMgrImpl().GetWiFiIfName() == nullptr)
                        {
                            ChipLogDetail(DeviceLayer, "No wifi interface name. Ignoring IP update event.");
                            continue;
                        }

                        if (strcmp(name, ConnectivityMgrImpl().GetWiFiIfName()) != 0)
                        {
                            continue;
                        }

                        char ipStrBuf[chip::Inet::IPAddress::kMaxStringLength] = { 0 };
                        inet_ntop(AF_INET, RTA_DATA(routeInfo), ipStrBuf, sizeof(ipStrBuf));
                        ChipLogDetail(DeviceLayer, "Got IP address on interface: %s IP: %s", name, ipStrBuf);

                        ChipDeviceEvent event{ .Type                       = DeviceEventType::kInternetConnectivityChange,
                                               .InternetConnectivityChange = { .IPv4 = kConnectivity_Established,
                                                                               .IPv6 = kConnectivity_NoChange } };

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
    else
    {
        ChipLogError(DeviceLayer, "EOF on netlink socket");
        return G_SOURCE_REMOVE;
    }

    return G_SOURCE_CONTINUE;
}

// The temporary hack for getting IP address change on linux for network provisioning in the rendezvous session.
// This should be removed or find a better place once we deprecate the rendezvous session.
CHIP_ERROR RunWiFiIPChangeListener()
{
    int sock;
    if ((sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to init netlink socket for IP addresses: %d", errno);
        return CHIP_ERROR_INTERNAL;
    }

    struct sockaddr_nl addr;
    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_IPV4_IFADDR;

    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to bind netlink socket for IP addresses: %d", errno);
        close(sock);
        return CHIP_ERROR_INTERNAL;
    }

    GIOChannel * ch       = g_io_channel_unix_new(sock);
    GSource * watchSource = g_io_create_watch(ch, G_IO_IN);
    g_source_set_callback(watchSource, G_SOURCE_FUNC(WiFiIPChangeListener), nullptr, nullptr);
    g_io_channel_set_close_on_unref(ch, TRUE);
    g_io_channel_set_encoding(ch, nullptr, nullptr);

    PlatformMgrImpl().GLibMatterContextAttachSource(watchSource);

    g_source_unref(watchSource);
    g_io_channel_unref(ch);

    return CHIP_NO_ERROR;
}

#endif // #if CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
#if CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP

    auto * context      = g_main_context_new();
    mGLibMainLoop       = g_main_loop_new(context, FALSE);
    mGLibMainLoopThread = g_thread_new("gmain-matter", GLibMainLoopThread, mGLibMainLoop);
    g_main_context_unref(context);

    {
        // Wait for the GLib main loop to start. It is required that the context used
        // by the main loop is acquired before any other GLib functions are called. Otherwise,
        // the GLibMatterContextInvokeSync() might run functions on the wrong thread.

        std::unique_lock<std::mutex> lock(mGLibMainLoopCallbackIndirectionMutex);
        GLibMatterContextInvokeData invokeData{};

        auto * idleSource = g_idle_source_new();
        g_source_set_callback(
            idleSource,
            [](void * userData_) {
                auto * data = reinterpret_cast<GLibMatterContextInvokeData *>(userData_);
                std::unique_lock<std::mutex> lock_(PlatformMgrImpl().mGLibMainLoopCallbackIndirectionMutex);
                data->mDone = true;
                data->mDoneCond.notify_one();
                return G_SOURCE_REMOVE;
            },
            &invokeData, nullptr);
        GLibMatterContextAttachSource(idleSource);
        g_source_unref(idleSource);

        invokeData.mDoneCond.wait(lock, [&invokeData]() { return invokeData.mDone; });
    }

#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    ReturnErrorOnFailure(RunWiFiIPChangeListener());
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

#if CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP
    if (mGLibMainLoop != nullptr)
    {
        g_main_loop_quit(mGLibMainLoop);
        g_thread_join(mGLibMainLoopThread);
        g_main_loop_unref(mGLibMainLoop);
        mGLibMainLoop = nullptr;
    }
#endif
}

#if CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP
void PlatformManagerImpl::_GLibMatterContextInvokeSync(LambdaBridge && bridge)
{
    // Because of TSAN false positives, we need to use a mutex to synchronize access to all members of
    // the GLibMatterContextInvokeData object (including constructor and destructor). This is a temporary
    // workaround until TSAN-enabled GLib will be used in our CI.
    std::unique_lock<std::mutex> lock(mGLibMainLoopCallbackIndirectionMutex);

    GLibMatterContextInvokeData invokeData{ std::move(bridge) };

    lock.unlock();

    g_main_context_invoke_full(
        g_main_loop_get_context(mGLibMainLoop), G_PRIORITY_HIGH_IDLE,
        [](void * userData_) {
            auto * data = reinterpret_cast<GLibMatterContextInvokeData *>(userData_);

            // XXX: Temporary workaround for TSAN false positives.
            std::unique_lock<std::mutex> lock_(PlatformMgrImpl().mGLibMainLoopCallbackIndirectionMutex);

            lock_.unlock();
            data->bridge();
            lock_.lock();

            data->mDone = true;
            data->mDoneCond.notify_one();

            return G_SOURCE_REMOVE;
        },
        &invokeData, nullptr);

    lock.lock();
    invokeData.mDoneCond.wait(lock, [&invokeData]() { return invokeData.mDone; });
}
#endif // CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP

} // namespace DeviceLayer
} // namespace chip
