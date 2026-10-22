#include <pw_unit_test/framework.h>

#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/tests/DeviceCommissionerTestAccess.h>
#include <controller/tests/SetUpCodePairerTestAccess.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <crypto/RawKeySessionKeystore.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/CHIPDeviceLayer.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#if CONFIG_NETWORK_LAYER_BLE && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <ble/Ble.h>
#include <platform/Linux/BLEManagerImpl.h>
#include <platform/Linux/bluez/BluezConnection.h>

namespace chip::DeviceLayer::Internal {
class BLEManagerImplTestAccess
{
public:
    static Ble::BleLayer & Layer(BLEManagerImpl & manager) { return manager; }
    static CHIP_ERROR InstallBackend(BLEManagerImpl & manager)
    {
        manager.Ble::BleLayer::Shutdown();
        return manager.Ble::BleLayer::Init(&manager, &manager, &manager, &SystemLayer());
    }

    static void Request(BLEManagerImpl & manager, unsigned & callbacks)
    {
        manager.mServiceMode                                     = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
        manager.Ble::BleConnectionDelegate::OnConnectionComplete = [](void * context, BLE_CONNECTION_OBJECT) {
            ++*static_cast<unsigned *>(context);
        };
        manager.Ble::BleConnectionDelegate::OnConnectionError = [](void * context, CHIP_ERROR) {
            ++*static_cast<unsigned *>(context);
        };
        manager.NewConnection(nullptr, &callbacks, SetupDiscriminator());
    }
    static CHIP_ERROR Cancel(BLEManagerImpl & manager) { return manager.CancelConnection(); }
    static void InstallScanningAdapter(BLEManagerImpl & manager, BluezAdapter1 * adapter)
    {
        auto & scanner = manager.mDeviceScanner;
        ASSERT_EQ(scanner.Init(adapter, &manager), CHIP_NO_ERROR);
        ASSERT_EQ(manager.mBluezObjectManager.SubscribeDeviceNotifications(adapter, &scanner), CHIP_NO_ERROR);
        scanner.mCancellable.reset(g_cancellable_new());
        scanner.mScannerState                = ChipDeviceScanner::ChipDeviceScannerState::SCANNING;
        manager.mBLEScanConfig.mBleScanState = BleScanState::kScanForDiscriminator;
    }
    static bool ScannerActive(BLEManagerImpl & manager) { return manager.mDeviceScanner.IsScanning(); }
    static void Scanning(BLEManagerImpl & manager)
    {
        ++manager.mScanRequestId;
        manager.mIsCentral                   = true;
        manager.mBLEScanConfig.mBleScanState = BleScanState::kScanForDiscriminator;
    }
    static CHIP_ERROR StartScan(BLEManagerImpl & manager, BluezAdapter1 * adapter)
    {
        Scanning(manager);
        ReturnErrorOnFailure(manager.mDeviceScanner.Init(adapter, &manager));
        return manager.mDeviceScanner.StartScan();
    }
    static void Scanned(BLEManagerImpl & manager, BluezDevice1 & device, const Ble::ChipBLEDeviceIdentificationInfo & info)
    {
        manager.OnDeviceScanned(device, info);
    }
    static bool IsConnecting(BLEManagerImpl & manager) { return manager.mBLEScanConfig.mBleScanState == BleScanState::kConnecting; }
    static BluezEndpoint & Connecting(BLEManagerImpl & manager, BluezDevice1 * device = nullptr)
    {
        uint64_t requestId;
        {
            StackLock lock;
            requestId = ++manager.mScanRequestId;
            manager.mEndpoint.PrepareConnect();
            manager.mIsCentral                   = true;
            manager.mBLEScanConfig.mBleScanState = BleScanState::kConnecting;
        }
        if (device != nullptr)
        {
            EXPECT_EQ(manager.mEndpoint.ConnectDevice(*device, requestId), CHIP_NO_ERROR);
        }
        return manager.mEndpoint;
    }
    static void RestorePeripheral(BLEManagerImpl & manager) { manager.mIsCentral = false; }
    static GCancellable * ConnectCancellable(BluezEndpoint & endpoint) { return endpoint.mConnectCancellable.get(); }
    static BluezEndpoint & InstallGatt(BLEManagerImpl & manager, GDBusObjectManager * objects)
    {
        manager.mBluezObjectManager.mObjectManager.reset(G_DBUS_OBJECT_MANAGER(g_object_ref(objects)));
        manager.mEndpoint.mIsCentral = true;
        return manager.mEndpoint;
    }
    static void RemoveGatt(BLEManagerImpl & manager)
    {
        manager.mBluezObjectManager.mObjectManager.reset();
        manager.mEndpoint.mIsCentral = false;
        manager.mIsCentral           = false;
    }
    static uint64_t PendingTicket(BluezEndpoint & endpoint)
    {
        return endpoint.mConnMap.empty() ? 0 : endpoint.mConnMap.begin()->second.mPendingTicket;
    }
    static ChipDeviceEvent PendingEvent(BLEManagerImpl & manager)
    {
        return { .Type     = DeviceEventType::kPlatformLinuxBLECentralConnected,
                 .Platform = { .BLECentralConnected = { .mPendingTicket = PendingTicket(manager.mEndpoint),
                                                        .mRequestId     = manager.mScanRequestId.load() } } };
    }
    static void ExpireConnect(BLEManagerImpl & manager)
    {
        StackLock lock;
        BLEManagerImpl::HandleConnectTimer(nullptr, &manager);
    }
    static void Dispatch(BLEManagerImpl & manager, const ChipDeviceEvent & event)
    {
        StackLock lock;
        manager._OnPlatformEvent(&event);
    }
};
} // namespace chip::DeviceLayer::Internal

using namespace chip;
using namespace chip::Controller;

namespace {

constexpr uint16_t kDiscriminator = 0xABC;

struct PrivateBluezPeer
{
    std::unique_ptr<GDBusServer, decltype(&g_object_unref)> listener{ nullptr, g_object_unref };
    std::unique_ptr<GDBusConnection, decltype(&g_object_unref)> server{ nullptr, g_object_unref };
    std::unique_ptr<GDBusConnection, decltype(&g_object_unref)> client{ nullptr, g_object_unref };
    std::mutex mutex;
    std::condition_variable connected;

    ~PrivateBluezPeer()
    {
        if (listener)
        {
            EXPECT_EQ(DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(
                          +[](PrivateBluezPeer * peer) {
                              g_dbus_server_stop(peer->listener.get());
                              return CHIP_NO_ERROR;
                          },
                          this),
                      CHIP_NO_ERROR);
        }
        if (client)
        {
            g_dbus_connection_close_sync(client.get(), nullptr, nullptr);
        }
        if (server)
        {
            g_dbus_connection_close_sync(server.get(), nullptr, nullptr);
        }
    }

    CHIP_ERROR Start()
    {
        ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(
            +[](PrivateBluezPeer * peer) {
                GAutoPtr<gchar> guid(g_dbus_generate_guid());
                GAutoPtr<GError> error;
                peer->listener.reset(g_dbus_server_new_sync("unix:tmpdir=/tmp", G_DBUS_SERVER_FLAGS_NONE, guid.get(), nullptr,
                                                            nullptr, &error.GetReceiver()));
                VerifyOrReturnError(peer->listener != nullptr, CHIP_ERROR_INTERNAL);
                g_signal_connect(
                    peer->listener.get(), "new-connection",
                    G_CALLBACK(+[](GDBusServer *, GDBusConnection * connection, PrivateBluezPeer * accepted) -> gboolean {
                        std::lock_guard<std::mutex> lock(accepted->mutex);
                        accepted->server.reset(G_DBUS_CONNECTION(g_object_ref(connection)));
                        accepted->connected.notify_one();
                        return TRUE;
                    }),
                    peer);
                g_dbus_server_start(peer->listener.get());
                return CHIP_NO_ERROR;
            },
            this));
        GAutoPtr<GError> error;
        client.reset(g_dbus_connection_new_for_address_sync(g_dbus_server_get_client_address(listener.get()),
                                                            G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr, nullptr,
                                                            &error.GetReceiver()));
        VerifyOrReturnError(client != nullptr, CHIP_ERROR_INTERNAL);
        std::unique_lock<std::mutex> lock(mutex);
        VerifyOrReturnError(connected.wait_for(lock, std::chrono::seconds(5), [this] { return server != nullptr; }),
                            CHIP_ERROR_TIMEOUT);
        return CHIP_NO_ERROR;
    }
};

struct Connection
{
    bool retired    = false;
    unsigned closes = 0;
    unsigned writes = 0;

    BLE_CONNECTION_OBJECT Object() { return reinterpret_cast<BLE_CONNECTION_OBJECT>(this); }
};

struct PrivateBluezGattPeer : PrivateBluezPeer
{
    static constexpr const char * kDevicePath = "/org/bluez/device";
    GAutoPtr<GDBusObjectManagerServer> root;
    GAutoPtr<GDBusObjectManager> objects;
    GAutoPtr<BluezDevice1> device;
    GAutoPtr<BluezDevice1> proxy;
    GAutoPtr<BluezAdapter1> adapter;
    DeviceLayer::Internal::BluezEndpoint * endpoint = nullptr;
    std::atomic<unsigned> disconnects{ 0 };
    std::atomic<unsigned> connects{ 0 };
    std::atomic<unsigned> scanStarts{ 0 };
    std::atomic<unsigned> scanStops{ 0 };
    guint discoveryFilter       = 0;
    bool disconnectChangesState = true;
    std::thread serviceThread;
    GMainLoop * serviceLoop = nullptr;
    bool ready              = false;
    bool observedConnected  = true;
    bool observedResolved   = false;
    size_t connections      = 0;

    ~PrivateBluezGattPeer()
    {
        if (discoveryFilter != 0)
        {
            g_dbus_connection_remove_filter(server.get(), discoveryFilter);
        }
        if (endpoint != nullptr)
        {
            EXPECT_EQ(DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(
                          +[](PrivateBluezGattPeer * peer) {
                              g_signal_handlers_disconnect_by_data(peer->objects.get(), peer);
                              g_dbus_proxy_set_cached_property(G_DBUS_PROXY(peer->proxy.get()), "Connected",
                                                               g_variant_new_boolean(FALSE));
                              peer->endpoint->OnDevicePropertyChanged(*peer->proxy, nullptr, nullptr);
                              DeviceLayer::Internal::BLEManagerImplTestAccess::RemoveGatt(DeviceLayer::Internal::BLEMgrImpl());
                              return CHIP_NO_ERROR;
                          },
                          this),
                      CHIP_NO_ERROR);
            DrainEvents();
        }
        if (serviceThread.joinable())
        {
            g_main_loop_quit(serviceLoop);
            serviceThread.join();
        }
    }

    static void DrainEvents()
    {
        ASSERT_EQ(DeviceLayer::PlatformMgr().ScheduleWork(
                      [](intptr_t) { EXPECT_EQ(DeviceLayer::PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
                  CHIP_NO_ERROR);
        DeviceLayer::PlatformMgr().RunEventLoop();
    }

    CHIP_ERROR Attach()
    {
        return DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(
            +[](PrivateBluezGattPeer * peer) {
                using namespace DeviceLayer::Internal;
                peer->endpoint = &BLEManagerImplTestAccess::InstallGatt(BLEMgrImpl(), peer->objects.get());
                std::unique_ptr<GDBusObject, decltype(&g_object_unref)> object(
                    g_dbus_object_manager_get_object(peer->objects.get(), kDevicePath), g_object_unref);
                peer->proxy.reset(bluez_object_get_device1(BLUEZ_OBJECT(object.get())));
                g_signal_connect(
                    peer->objects.get(), "interface-proxy-properties-changed",
                    G_CALLBACK(+[](GDBusObjectManagerClient *, GDBusObjectProxy *, GDBusProxy * interface, GVariant * changed,
                                   const gchar * const * invalidated, PrivateBluezGattPeer * owner) {
                        if (BLUEZ_IS_DEVICE1(interface))
                        {
                            owner->endpoint->OnDevicePropertyChanged(*BLUEZ_DEVICE1(interface), changed, invalidated);
                            std::lock_guard<std::mutex> lock(owner->mutex);
                            owner->observedConnected = bluez_device1_get_connected(owner->proxy.get());
                            owner->observedResolved  = bluez_device1_get_services_resolved(owner->proxy.get());
                            owner->connections       = owner->endpoint->GetNumConnections();
                            owner->connected.notify_one();
                        }
                    }),
                    peer);
                return CHIP_NO_ERROR;
            },
            this);
    }

    bool WaitState(bool isConnected, bool resolved, size_t count)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return connected.wait_for(lock, std::chrono::seconds(2), [=, this] {
            return observedConnected == isConnected && observedResolved == resolved && connections == count;
        });
    }

    CHIP_ERROR SetState(bool isConnected, bool resolved)
    {
        struct
        {
            PrivateBluezGattPeer * peer;
            bool isConnected;
            bool resolved;
        } state{ this, isConnected, resolved };
        return DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(
            +[](decltype(state) * update) {
                bluez_device1_set_connected(update->peer->device.get(), update->isConnected);
                bluez_device1_set_services_resolved(update->peer->device.get(), update->resolved);
                g_dbus_interface_skeleton_flush(G_DBUS_INTERFACE_SKELETON(update->peer->device.get()));
                return CHIP_NO_ERROR;
            },
            &state);
    }

    DeviceLayer::ChipDeviceEvent PendingEvent()
    {
        DeviceLayer::ChipDeviceEvent event{};
        EXPECT_EQ(DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(
                      +[](DeviceLayer::ChipDeviceEvent * pending) {
                          *pending =
                              DeviceLayer::Internal::BLEManagerImplTestAccess::PendingEvent(DeviceLayer::Internal::BLEMgrImpl());
                          return CHIP_NO_ERROR;
                      },
                      &event),
                  CHIP_NO_ERROR);
        return event;
    }

    bool WaitDisconnects(unsigned count)
    {
        std::unique_lock<std::mutex> lock(mutex);
        return connected.wait_for(lock, std::chrono::seconds(2), [=, this] { return disconnects.load() == count; });
    }

    CHIP_ERROR Start()
    {
        ReturnErrorOnFailure(PrivateBluezPeer::Start());
        discoveryFilter = g_dbus_connection_add_filter(
            server.get(),
            +[](GDBusConnection * connection, GDBusMessage * message, gboolean incoming, void * context) -> GDBusMessage * {
                if (incoming && g_dbus_message_get_message_type(message) == G_DBUS_MESSAGE_TYPE_METHOD_CALL &&
                    g_strcmp0(g_dbus_message_get_interface(message), "org.bluez.Adapter1") == 0)
                {
                    auto * peer = static_cast<PrivateBluezGattPeer *>(context);
                    if (g_strcmp0(g_dbus_message_get_member(message), "StartDiscovery") == 0)
                    {
                        ++peer->scanStarts;
                    }
                    if (g_strcmp0(g_dbus_message_get_member(message), "StopDiscovery") == 0)
                    {
                        ++peer->scanStops;
                    }
                    GDBusMessage * reply = g_dbus_message_new_method_reply(message);
                    g_dbus_connection_send_message(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, nullptr);
                    g_object_unref(reply);
                    g_object_unref(message);
                    return nullptr;
                }
                return message;
            },
            this, nullptr);
        serviceThread = std::thread([this] {
            auto * peer            = this;
            GMainContext * context = g_main_context_new();
            g_main_context_push_thread_default(context);
            serviceLoop = g_main_loop_new(context, FALSE);
            peer->root.reset(g_dbus_object_manager_server_new("/org/bluez"));
            peer->device.reset(bluez_device1_skeleton_new());
            bluez_device1_set_address(peer->device.get(), "00:11:22:33:44:55");
            bluez_device1_set_adapter(peer->device.get(), "/org/bluez/adapter");
            bluez_device1_set_connected(peer->device.get(), TRUE);
            bluez_device1_set_services_resolved(peer->device.get(), FALSE);
            std::unique_ptr<GDBusObject, decltype(&g_object_unref)> deviceObject(
                G_DBUS_OBJECT(bluez_object_skeleton_new(kDevicePath)), g_object_unref);
            bluez_object_skeleton_set_device1(BLUEZ_OBJECT_SKELETON(deviceObject.get()), peer->device.get());
            g_dbus_object_manager_server_export(peer->root.get(), G_DBUS_OBJECT_SKELETON(deviceObject.get()));
            g_signal_connect(peer->device.get(), "handle-connect",
                             G_CALLBACK(+[](BluezDevice1 * deviceProxy, GDBusMethodInvocation * invocation,
                                            PrivateBluezGattPeer * owner) -> gboolean {
                                 ++owner->connects;
                                 bluez_device1_complete_connect(deviceProxy, invocation);
                                 return TRUE;
                             }),
                             peer);
            g_signal_connect(peer->device.get(), "handle-disconnect",
                             G_CALLBACK(+[](BluezDevice1 * deviceProxy, GDBusMethodInvocation * invocation,
                                            PrivateBluezGattPeer * owner) -> gboolean {
                                 ++owner->disconnects;
                                 bluez_device1_complete_disconnect(deviceProxy, invocation);
                                 if (owner->disconnectChangesState)
                                 {
                                     bluez_device1_set_connected(deviceProxy, FALSE);
                                     g_dbus_interface_skeleton_flush(G_DBUS_INTERFACE_SKELETON(deviceProxy));
                                 }
                                 std::lock_guard<std::mutex> lock(owner->mutex);
                                 owner->connected.notify_one();
                                 return TRUE;
                             }),
                             peer);
            GAutoPtr<BluezGattService1> service(bluez_gatt_service1_skeleton_new());
            bluez_gatt_service1_set_uuid(service.get(), Ble::CHIP_BLE_SERVICE_LONG_UUID_STR);
            bluez_gatt_service1_set_device(service.get(), kDevicePath);
            std::unique_ptr<GDBusObject, decltype(&g_object_unref)> serviceObject(
                G_DBUS_OBJECT(bluez_object_skeleton_new("/org/bluez/device/service")), g_object_unref);
            bluez_object_skeleton_set_gatt_service1(BLUEZ_OBJECT_SKELETON(serviceObject.get()), service.get());
            g_dbus_object_manager_server_export(peer->root.get(), G_DBUS_OBJECT_SKELETON(serviceObject.get()));
            const char * writeFlags[]    = { "write", nullptr };
            const char * indicateFlags[] = { "indicate", nullptr };
            for (bool write : { true, false })
            {
                GAutoPtr<BluezGattCharacteristic1> characteristic(bluez_gatt_characteristic1_skeleton_new());
                bluez_gatt_characteristic1_set_service(characteristic.get(), "/org/bluez/device/service");
                bluez_gatt_characteristic1_set_uuid(characteristic.get(),
                                                    write ? Ble::CHIP_BLE_CHAR_1_UUID_STR : Ble::CHIP_BLE_CHAR_2_UUID_STR);
                bluez_gatt_characteristic1_set_flags(characteristic.get(), write ? writeFlags : indicateFlags);
                std::unique_ptr<GDBusObject, decltype(&g_object_unref)> object(
                    G_DBUS_OBJECT(
                        bluez_object_skeleton_new(write ? "/org/bluez/device/service/c1" : "/org/bluez/device/service/c2")),
                    g_object_unref);
                bluez_object_skeleton_set_gatt_characteristic1(BLUEZ_OBJECT_SKELETON(object.get()), characteristic.get());
                g_dbus_object_manager_server_export(peer->root.get(), G_DBUS_OBJECT_SKELETON(object.get()));
            }
            g_dbus_object_manager_server_set_connection(peer->root.get(), peer->server.get());
            {
                std::lock_guard<std::mutex> lock(mutex);
                ready = true;
                connected.notify_one();
            }
            g_main_loop_run(serviceLoop);
            g_main_loop_unref(serviceLoop);
            g_main_context_pop_thread_default(context);
            g_main_context_unref(context);
        });
        {
            std::unique_lock<std::mutex> lock(mutex);
            VerifyOrReturnError(connected.wait_for(lock, std::chrono::seconds(5), [this] { return ready; }), CHIP_ERROR_TIMEOUT);
        }
        return DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(
            +[](PrivateBluezGattPeer * peer) {
                GAutoPtr<GError> error;
                peer->objects.reset(bluez_object_manager_client_new_sync(peer->client.get(),
                                                                         G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, nullptr,
                                                                         "/org/bluez", nullptr, &error.GetReceiver()));
                VerifyOrReturnError(peer->objects, CHIP_ERROR_INTERNAL);
                peer->adapter.reset(bluez_adapter1_proxy_new_sync(peer->client.get(), G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                                  nullptr, "/org/bluez/adapter", nullptr, &error.GetReceiver()));
                return peer->adapter ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
            },
            this);
    }
};

struct ScannedDeviceSource
{
    BluezDevice1 * device;
    Ble::ChipBLEDeviceIdentificationInfo info{};
    std::mutex mutex;
    std::condition_variable changed;
    bool returned = false;

    explicit ScannedDeviceSource(BluezDevice1 * scanned) : device(scanned) {}

    void Queue()
    {
        GSource * source = g_idle_source_new();
        g_source_set_callback(
            source,
            [](void * context) -> gboolean {
                auto * scanned = static_cast<ScannedDeviceSource *>(context);
                DeviceLayer::Internal::BLEManagerImplTestAccess::Scanned(DeviceLayer::Internal::BLEMgrImpl(), *scanned->device,
                                                                         scanned->info);
                std::lock_guard<std::mutex> lock(scanned->mutex);
                scanned->returned = true;
                scanned->changed.notify_one();
                return G_SOURCE_REMOVE;
            },
            this, nullptr);
        EXPECT_NE(DeviceLayer::PlatformMgrImpl().GLibMatterContextAttachSource(source), 0u);
        g_source_unref(source);
    }

    bool Wait()
    {
        std::unique_lock<std::mutex> lock(mutex);
        return changed.wait_for(lock, std::chrono::seconds(2), [this] { return returned; });
    }
};

struct GLibConnectBarrier
{
    std::mutex mutex;
    std::condition_variable changed;
    bool entered  = false;
    bool released = false;

    ~GLibConnectBarrier() { Release(); }

    static void Drain()
    {
        GLibConnectBarrier barrier;
        EXPECT_TRUE(barrier.Start());
    }

    bool Start()
    {
        GSource * source = g_idle_source_new();
        g_source_set_callback(
            source,
            [](void * context) -> gboolean {
                auto * barrier = static_cast<GLibConnectBarrier *>(context);
                std::unique_lock<std::mutex> lock(barrier->mutex);
                barrier->entered = true;
                barrier->changed.notify_one();
                barrier->changed.wait(lock, [barrier] { return barrier->released; });
                return G_SOURCE_REMOVE;
            },
            this, nullptr);
        const auto sourceId = DeviceLayer::PlatformMgrImpl().GLibMatterContextAttachSource(source);
        g_source_unref(source);
        std::unique_lock<std::mutex> lock(mutex);
        return sourceId != 0 && changed.wait_for(lock, std::chrono::seconds(2), [this] { return entered; });
    }

    void Release()
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            released = true;
            changed.notify_one();
        }
        EXPECT_EQ(DeviceLayer::PlatformMgrImpl().GLibMatterContextInvokeSync(
                      +[](GLibConnectBarrier *) { return CHIP_NO_ERROR; }, this),
                  CHIP_NO_ERROR);
    }
};

struct BluezConnectionResult
{
    unsigned successes               = 0;
    unsigned errors                  = 0;
    BLE_CONNECTION_OBJECT connection = nullptr;
    CHIP_ERROR error                 = CHIP_NO_ERROR;

    static void Success(void * context, BLE_CONNECTION_OBJECT connection)
    {
        auto & result = *static_cast<BluezConnectionResult *>(context);
        ++result.successes;
        result.connection = connection;
    }

    static void Error(void * context, CHIP_ERROR error)
    {
        auto & result = *static_cast<BluezConnectionResult *>(context);
        ++result.errors;
        result.error = error;
    }
};

class FallbackBle : public Ble::BlePlatformDelegate, public Ble::BleApplicationDelegate, public Ble::BleConnectionDelegate
{
public:
    CHIP_ERROR Install(Ble::BleLayer & layer) { return layer.Init(this, this, this, &DeviceLayer::SystemLayer()); }

    unsigned requests       = 0;
    unsigned cancellations  = 0;
    unsigned invalidUses    = 0;
    CHIP_ERROR cancelResult = CHIP_NO_ERROR;
    std::unique_ptr<DeviceLayer::Internal::BluezConnection> closingConnection;
    SetupDiscriminator discriminator;
    bool active = false;

    void NewConnection(Ble::BleLayer *, void * context, const SetupDiscriminator & requested) override
    {
        EXPECT_FALSE(active);
        ++requests;
        discriminator = requested;
        mContext      = context;
        mSuccess      = OnConnectionComplete;
        mError        = OnConnectionError;
        active        = true;
    }

    void NewConnection(Ble::BleLayer *, void *, BLE_CONNECTION_OBJECT) override { FAIL(); }

    CHIP_ERROR CancelConnection() override
    {
        ++cancellations;
        if (cancelResult == CHIP_NO_ERROR)
        {
            active = false;
        }
        return cancelResult;
    }

    void Complete(Connection & connection)
    {
        ASSERT_TRUE(active);
        active = false;
        mSuccess(mContext, connection.Object());
    }

    void Fail()
    {
        ASSERT_TRUE(active);
        active = false;
        mError(mContext, CHIP_ERROR_TIMEOUT);
    }

private:
    void * mContext                    = nullptr;
    OnConnectionCompleteFunct mSuccess = nullptr;
    OnConnectionErrorFunct mError      = nullptr;

    Connection & Use(BLE_CONNECTION_OBJECT object)
    {
        auto & connection = *reinterpret_cast<Connection *>(object);
        if (connection.retired)
        {
            ++invalidUses;
        }
        return connection;
    }

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT) override {}
    CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT object, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *) override
    {
        Use(object);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT object, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *) override
    {
        Use(object);
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT object) override
    {
        auto & connection = Use(object);
        ++connection.closes;
        connection.retired = true;
        if (closingConnection)
        {
            CHIP_ERROR error = closingConnection->CloseConnection();
            closingConnection.reset();
            return error;
        }
        return CHIP_NO_ERROR;
    }
    uint16_t GetMTU(BLE_CONNECTION_OBJECT) const override { return 200; }
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *,
                              System::PacketBufferHandle) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT object, const Ble::ChipBleUUID *, const Ble::ChipBleUUID *,
                                System::PacketBufferHandle) override
    {
        ++Use(object).writes;
        return CHIP_NO_ERROR;
    }
};

class FallbackCredentials : public OperationalCredentialsDelegate
{
    CHIP_ERROR GenerateNOCChain(const ByteSpan &, const ByteSpan &, const ByteSpan &, const ByteSpan &, const ByteSpan &,
                                const ByteSpan &, Callback::Callback<OnNOCChainGeneration> *) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
};

class TestBleFallback : public ::testing::Test
{
public:
    struct State
    {
        TestPersistentStorageDelegate storage;
        Credentials::PersistentStorageOpCertStore certificates;
        Crypto::RawKeySessionKeystore keys;
        Credentials::GroupDataProviderImpl groups;
        FallbackCredentials credentials;
        FallbackBle ble;
        Connection discarded;
        Connection fresh;
        DeviceCommissioner commissioner;
    };

    void SetUp() override
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        state = std::make_unique<State>();
        ASSERT_EQ(state->certificates.Init(&state->storage), CHIP_NO_ERROR);
        state->groups.SetStorageDelegate(&state->storage);
        state->groups.SetSessionKeystore(&state->keys);
        ASSERT_EQ(state->groups.Init(), CHIP_NO_ERROR);
        Credentials::SetGroupDataProvider(&state->groups);
        InitializeFactory();
        auto * layer = DeviceControllerFactory::GetInstance().GetSystemState()->BleLayer();
        layer->Shutdown();
        ASSERT_EQ(state->ble.Install(*layer), CHIP_NO_ERROR);
        Initialize(state->commissioner);
    }

    void InitializeFactory()
    {
        FactoryInitParams factory;
        factory.fabricIndependentStorage = &state->storage;
        factory.opCertStore              = &state->certificates;
        factory.sessionKeystore          = &state->keys;
        factory.groupDataProvider        = &state->groups;
        factory.dataModelProvider        = app::CodegenDataModelProviderInstance(&state->storage);
        ASSERT_EQ(DeviceControllerFactory::GetInstance().Init(factory), CHIP_NO_ERROR);
        DeviceControllerFactory::GetInstance().RetainSystemState();
    }

    void TearDown() override
    {
        state->commissioner.Shutdown();
        EXPECT_EQ(state->ble.invalidUses, 0u);
        if (!factoryStopped)
        {
            DeviceControllerFactory::GetInstance().ReleaseSystemState();
            DeviceControllerFactory::GetInstance().Shutdown();
        }
        Credentials::SetGroupDataProvider(nullptr);
        state->groups.Finish();
        state.reset();
        Platform::MemoryShutdown();
    }

    void Initialize(DeviceCommissioner & commissioner)
    {
        SetupParams setup;
        setup.controllerVendorId             = VendorId::TestVendor1;
        setup.operationalCredentialsDelegate = &state->credentials;
        setup.deviceAttestationVerifier      = Credentials::GetDefaultDACVerifier(nullptr);
        ASSERT_EQ(DeviceControllerFactory::GetInstance().SetupCommissioner(setup, commissioner), CHIP_NO_ERROR);
    }

    RendezvousParameters Parameters()
    {
        RendezvousParameters params;
        SetupDiscriminator discriminator;
        discriminator.SetLongValue(kDiscriminator);
        return params.SetPeerAddress(Transport::PeerAddress::BLE()).SetSetupPINCode(20202021).SetSetupDiscriminator(discriminator);
    }

    void BeginUdp(DeviceCommissioner & commissioner)
    {
        auto & pairer = Testing::DeviceCommissionerTestAccess(&commissioner).GetSetUpCodePairer();
        Testing::SetUpCodePairerTestAccess access(&pairer);
        SetupPayload payload;
        payload.setUpPINCode = 20202021;
        payload.discriminator.SetLongValue(kDiscriminator);
        access.SetSetupPayload(payload);
        access.SetRemoteId(1);
        SetUpCodePairerParameters udp;
        udp.SetPeerAddress(Transport::PeerAddress::UDP(Inet::IPAddress::Any, 5540));
        access.SetCurrentPASEParameters(udp);
        access.ExpectPASEEstablishment();
        Inet::IPAddress loopback;
        ASSERT_TRUE(Inet::IPAddress::FromString("::1", loopback));
        RendezvousParameters parameters;
        parameters.SetPeerAddress(Transport::PeerAddress::UDP(loopback, 9)).SetSetupPINCode(20202021);
        commissioner.RegisterPairingDelegate(&pairer);
        ASSERT_EQ(commissioner.EstablishPASEConnection(1, parameters), CHIP_NO_ERROR);
    }

    void Defer(DeviceCommissioner & commissioner)
    {
        BeginUdp(commissioner);
        auto & pairer = Testing::DeviceCommissionerTestAccess(&commissioner).GetSetUpCodePairer();
        Testing::SetUpCodePairerTestAccess access(&pairer);
        access.CallOnDiscoveredDeviceOverBle(state->discarded.Object(), kDiscriminator);
        EXPECT_EQ(state->discarded.closes, 1u);
        EXPECT_EQ(state->discarded.writes, 0u);
        EXPECT_TRUE(state->discarded.retired);
        EXPECT_EQ(state->ble.requests, 0u);
    }

    CHIP_ERROR StartRequest(DeviceCommissioner & commissioner, NodeId node)
    {
        auto params = Parameters();
        return commissioner.EstablishPASEConnection(node, params);
    }

    void StartFallback(DeviceCommissioner & commissioner)
    {
        Defer(commissioner);
        Testing::DeviceCommissionerTestAccess(&commissioner).FailPASE(CHIP_ERROR_TIMEOUT);
        EXPECT_EQ(state->ble.requests, 1u);
        EXPECT_TRUE(state->ble.active);
        EXPECT_FALSE(state->ble.discriminator.IsShortDiscriminator());
        EXPECT_EQ(state->ble.discriminator.GetLongValue(), kDiscriminator);
    }

    void ExpectPASE(DeviceCommissioner & commissioner, Connection & connection)
    {
        EXPECT_EQ(Testing::DeviceCommissionerTestAccess(&commissioner).GetPASEConnectionObject(), connection.Object());
        EXPECT_GT(connection.writes, 0u);
        EXPECT_EQ(state->discarded.closes, 1u);
        EXPECT_EQ(state->discarded.writes, 0u);
    }

protected:
    std::unique_ptr<State> state;
    bool factoryStopped = false;
};

TEST_F(TestBleFallback, CompleteUdpToFreshBleFallback)
{
    StartFallback(state->commissioner);
    state->ble.Complete(state->fresh);
    ExpectPASE(state->commissioner, state->fresh);
    EXPECT_EQ(state->fresh.closes, 0u);
}

TEST_F(TestBleFallback, DeferredDisconnectDoesNotBlockControllerProgress)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    struct HeldDisconnect
    {
        std::mutex mutex;
        std::condition_variable changed;
        std::unique_ptr<GDBusMessage, decltype(&g_object_unref)> request{ nullptr, g_object_unref };
        unsigned calls             = 0;
        bool progressed            = false;
        bool progressedBeforeReply = false;
    } held;
    const guint filter = g_dbus_connection_add_filter(
        peer.server.get(),
        +[](GDBusConnection *, GDBusMessage * message, gboolean incoming, void * context) -> GDBusMessage * {
            if (incoming && g_dbus_message_get_message_type(message) == G_DBUS_MESSAGE_TYPE_METHOD_CALL &&
                g_strcmp0(g_dbus_message_get_interface(message), "org.bluez.Device1") == 0 &&
                g_strcmp0(g_dbus_message_get_member(message), "Disconnect") == 0)
            {
                auto & pending = *static_cast<HeldDisconnect *>(context);
                std::lock_guard<std::mutex> lock(pending.mutex);
                ++pending.calls;
                pending.request.reset(message);
                pending.changed.notify_one();
                return nullptr;
            }
            return message;
        },
        &held, nullptr);
    GAutoPtr<GError> error;
    GAutoPtr<BluezDevice1> device(bluez_device1_proxy_new_sync(peer.client.get(), G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                               nullptr, "/org/bluez/device", nullptr, &error.GetReceiver()));
    ASSERT_NE(device.get(), nullptr);
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(device.get()), 5000);
    state->ble.closingConnection = std::make_unique<BluezConnection>(*device);
    BeginUdp(state->commissioner);
    ASSERT_EQ(SystemLayer().ScheduleLambda([this] {
        auto & pairer = Testing::DeviceCommissionerTestAccess(&state->commissioner).GetSetUpCodePairer();
        Testing::SetUpCodePairerTestAccess(&pairer).CallOnDiscoveredDeviceOverBle(state->discarded.Object(), kDiscriminator);
        EXPECT_EQ(state->discarded.closes, 1u);
        EXPECT_EQ(state->discarded.writes, 0u);
    }),
              CHIP_NO_ERROR);
    ASSERT_EQ(SystemLayer().ScheduleLambda([&held] {
        {
            std::lock_guard<std::mutex> lock(held.mutex);
            held.progressed = true;
            held.changed.notify_one();
        }
        EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR);
    }),
              CHIP_NO_ERROR);
    std::thread reply([&] {
        std::unique_lock<std::mutex> lock(held.mutex);
        if (held.changed.wait_for(lock, std::chrono::seconds(5), [&] { return held.request != nullptr; }))
        {
            held.progressedBeforeReply =
                held.changed.wait_for(lock, std::chrono::milliseconds(500), [&] { return held.progressed; });
            std::unique_ptr<GDBusMessage, decltype(&g_object_unref)> response(
                g_dbus_message_new_method_error_literal(held.request.get(), "org.freedesktop.DBus.Error.UnknownObject",
                                                        "Injected device disappearance"),
                g_object_unref);
            g_dbus_connection_send_message(peer.server.get(), response.get(), G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, nullptr);
        }
    });
    PlatformMgr().RunEventLoop();
    reply.join();
    EXPECT_EQ(held.calls, 1u);
    EXPECT_TRUE(held.progressedBeforeReply);
    g_dbus_connection_remove_filter(peer.server.get(), filter);
}

TEST_F(TestBleFallback, ImmediateBleConnectionReachesPASE)
{
    auto params = Parameters();
    params.SetConnectionObject(state->fresh.Object());
    ASSERT_EQ(state->commissioner.EstablishPASEConnection(1, params), CHIP_NO_ERROR);
    EXPECT_EQ(Testing::DeviceCommissionerTestAccess(&state->commissioner).GetPASEConnectionObject(), state->fresh.Object());
    EXPECT_GT(state->fresh.writes, 0u);
    EXPECT_EQ(state->ble.requests, 0u);
    EXPECT_EQ(state->fresh.closes, 0u);
}

TEST_F(TestBleFallback, StopBeforeFallbackDiscardsCandidate)
{
    Defer(state->commissioner);
    ASSERT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
    Testing::SetUpCodePairerTestAccess access(&Testing::DeviceCommissionerTestAccess(&state->commissioner).GetSetUpCodePairer());
    EXPECT_EQ(access.GetDiscoveredParametersCount(), 0u);
    EXPECT_EQ(access.GetRemoteId(), kUndefinedNodeId);
    EXPECT_FALSE(access.GetWaitingForPASE());
    EXPECT_EQ(state->ble.requests, 0u);
}

TEST_F(TestBleFallback, StopDuringFallbackCancelsBackend)
{
    StartFallback(state->commissioner);
    ASSERT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.cancellations, 1u);
    EXPECT_FALSE(state->ble.active);
    EXPECT_EQ(state->fresh.writes, 0u);
}

TEST_F(TestBleFallback, FailedCancellationLateSuccessClosesConnection)
{
    StartFallback(state->commissioner);
    state->ble.cancelResult = CHIP_ERROR_INTERNAL;
    ASSERT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.cancellations, 1u);
    state->ble.Complete(state->fresh);
    EXPECT_EQ(state->fresh.writes, 0u);
    EXPECT_EQ(state->fresh.closes, 1u);
}

TEST_F(TestBleFallback, FailedCancellationLateErrorIsTerminal)
{
    StartFallback(state->commissioner);
    state->ble.cancelResult = CHIP_ERROR_INTERNAL;
    ASSERT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.cancellations, 1u);
    state->ble.Fail();
    EXPECT_EQ(state->ble.requests, 1u);
    EXPECT_EQ(state->fresh.writes, 0u);
    state->ble.cancelResult = CHIP_NO_ERROR;
    ASSERT_EQ(StartRequest(state->commissioner, 2), CHIP_NO_ERROR);
    state->ble.Complete(state->fresh);
    ExpectPASE(state->commissioner, state->fresh);
}

TEST_F(TestBleFallback, SubsequentPairingAfterSuccessfulCancellation)
{
    StartFallback(state->commissioner);
    ASSERT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
    ASSERT_EQ(StartRequest(state->commissioner, 2), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.requests, 2u);
    state->ble.Complete(state->fresh);
    ExpectPASE(state->commissioner, state->fresh);
}

TEST_F(TestBleFallback, FailedCancellationDoesNotReplaceLiveRequest)
{
    StartFallback(state->commissioner);
    state->ble.cancelResult = CHIP_ERROR_INTERNAL;
    ASSERT_EQ(state->commissioner.StopPairing(1), CHIP_NO_ERROR);
    EXPECT_EQ(StartRequest(state->commissioner, 2), CHIP_ERROR_BUSY);
    EXPECT_EQ(state->ble.requests, 1u);
    state->ble.Complete(state->fresh);
    EXPECT_EQ(state->fresh.writes, 0u);
    EXPECT_EQ(state->fresh.closes, 1u);
    state->ble.cancelResult = CHIP_NO_ERROR;
    ASSERT_EQ(StartRequest(state->commissioner, 3), CHIP_NO_ERROR);
    EXPECT_EQ(state->ble.requests, 2u);
    state->ble.Fail();
}

TEST_F(TestBleFallback, OtherCommissionerCannotCancelOwnedFallback)
{
    StartFallback(state->commissioner);
    auto other = std::make_unique<DeviceCommissioner>();
    Initialize(*other);
    EXPECT_EQ(StartRequest(*other, 2), CHIP_ERROR_BUSY);
    other->Shutdown();
    EXPECT_EQ(state->ble.cancellations, 0u);
    state->ble.Complete(state->fresh);
    ExpectPASE(state->commissioner, state->fresh);
}

TEST_F(TestBleFallback, DestroyWithFailedCancellationLateError)
{
    auto owner = std::make_unique<DeviceCommissioner>();
    Initialize(*owner);
    StartFallback(*owner);
    state->ble.cancelResult = CHIP_ERROR_INTERNAL;
    owner->Shutdown();
    owner.reset();
    state->ble.Fail();
    EXPECT_EQ(state->ble.requests, 1u);
}

TEST_F(TestBleFallback, DestroyWithFailedCancellationLateSuccess)
{
    auto owner = std::make_unique<DeviceCommissioner>();
    Initialize(*owner);
    StartFallback(*owner);
    state->ble.cancelResult = CHIP_ERROR_INTERNAL;
    owner->Shutdown();
    owner.reset();
    state->ble.Complete(state->fresh);
    EXPECT_EQ(state->fresh.writes, 0u);
    EXPECT_EQ(state->fresh.closes, 1u);
}

TEST_F(TestBleFallback, LinuxCancelledQueuedScanCannotCompleteReplacement)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    unsigned oldCallbacks = 0;
    unsigned newCallbacks = 0;
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), oldCallbacks);
    ASSERT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), newCallbacks);
    ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
              CHIP_NO_ERROR);
    PlatformMgr().RunEventLoop();
    EXPECT_EQ(oldCallbacks, 0u);
    EXPECT_EQ(newCallbacks, 1u);
    EXPECT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
}

TEST_F(TestBleFallback, LinuxCancelledConnectFailureCannotCompleteReplacement)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    struct HeldConnect
    {
        std::mutex mutex;
        std::condition_variable changed;
        bool called = false;
    } held;
    const guint filter = g_dbus_connection_add_filter(
        peer.server.get(),
        +[](GDBusConnection *, GDBusMessage * message, gboolean incoming, void * context) -> GDBusMessage * {
            if (incoming && g_dbus_message_get_message_type(message) == G_DBUS_MESSAGE_TYPE_METHOD_CALL &&
                g_strcmp0(g_dbus_message_get_interface(message), "org.bluez.Device1") == 0 &&
                g_strcmp0(g_dbus_message_get_member(message), "Connect") == 0)
            {
                auto & pending = *static_cast<HeldConnect *>(context);
                std::lock_guard<std::mutex> lock(pending.mutex);
                pending.called = true;
                pending.changed.notify_one();
                g_object_unref(message);
                return nullptr;
            }
            return message;
        },
        &held, nullptr);
    GAutoPtr<GError> error;
    GAutoPtr<BluezDevice1> device(bluez_device1_proxy_new_sync(peer.client.get(), G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                               nullptr, "/org/bluez/device", nullptr, &error.GetReceiver()));
    ASSERT_NE(device.get(), nullptr);
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(device.get()), 5000);
    unsigned oldCallbacks = 0;
    unsigned newCallbacks = 0;
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    auto onError = [](void * context, CHIP_ERROR) { ++*static_cast<unsigned *>(context); };
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &oldCallbacks, nullptr, onError), CHIP_NO_ERROR);
    auto & endpoint          = BLEManagerImplTestAccess::Connecting(BLEMgrImpl());
    CHIP_ERROR connectResult = CHIP_NO_ERROR;
    std::thread connecting([&] { connectResult = endpoint.ConnectDevice(*device); });
    {
        std::unique_lock<std::mutex> lock(held.mutex);
        EXPECT_TRUE(held.changed.wait_for(lock, std::chrono::seconds(5), [&] { return held.called; }));
    }
    GAutoPtr<GCancellable> cancelled(G_CANCELLABLE(g_object_ref(BLEManagerImplTestAccess::ConnectCancellable(endpoint))));
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&oldCallbacks), CHIP_ERROR_BUSY);
    EXPECT_EQ(BLEManagerImplTestAccess::ConnectCancellable(endpoint), cancelled.get());
    EXPECT_TRUE(g_cancellable_is_cancelled(cancelled.get()));
    endpoint.CancelConnect();
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &newCallbacks, nullptr, onError), CHIP_ERROR_BUSY);
    connecting.join();
    EXPECT_EQ(connectResult, CHIP_ERROR_INTERNAL);
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &newCallbacks, nullptr, onError), CHIP_ERROR_BUSY);
    ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
              CHIP_NO_ERROR);
    PlatformMgr().RunEventLoop();
    EXPECT_EQ(oldCallbacks, 0u);
    EXPECT_EQ(newCallbacks, 0u);
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &newCallbacks, nullptr, onError), CHIP_NO_ERROR);
    {
        std::lock_guard<std::mutex> lock(held.mutex);
        held.called = false;
    }
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl());
    std::thread reconnecting([&] { connectResult = endpoint.ConnectDevice(*device); });
    {
        std::unique_lock<std::mutex> lock(held.mutex);
        EXPECT_TRUE(held.changed.wait_for(lock, std::chrono::seconds(5), [&] { return held.called; }));
    }
    EXPECT_NE(BLEManagerImplTestAccess::ConnectCancellable(endpoint), cancelled.get());
    EXPECT_FALSE(g_cancellable_is_cancelled(BLEManagerImplTestAccess::ConnectCancellable(endpoint)));
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&newCallbacks), CHIP_ERROR_BUSY);
    reconnecting.join();
    EXPECT_EQ(connectResult, CHIP_ERROR_INTERNAL);
    ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
              CHIP_NO_ERROR);
    PlatformMgr().RunEventLoop();
    EXPECT_EQ(newCallbacks, 0u);
    BLEManagerImplTestAccess::RestorePeripheral(BLEMgrImpl());
    g_dbus_connection_remove_filter(peer.server.get(), filter);
}

TEST_F(TestBleFallback, LinuxCancellationBeforeConnectKeepsPublishedToken)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    const guint filter = g_dbus_connection_add_filter(
        peer.server.get(),
        +[](GDBusConnection * connection, GDBusMessage * message, gboolean incoming, gpointer) -> GDBusMessage * {
            if (incoming && g_dbus_message_get_message_type(message) == G_DBUS_MESSAGE_TYPE_METHOD_CALL &&
                g_strcmp0(g_dbus_message_get_member(message), "Connect") == 0)
            {
                std::unique_ptr<GDBusMessage, decltype(&g_object_unref)> reply(g_dbus_message_new_method_reply(message),
                                                                               g_object_unref);
                g_dbus_connection_send_message(connection, reply.get(), G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, nullptr);
                g_object_unref(message);
                return nullptr;
            }
            return message;
        },
        nullptr, nullptr);
    GAutoPtr<GError> error;
    GAutoPtr<BluezDevice1> device(bluez_device1_proxy_new_sync(peer.client.get(), G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                               nullptr, "/org/bluez/device", nullptr, &error.GetReceiver()));
    ASSERT_NE(device.get(), nullptr);
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(device.get()), 1000);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer       = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    unsigned callbacks = 0;
    auto onError       = [](void * context, CHIP_ERROR) { ++*static_cast<unsigned *>(context); };
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
    auto & endpoint = BLEManagerImplTestAccess::Connecting(BLEMgrImpl());
    GAutoPtr<GCancellable> cancelled(G_CANCELLABLE(g_object_ref(BLEManagerImplTestAccess::ConnectCancellable(endpoint))));
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_ERROR_BUSY);
    endpoint.CancelConnect();
    EXPECT_TRUE(g_cancellable_is_cancelled(cancelled.get()));
    EXPECT_EQ(endpoint.ConnectDevice(*device), CHIP_ERROR_INTERNAL);
    EXPECT_EQ(BLEManagerImplTestAccess::ConnectCancellable(endpoint), cancelled.get());
    ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
              CHIP_NO_ERROR);
    PlatformMgr().RunEventLoop();
    EXPECT_EQ(callbacks, 0u);
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl());
    EXPECT_NE(BLEManagerImplTestAccess::ConnectCancellable(endpoint), cancelled.get());
    EXPECT_FALSE(g_cancellable_is_cancelled(BLEManagerImplTestAccess::ConnectCancellable(endpoint)));
    EXPECT_EQ(endpoint.ConnectDevice(*device), CHIP_NO_ERROR);
    BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_TIMEOUT);
    ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
              CHIP_NO_ERROR);
    PlatformMgr().RunEventLoop();
    EXPECT_EQ(callbacks, 1u);
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::RestorePeripheral(BLEMgrImpl());
    g_dbus_connection_remove_filter(peer.server.get(), filter);
}

TEST_F(TestBleFallback, LinuxPrivateGattObjectsEnumerated)
{
    PrivateBluezGattPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    GList * objects = g_dbus_object_manager_get_objects(peer.objects.get());
    EXPECT_EQ(g_list_length(objects), 4u);
    g_list_free_full(objects, g_object_unref);
    std::unique_ptr<GDBusObject, decltype(&g_object_unref)> object(
        g_dbus_object_manager_get_object(peer.objects.get(), peer.kDevicePath), g_object_unref);
    ASSERT_NE(object.get(), nullptr);
    GAutoPtr<BluezDevice1> device(bluez_object_get_device1(BLUEZ_OBJECT(object.get())));
    ASSERT_NE(device.get(), nullptr);
    EXPECT_TRUE(bluez_device1_get_connected(device.get()));
    EXPECT_FALSE(bluez_device1_get_services_resolved(device.get()));
}

TEST_F(TestBleFallback, LinuxScannedCallbackReturnsWhileStackLockedAndRejectsOldResults)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezGattPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    ASSERT_EQ(peer.Attach(), CHIP_NO_ERROR);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer       = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    unsigned callbacks = 0;
    auto onError       = [](void * context, CHIP_ERROR) { ++*static_cast<unsigned *>(context); };
    ScannedDeviceSource scanned(peer.proxy.get());
    bool returnedWhileLocked = false;
    {
        StackLock lock;
        ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
        ASSERT_EQ(BLEManagerImplTestAccess::StartScan(BLEMgrImpl(), peer.adapter.get()), CHIP_NO_ERROR);
        ASSERT_TRUE(BLEManagerImplTestAccess::ScannerActive(BLEMgrImpl()));
        scanned.Queue();
        returnedWhileLocked = scanned.Wait();
    }
    EXPECT_TRUE(returnedWhileLocked);
    ASSERT_TRUE(scanned.Wait());
    if (!returnedWhileLocked)
    {
        EXPECT_EQ(PlatformMgrImpl().GLibMatterContextInvokeSync(
                      +[](void *) { return CHIP_NO_ERROR; }, static_cast<void *>(nullptr)),
                  CHIP_NO_ERROR);
        EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_ERROR_BUSY);
        BLEManagerImplTestAccess::ExpireConnect(BLEMgrImpl());
        peer.DrainEvents();
        return;
    }
    {
        StackLock lock;
        EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_NO_ERROR);
        ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
        ASSERT_EQ(BLEManagerImplTestAccess::StartScan(BLEMgrImpl(), peer.adapter.get()), CHIP_NO_ERROR);
    }
    peer.DrainEvents();
    EXPECT_EQ(callbacks, 0u);
    EXPECT_EQ(peer.connects.load(), 0u);
    EXPECT_TRUE(BLEManagerImplTestAccess::ScannerActive(BLEMgrImpl()));
    ScannedDeviceSource mismatch(peer.proxy.get());
    mismatch.info.SetDeviceDiscriminator(kDiscriminator);
    mismatch.Queue();
    ASSERT_TRUE(mismatch.Wait());
    mismatch.info.SetDeviceDiscriminator(0);
    peer.DrainEvents();
    EXPECT_EQ(peer.connects.load(), 0u);
    EXPECT_TRUE(BLEManagerImplTestAccess::ScannerActive(BLEMgrImpl()));
    ScannedDeviceSource match(peer.proxy.get());
    ScannedDeviceSource duplicate(peer.proxy.get());
    match.Queue();
    ASSERT_TRUE(match.Wait());
    duplicate.Queue();
    ASSERT_TRUE(duplicate.Wait());
    match.info.SetDeviceDiscriminator(kDiscriminator);
    duplicate.info.SetDeviceDiscriminator(kDiscriminator);
    peer.DrainEvents();
    GLibConnectBarrier::Drain();
    EXPECT_EQ(peer.connects.load(), 1u);
    EXPECT_EQ(peer.scanStarts.load(), 2u);
    EXPECT_EQ(peer.scanStops.load(), 2u);
    EXPECT_FALSE(BLEManagerImplTestAccess::ScannerActive(BLEMgrImpl()));
    EXPECT_TRUE(BLEManagerImplTestAccess::IsConnecting(BLEMgrImpl()));
    EXPECT_EQ(callbacks, 0u);
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_ERROR_BUSY);
    BLEManagerImplTestAccess::ExpireConnect(BLEMgrImpl());
    peer.DrainEvents();
    EXPECT_EQ(callbacks, 0u);
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_NO_ERROR);
    peer.DrainEvents();
}

TEST_F(TestBleFallback, LinuxQueuedConnectCancellationRetiresPublishedToken)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezGattPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    ASSERT_EQ(peer.Attach(), CHIP_NO_ERROR);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer       = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    unsigned callbacks = 0;
    auto onError       = [](void * context, CHIP_ERROR) { ++*static_cast<unsigned *>(context); };
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Scanning(BLEMgrImpl());
    ScannedDeviceSource scanned(peer.proxy.get());
    scanned.Queue();
    ASSERT_TRUE(scanned.Wait());
    GLibConnectBarrier barrier;
    ASSERT_TRUE(barrier.Start());
    peer.DrainEvents();
    ASSERT_TRUE(BLEManagerImplTestAccess::IsConnecting(BLEMgrImpl()));
    auto & endpoint = *peer.endpoint;
    GAutoPtr<GCancellable> token(G_CANCELLABLE(g_object_ref(BLEManagerImplTestAccess::ConnectCancellable(endpoint))));
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_ERROR_BUSY);
    EXPECT_EQ(BLEManagerImplTestAccess::ConnectCancellable(endpoint), token.get());
    EXPECT_TRUE(g_cancellable_is_cancelled(token.get()));
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_ERROR_BUSY);
    barrier.Release();
    GLibConnectBarrier::Drain();
    peer.DrainEvents();
    EXPECT_EQ(peer.connects.load(), 0u);
    EXPECT_EQ(callbacks, 0u);
    EXPECT_FALSE(BLEManagerImplTestAccess::IsConnecting(BLEMgrImpl()));
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
    void * retiredToken = token.get();
    g_object_add_weak_pointer(G_OBJECT(token.get()), &retiredToken);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl());
    EXPECT_NE(BLEManagerImplTestAccess::ConnectCancellable(endpoint), token.get());
    EXPECT_FALSE(g_cancellable_is_cancelled(BLEManagerImplTestAccess::ConnectCancellable(endpoint)));
    token.reset();
    EXPECT_EQ(retiredToken, nullptr);
    if (retiredToken != nullptr)
    {
        g_object_remove_weak_pointer(G_OBJECT(retiredToken), &retiredToken);
    }
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_ERROR_BUSY);
    BLEManagerImplTestAccess::ExpireConnect(BLEMgrImpl());
    peer.DrainEvents();
}

TEST_F(TestBleFallback, LinuxQueuedWorkerFailureKeepsCapturedRequestId)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezGattPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    ASSERT_EQ(peer.Attach(), CHIP_NO_ERROR);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer          = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    unsigned oldCallbacks = 0;
    unsigned newCallbacks = 0;
    auto onError          = [](void * context, CHIP_ERROR) { ++*static_cast<unsigned *>(context); };
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &oldCallbacks, nullptr, onError), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Scanning(BLEMgrImpl());
    ScannedDeviceSource scanned(peer.proxy.get());
    scanned.Queue();
    ASSERT_TRUE(scanned.Wait());
    GLibConnectBarrier barrier;
    ASSERT_TRUE(barrier.Start());
    peer.DrainEvents();
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&oldCallbacks), CHIP_ERROR_BUSY);
    BLEManagerImplTestAccess::ExpireConnect(BLEMgrImpl());
    peer.DrainEvents();
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &newCallbacks, nullptr, onError), CHIP_NO_ERROR);
    auto & endpoint = BLEManagerImplTestAccess::Connecting(BLEMgrImpl());
    GAutoPtr<GCancellable> replacement(G_CANCELLABLE(g_object_ref(BLEManagerImplTestAccess::ConnectCancellable(endpoint))));
    barrier.Release();
    GLibConnectBarrier::Drain();
    peer.DrainEvents();
    EXPECT_EQ(peer.connects.load(), 0u);
    EXPECT_EQ(oldCallbacks, 0u);
    EXPECT_EQ(newCallbacks, 0u);
    EXPECT_EQ(BLEManagerImplTestAccess::ConnectCancellable(endpoint), replacement.get());
    EXPECT_FALSE(g_cancellable_is_cancelled(replacement.get()));
    EXPECT_TRUE(BLEManagerImplTestAccess::IsConnecting(BLEMgrImpl()));
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &newCallbacks, nullptr, onError), CHIP_ERROR_BUSY);
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&newCallbacks), CHIP_ERROR_BUSY);
    BLEManagerImplTestAccess::ExpireConnect(BLEMgrImpl());
    peer.DrainEvents();
    EXPECT_EQ(newCallbacks, 0u);
}

TEST_F(TestBleFallback, LinuxQueuedSuccessWhileClosingClosesDiscardedLink)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezGattPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    ASSERT_EQ(peer.Attach(), CHIP_NO_ERROR);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer       = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    unsigned callbacks = 0;
    auto onError       = [](void * context, CHIP_ERROR) { ++*static_cast<unsigned *>(context); };
    auto onSuccess     = [](void * context, BLE_CONNECTION_OBJECT) { ++*static_cast<unsigned *>(context); };
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, onSuccess, onError), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl(), peer.proxy.get());
    ASSERT_EQ(peer.SetState(true, true), CHIP_NO_ERROR);
    ASSERT_TRUE(peer.WaitState(true, true, 1));
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_ERROR_BUSY);
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, onSuccess, onError), CHIP_ERROR_BUSY);
    layer.IndicateBleClosing();
    ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
              CHIP_NO_ERROR);
    PlatformMgr().RunEventLoop();
    EXPECT_EQ(callbacks, 0u);
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, onSuccess, onError),
              CHIP_ERROR_INCORRECT_STATE);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, onSuccess, onError), CHIP_NO_ERROR);
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_NO_ERROR);
    EXPECT_TRUE(peer.WaitState(false, true, 0));
    EXPECT_EQ(peer.disconnects.load(), 1u);
    BLEManagerImplTestAccess::RestorePeripheral(BLEMgrImpl());
}

TEST_F(TestBleFallback, LinuxTimedOutConnectClosesLateServicesResolvedAndRetries)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezGattPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    ASSERT_EQ(peer.Attach(), CHIP_NO_ERROR);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    BluezConnectionResult result;
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &result, result.Success, result.Error), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl(), peer.proxy.get());
    BLEManagerImplTestAccess::ExpireConnect(BLEMgrImpl());
    peer.DrainEvents();
    EXPECT_EQ(result.errors, 1u);
    EXPECT_EQ(result.error, CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(result.successes, 0u);

    ASSERT_EQ(peer.SetState(true, true), CHIP_NO_ERROR);
    ASSERT_TRUE(peer.WaitState(true, true, 1));
    const auto stale = peer.PendingEvent();
    ASSERT_NE(stale.Platform.BLECentralConnected.mPendingTicket, 0u);
    peer.DrainEvents();
    EXPECT_TRUE(peer.WaitState(false, true, 0));
    EXPECT_EQ(peer.disconnects.load(), 1u);
    peer.DrainEvents();
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), stale);
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), stale);
    EXPECT_EQ(result.errors, 1u);
    EXPECT_EQ(result.successes, 0u);

    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &result, result.Success, result.Error), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl(), peer.proxy.get());
    ASSERT_EQ(peer.SetState(true, true), CHIP_NO_ERROR);
    ASSERT_TRUE(peer.WaitState(true, true, 1));
    const auto fresh = peer.PendingEvent();
    EXPECT_GT(fresh.Platform.BLECentralConnected.mPendingTicket, stale.Platform.BLECentralConnected.mPendingTicket);
    peer.DrainEvents();
    EXPECT_EQ(result.successes, 1u);
    EXPECT_NE(result.connection, nullptr);
    EXPECT_EQ(result.errors, 1u);
    EXPECT_EQ(peer.PendingEvent().Platform.BLECentralConnected.mPendingTicket, 0u);
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), fresh);
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), stale);
    peer.DrainEvents();
    EXPECT_EQ(result.successes, 1u);
    EXPECT_EQ(peer.disconnects.load(), 1u);
    EXPECT_TRUE(bluez_device1_get_connected(peer.proxy.get()));
}

TEST_F(TestBleFallback, LinuxLateServicesResolvedCannotCompleteConnectingReplacement)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezGattPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    ASSERT_EQ(peer.Attach(), CHIP_NO_ERROR);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    BluezConnectionResult original;
    BluezConnectionResult replacement;
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &original, original.Success, original.Error),
              CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl(), peer.proxy.get());
    BLEManagerImplTestAccess::ExpireConnect(BLEMgrImpl());
    peer.DrainEvents();
    ASSERT_EQ(original.errors, 1u);
    PrivateBluezGattPeer nextPeer;
    ASSERT_EQ(nextPeer.Start(), CHIP_NO_ERROR);
    ASSERT_EQ(nextPeer.Attach(), CHIP_NO_ERROR);
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &replacement, replacement.Success, replacement.Error),
              CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl(), nextPeer.proxy.get());
    ASSERT_EQ(peer.SetState(true, true), CHIP_NO_ERROR);
    ASSERT_TRUE(peer.WaitState(true, true, 1));
    peer.DrainEvents();
    EXPECT_EQ(replacement.successes, 0u);
    EXPECT_EQ(replacement.errors, 0u);
    EXPECT_TRUE(BLEManagerImplTestAccess::IsConnecting(BLEMgrImpl()));
    EXPECT_TRUE(peer.WaitState(false, true, 0));
    EXPECT_EQ(peer.disconnects.load(), 1u);
    ASSERT_EQ(nextPeer.SetState(true, true), CHIP_NO_ERROR);
    ASSERT_TRUE(nextPeer.WaitState(true, true, 1));
    peer.DrainEvents();
    EXPECT_EQ(replacement.successes, 1u);
    EXPECT_EQ(replacement.errors, 0u);
    EXPECT_EQ(nextPeer.disconnects.load(), 0u);
}

TEST_F(TestBleFallback, LinuxDeletedTicketCannotRetireSamePathReplacement)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezGattPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    ASSERT_EQ(peer.Attach(), CHIP_NO_ERROR);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    BluezConnectionResult result;
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &result, result.Success, result.Error), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl(), peer.proxy.get());
    ASSERT_EQ(peer.SetState(true, true), CHIP_NO_ERROR);
    ASSERT_TRUE(peer.WaitState(true, true, 1));
    auto stale = peer.PendingEvent();
    ASSERT_NE(stale.Platform.BLECentralConnected.mPendingTicket, 0u);
    ASSERT_EQ(peer.SetState(false, false), CHIP_NO_ERROR);
    ASSERT_TRUE(peer.WaitState(false, false, 0));
    peer.DrainEvents();
    EXPECT_EQ(result.successes, 0u);
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), stale);
    BLEManagerImplTestAccess::ExpireConnect(BLEMgrImpl());
    peer.DrainEvents();
    EXPECT_EQ(result.errors, 1u);

    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &result, result.Success, result.Error), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl(), peer.proxy.get());
    ASSERT_EQ(peer.SetState(true, true), CHIP_NO_ERROR);
    ASSERT_TRUE(peer.WaitState(true, true, 1));
    const auto fresh = peer.PendingEvent();
    ASSERT_GT(fresh.Platform.BLECentralConnected.mPendingTicket, stale.Platform.BLECentralConnected.mPendingTicket);
    EXPECT_NE(fresh.Platform.BLECentralConnected.mRequestId, stale.Platform.BLECentralConnected.mRequestId);
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), stale);
    EXPECT_EQ(peer.PendingEvent().Platform.BLECentralConnected.mPendingTicket, fresh.Platform.BLECentralConnected.mPendingTicket);
    stale.Platform.BLECentralConnected.mRequestId = fresh.Platform.BLECentralConnected.mRequestId;
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), stale);
    EXPECT_EQ(peer.PendingEvent().Platform.BLECentralConnected.mPendingTicket, fresh.Platform.BLECentralConnected.mPendingTicket);
    EXPECT_EQ(result.successes, 0u);
    EXPECT_EQ(peer.disconnects.load(), 0u);
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), fresh);
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), fresh);
    peer.DrainEvents();
    EXPECT_EQ(result.successes, 1u);
    EXPECT_EQ(result.errors, 1u);
    EXPECT_EQ(peer.disconnects.load(), 0u);
    ASSERT_EQ(peer.SetState(false, false), CHIP_NO_ERROR);
    ASSERT_TRUE(peer.WaitState(false, false, 0));
    peer.DrainEvents();
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), fresh);
    EXPECT_EQ(result.successes, 1u);
    EXPECT_EQ(peer.disconnects.load(), 0u);
}

TEST_F(TestBleFallback, LinuxRejectedTicketClosesOnlyOnceBeforeDisconnection)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezGattPeer peer;
    peer.disconnectChangesState = false;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    ASSERT_EQ(peer.Attach(), CHIP_NO_ERROR);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    BluezConnectionResult result;
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &result, result.Success, result.Error), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl(), peer.proxy.get());
    BLEManagerImplTestAccess::ExpireConnect(BLEMgrImpl());
    peer.DrainEvents();
    ASSERT_EQ(result.errors, 1u);
    ASSERT_EQ(peer.SetState(true, true), CHIP_NO_ERROR);
    ASSERT_TRUE(peer.WaitState(true, true, 1));
    const auto pending = peer.PendingEvent();
    peer.DrainEvents();
    EXPECT_TRUE(peer.WaitDisconnects(1));
    EXPECT_EQ(peer.PendingEvent().Platform.BLECentralConnected.mPendingTicket, 0u);
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), pending);
    BLEManagerImplTestAccess::Dispatch(BLEMgrImpl(), pending);
    peer.DrainEvents();
    EXPECT_EQ(peer.disconnects.load(), 1u);
    EXPECT_EQ(result.successes, 0u);
    EXPECT_EQ(result.errors, 1u);
    EXPECT_TRUE(bluez_device1_get_connected(peer.proxy.get()));
    ASSERT_EQ(peer.SetState(false, false), CHIP_NO_ERROR);
    ASSERT_TRUE(peer.WaitState(false, false, 0));
    peer.DrainEvents();
}

TEST_F(TestBleFallback, LinuxStopDiscoveryFailureDoesNotStrandNextPairing)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    std::atomic<unsigned> stopCalls{ 0 };
    const guint filter = g_dbus_connection_add_filter(
        peer.server.get(),
        +[](GDBusConnection * connection, GDBusMessage * message, gboolean incoming, void * context) -> GDBusMessage * {
            if (incoming && g_dbus_message_get_message_type(message) == G_DBUS_MESSAGE_TYPE_METHOD_CALL &&
                g_strcmp0(g_dbus_message_get_interface(message), "org.bluez.Adapter1") == 0 &&
                g_strcmp0(g_dbus_message_get_member(message), "StopDiscovery") == 0)
            {
                ++*static_cast<std::atomic<unsigned> *>(context);
                GDBusMessage * reply =
                    g_dbus_message_new_method_error_literal(message, "org.bluez.Error.Failed", "Injected StopDiscovery failure");
                g_dbus_connection_send_message(connection, reply, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, nullptr);
                g_object_unref(reply);
                g_object_unref(message);
                return nullptr;
            }
            return message;
        },
        &stopCalls, nullptr);
    GAutoPtr<GError> error;
    GAutoPtr<BluezAdapter1> adapter(bluez_adapter1_proxy_new_sync(peer.client.get(), G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                                  nullptr, "/org/bluez/adapter", nullptr, &error.GetReceiver()));
    ASSERT_NE(adapter.get(), nullptr);
    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(adapter.get()), 1000);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer       = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    unsigned callbacks = 0;
    auto onError       = [](void * context, CHIP_ERROR) { ++*static_cast<unsigned *>(context); };
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::InstallScanningAdapter(BLEMgrImpl(), adapter.get());
    EXPECT_EQ(layer.CancelBleIncompleteConnection(), CHIP_NO_ERROR);
    EXPECT_EQ(stopCalls.load(), 1u);
    EXPECT_FALSE(BLEManagerImplTestAccess::ScannerActive(BLEMgrImpl()));
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
    EXPECT_EQ(layer.CancelBleIncompleteConnection(), CHIP_NO_ERROR);
    ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
              CHIP_NO_ERROR);
    PlatformMgr().RunEventLoop();
    EXPECT_EQ(callbacks, 0u);
    g_dbus_connection_remove_filter(peer.server.get(), filter);
}

TEST_F(TestBleFallback, LinuxFactoryShutdownWithQueuedCancelledCompletion)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto * layer       = &BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    unsigned callbacks = 0;
    auto onError       = [](void * context, CHIP_ERROR) { ++*static_cast<unsigned *>(context); };
    ASSERT_EQ(layer->NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl());
    BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_INTERNAL);
    EXPECT_EQ(layer->CancelBleIncompleteConnection(&callbacks), CHIP_ERROR_BUSY);
    state->commissioner.Shutdown();
    DeviceControllerFactory::GetInstance().ReleaseSystemState();
    DeviceControllerFactory::GetInstance().Shutdown();
    factoryStopped = true;
    EXPECT_EQ(layer, &BLEManagerImplTestAccess::Layer(BLEMgrImpl()));
    EXPECT_EQ(callbacks, 0u);
    BLEManagerImplTestAccess::RestorePeripheral(BLEMgrImpl());
    InitializeFactory();
    factoryStopped = false;
    Initialize(state->commissioner);
    unsigned newCallbacks = 0;
    EXPECT_EQ(layer->NewBleConnectionByDiscriminator(SetupDiscriminator(), &newCallbacks, nullptr, onError), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl());
    ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
              CHIP_NO_ERROR);
    PlatformMgr().RunEventLoop();
    EXPECT_EQ(callbacks, 0u);
    EXPECT_EQ(newCallbacks, 0u);
    EXPECT_EQ(layer->NewBleConnectionByDiscriminator(SetupDiscriminator(), &newCallbacks, nullptr, onError), CHIP_ERROR_BUSY);
    BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_TIMEOUT);
    ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
              CHIP_NO_ERROR);
    PlatformMgr().RunEventLoop();
    EXPECT_EQ(newCallbacks, 1u);
    EXPECT_EQ(layer->NewBleConnectionByDiscriminator(SetupDiscriminator(), &newCallbacks, nullptr, onError), CHIP_NO_ERROR);
    EXPECT_EQ(layer->CancelBleIncompleteConnection(&newCallbacks), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::RestorePeripheral(BLEMgrImpl());
}

} // namespace
#else
TEST(TestBleFallback, BleTransportDisabled)
{
    EXPECT_FALSE(CONFIG_NETWORK_LAYER_BLE && CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE);
}
#endif
