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
using otbr::DBus::LinkModeConfig;
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
            // The dbus_connection_read_write will lock the connection until new message comes or timeout.
            // This will block ot-br-posix APIs. Set timeout to 10ms so it can work.
            // TODO: we should have a global event loop for dbus to take care of this.
            dbus_connection_read_write_dispatch(dispatchConnection, kDBusConnectionPollingTimeoutMS);
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

CHIP_ERROR ThreadStackManagerImpl::_SetThreadProvision(ByteSpan netInfo)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(Thread::OperationalDataset::IsValid(netInfo), err = CHIP_ERROR_INVALID_ARGUMENT);

    {
        std::vector<uint8_t> data(netInfo.data(), netInfo.data() + netInfo.size());

        SuccessOrExit(err = OTBR_TO_CHIP_ERROR(mThreadApi->SetActiveDatasetTlvs(data)));

        // post an event alerting other subsystems about change in provisioning state
        ChipDeviceEvent event;
        event.Type                                           = DeviceEventType::kServiceProvisioningChange;
        event.ServiceProvisioningChange.IsServiceProvisioned = true;
        PlatformMgr().PostEvent(&event);
    }

exit:
    return err;
}

CHIP_ERROR ThreadStackManagerImpl::_GetThreadProvision(ByteSpan & netInfo)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    std::vector<uint8_t> data(Thread::kSizeOperationalDataset);

    SuccessOrExit(err = OTBR_TO_CHIP_ERROR(mThreadApi->GetActiveDatasetTlvs(data)));
    SuccessOrExit(err = mDataset.Init(ByteSpan(data.data(), data.size())));

    netInfo = mDataset.AsByteSpan();

exit:
    return err;
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
        SuccessOrExit(error = mThreadApi->Attach([](ClientError result) {
            // ThreadDevcieRoleChangedHandler should take care of this, so we don't emit another event.
            ChipLogProgress(DeviceLayer, "Thread attach result %d", result);
        }));
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
        break;
    case DeviceRole::OTBR_DEVICE_ROLE_ROUTER:
    case DeviceRole::OTBR_DEVICE_ROLE_LEADER:
        type = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_Router;
        break;
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
    LinkModeConfig linkMode{ true, true, true };
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

CHIP_ERROR ThreadStackManagerImpl::_GetFactoryAssignedEUI64(uint8_t (&buf)[8])
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetExternalIPv6Address(chip::Inet::IPAddress & addr)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetPollPeriod(uint32_t & buf)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_JoinerStart()
{
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
