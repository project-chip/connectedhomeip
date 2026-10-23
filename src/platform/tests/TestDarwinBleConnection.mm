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
@property (nonatomic, readonly) BOOL hasPendingConnectionCallbacks;
@property (nonatomic, readonly) dispatch_source_t timer;
@property (nonatomic, strong) CBCentralManager * centralManager;
@property (nonatomic, strong) NSNumber * matchedLongDiscriminator;
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

struct ConnectionCallbacks {
    unsigned successes = 0;
    unsigned errors = 0;
    uint16_t matchedDiscriminator = 0;
    CHIP_ERROR lastError = CHIP_NO_ERROR;

    static void OnComplete(void * context, BLE_CONNECTION_OBJECT)
    {
        ++static_cast<ConnectionCallbacks *>(context)->successes;
    }

    static void OnPluralComplete(void * context, uint16_t discriminator, BLE_CONNECTION_OBJECT)
    {
        auto & callbacks = *static_cast<ConnectionCallbacks *>(context);
        ++callbacks.successes;
        callbacks.matchedDiscriminator = discriminator;
    }

    static void OnError(void * context, CHIP_ERROR error)
    {
        auto & callbacks = *static_cast<ConnectionCallbacks *>(context);
        ++callbacks.errors;
        callbacks.lastError = error;
    }
};

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
    ConnectionCallbacks plural;
    ConnectionCallbacks rejected;

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

    CHIP_ERROR Request(bool byObject = false)
    {
        auto onSuccess = [](void * context, BLE_CONNECTION_OBJECT) { ++static_cast<TestDarwinBleConnection *>(context)->successes; };
        auto onError = [](void * context, CHIP_ERROR error) {
            auto & test = *static_cast<TestDarwinBleConnection *>(context);
            ++test.errors;
            test.lastError = error;
            if (test.retryOnError) {
                test.retryResult = test.Request();
            }
        };
        if (byObject) {
            return layer.NewBleConnectionByObject(BLE_CONNECTION_UNINITIALIZED, this, onSuccess, onError);
        }
        return layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), this, onSuccess, onError);
    }

    CHIP_ERROR RequestPlural(ConnectionCallbacks & callbacks, bool direct = false)
    {
        SetupDiscriminator discriminators[2];
        discriminators[0].SetLongValue(1234);
        discriminators[1].SetLongValue(2345);
        if (direct) {
            return backend.NewConnection(&layer, &callbacks, Span<const SetupDiscriminator>(discriminators),
                ConnectionCallbacks::OnPluralComplete, ConnectionCallbacks::OnError);
        }
        return layer.NewBleConnectionByDiscriminators(Span<const SetupDiscriminator>(discriminators), &callbacks,
            ConnectionCallbacks::OnPluralComplete, ConnectionCallbacks::OnError);
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

TEST_F(TestDarwinBleConnection, CanceledObjectErrorDoesNotRetireReplacement)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_TRUE(ble == nil);
        ASSERT_EQ(layer.NewBleConnectionByObject(BLE_CONNECTION_UNINITIALIZED, this, nullptr, nullptr), CHIP_NO_ERROR);
        ASSERT_EQ(layer.CancelBleIncompleteConnection(this), CHIP_NO_ERROR);
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        EXPECT_EQ(errors, 0u);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        EXPECT_EQ(errors, 0u);
        ASSERT_EQ(Request(), CHIP_ERROR_BUSY);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 2u);
    });
}

TEST_F(TestDarwinBleConnection, ObjectErrorsRemainAsynchronousAndReusable)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(Request(true), CHIP_NO_ERROR);
        EXPECT_EQ(errors, 0u);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        EXPECT_EQ(errors, 1u);
        EXPECT_EQ(lastError, CHIP_ERROR_INCORRECT_STATE);
        ASSERT_EQ(Request(true), CHIP_NO_ERROR);
        EXPECT_EQ(errors, 1u);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        EXPECT_EQ(errors, 2u);
        EXPECT_EQ(lastError, CHIP_ERROR_INCORRECT_STATE);
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
    });
}

TEST_F(TestDarwinBleConnection, CanceledObjectErrorDoesNotConsumeFreshObjectError)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(Request(true), CHIP_NO_ERROR);
        ASSERT_EQ(layer.CancelBleIncompleteConnection(this), CHIP_NO_ERROR);
        dispatch_async(PlatformMgrImpl().GetWorkQueue(), ^{
            EXPECT_EQ(errors, 0u);
            EXPECT_EQ(Request(), CHIP_ERROR_BUSY);
        });
        ASSERT_EQ(Request(true), CHIP_NO_ERROR);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        EXPECT_EQ(errors, 1u);
        EXPECT_EQ(lastError, CHIP_ERROR_INCORRECT_STATE);
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
    });
}

TEST_F(TestDarwinBleConnection, NormalCancellationAllowsFreshRequest)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        auto * previous = ble;
        ASSERT_TRUE(previous.timer != nullptr);
        ASSERT_EQ(layer.CancelBleIncompleteConnection(this), CHIP_NO_ERROR);
        EXPECT_TRUE(previous.timer == nullptr);
        EXPECT_TRUE(ble == nil);
        EXPECT_EQ(errors, 0u);
        EXPECT_EQ(successes, 0u);
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{ EXPECT_EQ(errors, 0u); });
}

TEST_F(TestDarwinBleConnection, SinglePendingRejectsPluralCommonAndBackend)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        auto * previous = ble;
        auto timer = previous.timer;
        EXPECT_EQ(RequestPlural(rejected), CHIP_ERROR_BUSY);
        EXPECT_EQ(RequestPlural(rejected, true), CHIP_ERROR_BUSY);
        EXPECT_TRUE(ble == previous);
        EXPECT_TRUE(previous.timer == timer);
        EXPECT_TRUE(previous.hasPendingConnectionCallbacks);
        [previous dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
        EXPECT_EQ(errors, 0u);
        EXPECT_EQ(rejected.successes, 0u);
        EXPECT_EQ(rejected.errors, 0u);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{ EXPECT_EQ(rejected.errors, 0u); });
}

TEST_F(TestDarwinBleConnection, PluralPendingRejectsPluralCommonAndBackend)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
        auto * previous = ble;
        auto timer = previous.timer;
        EXPECT_EQ(RequestPlural(rejected), CHIP_ERROR_BUSY);
        EXPECT_EQ(RequestPlural(rejected, true), CHIP_ERROR_BUSY);
        EXPECT_TRUE(ble == previous);
        EXPECT_TRUE(previous.timer == timer);
        EXPECT_TRUE(previous.hasPendingConnectionCallbacks);
        previous.matchedLongDiscriminator = @1234;
        [previous dispatchConnectionComplete:nil];
        EXPECT_EQ(plural.successes, 1u);
        EXPECT_EQ(plural.matchedDiscriminator, 1234u);
        EXPECT_EQ(plural.errors, 0u);
        EXPECT_EQ(rejected.successes, 0u);
        EXPECT_EQ(rejected.errors, 0u);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{ EXPECT_EQ(rejected.errors, 0u); });
}

TEST_F(TestDarwinBleConnection, PluralPendingRejectsSingleAndObjectAsynchronously)
{
    for (bool byObject : { false, true }) {
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
            auto * previous = ble;
            auto timer = previous.timer;
            ASSERT_EQ(Request(byObject), CHIP_NO_ERROR);
            EXPECT_EQ(errors, byObject ? 1u : 0u);
            EXPECT_EQ(Request(), CHIP_ERROR_BUSY);
            EXPECT_TRUE(ble == previous);
            EXPECT_TRUE(previous.timer == timer);
            EXPECT_TRUE(previous.hasPendingConnectionCallbacks);
            backend.OnConnectionError = nullptr;
        });
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            EXPECT_EQ(errors, byObject ? 2u : 1u);
            EXPECT_EQ(lastError, CHIP_ERROR_BUSY);
            EXPECT_TRUE(ble.hasPendingConnectionCallbacks);
            ble.matchedLongDiscriminator = @1234;
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(plural.successes, byObject ? 2u : 1u);
            EXPECT_EQ(plural.errors, 0u);
            ASSERT_EQ(Request(), CHIP_NO_ERROR);
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(successes, byObject ? 2u : 1u);
        });
    }
}

TEST_F(TestDarwinBleConnection, SinglePendingRejectsDirectSingleAndObjectAsynchronously)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        auto * previous = ble;
        auto timer = previous.timer;
        backend.OnConnectionComplete = ConnectionCallbacks::OnComplete;
        backend.OnConnectionError = ConnectionCallbacks::OnError;
        backend.NewConnection(&layer, &rejected, SetupDiscriminator());
        backend.NewConnection(&layer, &rejected, BLE_CONNECTION_UNINITIALIZED);
        backend.OnConnectionError = nullptr;
        EXPECT_EQ(rejected.errors, 0u);
        EXPECT_TRUE(ble == previous);
        EXPECT_TRUE(previous.timer == timer);
        EXPECT_EQ(Request(), CHIP_ERROR_BUSY);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        EXPECT_EQ(rejected.errors, 2u);
        EXPECT_EQ(rejected.lastError, CHIP_ERROR_BUSY);
        EXPECT_EQ(rejected.successes, 0u);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
        EXPECT_EQ(errors, 0u);
    });
}

TEST_F(TestDarwinBleConnection, EachPendingCallbackSlotRejectsOverlap)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        SetupDiscriminator discriminator;
        backend.OnConnectionComplete = ConnectionCallbacks::OnComplete;
        backend.OnConnectionError = nullptr;
        backend.NewConnection(&layer, &plural, discriminator);
        EXPECT_TRUE(ble.hasPendingConnectionCallbacks);
        EXPECT_EQ(RequestPlural(rejected, true), CHIP_ERROR_BUSY);
        ASSERT_EQ(backend.CancelConnection(), CHIP_NO_ERROR);

        ASSERT_EQ(backend.NewConnection(&layer, &plural, Span<const SetupDiscriminator>(&discriminator, 1),
                      ConnectionCallbacks::OnPluralComplete, nullptr),
            CHIP_NO_ERROR);
        EXPECT_TRUE(ble.hasPendingConnectionCallbacks);
        EXPECT_EQ(RequestPlural(rejected, true), CHIP_ERROR_BUSY);
        ASSERT_EQ(backend.CancelConnection(), CHIP_NO_ERROR);

        ASSERT_EQ(backend.NewConnection(&layer, &plural, Span<const SetupDiscriminator>(&discriminator, 1),
                      nullptr, ConnectionCallbacks::OnError),
            CHIP_NO_ERROR);
        EXPECT_TRUE(ble.hasPendingConnectionCallbacks);
        EXPECT_EQ(RequestPlural(rejected, true), CHIP_ERROR_BUSY);
        [ble dispatchConnectionError:CHIP_ERROR_INTERNAL];
        EXPECT_EQ(plural.errors, 1u);
        EXPECT_EQ(plural.lastError, CHIP_ERROR_INTERNAL);
        EXPECT_EQ(rejected.errors, 0u);
        EXPECT_EQ(rejected.successes, 0u);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{ EXPECT_EQ(rejected.errors, 0u); });
}

TEST_F(TestDarwinBleConnection, CompletedPluralAllowsFreshSingleAndPlural)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
        ble.matchedLongDiscriminator = @1234;
        [ble dispatchConnectionComplete:nil];
        EXPECT_TRUE(ble.isConnecting);
        EXPECT_FALSE(ble.hasPendingConnectionCallbacks);
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        [ble dispatchConnectionComplete:nil];
        ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
        ble.matchedLongDiscriminator = @1234;
        [ble dispatchConnectionComplete:nil];
        EXPECT_TRUE(ble.isConnecting);
        EXPECT_FALSE(ble.hasPendingConnectionCallbacks);
        ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
        ble.matchedLongDiscriminator = @2345;
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(plural.successes, 3u);
        EXPECT_EQ(plural.matchedDiscriminator, 2345u);
        EXPECT_EQ(plural.errors, 0u);
        EXPECT_EQ(successes, 1u);
        EXPECT_EQ(errors, 0u);
    });
}

TEST_F(TestDarwinBleConnection, PluralCompletionAllowsReentrantSingle)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        SetupDiscriminator discriminator;
        ASSERT_EQ(layer.NewBleConnectionByDiscriminators(
                      Span<const SetupDiscriminator>(&discriminator, 1), this,
                      [](void * context, uint16_t, BLE_CONNECTION_OBJECT) {
                          auto & test = *static_cast<TestDarwinBleConnection *>(context);
                          ++test.plural.successes;
                          test.retryResult = test.Request();
                      }, nullptr),
            CHIP_NO_ERROR);
        auto * previous = ble;
        previous.matchedLongDiscriminator = @1234;
        [previous dispatchConnectionComplete:nil];
        EXPECT_EQ(plural.successes, 1u);
        EXPECT_EQ(retryResult, CHIP_NO_ERROR);
        EXPECT_TRUE(ble != previous);
        EXPECT_TRUE(ble.hasPendingConnectionCallbacks);
        [previous dispatchConnectionError:CHIP_ERROR_INTERNAL];
        EXPECT_EQ(errors, 0u);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
    });
}

TEST_F(TestDarwinBleConnection, QueuedBusyErrorPreservesReentrantPluralReplacement)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        SetupDiscriminator discriminator;
        ASSERT_EQ(layer.NewBleConnectionByDiscriminators(
                      Span<const SetupDiscriminator>(&discriminator, 1), this,
                      [](void * context, uint16_t, BLE_CONNECTION_OBJECT) {
                          auto & test = *static_cast<TestDarwinBleConnection *>(context);
                          ++test.plural.successes;
                          test.retryResult = test.RequestPlural(test.plural, true);
                      }, nullptr),
            CHIP_NO_ERROR);
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        EXPECT_EQ(errors, 0u);
        auto * previous = ble;
        previous.matchedLongDiscriminator = @1234;
        [previous dispatchConnectionComplete:nil];
        EXPECT_EQ(plural.successes, 1u);
        EXPECT_EQ(retryResult, CHIP_NO_ERROR);
        EXPECT_TRUE(ble != previous);
        EXPECT_TRUE(ble.hasPendingConnectionCallbacks);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        EXPECT_EQ(errors, 1u);
        EXPECT_EQ(lastError, CHIP_ERROR_BUSY);
        EXPECT_TRUE(ble.hasPendingConnectionCallbacks);
        ble.matchedLongDiscriminator = @2345;
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(plural.successes, 2u);
        EXPECT_EQ(plural.matchedDiscriminator, 2345u);
        EXPECT_EQ(plural.errors, 0u);
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
    });
}

TEST_F(TestDarwinBleConnection, PluralErrorAllowsReentrantPlural)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        SetupDiscriminator discriminator;
        ASSERT_EQ(layer.NewBleConnectionByDiscriminators(Span<const SetupDiscriminator>(&discriminator, 1), this, nullptr,
                      [](void * context, CHIP_ERROR error) {
                          auto & test = *static_cast<TestDarwinBleConnection *>(context);
                          ++test.plural.errors;
                          test.plural.lastError = error;
                          test.retryResult = test.RequestPlural(test.plural);
                      }),
            CHIP_NO_ERROR);
        auto * previous = ble;
        [previous dispatchConnectionError:CHIP_ERROR_INTERNAL];
        EXPECT_EQ(plural.errors, 1u);
        EXPECT_EQ(plural.lastError, CHIP_ERROR_INTERNAL);
        EXPECT_EQ(retryResult, CHIP_NO_ERROR);
        EXPECT_TRUE(ble != previous);
        EXPECT_TRUE(ble.hasPendingConnectionCallbacks);
        [previous dispatchConnectionError:CHIP_ERROR_INTERNAL];
        EXPECT_EQ(plural.errors, 1u);
        ble.matchedLongDiscriminator = @1234;
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(plural.successes, 1u);
    });
}

TEST_F(TestDarwinBleConnection, DirectBusyErrorsDoNotPreventAcceptedOwnerCancellation)
{
    for (bool byObject : { false, true }) {
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            ASSERT_EQ(Request(), CHIP_NO_ERROR);
            auto * previous = ble;
            ASSERT_TRUE(previous.timer != nullptr);
            backend.OnConnectionComplete = ConnectionCallbacks::OnComplete;
            backend.OnConnectionError = ConnectionCallbacks::OnError;
            if (byObject) {
                backend.NewConnection(&layer, &rejected, BLE_CONNECTION_UNINITIALIZED);
            } else {
                backend.NewConnection(&layer, &rejected, SetupDiscriminator());
            }
            EXPECT_EQ(rejected.errors, byObject ? 1u : 0u);
            EXPECT_TRUE(ble == previous);
            EXPECT_EQ(Request(), CHIP_ERROR_BUSY);
            ASSERT_EQ(layer.CancelBleIncompleteConnection(this), CHIP_NO_ERROR);
            EXPECT_TRUE(ble == nil);
            EXPECT_TRUE(previous.timer == nullptr);
            EXPECT_FALSE(previous.hasPendingConnectionCallbacks);
            ASSERT_EQ(Request(), CHIP_NO_ERROR);
            EXPECT_TRUE(ble != previous);
            [previous dispatchConnectionError:CHIP_ERROR_INTERNAL];
            EXPECT_EQ(errors, 0u);
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(successes, byObject ? 2u : 1u);
        });
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            EXPECT_EQ(rejected.errors, byObject ? 2u : 1u);
            EXPECT_EQ(rejected.lastError, CHIP_ERROR_BUSY);
            EXPECT_EQ(rejected.successes, 0u);
            EXPECT_EQ(errors, 0u);
            EXPECT_EQ(successes, byObject ? 2u : 1u);
        });
    }
}

TEST_F(TestDarwinBleConnection, UnconditionalCancellationStopsPluralWithQueuedRejection)
{
    for (bool byObject : { false, true }) {
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
            auto * previous = ble;
            ASSERT_TRUE(previous.timer != nullptr);
            ASSERT_EQ(Request(byObject), CHIP_NO_ERROR);
            ASSERT_EQ(layer.CancelBleIncompleteConnection(), CHIP_NO_ERROR);
            EXPECT_TRUE(ble == nil);
            EXPECT_TRUE(previous.timer == nullptr);
            EXPECT_FALSE(previous.hasPendingConnectionCallbacks);
            [previous dispatchConnectionError:CHIP_ERROR_INTERNAL];
            [previous dispatchConnectionComplete:nil];
            EXPECT_EQ(plural.errors, 0u);
            EXPECT_EQ(plural.successes, 0u);
            ASSERT_EQ(Request(), CHIP_NO_ERROR);
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(successes, byObject ? 2u : 1u);
        });
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            EXPECT_EQ(errors, 0u);
            EXPECT_EQ(plural.errors, 0u);
            EXPECT_EQ(plural.successes, 0u);
            EXPECT_EQ(successes, byObject ? 2u : 1u);
        });
    }
}

TEST_F(TestDarwinBleConnection, OwnerCancellationPreservesUnrelatedQueuedError)
{
    for (bool byObject : { false, true }) {
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
            auto * previous = ble;
            auto timer = previous.timer;
            ASSERT_EQ(Request(byObject), CHIP_NO_ERROR);
            backend.OnConnectionComplete = ConnectionCallbacks::OnComplete;
            backend.OnConnectionError = ConnectionCallbacks::OnError;
            if (byObject) {
                backend.NewConnection(&layer, &rejected, BLE_CONNECTION_UNINITIALIZED);
            } else {
                backend.NewConnection(&layer, &rejected, SetupDiscriminator());
            }
            ASSERT_EQ(layer.CancelBleIncompleteConnection(this), CHIP_NO_ERROR);
            EXPECT_TRUE(ble == previous);
            EXPECT_TRUE(previous.timer == timer);
            EXPECT_TRUE(previous.hasPendingConnectionCallbacks);
        });
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            EXPECT_EQ(errors, 0u);
            EXPECT_EQ(rejected.errors, byObject ? 2u : 1u);
            EXPECT_EQ(rejected.lastError, CHIP_ERROR_BUSY);
            EXPECT_EQ(rejected.successes, 0u);
            ASSERT_TRUE(ble.hasPendingConnectionCallbacks);
            ble.matchedLongDiscriminator = @1234;
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(plural.successes, byObject ? 2u : 1u);
            EXPECT_EQ(plural.errors, 0u);
            ASSERT_EQ(Request(), CHIP_NO_ERROR);
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(successes, byObject ? 2u : 1u);
        });
    }
}

TEST_F(TestDarwinBleConnection, CanceledBusyErrorsDoNotRetireReplacement)
{
    for (bool byObject : { false, true }) {
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
            auto * previous = ble;
            auto timer = previous.timer;
            ASSERT_EQ(Request(byObject), CHIP_NO_ERROR);
            EXPECT_EQ(errors, byObject ? 1u : 0u);
            EXPECT_EQ(RequestPlural(rejected, true), CHIP_ERROR_BUSY);
            ASSERT_EQ(layer.CancelBleIncompleteConnection(this), CHIP_NO_ERROR);
            EXPECT_TRUE(ble == previous);
            EXPECT_TRUE(previous.timer == timer);
            EXPECT_TRUE(previous.hasPendingConnectionCallbacks);
            ASSERT_EQ(Request(), CHIP_NO_ERROR);
            EXPECT_EQ(Request(), CHIP_ERROR_BUSY);
        });
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            EXPECT_EQ(errors, byObject ? 2u : 1u);
            EXPECT_EQ(lastError, CHIP_ERROR_BUSY);
            ASSERT_TRUE(ble.hasPendingConnectionCallbacks);
            ble.matchedLongDiscriminator = @1234;
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(plural.successes, byObject ? 2u : 1u);
            EXPECT_EQ(plural.errors, 0u);
            ASSERT_EQ(Request(), CHIP_NO_ERROR);
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(successes, byObject ? 2u : 1u);
            EXPECT_EQ(rejected.errors, 0u);
            EXPECT_EQ(rejected.successes, 0u);
        });
    }
}

TEST_F(TestDarwinBleConnection, CanceledBusyErrorDoesNotConsumeFreshBusyError)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
        auto * previous = ble;
        auto timer = previous.timer;
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        ASSERT_EQ(layer.CancelBleIncompleteConnection(this), CHIP_NO_ERROR);
        EXPECT_TRUE(ble == previous);
        EXPECT_TRUE(previous.timer == timer);
        EXPECT_TRUE(previous.hasPendingConnectionCallbacks);
        dispatch_async(PlatformMgrImpl().GetWorkQueue(), ^{
            EXPECT_EQ(errors, 0u);
            EXPECT_EQ(Request(), CHIP_ERROR_BUSY);
        });
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        EXPECT_EQ(errors, 0u);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        EXPECT_EQ(errors, 1u);
        EXPECT_EQ(lastError, CHIP_ERROR_BUSY);
        EXPECT_TRUE(ble.hasPendingConnectionCallbacks);
        ble.matchedLongDiscriminator = @1234;
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(plural.successes, 1u);
        EXPECT_EQ(plural.errors, 0u);
        ASSERT_EQ(Request(), CHIP_NO_ERROR);
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(successes, 1u);
    });
}

TEST_F(TestDarwinBleConnection, CanceledBusyErrorDoesNotConsumeFreshBusyCancellation)
{
    for (bool byObject : { false, true }) {
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
            auto * previous = ble;
            auto timer = previous.timer;
            ASSERT_EQ(Request(byObject), CHIP_NO_ERROR);
            ASSERT_EQ(layer.CancelBleIncompleteConnection(this), CHIP_NO_ERROR);
            dispatch_async(PlatformMgrImpl().GetWorkQueue(), ^{
                EXPECT_EQ(errors, 0u);
                EXPECT_EQ(Request(), CHIP_ERROR_BUSY);
                ASSERT_EQ(layer.CancelBleIncompleteConnection(this), CHIP_NO_ERROR);
                EXPECT_TRUE(ble == previous);
                EXPECT_TRUE(previous.timer == timer);
                EXPECT_TRUE(previous.hasPendingConnectionCallbacks);
            });
            ASSERT_EQ(Request(byObject), CHIP_NO_ERROR);
        });
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            EXPECT_EQ(errors, 0u);
            ASSERT_TRUE(ble.hasPendingConnectionCallbacks);
            ble.matchedLongDiscriminator = @1234;
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(plural.successes, byObject ? 2u : 1u);
            EXPECT_EQ(plural.errors, 0u);
            ASSERT_EQ(Request(), CHIP_NO_ERROR);
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(successes, byObject ? 2u : 1u);
        });
    }
}

TEST_F(TestDarwinBleConnection, DeliveredBusyErrorAllowsReentrantActiveCancellation)
{
    for (bool byObject : { false, true }) {
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
            auto onError = [](void * context, CHIP_ERROR error) {
                auto & test = *static_cast<TestDarwinBleConnection *>(context);
                ++test.errors;
                test.lastError = error;
                test.retryResult = test.layer.CancelBleIncompleteConnection();
            };
            if (byObject) {
                ASSERT_EQ(layer.NewBleConnectionByObject(BLE_CONNECTION_UNINITIALIZED, this, nullptr, onError), CHIP_NO_ERROR);
            } else {
                ASSERT_EQ(layer.NewBleConnectionByDiscriminator(SetupDiscriminator(), this, nullptr, onError), CHIP_NO_ERROR);
            }
            EXPECT_EQ(errors, byObject ? 1u : 0u);
        });
        dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
            EXPECT_EQ(errors, byObject ? 2u : 1u);
            EXPECT_EQ(lastError, CHIP_ERROR_BUSY);
            EXPECT_EQ(retryResult, CHIP_NO_ERROR);
            EXPECT_TRUE(ble == nil);
            EXPECT_EQ(plural.successes, 0u);
            EXPECT_EQ(plural.errors, 0u);
            ASSERT_EQ(Request(), CHIP_NO_ERROR);
            [ble dispatchConnectionComplete:nil];
            EXPECT_EQ(successes, byObject ? 2u : 1u);
        });
    }
}

TEST_F(TestDarwinBleConnection, PluralBrowsingRetiresPendingRequest)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
        auto * previous = ble;
        backend.StartScan(&scanner);
        EXPECT_EQ(plural.errors, 1u);
        EXPECT_EQ(plural.lastError, BLE_ERROR_APP_CLOSED_CONNECTION);
        EXPECT_TRUE(previous.timer == nullptr);
        EXPECT_FALSE(previous.hasPendingConnectionCallbacks);
        ASSERT_FALSE(ble.isConnecting);
        [ble centralManagerDidUpdateState:ble.centralManager];
        EXPECT_EQ(((MTRTestCentralManager *) ble.centralManager).scans, 1u);
        backend.StopScan();
        EXPECT_EQ(scanner.stopped, 1u);
        EXPECT_TRUE(ble == nil);
        ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
        [previous dispatchConnectionError:CHIP_ERROR_INTERNAL];
        EXPECT_EQ(plural.errors, 1u);
        ble.matchedLongDiscriminator = @1234;
        [ble dispatchConnectionComplete:nil];
        EXPECT_EQ(plural.successes, 1u);
    });
}

TEST_F(TestDarwinBleConnection, PluralPrewarmPreservesPendingRequest)
{
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{
        ASSERT_EQ(RequestPlural(plural), CHIP_NO_ERROR);
        auto * previous = ble;
        auto timer = previous.timer;
        backend.StartScan(&scanner, BleScanMode::kPreWarm);
        EXPECT_TRUE(ble == previous);
        EXPECT_TRUE(previous.timer == timer);
        EXPECT_TRUE(previous.hasPendingConnectionCallbacks);
        EXPECT_EQ(plural.errors, 0u);
        previous.matchedLongDiscriminator = @1234;
        [previous dispatchConnectionComplete:nil];
        EXPECT_EQ(plural.successes, 1u);
    });
    dispatch_sync(PlatformMgrImpl().GetWorkQueue(), ^{ EXPECT_EQ(scanner.stopped, 1u); });
}

}
