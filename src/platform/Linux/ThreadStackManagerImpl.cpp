/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/NetworkCommissioningDriver.h>
#include <platform/PlatformManager.h>
#include <platform/ThreadStackManager.h>

#include <app-common/zap-generated/ids/Attributes.h>

using namespace ::chip::app;
using namespace ::chip::app::Clusters;
using namespace chip::DeviceLayer::NetworkCommissioning;

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

namespace {

struct SetActiveDatasetContext
{
    OpenthreadBorderRouter * proxy;
    ByteSpan netInfo;
};

CHIP_ERROR GLibMatterContextSetActiveDataset(SetActiveDatasetContext * context)
{
    // When creating D-Bus proxy object, the thread default context must be initialized. Otherwise,
    // all D-Bus signals will be delivered to the GLib global default main context.
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    GAutoPtr<GBytes> bytes(g_bytes_new(context->netInfo.data(), context->netInfo.size()));
    if (!bytes)
        return CHIP_ERROR_NO_MEMORY;
    GAutoPtr<GVariant> value(g_variant_new_from_bytes(G_VARIANT_TYPE_BYTESTRING, bytes.release(), true));
    if (!value)
        return CHIP_ERROR_NO_MEMORY;
    openthread_border_router_set_active_dataset_tlvs(context->proxy, value.release());
    return CHIP_NO_ERROR;
}

} // namespace

ThreadStackManagerImpl::ThreadStackManagerImpl() : mAttached(false) {}

CHIP_ERROR ThreadStackManagerImpl::GLibMatterContextInitThreadStack(ThreadStackManagerImpl * self)
{
    // When creating D-Bus proxy object, the thread default context must be initialized. Otherwise,
    // all D-Bus signals will be delivered to the GLib global default main context.
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);

    GAutoPtr<GError> err;
    self->mProxy.reset(openthread_border_router_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_PROXY_FLAGS_NONE,
                                                                       kDBusOpenThreadService, kDBusOpenThreadObjectPath, nullptr,
                                                                       &err.GetReceiver()));
    VerifyOrReturnError(
        self->mProxy != nullptr, CHIP_ERROR_INTERNAL,
        ChipLogError(DeviceLayer, "openthread: failed to create openthread dbus proxy %s", err ? err->message : "unknown error"));

    g_signal_connect(self->mProxy.get(), "g-properties-changed", G_CALLBACK(OnDbusPropertiesChanged), self);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack()
{
    CHIP_ERROR err;

    err = PlatformMgrImpl().GLibMatterContextInvokeSync(GLibMatterContextInitThreadStack, this);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "openthread: failed to init dbus proxy"));

    // If get property is called inside dbus thread (we are going to make it so), XXX_get_XXX can be used instead of XXX_dup_XXX
    // which is a little bit faster and the returned object doesn't need to be freed. Same for all following get properties.
    GAutoPtr<char> role(openthread_border_router_dup_device_role(mProxy.get()));
    if (role)
    {
        ThreadDeviceRoleChangedHandler(role.get());
    }

    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::OnDbusPropertiesChanged(OpenthreadBorderRouter * proxy, GVariant * changed_properties,
                                                     const gchar * const * invalidated_properties, gpointer user_data)
{
    ThreadStackManagerImpl * me = reinterpret_cast<ThreadStackManagerImpl *>(user_data);
    if (g_variant_n_children(changed_properties) > 0)
    {
        const gchar * key;
        GVariant * value;

        GAutoPtr<GVariantIter> iter;
        g_variant_get(changed_properties, "a{sv}", &iter.GetReceiver());
        if (!iter)
            return;
        while (g_variant_iter_loop(iter.get(), "{&sv}", &key, &value))
        {
            if (key == nullptr || value == nullptr)
                continue;
            // ownership of key and value is still holding by the iter
            DeviceLayer::SystemLayer().ScheduleLambda([me]() { me->_UpdateNetworkStatus(); });

            if (strcmp(key, kPropertyDeviceRole) == 0)
            {
                const gchar * value_str = g_variant_get_string(value, nullptr);
                if (value_str == nullptr)
                    continue;
                ChipLogProgress(DeviceLayer, "Thread role changed to: %s", StringOrNullMarker(value_str));
                me->ThreadDeviceRoleChangedHandler(value_str);
            }
        }
    }
}

void ThreadStackManagerImpl::ThreadDeviceRoleChangedHandler(const gchar * role)
{
    bool attached = strcmp(role, kOpenthreadDeviceRoleDetached) != 0 && strcmp(role, kOpenthreadDeviceRoleDisabled) != 0;

    if (attached != mAttached)
    {
        ChipDeviceEvent event{ .Type                     = DeviceEventType::kThreadConnectivityChange,
                               .ThreadConnectivityChange = { .Result = attached ? ConnectivityChange::kConnectivity_Established
                                                                                : ConnectivityChange::kConnectivity_Lost } };
        CHIP_ERROR status = PlatformMgr().PostEvent(&event);
        if (status != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Failed to post thread connectivity change: %" CHIP_ERROR_FORMAT, status.Format());
        }
    }
    mAttached = attached;

    ChipDeviceEvent event{ .Type = DeviceEventType::kThreadStateChange, .ThreadStateChange = { .RoleChanged = true } };
    CHIP_ERROR status = PlatformMgr().PostEvent(&event);
    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to post thread state change: %" CHIP_ERROR_FORMAT, status.Format());
    }
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

    GAutoPtr<GVariant> routes(openthread_border_router_dup_external_routes(mProxy.get()));
    if (!routes)
        return false;

    if (g_variant_n_children(routes.get()) > 0)
    {
        GAutoPtr<GVariantIter> iter;
        g_variant_get(routes.get(), "av", &iter.GetReceiver());
        if (!iter)
            return false;

        GVariant * route;
        while (g_variant_iter_loop(iter.get(), "&v", &route))
        {
            if (route == nullptr)
                continue;
            GAutoPtr<GVariant> prefix;
            guint16 rloc16;
            guchar preference;
            gboolean stable;
            gboolean nextHopIsThisDevice;
            g_variant_get(route, "(&vqybb)", &prefix.GetReceiver(), &rloc16, &preference, &stable, &nextHopIsThisDevice);
            if (!prefix)
                continue;

            GAutoPtr<GVariant> address;
            guchar prefixLength;
            g_variant_get(prefix.get(), "(&vy)", &address.GetReceiver(), &prefixLength);
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
            p.IPAddr = Inet::IPAddress(*reinterpret_cast<const struct in6_addr *>(data));
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

    SetActiveDatasetContext context = { mProxy.get(), netInfo };
    CHIP_ERROR err                  = PlatformMgrImpl().GLibMatterContextInvokeSync(GLibMatterContextSetActiveDataset, &context);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "openthread: failed to set active dataset"));

    // post an event alerting other subsystems about change in provisioning state
    ChipDeviceEvent event{ .Type                      = DeviceEventType::kServiceProvisioningChange,
                           .ServiceProvisioningChange = { .IsServiceProvisioned = true } };
    return PlatformMgr().PostEvent(&event);
}

CHIP_ERROR ThreadStackManagerImpl::_GetThreadProvision(Thread::OperationalDataset & dataset)
{
    VerifyOrReturnError(mProxy, CHIP_ERROR_INCORRECT_STATE);

    {
        GAutoPtr<GError> err;
        GAutoPtr<GVariant> response(g_dbus_proxy_call_sync(G_DBUS_PROXY(mProxy.get()), "org.freedesktop.DBus.Properties.Get",
                                                           g_variant_new("(ss)", "io.openthread.BorderRouter", "ActiveDatasetTlvs"),
                                                           G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &err.GetReceiver()));

        if (err)
        {
            ChipLogError(DeviceLayer, "openthread: failed to read ActiveDatasetTlvs property: %s", err->message);
            return CHIP_ERROR_INTERNAL;
        }

        // Note: The actual value is wrapped by a GVariant container, wrapped in another GVariant with tuple type.

        if (response == nullptr)
        {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }

        GAutoPtr<GVariant> tupleContent(g_variant_get_child_value(response.get(), 0));

        if (tupleContent == nullptr)
        {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }

        GAutoPtr<GVariant> value(g_variant_get_variant(tupleContent.get()));

        if (value == nullptr)
        {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }

        gsize size;
        const uint8_t * data = reinterpret_cast<const uint8_t *>(g_variant_get_fixed_array(value.get(), &size, sizeof(guchar)));
        ReturnErrorOnFailure(mDataset.Init(ByteSpan(data, size)));
    }

    dataset.Init(mDataset.AsByteSpan());

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
    VerifyOrReturnError(mProxy, false);

    GAutoPtr<GError> err;
    GAutoPtr<GVariant> response(g_dbus_proxy_call_sync(G_DBUS_PROXY(mProxy.get()), "org.freedesktop.DBus.Properties.Get",
                                                       g_variant_new("(ss)", "io.openthread.BorderRouter", "DeviceRole"),
                                                       G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &err.GetReceiver()));

    if (err)
    {
        ChipLogError(DeviceLayer, "openthread: failed to read DeviceRole property: %s", err->message);
        return false;
    }

    if (response == nullptr)
    {
        return false;
    }

    GAutoPtr<GVariant> tupleContent(g_variant_get_child_value(response.get(), 0));

    if (tupleContent == nullptr)
    {
        return false;
    }

    GAutoPtr<GVariant> value(g_variant_get_variant(tupleContent.get()));

    if (value == nullptr)
    {
        return false;
    }

    const gchar * role = g_variant_get_string(value.get(), nullptr);

    if (role == nullptr)
    {
        return false;
    }

    return (strcmp(role, kOpenthreadDeviceRoleDisabled) != 0);
}

bool ThreadStackManagerImpl::_IsThreadAttached() const
{
    return mAttached;
}

CHIP_ERROR ThreadStackManagerImpl::GLibMatterContextCallAttach(ThreadStackManagerImpl * self)
{
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);
    openthread_border_router_call_attach(self->mProxy.get(), nullptr, _OnThreadBrAttachFinished, self);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_SetThreadEnabled(bool val)
{
    VerifyOrReturnError(mProxy, CHIP_ERROR_INCORRECT_STATE);
    if (val)
    {
        CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(GLibMatterContextCallAttach, this);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "openthread: failed to attach"));
    }
    else
    {
        GAutoPtr<GError> err;
        gboolean result = openthread_border_router_call_reset_sync(mProxy.get(), nullptr, &err.GetReceiver());
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

void ThreadStackManagerImpl::_OnThreadBrAttachFinished(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    ThreadStackManagerImpl * this_ = reinterpret_cast<ThreadStackManagerImpl *>(user_data);
    GAutoPtr<GVariant> attachRes;
    GAutoPtr<GError> err;
    {
        gboolean result = openthread_border_router_call_attach_finish(this_->mProxy.get(), res, &err.GetReceiver());
        if (!result)
        {
            ChipLogError(DeviceLayer, "Failed to perform finish Thread network scan: %s",
                         err == nullptr ? "unknown error" : err->message);
            DeviceLayer::SystemLayer().ScheduleLambda([this_]() {
                if (this_->mpConnectCallback != nullptr)
                {
                    // TODO: Replace this with actual thread attach result.
                    this_->mpConnectCallback->OnResult(NetworkCommissioning::Status::kUnknownError, CharSpan(), 0);
                    this_->mpConnectCallback = nullptr;
                }
            });
        }
        else
        {
            DeviceLayer::SystemLayer().ScheduleLambda([this_]() {
                if (this_->mpConnectCallback != nullptr)
                {
                    // TODO: Replace this with actual thread attach result.
                    this_->mpConnectCallback->OnResult(NetworkCommissioning::Status::kSuccess, CharSpan(), 0);
                    this_->mpConnectCallback = nullptr;
                }
            });
        }
    }
}

ConnectivityManager::ThreadDeviceType ThreadStackManagerImpl::_GetThreadDeviceType()
{
    ConnectivityManager::ThreadDeviceType type = ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    if (!mProxy)
    {
        ChipLogError(DeviceLayer, "Cannot get device role with Thread api client: %s", "");
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    }

    GAutoPtr<char> role(openthread_border_router_dup_device_role(mProxy.get()));
    if (!role)
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    if (strcmp(role.get(), kOpenthreadDeviceRoleDetached) == 0 || strcmp(role.get(), kOpenthreadDeviceRoleDisabled) == 0)
    {
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
    }
    if (strcmp(role.get(), kOpenthreadDeviceRoleChild) == 0)
    {
        GAutoPtr<GVariant> linkMode(openthread_border_router_dup_link_mode(mProxy.get()));
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
    if (strcmp(role.get(), kOpenthreadDeviceRoleLeader) == 0 || strcmp(role.get(), kOpenthreadDeviceRoleRouter) == 0)
    {
        return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_Router;
    }

    ChipLogError(DeviceLayer, "Unknown Thread role: %s", role.get());
    return ConnectivityManager::ThreadDeviceType::kThreadDeviceType_NotSupported;
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
        GAutoPtr<GVariant> linkMode(g_variant_new("(bbb)", rx_on_when_idle, device_type, network_data));
        if (!linkMode)
            return CHIP_ERROR_NO_MEMORY;
        openthread_border_router_set_link_mode(mProxy.get(), linkMode.release());
    }

    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_ENABLE_ICD_SERVER
CHIP_ERROR ThreadStackManagerImpl::_SetPollingInterval(System::Clock::Milliseconds32 pollingInterval)
{
    (void) pollingInterval;
    ChipLogError(DeviceLayer, "Set ICD Polling on linux");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#endif /* CHIP_CONFIG_ENABLE_ICD_SERVER */

bool ThreadStackManagerImpl::_HaveMeshConnectivity()
{
    // TODO: Remove Weave legacy APIs
    // For a leader with a child, the child is considered to have mesh connectivity
    // and the leader is not, which is a very confusing definition.
    // This API is Weave legacy and should be removed.

    ChipLogError(DeviceLayer, "HaveMeshConnectivity has confusing behavior and shouldn't be called");
    return false;
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
    guint64 extAddr = openthread_border_router_get_extended_address(mProxy.get());

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

CHIP_ERROR ThreadStackManagerImpl::_GetThreadVersion(uint16_t & version)
{
    // TODO https://github.com/project-chip/connectedhomeip/issues/30602
    // Needs to be implemented with DBUS io.openthread.BorderRouter Thread API
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::_GetPollPeriod(uint32_t & buf)
{
    // TODO: Remove Weave legacy APIs
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadStackManagerImpl::GLibMatterContextCallScan(ThreadStackManagerImpl * self)
{
    VerifyOrDie(g_main_context_get_thread_default() != nullptr);
    openthread_border_router_call_scan(self->mProxy.get(), nullptr, _OnNetworkScanFinished, self);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadScan(ThreadDriver::ScanCallback * callback)
{
    // There is another ongoing scan request, reject the new one.
    VerifyOrReturnError(mpScanCallback == nullptr, CHIP_ERROR_INCORRECT_STATE);
    mpScanCallback = callback;
    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(GLibMatterContextCallScan, this);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "openthread: failed to start scan"));
    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::_OnNetworkScanFinished(GObject * source_object, GAsyncResult * res, gpointer user_data)
{
    ThreadStackManagerImpl * this_ = reinterpret_cast<ThreadStackManagerImpl *>(user_data);
    this_->_OnNetworkScanFinished(res);
}

void ThreadStackManagerImpl::_OnNetworkScanFinished(GAsyncResult * res)
{
    GAutoPtr<GVariant> scan_result;
    GAutoPtr<GError> err;
    {
        gboolean result =
            openthread_border_router_call_scan_finish(mProxy.get(), &scan_result.GetReceiver(), res, &err.GetReceiver());
        if (!result)
        {
            ChipLogError(DeviceLayer, "Failed to perform finish Thread network scan: %s",
                         err == nullptr ? "unknown error" : err->message);
            DeviceLayer::SystemLayer().ScheduleLambda([this]() {
                if (mpScanCallback != nullptr)
                {
                    LinuxScanResponseIterator<ThreadScanResponse> iter(nullptr);
                    mpScanCallback->OnFinished(Status::kUnknownError, CharSpan(), &iter);
                }
                mpScanCallback = nullptr;
            });
        }
    }

    std::vector<NetworkCommissioning::ThreadScanResponse> * scanResult =
        new std::vector<NetworkCommissioning::ThreadScanResponse>();

    if (g_variant_n_children(scan_result.get()) > 0)
    {
        GAutoPtr<GVariantIter> iter;
        g_variant_get(scan_result.get(), "a(tstayqqynyybb)", &iter.GetReceiver());
        if (!iter)
        {
            delete scanResult;
            return;
        }

        guint64 ext_address;
        const gchar * network_name;
        guint64 ext_panid;
        const gchar * steering_data;
        guint16 panid;
        guint16 joiner_udp_port;
        guint8 channel;
        gint16 rssi;
        guint8 lqi;
        guint8 version;
        gboolean is_native;
        gboolean is_joinable;

        while (g_variant_iter_loop(iter.get(), "(tstayqqynyybb)", &ext_address, &network_name, &ext_panid, &steering_data, &panid,
                                   &joiner_udp_port, &channel, &rssi, &lqi, &version, &is_native, &is_joinable))
        {
            ChipLogProgress(DeviceLayer,
                            "Thread Network: %s (" ChipLogFormatX64 ") ExtPanId(" ChipLogFormatX64 ") RSSI %d LQI %u"
                            " Version %u",
                            network_name, ChipLogValueX64(ext_address), ChipLogValueX64(ext_panid), rssi, lqi, version);
            NetworkCommissioning::ThreadScanResponse networkScanned;
            networkScanned.panId         = panid;
            networkScanned.extendedPanId = ext_panid;
            size_t networkNameLen        = strlen(network_name);
            if (networkNameLen > 16)
            {
                ChipLogProgress(DeviceLayer, "Network name is too long, ignore it.");
                continue;
            }
            networkScanned.networkNameLen = static_cast<uint8_t>(networkNameLen);
            memcpy(networkScanned.networkName, network_name, networkNameLen);
            networkScanned.channel         = channel;
            networkScanned.version         = version;
            networkScanned.extendedAddress = 0;
            if (rssi > std::numeric_limits<int8_t>::max())
            {
                networkScanned.rssi = std::numeric_limits<int8_t>::max();
            }
            else if (rssi < std::numeric_limits<int8_t>::min())
            {
                networkScanned.rssi = std::numeric_limits<int8_t>::min();
            }
            else
            {
                networkScanned.rssi = static_cast<int8_t>(rssi);
            }
            networkScanned.lqi = lqi;

            scanResult->push_back(networkScanned);
        }
    }

    DeviceLayer::SystemLayer().ScheduleLambda([this, scanResult]() {
        // Note: We cannot post a event in ScheduleLambda since std::vector is not trivial copiable. This results in the use of
        // const_cast but should be fine for almost all cases, since we actually handled the ownership of this element to this
        // lambda.
        if (mpScanCallback != nullptr)
        {
            LinuxScanResponseIterator<NetworkCommissioning::ThreadScanResponse> iter(
                const_cast<std::vector<ThreadScanResponse> *>(scanResult));
            mpScanCallback->OnFinished(Status::kSuccess, CharSpan(), &iter);
            mpScanCallback = nullptr;
        }
        delete const_cast<std::vector<ThreadScanResponse> *>(scanResult);
    });
}

void ThreadStackManagerImpl::_ResetThreadNetworkDiagnosticsCounts() {}

CHIP_ERROR
ThreadStackManagerImpl::_AttachToThreadNetwork(const Thread::OperationalDataset & dataset,
                                               NetworkCommissioning::Internal::WirelessDriver::ConnectCallback * callback)
{
    // Reset the previously set callback since it will never be called in case incorrect dataset was supplied.
    mpConnectCallback = nullptr;
    ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().SetThreadEnabled(false));
    ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().SetThreadProvision(dataset.AsByteSpan()));

    if (dataset.IsCommissioned())
    {
        ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().SetThreadEnabled(true));
        mpConnectCallback = callback;
    }

    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::_UpdateNetworkStatus()
{
    // Thread is not enabled, then we are not trying to connect to the network.
    VerifyOrReturn(IsThreadEnabled() && mpStatusChangeCallback != nullptr);

    Thread::OperationalDataset dataset;
    uint8_t extpanid[Thread::kSizeExtendedPanId];

    // If we have not provisioned any Thread network, return the status from last network scan,
    // If we have provisioned a network, we assume the ot-br-posix is activitely connecting to that network.
    CHIP_ERROR err = ThreadStackMgrImpl().GetThreadProvision(dataset);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to get configured network when updating network status: %s", err.AsString());
        return;
    }

    // The Thread network is not enabled, but has a different extended pan id.
    VerifyOrReturn(dataset.GetExtendedPanId(extpanid) == CHIP_NO_ERROR);

    // We have already connected to the network, thus return success.
    if (ThreadStackMgrImpl().IsThreadAttached())
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(Status::kSuccess, MakeOptional(ByteSpan(extpanid)), NullOptional);
    }
    else
    {
        mpStatusChangeCallback->OnNetworkingStatusChange(Status::kNetworkNotFound, MakeOptional(ByteSpan(extpanid)), NullOptional);
    }
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
