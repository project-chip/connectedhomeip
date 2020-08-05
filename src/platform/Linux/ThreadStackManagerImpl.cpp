/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <array>
#include <limits.h>
#include <string.h>

#include "platform/internal/CHIPDeviceLayerInternal.h"
#include "platform/internal/DeviceNetworkInfo.h"

#include "platform/PlatformManager.h"
#include "platform/ThreadStackManager.h"
#include "support/CodeUtils.h"
#include "support/logging/CHIPLogging.h"

#include "dbus/client/thread_api_dbus.hpp"

using chip::DeviceLayer::Internal::DeviceNetworkInfo;
using otbr::DBus::ClientError;
using otbr::DBus::DeviceRole;
using otbr::DBus::IpCounters;
using otbr::DBus::LinkModeConfig;
using otbr::DBus::MacCounters;
using otbr::DBus::NeighborInfo;

#ifndef CHIP_CONFIG_OTBR_CLIENT_ERROR_MIN
#define CHIP_CONFIG_OTBR_CLIENT_ERROR_MIN 8000000
#endif

#define OTBR_TO_CHIP_ERROR(x)                                                                                                      \
    (x == ClientError::ERROR_NONE ? CHIP_NO_ERROR : _CHIP_ERROR(CHIP_CONFIG_OTBR_CLIENT_ERROR_MIN + static_cast<int>(x)))

#define LogClientError(error)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        if (error != ClientError::ERROR_NONE)                                                                                      \
        {                                                                                                                          \
            ChipLogError(DeviceLayer, __FILE__ " %d: Otbr ClientError %d", __LINE__, static_cast<int>(error));                     \
        }                                                                                                                          \
    } while (0)

namespace chip {
namespace DeviceLayer {

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

ThreadStackManagerImpl::ThreadStackManagerImpl() : mThreadApi(nullptr), mConnection(nullptr), mNetworkInfo(), mAttached(false) {}

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    ClientError error;
    DeviceRole role;
    DBusError dbusError;
    DBusConnection * dispatchConnection;

    dbus_error_init(&dbusError);
    mConnection = UniqueDBusConnection(dbus_bus_get(DBUS_BUS_SYSTEM, &dbusError));
    VerifyOrExit(dbus_bus_register(mConnection.get(), &dbusError), error = ClientError::ERROR_DBUS);

    VerifyOrExit(mConnection != nullptr, error = ClientError::ERROR_DBUS);
    mThreadApi = std::unique_ptr<otbr::DBus::ThreadApiDBus>(new otbr::DBus::ThreadApiDBus(mConnection.get()));
    mThreadApi->AddDeviceRoleHandler([this](DeviceRole newRole) { this->_ThreadDevcieRoleChangedHandler(newRole); });

    SuccessOrExit(error = mThreadApi->GetDeviceRole(role));
    _ThreadDevcieRoleChangedHandler(role);
    mAttached = (role != DeviceRole::OTBR_DEVICE_ROLE_DETACHED && role != DeviceRole::OTBR_DEVICE_ROLE_DISABLED);

    dispatchConnection = mConnection.get();
    mDBusEventLoop     = std::thread([dispatchConnection]() {
        while (true)
        {
            dbus_connection_read_write_dispatch(dispatchConnection, -1);
        }
    });
    mDBusEventLoop.detach();
exit:
    dbus_error_free(&dbusError);
    LogClientError(error);
    return OTBR_TO_CHIP_ERROR(error);
}

void ThreadStackManagerImpl::_ThreadDevcieRoleChangedHandler(DeviceRole role)
{
    bool attached         = (role != DeviceRole::OTBR_DEVICE_ROLE_DETACHED && role != DeviceRole::OTBR_DEVICE_ROLE_DISABLED);
    ChipDeviceEvent event = ChipDeviceEvent{};

    if (attached != mAttached)
    {
        event.Type = DeviceEventType::kThreadConnectivityChange;
        event.ThreadConnectivityChange.Result =
            attached ? ConnectivityChange::kConnectivity_Established : ConnectivityChange::kConnectivity_Lost;
        PlatformMgr().PostEvent(&event);
    }

    event.Type                          = DeviceEventType::kThreadStateChange;
    event.ThreadStateChange.RoleChanged = true;
    PlatformMgr().PostEvent(&event);
}

void ThreadStackManagerImpl::_ProcessThreadActivity() {}

static bool RouteMatch(const otbr::DBus::Ip6Prefix & prefix, const Inet::IPAddress & addr)
{
    bool match = true;
    const uint8_t * prefixBuffer;
    const uint8_t * addrBuffer;
    uint8_t wholeBytes  = prefix.mLength / CHAR_BIT;
    uint8_t pendingBits = prefix.mLength % CHAR_BIT;

    VerifyOrExit(addr.IsIPv6(), match = false);
    VerifyOrExit(prefix.mLength > 0, match = false);

    prefixBuffer = static_cast<const uint8_t *>(&prefix.mPrefix[0]);
    addrBuffer   = reinterpret_cast<const uint8_t *>(&addr.Addr);
    VerifyOrExit(memcmp(addrBuffer, prefixBuffer, wholeBytes) == 0, match = false);
    if (pendingBits)
    {
        uint8_t mask = static_cast<uint8_t>(((UINT8_MAX >> pendingBits) << (CHAR_BIT - pendingBits)));

        VerifyOrExit((addrBuffer[wholeBytes] & mask) == (addrBuffer[wholeBytes] & mask), match = false);
    }
    VerifyOrExit(memcmp(addrBuffer, prefixBuffer, wholeBytes) == 0, match = false);

exit:
    return match;
}

bool ThreadStackManagerImpl::_HaveRouteToAddress(const Inet::IPAddress & destAddr)
{
    std::vector<otbr::DBus::ExternalRoute> routes;
    bool match = false;

    VerifyOrExit(mThreadApi->GetExternalRoutes(routes) == ClientError::ERROR_NONE, match = false);
    VerifyOrExit(_IsThreadAttached(), match = false);
    VerifyOrExit(destAddr.IsIPv6LinkLocal(), match = true);
    for (const auto & route : routes)
    {
        VerifyOrExit(!(match = RouteMatch(route.mPrefix, destAddr)), );
    }

exit:
    return match;
}

void ThreadStackManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    (void) event;
    // The otbr-agent processes the Thread state handling by itself so there
    // isn't much to do in the Chip stack.
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadProvision(const Internal::DeviceNetworkInfo & netInfo)
{
    mNetworkInfo = netInfo;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_GetThreadProvision(Internal::DeviceNetworkInfo & netInfo, bool includeCredentials)
{
    netInfo = mNetworkInfo;

    if (!includeCredentials)
    {
        memset(&netInfo.ThreadNetworkKey, 0, sizeof(netInfo.ThreadNetworkKey));
        memset(&netInfo.ThreadPSKc, 0, sizeof(netInfo.ThreadPSKc));
        netInfo.FieldPresent.ThreadPSKc = false;
    }

    return CHIP_NO_ERROR;
}

bool ThreadStackManagerImpl::_IsThreadProvisioned()
{
    return mNetworkInfo.ThreadNetworkName[0] != '\0';
}

void ThreadStackManagerImpl::_ErasePersistentInfo()
{
    mNetworkInfo = Internal::DeviceNetworkInfo{};
}

bool ThreadStackManagerImpl::_IsThreadEnabled()
{
    bool enabled = false;
    DeviceRole role;
    ClientError error;

    SuccessOrExit(error = mThreadApi->GetDeviceRole(role));
    enabled = (role != DeviceRole::OTBR_DEVICE_ROLE_DISABLED);
exit:
    LogClientError(error);
    return enabled;
}

bool ThreadStackManagerImpl::_IsThreadAttached()
{
    return mAttached;
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadEnabled(bool val)
{
    ClientError error = ClientError::ERROR_NONE;

    if (val)
    {
        std::vector<uint8_t> masterkey(std::begin(mNetworkInfo.ThreadNetworkKey), std::end(mNetworkInfo.ThreadNetworkKey));
        std::vector<uint8_t> pskc;
        uint64_t extPanId    = UINT64_MAX;
        uint32_t channelMask = UINT32_MAX;

        if (mNetworkInfo.FieldPresent.ThreadExtendedPANId)
        {
            extPanId = 0;
            for (size_t i = 0; i < extPanId; i++)
            {
                extPanId <<= CHAR_BIT;
                extPanId |= mNetworkInfo.ThreadExtendedPANId[i];
            }
        }
        if (mNetworkInfo.FieldPresent.ThreadPSKc)
        {
            pskc = std::vector<uint8_t>(std::begin(mNetworkInfo.ThreadPSKc), std::end(mNetworkInfo.ThreadPSKc));
        }
        if (mNetworkInfo.ThreadChannel != Internal::kThreadChannel_NotSpecified)
        {
            channelMask = 1 << mNetworkInfo.ThreadChannel;
        }

        if (mNetworkInfo.FieldPresent.ThreadMeshPrefix)
        {
            std::array<uint8_t, Internal::kThreadMeshPrefixLength> prefix;

            std::copy(std::begin(mNetworkInfo.ThreadMeshPrefix), std::end(mNetworkInfo.ThreadMeshPrefix), std::begin(prefix));
            SuccessOrExit(error = mThreadApi->SetMeshLocalPrefix(prefix));
        }

        mThreadApi->Attach(mNetworkInfo.ThreadNetworkName, mNetworkInfo.ThreadPANId, extPanId, masterkey, pskc, channelMask,
                           [](ClientError result) { ChipLogProgress(DeviceLayer, "Thread attach result %d", result); });
    }
    else
    {
        mThreadApi->Reset();
    }
exit:
    LogClientError(error);
    return OTBR_TO_CHIP_ERROR(error);
}

ConnectivityManager::ThreadDeviceType ThreadStackManagerImpl::_GetThreadDeviceType()
{
    ClientError error;
    DeviceRole role;
    LinkModeConfig linkMode;
    ConnectivityManager::ThreadDeviceType type = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;

    SuccessOrExit(error = mThreadApi->GetDeviceRole(role));

    switch (role)
    {
    case DeviceRole::OTBR_DEVICE_ROLE_DISABLED:
    case DeviceRole::OTBR_DEVICE_ROLE_DETACHED:
        break;
    case DeviceRole::OTBR_DEVICE_ROLE_CHILD:
        SuccessOrExit(error = mThreadApi->GetLinkMode(linkMode));
        if (!linkMode.mRxOnWhenIdle)
        {
            type = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_SleepyEndDevice;
        }
        else
        {
            type = linkMode.mDeviceType ? ConnectivityManager::ThreadDeviceType::kThreadDeviceType_FullEndDevice
                                        : ConnectivityManager::ThreadDeviceType::kThreadDeviceType_MinimalEndDevice;
        }
    case DeviceRole::OTBR_DEVICE_ROLE_ROUTER:
    case DeviceRole::OTBR_DEVICE_ROLE_LEADER:
        type = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_Router;
    default:
        ChipLogError(DeviceLayer, "Unknown Thread role: %d", static_cast<int>(role));
        break;
    }

exit:
    LogClientError(error);
    return type;
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    LinkModeConfig linkMode{ true, true, true, true };
    ClientError error = ClientError::ERROR_NONE;

    if (deviceType == ConnectivityManager::ThreadDeviceType::kThreadDeviceType_MinimalEndDevice)
    {
        linkMode.mNetworkData = false;
    }
    else if (deviceType == ConnectivityManager::ThreadDeviceType::kThreadDeviceType_SleepyEndDevice)
    {
        linkMode.mRxOnWhenIdle = false;
        linkMode.mNetworkData  = false;
    }

    if (!linkMode.mNetworkData)
    {
        error = mThreadApi->SetLinkMode(linkMode);
    }

    LogClientError(error);
    return OTBR_TO_CHIP_ERROR(error);
}

void ThreadStackManagerImpl::_GetThreadPollingConfig(ConnectivityManager::ThreadPollingConfig & pollingConfig)
{
    (void) pollingConfig;

    ChipLogError(DeviceLayer, "Polling config is not supported on linux");
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadPollingConfig(const ConnectivityManager::ThreadPollingConfig & pollingConfig)
{
    (void) pollingConfig;

    ChipLogError(DeviceLayer, "Polling config is not supported on linux");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool ThreadStackManagerImpl::_HaveMeshConnectivity()
{
    DeviceRole role;
    ClientError error;
    bool hasConnectivity = false;
    std::vector<NeighborInfo> neighbors;

    SuccessOrExit(error = mThreadApi->GetDeviceRole(role));
    VerifyOrExit(role != DeviceRole::OTBR_DEVICE_ROLE_DETACHED && role != DeviceRole::OTBR_DEVICE_ROLE_DISABLED, );
    if (role == DeviceRole::OTBR_DEVICE_ROLE_CHILD || role == DeviceRole::OTBR_DEVICE_ROLE_ROUTER)
    {
        hasConnectivity = true;
    }

    SuccessOrExit(error = mThreadApi->GetNeighborTable(neighbors));
    for (const auto & neighbor : neighbors)
    {
        if (!neighbor.mIsChild)
        {
            hasConnectivity = true;
            break;
        }
    }

exit:
    LogClientError(error);
    return hasConnectivity;
}

void ThreadStackManagerImpl::_OnMessageLayerActivityChanged(bool messageLayerIsActive)
{
    (void) messageLayerIsActive;
}

void ThreadStackManagerImpl::_OnCHIPoBLEAdvertisingStart() {}

void ThreadStackManagerImpl::_OnCHIPoBLEAdvertisingStop() {}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadStatsCounters()
{
    // TODO: implement after we decide on the profiling protocol
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadTopologyMinimal()
{
    // TODO: implement after we decide on the profiling protocol
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadTopologyFull()
{
    // TODO: implement after we decide on the profiling protocol
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetPrimary802154MACAddress(uint8_t * buf)
{
    uint64_t extAddr;
    ClientError error;
    SuccessOrExit(error = mThreadApi->GetExtendedAddress(extAddr));

    for (size_t i = 0; i < sizeof(extAddr); i++)
    {
        buf[sizeof(uint64_t) - i - 1] = (extAddr & UINT8_MAX);
        extAddr >>= CHAR_BIT;
    }

exit:
    LogClientError(error);
    return OTBR_TO_CHIP_ERROR(error);
}

CHIP_ERROR ThreadStackManagerImpl::_JoinerStart(void)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

ThreadStackManager & ThreadStackMgr(void)
{
    return chip::DeviceLayer::ThreadStackManagerImpl::sInstance;
}

ThreadStackManagerImpl & ThreadStackMgrImpl(void)
{
    return chip::DeviceLayer::ThreadStackManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
