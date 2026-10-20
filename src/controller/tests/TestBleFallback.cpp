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
    static BluezEndpoint & Connecting(BLEManagerImpl & manager)
    {
        ++manager.mScanRequestId;
        manager.mIsCentral                   = true;
        manager.mBLEScanConfig.mBleScanState = BleScanState::kConnecting;
        return manager.mEndpoint;
    }
    static void RestorePeripheral(BLEManagerImpl & manager) { manager.mIsCentral = false; }
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
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&oldCallbacks), CHIP_ERROR_BUSY);
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
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&newCallbacks), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::RestorePeripheral(BLEMgrImpl());
    g_dbus_connection_remove_filter(peer.server.get(), filter);
}

TEST_F(TestBleFallback, LinuxQueuedSuccessAfterCancellationClosesDiscardedLink)
{
    using namespace DeviceLayer;
    using namespace DeviceLayer::Internal;
    PrivateBluezPeer peer;
    ASSERT_EQ(peer.Start(), CHIP_NO_ERROR);
    struct CloseResult
    {
        std::mutex mutex;
        std::condition_variable changed;
        unsigned calls = 0;
    } closes;
    const guint filter = g_dbus_connection_add_filter(
        peer.server.get(),
        +[](GDBusConnection * connection, GDBusMessage * message, gboolean incoming, void * context) -> GDBusMessage * {
            if (incoming && g_dbus_message_get_message_type(message) == G_DBUS_MESSAGE_TYPE_METHOD_CALL &&
                g_strcmp0(g_dbus_message_get_member(message), "Disconnect") == 0)
            {
                auto & closed = *static_cast<CloseResult *>(context);
                std::lock_guard<std::mutex> lock(closed.mutex);
                ++closed.calls;
                GDBusMessage * response = g_dbus_message_new_method_reply(message);
                g_dbus_connection_send_message(connection, response, G_DBUS_SEND_MESSAGE_FLAGS_NONE, nullptr, nullptr);
                g_object_unref(response);
                g_object_unref(message);
                closed.changed.notify_one();
                return nullptr;
            }
            return message;
        },
        &closes, nullptr);
    GAutoPtr<GError> error;
    GAutoPtr<BluezDevice1> device(bluez_device1_proxy_new_sync(peer.client.get(), G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                               nullptr, "/org/bluez/device", nullptr, &error.GetReceiver()));
    ASSERT_NE(device.get(), nullptr);
    BluezConnection connection(*device);
    ASSERT_EQ(BLEManagerImplTestAccess::InstallBackend(BLEMgrImpl()), CHIP_NO_ERROR);
    auto & layer       = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    unsigned callbacks = 0;
    auto onError       = [](void * context, CHIP_ERROR) { ++*static_cast<unsigned *>(context); };
    auto onSuccess     = [](void * context, BLE_CONNECTION_OBJECT) { ++*static_cast<unsigned *>(context); };
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, onSuccess, onError), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Connecting(BLEMgrImpl());
    ASSERT_EQ(PlatformMgrImpl().GLibMatterContextInvokeSync(
                  +[](BluezConnection * pending) {
                      BLEManagerImpl::HandleNewConnection(pending);
                      return CHIP_NO_ERROR;
                  },
                  &connection),
              CHIP_NO_ERROR);
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_ERROR_BUSY);
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, onSuccess, onError), CHIP_ERROR_BUSY);
    ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
              CHIP_NO_ERROR);
    PlatformMgr().RunEventLoop();
    EXPECT_EQ(callbacks, 0u);
    EXPECT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, onSuccess, onError), CHIP_NO_ERROR);
    EXPECT_EQ(layer.CancelBleIncompleteConnection(&callbacks), CHIP_NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(closes.mutex);
        EXPECT_TRUE(closes.changed.wait_for(lock, std::chrono::seconds(5), [&] { return closes.calls == 1; }));
    }
    g_dbus_connection_flush_sync(peer.client.get(), nullptr, nullptr);
    g_dbus_connection_close_sync(peer.client.get(), nullptr, nullptr);
    g_dbus_connection_remove_filter(peer.server.get(), filter);
    EXPECT_EQ(closes.calls, 1u);
    BLEManagerImplTestAccess::RestorePeripheral(BLEMgrImpl());
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
    EXPECT_EQ(layer->NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, onError), CHIP_NO_ERROR);
    EXPECT_EQ(layer->CancelBleIncompleteConnection(&callbacks), CHIP_NO_ERROR);
}

} // namespace
#else
TEST(TestBleFallback, BleTransportDisabled)
{
    EXPECT_EQ(CONFIG_NETWORK_LAYER_BLE, 0);
}
#endif
