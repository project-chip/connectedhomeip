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

using otbr::DBus::ClientError;
using otbr::DBus::DeviceRole;
using otbr::DBus::LinkModeConfig;

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

constexpr int kDBusConnectionPollingTimeoutMS = 10;

namespace chip {
namespace DeviceLayer {

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

ThreadStackManagerImpl::ThreadStackManagerImpl() : mThreadApi(nullptr), mConnection(nullptr), mAttached(false) {}

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    ClientError error;
    DeviceRole role;
    DBusError dbusError;
    DBusConnection * dispatchConnection;

    dbus_error_init(&dbusError);
    mConnection = UniqueDBusConnection(dbus_bus_get_private(DBUS_BUS_SYSTEM, &dbusError));

    if (mConnection == nullptr)
    {
        dbus_error_free(&dbusError);
        error = ClientError::ERROR_DBUS;
        LogClientError(error);

        return OTBR_TO_CHIP_ERROR(error);
    }
    mThreadApi = std::unique_ptr<otbr::DBus::ThreadApiDBus>(new otbr::DBus::ThreadApiDBus(mConnection.get()));
    mThreadApi->AddDeviceRoleHandler([this](DeviceRole newRole) { this->_ThreadDevcieRoleChangedHandler(newRole); });

    if (mThreadApi->GetDeviceRole(role) != ClientError::ERROR_NONE)
    {
        dbus_error_free(&dbusError);
        error = ClientError::ERROR_DBUS;
        LogClientError(error);

        return OTBR_TO_CHIP_ERROR(error);
    }
    _ThreadDevcieRoleChangedHandler(role);
    mAttached = (role != DeviceRole::OTBR_DEVICE_ROLE_DETACHED && role != DeviceRole::OTBR_DEVICE_ROLE_DISABLED);

    dispatchConnection = mConnection.get();
    mDBusEventLoop     = std::thread([dispatchConnection]() {
        while (true)
        {
            // The dbus_connection_read_write will lock the connection until new message comes or timeout.
            // This will block ot-br-posix APIs. Set timeout to 10ms so it can work.
            // TODO: we should have a global event loop for dbus to take care of this.
            dbus_connection_read_write_dispatch(dispatchConnection, kDBusConnectionPollingTimeoutMS);
        }
    });
    mDBusEventLoop.detach();
    dbus_error_free(&dbusError);
    return CHIP_NO_ERROR;
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
    const uint8_t * prefixBuffer;
    const uint8_t * addrBuffer;
    uint8_t wholeBytes  = prefix.mLength / CHAR_BIT;
    uint8_t pendingBits = prefix.mLength % CHAR_BIT;

    if (!addr.IsIPv6() || prefix.mLength == 0)
    {
        return false;
    }

    prefixBuffer = static_cast<const uint8_t *>(&prefix.mPrefix[0]);
    addrBuffer   = reinterpret_cast<const uint8_t *>(&addr.Addr);
    if (pendingBits)
    {
        uint8_t mask = static_cast<uint8_t>(((UINT8_MAX >> pendingBits) << (CHAR_BIT - pendingBits)));

        if ((addrBuffer[wholeBytes] & mask) != (addrBuffer[wholeBytes] & mask))
        {
            return false;
        }
    }
    return memcmp(addrBuffer, prefixBuffer, wholeBytes);
}

bool ThreadStackManagerImpl::_HaveRouteToAddress(const Inet::IPAddress & destAddr)
{
    // TODO: Remove Weave legacy APIs
    std::vector<otbr::DBus::ExternalRoute> routes;
    bool match = false;

    if (mThreadApi == nullptr || !_IsThreadAttached())
    {
        return false;
    }
    if (destAddr.IsIPv6LinkLocal())
    {
        return true;
    }
    if (mThreadApi->GetExternalRoutes(routes) != ClientError::ERROR_NONE)
    {
        return false;
    }
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

CHIP_ERROR ThreadStackManagerImpl::_SetThreadProvision(ByteSpan netInfo)
{
    VerifyOrReturnError(mThreadApi != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(Thread::OperationalDataset::IsValid(netInfo), CHIP_ERROR_INVALID_ARGUMENT);
    std::vector<uint8_t> data(netInfo.data(), netInfo.data() + netInfo.size());

    ReturnErrorOnFailure(OTBR_TO_CHIP_ERROR(mThreadApi->SetActiveDatasetTlvs(data)));

    // post an event alerting other subsystems about change in provisioning state
    ChipDeviceEvent event;
    event.Type                                           = DeviceEventType::kServiceProvisioningChange;
    event.ServiceProvisioningChange.IsServiceProvisioned = true;
    PlatformMgr().PostEvent(&event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_GetThreadProvision(ByteSpan & netInfo)
{
    std::vector<uint8_t> data(Thread::kSizeOperationalDataset);

    VerifyOrReturnError(mThreadApi != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(OTBR_TO_CHIP_ERROR(mThreadApi->GetActiveDatasetTlvs(data)));
    ReturnErrorOnFailure(mDataset.Init(ByteSpan(data.data(), data.size())));

    netInfo = mDataset.AsByteSpan();

    return CHIP_NO_ERROR;
}

bool ThreadStackManagerImpl::_IsThreadProvisioned()
{
    return static_cast<Thread::OperationalDataset &>(mDataset).IsCommissioned();
}

void ThreadStackManagerImpl::_ErasePersistentInfo()
{
    static_cast<Thread::OperationalDataset &>(mDataset).Clear();
}

bool ThreadStackManagerImpl::_IsThreadEnabled()
{
    bool enabled = false;
    DeviceRole role;
    ClientError error;

    if (mThreadApi == nullptr)
    {
        return false;
    }
    error = mThreadApi->GetDeviceRole(role);
    if (error != ClientError::ERROR_NONE)
    {
        LogClientError(error);
        return false;
    }
    enabled = (role != DeviceRole::OTBR_DEVICE_ROLE_DISABLED);
    return enabled;
}

bool ThreadStackManagerImpl::_IsThreadAttached()
{
    return mAttached;
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadEnabled(bool val)
{
    VerifyOrReturnError(mThreadApi != nullptr, CHIP_ERROR_INCORRECT_STATE);
    if (val)
    {
        ReturnErrorOnFailure(OTBR_TO_CHIP_ERROR(mThreadApi->Attach([](ClientError result) {
            // ThreadDevcieRoleChangedHandler should take care of this, so we don't emit another event.
            ChipLogProgress(DeviceLayer, "Thread attach result %d", static_cast<int>(result));
        })));
    }
    else
    {
        mThreadApi->Reset();
    }
    return CHIP_NO_ERROR;
}

ConnectivityManager::ThreadDeviceType ThreadStackManagerImpl::_GetThreadDeviceType()
{
    DeviceRole role;
    LinkModeConfig linkMode;
    ConnectivityManager::ThreadDeviceType type = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;

    if (mThreadApi == nullptr || mThreadApi->GetDeviceRole(role) != ClientError::ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "Cannot get device role with Thread api client");
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    }

    switch (role)
    {
    case DeviceRole::OTBR_DEVICE_ROLE_DISABLED:
    case DeviceRole::OTBR_DEVICE_ROLE_DETACHED:
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    case DeviceRole::OTBR_DEVICE_ROLE_CHILD:
        if (mThreadApi->GetLinkMode(linkMode) != ClientError::ERROR_NONE)
        {
            ChipLogError(DeviceLayer, "Cannot get link mode with Thread api client");
            return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
        }
        if (!linkMode.mRxOnWhenIdle)
        {
            type = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_SleepyEndDevice;
        }
        else
        {
            type = linkMode.mDeviceType ? ConnectivityManager::ThreadDeviceType::kThreadDeviceType_FullEndDevice
                                        : ConnectivityManager::ThreadDeviceType::kThreadDeviceType_MinimalEndDevice;
        }
        return type;
    case DeviceRole::OTBR_DEVICE_ROLE_ROUTER:
    case DeviceRole::OTBR_DEVICE_ROLE_LEADER:
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_Router;
    default:
        ChipLogError(DeviceLayer, "Unknown Thread role: %d", static_cast<int>(role));
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
        break;
    }
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    LinkModeConfig linkMode{ true, true, true };
    ClientError error = ClientError::ERROR_NONE;

    VerifyOrReturnError(mThreadApi != nullptr, CHIP_ERROR_INCORRECT_STATE);
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
    // TODO: Remove Weave legacy APIs
    // For a leader with a child, the child is considered to have mesh connectivity
    // and the leader is not, which is a very confusing definition.
    // This API is Weave legacy and should be removed.

    ChipLogError(DeviceLayer, "HaveMeshConnectivity has confusing behavior and shouldn't be called");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void ThreadStackManagerImpl::_OnMessageLayerActivityChanged(bool messageLayerIsActive)
{
    (void) messageLayerIsActive;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadStatsCounters()
{
    // TODO: Remove Weave legacy APIs
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadTopologyMinimal()
{
    // TODO: Remove Weave legacy APIs
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetAndLogThreadTopologyFull()
{
    // TODO: Remove Weave legacy APIs
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetPrimary802154MACAddress(uint8_t * buf)
{
    uint64_t extAddr;

    VerifyOrReturnError(mThreadApi != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(OTBR_TO_CHIP_ERROR(mThreadApi->GetExtendedAddress(extAddr)));

    for (size_t i = 0; i < sizeof(extAddr); i++)
    {
        buf[sizeof(uint64_t) - i - 1] = (extAddr & UINT8_MAX);
        extAddr >>= CHAR_BIT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_GetExternalIPv6Address(chip::Inet::IPAddress & addr)
{
    // TODO: Remove Weave legacy APIs
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetPollPeriod(uint32_t & buf)
{
    // TODO: Remove Weave legacy APIs
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_JoinerStart()
{
    // TODO: Remove Weave legacy APIs
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

ThreadStackManager & ThreadStackMgr()
{
    return chip::DeviceLayer::ThreadStackManagerImpl::sInstance;
}

ThreadStackManagerImpl & ThreadStackMgrImpl()
{
    return chip::DeviceLayer::ThreadStackManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
