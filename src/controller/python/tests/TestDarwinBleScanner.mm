#import <objc/runtime.h>

#include <atomic>
#include <thread>

#include <pw_unit_test/framework.h>

#include "../matter/ble/darwin/Scanning.mm"

@interface ScannerTestCentralManager : NSObject
@property (weak, nonatomic) id<CBCentralManagerDelegate> delegate;
@property (nonatomic) NSUInteger stopCount;
@end

@implementation ScannerTestCentralManager
- (void)stopScan
{
    ++_stopCount;
}
@end

@interface CBCentralManager (ScannerLifetimeTests)
- (instancetype)initForScannerTestWithDelegate:(id<CBCentralManagerDelegate>)delegate queue:(dispatch_queue_t)queue;
@end

@implementation CBCentralManager (ScannerLifetimeTests)
- (instancetype)initForScannerTestWithDelegate:(id<CBCentralManagerDelegate>)delegate queue:(dispatch_queue_t)queue
{
    self = (CBCentralManager *) [[ScannerTestCentralManager alloc] init];
    self.delegate = delegate;
    return self;
}
@end

namespace {

struct ScannerContext {
    std::atomic<unsigned> completions { 0 };
    std::atomic<unsigned> errors { 0 };
    std::atomic<unsigned> devices { 0 };
    std::atomic<unsigned> lateCallbacks { 0 };
    std::atomic<bool> alive { true };
    void * scanner = nullptr;
    bool deleteInCallback = false;
    dispatch_semaphore_t entered = nil;
    dispatch_semaphore_t resume = nil;
};

void Found(PyObject * opaque, const char *, uint16_t, uint16_t, uint16_t)
{
    auto & context = *reinterpret_cast<ScannerContext *>(opaque);
    ++context.devices;
    if (!context.alive) {
        ++context.lateCallbacks;
    }
}

void Complete(PyObject * opaque)
{
    auto & context = *reinterpret_cast<ScannerContext *>(opaque);
    ++context.completions;
    if (!context.alive) {
        ++context.lateCallbacks;
    }
    if (context.entered != nil) {
        dispatch_semaphore_signal(context.entered);
        dispatch_semaphore_wait(context.resume, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));
    }
    if (context.deleteInCallback) {
        pychip_ble_scanner_delete(context.scanner);
    }
}

void Error(PyObject * opaque, uint32_t)
{
    auto & context = *reinterpret_cast<ScannerContext *>(opaque);
    ++context.errors;
    if (!context.alive) {
        ++context.lateCallbacks;
    }
}

class TestDarwinBleScanner : public ::testing::Test {
public:
    void SetUp() override { ExchangeInitializer(); }
    void TearDown() override { ExchangeInitializer(); }

    static void ExchangeInitializer()
    {
        method_exchangeImplementations(class_getInstanceMethod([CBCentralManager class], @selector(initWithDelegate:queue:)),
            class_getInstanceMethod([CBCentralManager class], @selector(initForScannerTestWithDelegate:queue:)));
    }

    static void * Start(ScannerContext & context)
    {
        context.scanner = pychip_ble_scanner_start(reinterpret_cast<PyObject *>(&context), nullptr, 60000, Found, Complete, Error);
        return context.scanner;
    }
};

TEST_F(TestDarwinBleScanner, DeleteBeforeTimeoutReleasesScannerAndCancelsTimer)
{
    ScannerContext context;
    __weak ChipDeviceBleScanner * weakScanner;
    @autoreleasepool {
        void * handle = Start(context);
        weakScanner = (__bridge ChipDeviceBleScanner *) handle;
        pychip_ble_scanner_delete(handle);
    }
    EXPECT_TRUE(weakScanner == nil);
    EXPECT_EQ(context.completions.load(), 0u);
    EXPECT_EQ(context.errors.load(), 0u);
}

TEST_F(TestDarwinBleScanner, TimerCompletesOnceAndDeleteAfterTimeoutDoesNotStopAgain)
{
    ScannerContext context;
    context.entered = dispatch_semaphore_create(0);
    context.resume = dispatch_semaphore_create(1);
    void * handle = Start(context);
    ChipDeviceBleScanner * scanner = (__bridge ChipDeviceBleScanner *) handle;
    ScannerTestCentralManager * central = (ScannerTestCentralManager *) scanner.centralManager;
    dispatch_source_set_timer(scanner.timer, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_MSEC), DISPATCH_TIME_FOREVER, 0);
    EXPECT_EQ(dispatch_semaphore_wait(context.entered, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC)), 0);
    dispatch_sync(scanner.workQueue, ^{
        [scanner stopTimeoutReached];
    });
    pychip_ble_scanner_delete(handle);
    EXPECT_EQ(context.completions.load(), 1u);
    EXPECT_EQ(context.errors.load(), 0u);
    EXPECT_EQ(central.stopCount, 1u);
    EXPECT_TRUE(scanner.context == nullptr);
    EXPECT_TRUE(scanner.timer == nil);
    EXPECT_TRUE(central.delegate == nil);
}

TEST_F(TestDarwinBleScanner, QueuedCallbacksCannotUseContextAfterDeletion)
{
    ScannerContext context;
    void * handle = Start(context);
    ChipDeviceBleScanner * scanner = (__bridge ChipDeviceBleScanner *) handle;
    pychip_ble_scanner_delete(handle);
    context.alive = false;
    dispatch_sync(scanner.workQueue, ^{
        [scanner stopTimeoutReached];
        [scanner centralManager:nil didDiscoverPeripheral:nil advertisementData:@{} RSSI:@0];
        [scanner centralManagerDidUpdateState:nil];
    });
    EXPECT_EQ(context.completions.load(), 0u);
    EXPECT_EQ(context.errors.load(), 0u);
    EXPECT_EQ(context.devices.load(), 0u);
    EXPECT_EQ(context.lateCallbacks.load(), 0u);
}

TEST_F(TestDarwinBleScanner, DeleteOnCallbackQueueDoesNotSynchronouslyDispatchToItself)
{
    ScannerContext context;
    void * handle = Start(context);
    ChipDeviceBleScanner * scanner = (__bridge ChipDeviceBleScanner *) handle;
    dispatch_sync(scanner.workQueue, ^{
        pychip_ble_scanner_delete(handle);
    });
    EXPECT_TRUE(scanner.context == nullptr);
    EXPECT_EQ(context.completions.load(), 0u);
}

TEST_F(TestDarwinBleScanner, CompletionCallbackCanDeleteScanner)
{
    ScannerContext context;
    void * handle = Start(context);
    context.deleteInCallback = true;
    ChipDeviceBleScanner * scanner = (__bridge ChipDeviceBleScanner *) handle;
    dispatch_sync(scanner.workQueue, ^{
        [scanner stopTimeoutReached];
    });
    EXPECT_EQ(context.completions.load(), 1u);
    EXPECT_EQ(context.errors.load(), 0u);
    EXPECT_TRUE(scanner.context == nullptr);
}

TEST_F(TestDarwinBleScanner, DeleteWaitsForInFlightCallback)
{
    ScannerContext context;
    context.entered = dispatch_semaphore_create(0);
    context.resume = dispatch_semaphore_create(0);
    void * handle = Start(context);
    ChipDeviceBleScanner * scanner = (__bridge ChipDeviceBleScanner *) handle;
    dispatch_async(scanner.workQueue, ^{
        [scanner stopTimeoutReached];
    });
    long entered = dispatch_semaphore_wait(context.entered, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));
    EXPECT_EQ(entered, 0);
    if (entered != 0) {
        dispatch_semaphore_signal(context.resume);
        pychip_ble_scanner_delete(handle);
        return;
    }
    dispatch_semaphore_t deleting = dispatch_semaphore_create(0);
    dispatch_semaphore_t deleted = dispatch_semaphore_create(0);
    std::thread deleter([&] {
        dispatch_semaphore_signal(deleting);
        pychip_ble_scanner_delete(handle);
        dispatch_semaphore_signal(deleted);
    });
    dispatch_semaphore_wait(deleting, DISPATCH_TIME_FOREVER);
    EXPECT_NE(dispatch_semaphore_wait(deleted, DISPATCH_TIME_NOW), 0);
    dispatch_semaphore_signal(context.resume);
    deleter.join();
    EXPECT_EQ(dispatch_semaphore_wait(deleted, DISPATCH_TIME_NOW), 0);
    context.alive = false;
    dispatch_sync(scanner.workQueue, ^{
        [scanner stopTimeoutReached];
    });
    EXPECT_EQ(context.completions.load(), 1u);
    EXPECT_EQ(context.lateCallbacks.load(), 0u);
}

}
