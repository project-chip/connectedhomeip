#include <pw_unit_test/framework.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Linux/BLEManagerImpl.h>

namespace chip::DeviceLayer::Internal {

class BLEManagerImplTestAccess
{
public:
    static Ble::BleLayer & Layer(BLEManagerImpl & manager) { return manager; }
    static void DisableService(BLEManagerImpl & manager)
    {
        manager.mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }

    static void Request(BLEManagerImpl & manager, void * context)
    {
        manager.Ble::BleConnectionDelegate::OnConnectionComplete = [](void * state, BLE_CONNECTION_OBJECT) {
            ++*static_cast<unsigned *>(state);
        };
        manager.Ble::BleConnectionDelegate::OnConnectionError = [](void * state, CHIP_ERROR) { ++*static_cast<unsigned *>(state); };
        manager.NewConnection(nullptr, context, SetupDiscriminator());
    }

    static CHIP_ERROR Cancel(BLEManagerImpl & manager) { return manager.CancelConnection(); }
    static void RemoveAdapter(BLEManagerImpl & manager)
    {
        ChipDeviceEvent event{ .Type = DeviceEventType::kPlatformLinuxBLEAdapterRemoved };
        event.Platform.BLEAdapter.mAdapterId = manager.mAdapterId;
        manager.HandlePlatformSpecificBLEEvent(&event);
    }
    static uint64_t Generation(BLEManagerImpl & manager) { return manager.mConnectionGeneration.load(); }
    static bool Pending(BLEManagerImpl & manager) { return manager.mBLEScanConfig.mBleScanState == BleScanState::kScanPending; }
    static void SetConnecting(BLEManagerImpl & manager) { manager.mBLEScanConfig.mBleScanState = BleScanState::kConnecting; }
    static void SetScanning(BLEManagerImpl & manager)
    {
        manager.mBLEScanConfig.mBleScanState = BleScanState::kScanForDiscriminator;
    }
    static void QueueScannedDevice(BLEManagerImpl & manager, BluezDevice1 & device)
    {
        manager.OnDeviceScanned(device, Ble::ChipBLEDeviceIdentificationInfo{});
    }
    static void DeliverError(BLEManagerImpl & manager, uint64_t generation)
    {
        ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLECentralConnectFailed,
                               .Platform = {
                                   .BLECentralConnectFailed = { .mError = CHIP_ERROR_TIMEOUT, .mGeneration = generation } } };
        manager.HandlePlatformSpecificBLEEvent(&event);
    }
    static void DeliverSuccess(BLEManagerImpl & manager, uint64_t generation, BLE_CONNECTION_OBJECT connection)
    {
        ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLECentralConnected,
                               .Platform = { .BLECentralConnected = { .mConnection = connection, .mGeneration = generation } } };
        manager.HandlePlatformSpecificBLEEvent(&event);
    }
};

namespace {

class TestBLEManagerImpl : public ::testing::Test, public Ble::BleLayerDelegate
{
public:
    void SetUp() override
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        BLEManagerImplTestAccess::DisableService(BLEMgrImpl());
        BLEManagerImplTestAccess::Layer(BLEMgrImpl()).mBleTransport = this;
    }

    void TearDown() override
    {
        EXPECT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
        Drain();
        BLEManagerImplTestAccess::Layer(BLEMgrImpl()).mBleTransport = nullptr;
        PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

    void Drain()
    {
        ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
                  CHIP_NO_ERROR);
        PlatformMgr().RunEventLoop();
    }

    void OnBleConnectionComplete(Ble::BLEEndPoint *) override { FAIL(); }
    void OnBleConnectionError(CHIP_ERROR) override { FAIL(); }
    void OnEndPointConnectComplete(Ble::BLEEndPoint *, CHIP_ERROR) override { FAIL(); }
    void OnEndPointMessageReceived(Ble::BLEEndPoint *, System::PacketBufferHandle &&) override { FAIL(); }
    void OnEndPointConnectionClosed(Ble::BLEEndPoint *, CHIP_ERROR) override { FAIL(); }
    CHIP_ERROR SetEndPoint(Ble::BLEEndPoint *) override { return CHIP_ERROR_INCORRECT_STATE; }
};

class TestBluezConnect : public TestBLEManagerImpl
{
public:
    void SetUp() override
    {
        mBus.reset(g_test_dbus_new(G_TEST_DBUS_NONE));
        g_test_dbus_up(mBus.get());
        TestBLEManagerImpl::SetUp();
        GAutoPtr<GError> error;
        const auto flags = static_cast<GDBusConnectionFlags>(G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
                                                             G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION);
        mServer.reset(g_dbus_connection_new_for_address_sync(g_test_dbus_get_bus_address(mBus.get()), flags, nullptr, nullptr,
                                                             &error.GetReceiver()));
        ASSERT_NE(mServer.get(), nullptr);
        mClient.reset(g_dbus_connection_new_for_address_sync(g_test_dbus_get_bus_address(mBus.get()), flags, nullptr, nullptr,
                                                             &error.GetReceiver()));
        ASSERT_NE(mClient.get(), nullptr);
        mDevice.reset(bluez_device1_skeleton_new());
        g_signal_connect(mDevice.get(), "handle-connect",
                         G_CALLBACK(+[](BluezDevice1 *, GDBusMethodInvocation * invocation, TestBluezConnect * self) -> gboolean {
                             std::lock_guard<std::mutex> lock(self->mMutex);
                             self->mInvocation.reset(static_cast<GDBusMethodInvocation *>(g_object_ref(invocation)));
                             self->mConnected.notify_one();
                             return TRUE;
                         }),
                         this);
        g_signal_connect(mDevice.get(), "handle-disconnect",
                         G_CALLBACK(+[](BluezDevice1 * device, GDBusMethodInvocation * invocation, void *) -> gboolean {
                             bluez_device1_complete_disconnect(device, invocation);
                             return TRUE;
                         }),
                         nullptr);
        ASSERT_EQ(PlatformMgrImpl().GLibMatterContextInvokeSync(
                      +[](TestBluezConnect * self) {
                          return g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(self->mDevice.get()),
                                                                  self->mServer.get(), "/org/bluez/test", nullptr)
                              ? CHIP_NO_ERROR
                              : CHIP_ERROR_INTERNAL;
                      },
                      this),
                  CHIP_NO_ERROR);
        mProxy.reset(bluez_device1_proxy_new_sync(mClient.get(), G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES,
                                                  g_dbus_connection_get_unique_name(mServer.get()), "/org/bluez/test", nullptr,
                                                  &error.GetReceiver()));
        ASSERT_NE(mProxy.get(), nullptr);
        g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(mProxy.get()), 5000);
    }

    void TearDown() override
    {
        EXPECT_EQ(PlatformMgrImpl().GLibMatterContextInvokeSync(
                      +[](TestBluezConnect * self) {
                          if (self->mInvocation)
                          {
                              bluez_device1_complete_connect(self->mDevice.get(), self->mInvocation.get());
                              self->mInvocation.reset();
                          }
                          g_dbus_interface_skeleton_unexport(G_DBUS_INTERFACE_SKELETON(self->mDevice.get()));
                          return CHIP_NO_ERROR;
                      },
                      this),
                  CHIP_NO_ERROR);
        mProxy.reset();
        mDevice.reset();
        g_dbus_connection_close_sync(mClient.get(), nullptr, nullptr);
        g_dbus_connection_close_sync(mServer.get(), nullptr, nullptr);
        mClient.reset();
        mServer.reset();
        TestBLEManagerImpl::TearDown();
        g_test_dbus_down(mBus.get());
        mBus.reset();
    }

    std::unique_ptr<GTestDBus, decltype(&g_object_unref)> mBus{ nullptr, g_object_unref };
    GAutoPtr<GDBusConnection> mServer;
    GAutoPtr<GDBusConnection> mClient;
    GAutoPtr<BluezDevice1> mDevice;
    GAutoPtr<BluezDevice1> mProxy;
    std::unique_ptr<GDBusMethodInvocation, decltype(&g_object_unref)> mInvocation{ nullptr, g_object_unref };
    std::mutex mMutex;
    std::condition_variable mConnected;
};

TEST_F(TestBluezConnect, PendingDbusConnectDoesNotBlockMatterCancellation)
{
    BluezObjectManager objectManager;
    BluezEndpoint endpoint(objectManager);
    ASSERT_EQ(endpoint.ConnectDevice(*mProxy, 1), CHIP_NO_ERROR);
    {
        std::unique_lock<std::mutex> lock(mMutex);
        ASSERT_TRUE(mConnected.wait_for(lock, std::chrono::seconds(5), [this] { return mInvocation != nullptr; }));
    }
    bool cancelled = false;
    ASSERT_EQ(SystemLayer().ScheduleLambda([&endpoint, &cancelled] {
        endpoint.CancelConnect();
        cancelled = true;
    }),
              CHIP_NO_ERROR);
    Drain();
    EXPECT_TRUE(cancelled);
}

TEST_F(TestBLEManagerImpl, CancelBeforeScheduledScanStarts)
{
    unsigned callbacks = 0;
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), &callbacks);
    ASSERT_TRUE(BLEManagerImplTestAccess::Pending(BLEMgrImpl()));
    ASSERT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
    Drain();
    EXPECT_FALSE(BLEManagerImplTestAccess::Pending(BLEMgrImpl()));
    EXPECT_EQ(callbacks, 0u);
}

TEST_F(TestBLEManagerImpl, UnsupportedObjectRequestReportsError)
{
    unsigned callbacks = 0;
    auto onError       = [](void * state, CHIP_ERROR error) {
        EXPECT_EQ(error, CHIP_ERROR_NOT_IMPLEMENTED);
        ++*static_cast<unsigned *>(state);
    };
    auto & layer = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    ASSERT_EQ(layer.NewBleConnectionByObject(BLE_CONNECTION_UNINITIALIZED, &callbacks, nullptr, onError), CHIP_NO_ERROR);
    EXPECT_EQ(callbacks, 1u);
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &callbacks, nullptr, nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(BLEManagerImplTestAccess::Pending(BLEMgrImpl()));
    ASSERT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
    Drain();
    EXPECT_EQ(callbacks, 1u);
}

TEST_F(TestBLEManagerImpl, AdapterRemovalReportsErrorAndInvalidatesQueuedStart)
{
    unsigned oldCallbacks = 0;
    unsigned newCallbacks = 0;
    auto onError          = [](void * state, CHIP_ERROR error) {
        EXPECT_EQ(error, BLE_ERROR_ADAPTER_UNAVAILABLE);
        ++*static_cast<unsigned *>(state);
    };
    auto & layer = BLEManagerImplTestAccess::Layer(BLEMgrImpl());
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &oldCallbacks, nullptr, onError), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::SetScanning(BLEMgrImpl());
    auto oldGeneration = BLEManagerImplTestAccess::Generation(BLEMgrImpl());
    BLEManagerImplTestAccess::RemoveAdapter(BLEMgrImpl());
    EXPECT_EQ(oldCallbacks, 1u);
    EXPECT_NE(BLEManagerImplTestAccess::Generation(BLEMgrImpl()), oldGeneration);
    ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), &newCallbacks, nullptr, onError), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::SetConnecting(BLEMgrImpl());
    BLEManagerImplTestAccess::DeliverError(BLEMgrImpl(), oldGeneration);
    EXPECT_EQ(newCallbacks, 0u);
    ASSERT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
    Drain();
    EXPECT_EQ(oldCallbacks, 1u);
    EXPECT_EQ(newCallbacks, 0u);
}

TEST_F(TestBLEManagerImpl, CancelReleasesQueuedScannedDeviceBeforeWorkRuns)
{
    unsigned callbacks = 0;
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), &callbacks);
    auto * device  = bluez_device1_skeleton_new();
    bool finalized = false;
    g_object_weak_ref(
        G_OBJECT(device), [](void * context, GObject *) { *static_cast<bool *>(context) = true; }, &finalized);
    BLEManagerImplTestAccess::QueueScannedDevice(BLEMgrImpl(), *device);
    g_object_unref(device);
    EXPECT_FALSE(finalized);
    ASSERT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
    EXPECT_TRUE(finalized);
    Drain();
    EXPECT_EQ(callbacks, 0u);
}

TEST_F(TestBLEManagerImpl, CancelledQueuedStartCannotStartNewRequest)
{
    unsigned cancelledCallbacks = 0;
    unsigned newCallbacks       = 0;
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), &cancelledCallbacks);
    ASSERT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
    ASSERT_EQ(SystemLayer().ScheduleLambda([&newCallbacks] {
        EXPECT_EQ(newCallbacks, 0u);
        EXPECT_TRUE(BLEManagerImplTestAccess::Pending(BLEMgrImpl()));
    }),
              CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), &newCallbacks);
    Drain();
    EXPECT_EQ(cancelledCallbacks, 0u);
    EXPECT_EQ(newCallbacks, 1u);
}

TEST_F(TestBLEManagerImpl, OldErrorCannotRetireNewRequest)
{
    unsigned cancelledCallbacks = 0;
    unsigned newCallbacks       = 0;
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), &cancelledCallbacks);
    auto oldGeneration = BLEManagerImplTestAccess::Generation(BLEMgrImpl());
    ASSERT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), &newCallbacks);
    BLEManagerImplTestAccess::SetConnecting(BLEMgrImpl());
    BLEManagerImplTestAccess::DeliverError(BLEMgrImpl(), oldGeneration);
    EXPECT_EQ(cancelledCallbacks, 0u);
    EXPECT_EQ(newCallbacks, 0u);
    BLEManagerImplTestAccess::DeliverError(BLEMgrImpl(), BLEManagerImplTestAccess::Generation(BLEMgrImpl()));
    EXPECT_EQ(newCallbacks, 1u);
}

TEST_F(TestBLEManagerImpl, OldSuccessCannotReachNewCallerOrDereferenceRetiredConnection)
{
    unsigned cancelledCallbacks = 0;
    unsigned newCallbacks       = 0;
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), &cancelledCallbacks);
    auto oldGeneration = BLEManagerImplTestAccess::Generation(BLEMgrImpl());
    ASSERT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), &newCallbacks);
    BLEManagerImplTestAccess::SetConnecting(BLEMgrImpl());
    uint8_t retiredStorage = 0;
    auto * retired         = reinterpret_cast<BLE_CONNECTION_OBJECT>(&retiredStorage);
    BLEManagerImplTestAccess::DeliverSuccess(BLEMgrImpl(), oldGeneration, retired);
    EXPECT_EQ(cancelledCallbacks, 0u);
    EXPECT_EQ(newCallbacks, 0u);
    BLEManagerImplTestAccess::DeliverSuccess(BLEMgrImpl(), BLEManagerImplTestAccess::Generation(BLEMgrImpl()), retired);
    EXPECT_EQ(newCallbacks, 1u);
}

} // namespace
} // namespace chip::DeviceLayer::Internal
