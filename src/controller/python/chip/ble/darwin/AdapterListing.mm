#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

#import <CoreBluetooth/CoreBluetooth.h>

/// MAC does not have specific adapters
@interface FakeBleAdapterInformation : NSObject <CBCentralManagerDelegate>
@property (strong, nonatomic) dispatch_queue_t workQueue;
@property (strong, nonatomic) CBCentralManager * centralManager;
@property (nonatomic) bool advanced;
@property (nonatomic) bool hasStatus;
@property (strong, nonatomic) dispatch_semaphore_t statusSemaphore;

- (id)init;
- (bool)isPoweredOn;

@end

@implementation FakeBleAdapterInformation

- (id)init
{
    self = [super init];
    if (self) {
        _advanced = false;
        _hasStatus = false;
        _workQueue = dispatch_queue_create("com.chip.python.ble.work_queue", DISPATCH_QUEUE_SERIAL);
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:_workQueue];
        _statusSemaphore = dispatch_semaphore_create(0);
    }
    return self;
}

- (bool)isPoweredOn
{
    if (!self.hasStatus) {
        constexpr uint64_t kStateWaitTimeoutNs = 5 * 1000 * 1000 * 1000ll;
        dispatch_semaphore_wait(self.statusSemaphore, dispatch_time(DISPATCH_TIME_NOW, kStateWaitTimeoutNs));

        if (!self.hasStatus) {
            ChipLogError(Ble, "Timeout waiting for central state update.");
        }
    }
    return self.centralManager.state == CBManagerStatePoweredOn;
}

// Start CBCentralManagerDelegate
- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    switch (central.state) {
    case CBManagerStatePoweredOn:
        ChipLogDetail(Ble, "CBManagerState: ON");
        break;
    case CBManagerStatePoweredOff:
        ChipLogDetail(Ble, "CBManagerState: OFF");
        break;
    case CBManagerStateUnauthorized:
        ChipLogDetail(Ble, "CBManagerState: Unauthorized");
        break;
    case CBManagerStateResetting:
        ChipLogDetail(Ble, "CBManagerState: RESETTING");
        break;
    case CBManagerStateUnsupported:
        ChipLogDetail(Ble, "CBManagerState: UNSUPPORTED");
        break;
    case CBManagerStateUnknown:
        ChipLogDetail(Ble, "CBManagerState: UNKNOWN");
        break;
    }
    self.hasStatus = true;
    dispatch_semaphore_signal(self.statusSemaphore);
}

- (void)centralManager:(CBCentralManager *)central
    didDiscoverPeripheral:(CBPeripheral *)peripheral
        advertisementData:(NSDictionary *)advertisementData
                     RSSI:(NSNumber *)RSSI
{
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral
{
}

@end

extern "C" void * pychip_ble_adapter_list_new() { return (__bridge_retained void *) [[FakeBleAdapterInformation alloc] init]; }

extern "C" void pychip_ble_adapter_list_delete(FakeBleAdapterInformation * adapterIterator)
{
    CFRelease((CFTypeRef) adapterIterator);
}

extern "C" bool pychip_ble_adapter_list_next(FakeBleAdapterInformation * adapterIterator)
{
    if (!adapterIterator.advanced) {
        adapterIterator.advanced = true;
        return true;
    }
    return false;
}

extern "C" uint32_t pychip_ble_adapter_list_get_index(FakeBleAdapterInformation * adapterIterator) { return 0; }

extern "C" const char * pychip_ble_adapter_list_get_address(FakeBleAdapterInformation * adapterIterator)
{
    return "NOT_IMPLEMENTED";
}

extern "C" const char * pychip_ble_adapter_list_get_alias(FakeBleAdapterInformation * adapterIterator) { return "DarwinBLE"; }

extern "C" const char * pychip_ble_adapter_list_get_name(FakeBleAdapterInformation * adapterIterator) { return "DarwinBLE"; }

extern "C" bool pychip_ble_adapter_list_is_powered(FakeBleAdapterInformation * adapterIterator)
{
    return [adapterIterator isPoweredOn];
}

extern "C" void * pychip_ble_adapter_list_get_raw_adapter(FakeBleAdapterInformation * adapterIterator)
{
    return (__bridge void *) adapterIterator;
}
