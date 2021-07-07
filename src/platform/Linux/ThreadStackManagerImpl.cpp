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

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/DeviceNetworkInfo.h>

#include <platform/PlatformManager.h>
#include <platform/ThreadStackManager.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

constexpr char ThreadStackManagerImpl::kDBusOpenThreadService[];
constexpr char ThreadStackManagerImpl::kDBusOpenThreadObjectPath[];

constexpr char ThreadStackManagerImpl::kOpenthreadDeviceRoleDisabled[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceRoleDetached[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceRoleChild[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceRoleRouter[];
constexpr char ThreadStackManagerImpl::kOpenthreadDeviceRoleLeader[];

constexpr char ThreadStackManagerImpl::kPropertyDeviceRole[];

ThreadStackManagerImpl::ThreadStackManagerImpl() : mAttached(false) {}

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    std::unique_ptr<GError, GErrorDeleter> err;
    mProxy.reset(openthread_io_openthread_border_router_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                                               kDBusOpenThreadService, kDBusOpenThreadObjectPath,
                                                                               nullptr, &MakeUniquePointerReceiver(err).Get()));
    if (!mProxy)
    {
        ChipLogError(DeviceLayer, "openthread: failed to create openthread dbus proxy %s", err ? err->message : "unknown error");
        return CHIP_ERROR_INTERNAL;
    }

    g_signal_connect(mProxy.get(), "g-properties-changed", G_CALLBACK(OnDbusPropertiesChanged), this);

    // If get property is called inside dbus thread (we are going to make it so), XXX_get_XXX can be used instead of XXX_dup_XXX
    // which is a little bit faster and the returned object doesn't need to be freed. Same for all following get properties.
    std::unique_ptr<gchar, GFree> role(openthread_io_openthread_border_router_dup_device_role(mProxy.get()));
    if (role)
    {
        ThreadDevcieRoleChangedHandler(role.get());
    }

    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::OnDbusPropertiesChanged(OpenthreadIoOpenthreadBorderRouter * proxy, GVariant * changed_properties,
                                                     const gchar * const * invalidated_properties, gpointer user_data)
{
    ThreadStackManagerImpl * me = reinterpret_cast<ThreadStackManagerImpl *>(user_data);
    if (g_variant_n_children(changed_properties) > 0)
    {
        const gchar * key;
        GVariant * value;

        std::unique_ptr<GVariantIter, GVariantIterDeleter> iter;
        g_variant_get(changed_properties, "a{sv}", &MakeUniquePointerReceiver(iter).Get());
        if (!iter)
            return;
        while (g_variant_iter_loop(iter.get(), "{&sv}", &key, &value))
        {
            if (key == nullptr || value == nullptr)
                continue;
            // ownership of key and value is still holding by the iter
            if (strcmp(key, kPropertyDeviceRole) == 0)
            {
                const gchar * value_str = g_variant_get_string(value, nullptr);
                if (value_str == nullptr)
                    continue;
                ChipLogProgress(DeviceLayer, "Thread role changed to: %s", value_str);
                me->ThreadDevcieRoleChangedHandler(value_str);
            }
        }
    }
}

void ThreadStackManagerImpl::ThreadDevcieRoleChangedHandler(const gchar * role)
{
    bool attached = strcmp(role, kOpenthreadDeviceRoleDetached) != 0 && strcmp(role, kOpenthreadDeviceRoleDisabled) != 0;

    ChipDeviceEvent event = ChipDeviceEvent{};

    if (attached != mAttached)
    {
        event.Type = DeviceEventType::kThreadConnectivityChange;
        event.ThreadConnectivityChange.Result =
            attached ? ConnectivityChange::kConnectivity_Established : ConnectivityChange::kConnectivity_Lost;
        PlatformMgr().PostEvent(&event);
    }
    mAttached = attached;

    event.Type                          = DeviceEventType::kThreadStateChange;
    event.ThreadStateChange.RoleChanged = true;
    PlatformMgr().PostEvent(&event);
}

void ThreadStackManagerImpl::_ProcessThreadActivity() {}

bool ThreadStackManagerImpl::_HaveRouteToAddress(const Inet::IPAddress & destAddr)
{
    if (!mProxy || !_IsThreadAttached())
    {
        return false;
    }
    if (destAddr.IsIPv6LinkLocal())
    {
        return true;
    }

    std::unique_ptr<GVariant, GVariantDeleter> routes(openthread_io_openthread_border_router_dup_external_routes(mProxy.get()));
    if (!routes)
        return false;

    if (g_variant_n_children(routes.get()) > 0)
    {
        std::unique_ptr<GVariantIter, GVariantIterDeleter> iter;
        g_variant_get(routes.get(), "av", &MakeUniquePointerReceiver(iter).Get());
        if (!iter)
            return false;

        GVariant * route;
        while (g_variant_iter_loop(iter.get(), "&v", &route))
        {
            if (route == nullptr)
                continue;
            std::unique_ptr<GVariant, GVariantDeleter> prefix;
            guint16 rloc16;
            guchar preference;
            gboolean stable;
            gboolean nextHopIsThisDevice;
            g_variant_get(route, "(&vqybb)", &MakeUniquePointerReceiver(prefix).Get(), &rloc16, &preference, &stable,
                          &nextHopIsThisDevice);
            if (!prefix)
                continue;

            std::unique_ptr<GVariant, GVariantDeleter> address;
            guchar prefixLength;
            g_variant_get(prefix.get(), "(&vy)", &MakeUniquePointerReceiver(address).Get(), &prefixLength);
            if (!address)
                continue;

            GBytes * bytes = g_variant_get_data_as_bytes(address.get()); // the ownership still hold by address
            if (bytes == nullptr)
                continue;
            gsize size;
            gconstpointer data = g_bytes_get_data(bytes, &size);
            if (data == nullptr)
                continue;
            if (size != sizeof(struct in6_addr))
                continue;

            Inet::IPPrefix p;
            p.IPAddr = Inet::IPAddress::FromIPv6(*reinterpret_cast<const struct in6_addr *>(data));
            p.Length = prefixLength;

            if (p.MatchAddress(destAddr))
            {
                return true;
            }
        }
    }

    return false;
}

void ThreadStackManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    (void) event;
    // The otbr-agent processes the Thread state handling by itself so there
    // isn't much to do in the Chip stack.
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadProvision(ByteSpan netInfo)
{
    VerifyOrReturnError(mProxy, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(Thread::OperationalDataset::IsValid(netInfo), CHIP_ERROR_INVALID_ARGUMENT);

    {
        std::unique_ptr<GBytes, GBytesDeleter> bytes(g_bytes_new(netInfo.data(), netInfo.size()));
        if (!bytes)
            return CHIP_ERROR_NO_MEMORY;
        std::unique_ptr<GVariant, GVariantDeleter> value(
            g_variant_new_from_bytes(G_VARIANT_TYPE_BYTESTRING, bytes.release(), true));
        if (!value)
            return CHIP_ERROR_NO_MEMORY;
        openthread_io_openthread_border_router_set_active_dataset_tlvs(mProxy.get(), value.release());
    }

    // post an event alerting other subsystems about change in provisioning state
    ChipDeviceEvent event;
    event.Type                                           = DeviceEventType::kServiceProvisioningChange;
    event.ServiceProvisioningChange.IsServiceProvisioned = true;
    PlatformMgr().PostEvent(&event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_GetThreadProvision(ByteSpan & netInfo)
{
    VerifyOrReturnError(mProxy, CHIP_ERROR_INCORRECT_STATE);

    {
        std::unique_ptr<GVariant, GVariantDeleter> value(
            openthread_io_openthread_border_router_dup_active_dataset_tlvs(mProxy.get()));
        GBytes * bytes = g_variant_get_data_as_bytes(value.get());
        gsize size;
        const uint8_t * data = reinterpret_cast<const uint8_t *>(g_bytes_get_data(bytes, &size));
        ReturnErrorOnFailure(mDataset.Init(ByteSpan(data, size)));
    }

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
    if (!mProxy)
    {
        return false;
    }

    std::unique_ptr<gchar, GFree> role(openthread_io_openthread_border_router_dup_device_role(mProxy.get()));
    return (strcmp(role.get(), kOpenthreadDeviceRoleDisabled) != 0);
}

bool ThreadStackManagerImpl::_IsThreadAttached()
{
    return mAttached;
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadEnabled(bool val)
{
    VerifyOrReturnError(mProxy, CHIP_ERROR_INCORRECT_STATE);
    if (val)
    {
        std::unique_ptr<GError, GErrorDeleter> err;
        gboolean result =
            openthread_io_openthread_border_router_call_attach_sync(mProxy.get(), nullptr, &MakeUniquePointerReceiver(err).Get());
        if (err)
        {
            ChipLogError(DeviceLayer, "openthread: _SetThreadEnabled calling %s failed: %s", "Attach", err->message);
            return CHIP_ERROR_INTERNAL;
        }

        if (!result)
        {
            ChipLogError(DeviceLayer, "openthread: _SetThreadEnabled calling %s failed: %s", "Attach", "return false");
            return CHIP_ERROR_INTERNAL;
        }
    }
    else
    {
        std::unique_ptr<GError, GErrorDeleter> err;
        gboolean result =
            openthread_io_openthread_border_router_call_reset_sync(mProxy.get(), nullptr, &MakeUniquePointerReceiver(err).Get());
        if (err)
        {
            ChipLogError(DeviceLayer, "openthread: _SetThreadEnabled calling %s failed: %s", "Reset", err->message);
            return CHIP_ERROR_INTERNAL;
        }

        if (!result)
        {
            ChipLogError(DeviceLayer, "openthread: _SetThreadEnabled calling %s failed: %s", "Reset", "return false");
            return CHIP_ERROR_INTERNAL;
        }
    }
    return CHIP_NO_ERROR;
}

ConnectivityManager::ThreadDeviceType ThreadStackManagerImpl::_GetThreadDeviceType()
{
    ConnectivityManager::ThreadDeviceType type = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    if (!mProxy)
    {
        ChipLogError(DeviceLayer, "Cannot get device role with Thread api client: %s", "");
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    }

    std::unique_ptr<gchar, GFree> role(openthread_io_openthread_border_router_dup_device_role(mProxy.get()));
    if (!role)
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    if (strcmp(role.get(), kOpenthreadDeviceRoleDetached) == 0 || strcmp(role.get(), kOpenthreadDeviceRoleDisabled) == 0)
    {
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    }
    else if (strcmp(role.get(), kOpenthreadDeviceRoleChild) == 0)
    {
        std::unique_ptr<GVariant, GVariantDeleter> linkMode(openthread_io_openthread_border_router_dup_link_mode(mProxy.get()));
        if (!linkMode)
            return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
        gboolean rx_on_when_idle;
        gboolean device_type;
        gboolean network_data;
        g_variant_get(linkMode.get(), "(bbb)", &rx_on_when_idle, &device_type, &network_data);
        if (!rx_on_when_idle)
        {
            type = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_SleepyEndDevice;
        }
        else
        {
            type = device_type ? ConnectivityManager::ThreadDeviceType::kThreadDeviceType_FullEndDevice
                               : ConnectivityManager::ThreadDeviceType::kThreadDeviceType_MinimalEndDevice;
        }
        return type;
    }
    else if (strcmp(role.get(), kOpenthreadDeviceRoleLeader) == 0 || strcmp(role.get(), kOpenthreadDeviceRoleRouter) == 0)
    {
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_Router;
    }
    else
    {
        ChipLogError(DeviceLayer, "Unknown Thread role: %s", role.get());
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    }
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadDeviceType(ConnectivityManager::ThreadDeviceType deviceType)
{
    gboolean rx_on_when_idle = true;
    gboolean device_type     = true;
    gboolean network_data    = true;
    VerifyOrReturnError(mProxy, CHIP_ERROR_INCORRECT_STATE);
    if (deviceType == ConnectivityManager::ThreadDeviceType::kThreadDeviceType_MinimalEndDevice)
    {
        network_data = false;
    }
    else if (deviceType == ConnectivityManager::ThreadDeviceType::kThreadDeviceType_SleepyEndDevice)
    {
        rx_on_when_idle = false;
        network_data    = false;
    }

    if (!network_data)
    {
        std::unique_ptr<GVariant, GVariantDeleter> linkMode(g_variant_new("(bbb)", rx_on_when_idle, device_type, network_data));
        if (!linkMode)
            return CHIP_ERROR_NO_MEMORY;
        openthread_io_openthread_border_router_set_link_mode(mProxy.get(), linkMode.release());
    }

    return CHIP_NO_ERROR;
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
    VerifyOrReturnError(mProxy, CHIP_ERROR_INCORRECT_STATE);
    guint64 extAddr = openthread_io_openthread_border_router_get_extended_address(mProxy.get());

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
