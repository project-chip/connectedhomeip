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

using namespace ::chip::app::Clusters;

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

namespace {

#if CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP
void * GLibMainLoopThread(void * loop)
{
    g_main_loop_run(static_cast<GMainLoop *>(loop));
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

    GIOChannel * ch = g_io_channel_unix_new(sock);
    g_io_add_watch_full(ch, G_PRIORITY_DEFAULT, G_IO_IN, WiFiIPChangeListener, nullptr, nullptr);

    g_io_channel_set_close_on_unref(ch, TRUE);
    g_io_channel_set_encoding(ch, nullptr, nullptr);
    g_io_channel_unref(ch);

    return CHIP_NO_ERROR;
}

#endif // #if CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
#if CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP

    mGLibMainLoop       = g_main_loop_new(nullptr, FALSE);
    mGLibMainLoopThread = g_thread_new("gmain-matter", GLibMainLoopThread, mGLibMainLoop);

    {
        std::unique_lock<std::mutex> lock(mGLibMainLoopCallbackIndirectionMutex);
        CallbackIndirection startedInd([](void *) { return G_SOURCE_REMOVE; }, nullptr);
        g_idle_add(G_SOURCE_FUNC(&CallbackIndirection::Callback), &startedInd);
        startedInd.Wait(lock);
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
    g_main_loop_quit(mGLibMainLoop);
    g_main_loop_unref(mGLibMainLoop);
    g_thread_join(mGLibMainLoopThread);
#endif
}

#if CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP

void PlatformManagerImpl::CallbackIndirection::Wait(std::unique_lock<std::mutex> & lock)
{
    mDoneCond.wait(lock, [this]() { return mDone; });
}

gboolean PlatformManagerImpl::CallbackIndirection::Callback(CallbackIndirection * self)
{
    // We can not access "self" before acquiring the lock, because TSAN will complain that
    // there is a race condition between the thread that created the object and the thread
    // that is executing the callback.
    std::unique_lock<std::mutex> lock(PlatformMgrImpl().mGLibMainLoopCallbackIndirectionMutex);

    auto callback = self->mCallback;
    auto userData = self->mUserData;

    lock.unlock();
    auto result = callback(userData);
    lock.lock();

    self->mDone = true;
    self->mDoneCond.notify_all();

    return result;
}

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
CHIP_ERROR PlatformManagerImpl::RunOnGLibMainLoopThread(GSourceFunc callback, void * userData, bool wait)
{

    GMainContext * context = g_main_loop_get_context(mGLibMainLoop);
    VerifyOrReturnError(context != nullptr,
                        (ChipLogDetail(DeviceLayer, "Failed to get GLib main loop context"), CHIP_ERROR_INTERNAL));

    if (wait)
    {
        std::unique_lock<std::mutex> lock(mGLibMainLoopCallbackIndirectionMutex);
        CallbackIndirection indirection(callback, userData);
        g_main_context_invoke(context, G_SOURCE_FUNC(&CallbackIndirection::Callback), &indirection);
        indirection.Wait(lock);
        return CHIP_NO_ERROR;
    }

    g_main_context_invoke(context, callback, userData);
    return CHIP_NO_ERROR;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#endif // CHIP_DEVICE_CONFIG_WITH_GLIB_MAIN_LOOP

} // namespace DeviceLayer
} // namespace chip
