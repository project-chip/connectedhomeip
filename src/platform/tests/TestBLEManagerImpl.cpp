#include <pw_unit_test/framework.h>

#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Linux/BLEManagerImpl.h>

namespace chip::DeviceLayer::Internal {

class BLEManagerImplTestAccess
{
public:
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
    static uint64_t Generation(BLEManagerImpl & manager) { return manager.mConnectionGeneration.load(); }
    static bool Pending(BLEManagerImpl & manager) { return manager.mBLEScanConfig.mBleScanState == BleScanState::kScanPending; }
    static void SetConnecting(BLEManagerImpl & manager) { manager.mBLEScanConfig.mBleScanState = BleScanState::kConnecting; }
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

class TestBLEManagerImpl : public ::testing::Test
{
public:
    void SetUp() override
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        BLEManagerImplTestAccess::DisableService(BLEMgrImpl());
    }

    void TearDown() override
    {
        EXPECT_EQ(BLEManagerImplTestAccess::Cancel(BLEMgrImpl()), CHIP_NO_ERROR);
        Drain();
        PlatformMgr().Shutdown();
        Platform::MemoryShutdown();
    }

    void Drain()
    {
        ASSERT_EQ(PlatformMgr().ScheduleWork([](intptr_t) { EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR); }),
                  CHIP_NO_ERROR);
        PlatformMgr().RunEventLoop();
    }
};

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

TEST_F(TestBLEManagerImpl, CancelReleasesQueuedScannedDeviceBeforeWorkRuns)
{
    unsigned callbacks = 0;
    BLEManagerImplTestAccess::Request(BLEMgrImpl(), &callbacks);
    auto * device  = bluez_device1_skeleton_new();
    bool finalized = false;
    g_object_weak_ref(G_OBJECT(device), [](void * context, GObject *) { *static_cast<bool *>(context) = true; }, &finalized);
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
