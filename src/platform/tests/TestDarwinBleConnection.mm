#include <pw_unit_test/framework.h>

#include <ble/Ble.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/Darwin/BleApplicationDelegateImpl.h>
#include <platform/Darwin/BleConnectionDelegateImpl.h>
#include <platform/Darwin/BlePlatformDelegateImpl.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>
#import <objc/runtime.h>

@interface MTRBleConnection : NSObject
@property (nonatomic, readonly) BOOL isConnecting;
@property (nonatomic, readonly) dispatch_source_t timer;
@property (nonatomic, strong) CBCentralManager * centralManager;
- (void)centralManagerDidUpdateState:(CBCentralManager *)central;
- (void)dispatchConnectionComplete:(CBPeripheral *)peripheral;
- (void)dispatchConnectionError:(CHIP_ERROR)error;
@end

namespace chip::DeviceLayer::Internal {
extern MTRBleConnection * ble;
}

@interface MTRTestCentralManager : NSObject
@property (nonatomic, weak) id delegate;
@property (nonatomic) unsigned scans;
- (instancetype)initWithDelegate:(id)delegate queue:(dispatch_queue_t)queue;
- (CBManagerState)state;
- (void)scanForPeripheralsWithServices:(NSArray *)services options:(NSDictionary *)options;
- (void)stopScan;
@end

@implementation MTRTestCentralManager
- (instancetype)initWithDelegate:(id)delegate queue:(dispatch_queue_t)queue
{
    self = [super init];
    if (self) {
        _delegate = delegate;
    }
    return self;
}
- (CBManagerState)state
{
    return CBManagerStatePoweredOn;
}
- (void)scanForPeripheralsWithServices:(NSArray *)services options:(NSDictionary *)options
{
    ++_scans;
}
- (void)stopScan
{
}
@end

namespace {

using namespace chip;
using namespace chip::Ble;
using namespace chip::DeviceLayer;
using namespace chip::DeviceLayer::Internal;

id AllocateTestCentral(id, SEL) __attribute__((ns_returns_retained));
id AllocateTestCentral(id, SEL)
{
    return [MTRTestCentralManager alloc];
}

class TestDarwinBleConnection : public ::testing::Test, public BleLayerDelegate {
public:
    class Scanner : public BleScannerDelegate {
    public:
        unsigned stopped = 0;
        void OnBleScanStopped() override { ++stopped; }
    } scanner;

    BleLayer layer;
    BleConnectionDelegateImpl backend;
    BleApplicationDelegateImpl application;
    BlePlatformDelegateImpl platform;
    unsigned successes = 0;
    unsigned errors = 0;
    CHIP_ERROR lastError = CHIP_NO_ERROR;
    bool retryOnError = false;
    CHIP_ERROR retryResult = CHIP_ERROR_INTERNAL;

    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        static TestOnlyCommissionableDataProvider provider;
        SetCommissionableDataProvider(&provider);
    }

    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        auto selector = @selector(alloc);
        auto method = class_getClassMethod([CBCentralManager class], selector);
        mOriginalAllocation = method_getImplementation(method);
        class_replaceMethod(object_getClass([CBCentralManager class]), selector,
            reinterpret_cast<IMP>(AllocateTestCentral), method_getTypeEncoding(method));
        ASSERT_EQ(PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
        ASSERT_EQ(layer.Init(&platform, &backend, &application, &SystemLayer()), CHIP_NO_ERROR);
        layer.mBleTransport = this;
        ASSERT_EQ(PlatformMgr().StartEventLoopTask(), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            EXPECT_EQ(layer.CancelBleIncompleteConnection(), CHIP_NO_ERROR);
            backend.StopScan();
            layer.mBleTransport = nullptr;
            layer.Shutdown();
        });
        EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR);
        PlatformMgr().Shutdown();
        class_replaceMethod(object_getClass([CBCentralManager class]), @selector(alloc),
            mOriginalAllocation, method_getTypeEncoding(class_getClassMethod([CBCentralManager class], @selector(alloc))));
    }

    CHIP_ERROR Request()
    {
        return layer.NewBleConnectionByDiscriminator(
            SetupDiscriminator(), this,
            [](void * context, BLE_CONNECTION_OBJECT) { ++static_cast<TestDarwinBleConnection *>(context)->successes; },
            [](void * context, CHIP_ERROR error) {
                auto & test = *static_cast<TestDarwinBleConnection *>(context);
                ++test.errors;
                test.lastError = error;
                if (test.retryOnError) {
                    test.retryResult = test.Request();
                }
            });
    }

    void OnBleConnectionComplete(BLEEndPoint *) override {}
    void OnBleConnectionError(CHIP_ERROR) override {}
    void OnEndPointConnectComplete(BLEEndPoint *, CHIP_ERROR) override {}
    void OnEndPointMessageReceived(BLEEndPoint *, System::PacketBufferHandle &&) override {}
    void OnEndPointConnectionClosed(BLEEndPoint *, CHIP_ERROR) override {}
    CHIP_ERROR SetEndPoint(BLEEndPoint *) override { return CHIP_NO_ERROR; }

private:
    IMP mOriginalAllocation = nullptr;
};

TEST_F(TestDarwinBleConnection, NormalBrowsingRetiresPendingRequest)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        auto * previous = ble;
        ASSERT_TRUE(previous.timer != nullptr);

        backend.StartScan(&scanner);
        EXPECT_EQ(errors, 1u);
        EXPECT_EQ(lastError, BLE_ERROR_APP_CLOSED_CONNECTION);
        EXPECT_TRUE(previous.timer == nullptr);
        ASSERT_FALSE(ble.isConnecting);
        [ble centralManagerDidUpdateState:ble.centralManager];
        EXPECT_EQ(((MTRTestCentralManager *) ble.centralManager).scans, 1u);
        backend.StopScan();
        EXPECT_EQ(scanner.stopped, 1u);
        EXPECT_TRUE(ble == nil);

        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        [previous dispatchConnectionError:CHIP_ERROR_INTERNAL];
        EXPECT_EQ(errors, 1u);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
    });
}

TEST_F(TestDarwinBleConnection, BrowsingReplacementAllowsReentrantRetry)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        retryOnError = true;
        backend.StartScan(&scanner);
        EXPECT_EQ(errors, 1u);
        EXPECT_EQ(retryResult, CHIP_NO_ERROR);
        EXPECT_EQ(scanner.stopped, 1u);
        EXPECT_TRUE(ble.isConnecting);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
    });
}

TEST_F(TestDarwinBleConnection, PrewarmPreservesPendingRequest)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        auto * previous = ble;
        backend.StartScan(&scanner, BleScanMode::kPreWarm);
        EXPECT_TRUE(ble == previous);
        EXPECT_TRUE(previous.timer != nullptr);
        EXPECT_EQ(errors, 0u);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{ EXPECT_EQ(scanner.stopped, 1u); });
}

}
