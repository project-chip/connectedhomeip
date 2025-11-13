#include <ble/Ble.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Darwin/BleUtils.h>

#import <CoreBluetooth/CoreBluetooth.h>

namespace {
struct PyObject;
using DeviceScannedCallback
    = void (*)(PyObject * context, const char * address, uint16_t discriminator, uint16_t vendorId, uint16_t productId);
using ScanCompleteCallback = void (*)(PyObject * context);
using ScanErrorCallback = void (*)(PyObject * context, uint32_t error);
} // namespace

@interface ChipDeviceBleScanner : NSObject <CBCentralManagerDelegate>

@property (strong, nonatomic) dispatch_queue_t workQueue;
@property (nonatomic, readonly, nullable) dispatch_source_t timer;
@property (strong, nonatomic) CBCentralManager * centralManager;
@property (strong, nonatomic) CBUUID * shortServiceUUID;

@property (assign, nonatomic) PyObject * context;
@property (assign, nonatomic) DeviceScannedCallback scanCallback;
@property (assign, nonatomic) ScanCompleteCallback completeCallback;
@property (assign, nonatomic) ScanErrorCallback errorCallback;

- (id)initWithContext:(PyObject *)context
         scanCallback:(DeviceScannedCallback)scanCallback
     completeCallback:(ScanCompleteCallback)completeCallback
        errorCallback:(ScanErrorCallback)errorCallback
            timeoutMs:(uint32_t)timeout;

- (void)stopTimeoutReached;

@end

@implementation ChipDeviceBleScanner

- (id)initWithContext:(PyObject *)context
         scanCallback:(DeviceScannedCallback)scanCallback
     completeCallback:(ScanCompleteCallback)completeCallback
        errorCallback:(ScanErrorCallback)errorCallback
            timeoutMs:(uint32_t)timeout
{
    self = [super init];
    if (self) {
        self.shortServiceUUID = chip::DeviceLayer::Internal::CBUUIDFromBleUUID(chip::Ble::CHIP_BLE_SVC_ID);

        _workQueue = dispatch_queue_create("com.chip.python.ble.work_queue", DISPATCH_QUEUE_SERIAL);
        _timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _workQueue);
        _centralManager = [[CBCentralManager alloc] initWithDelegate:self queue:_workQueue];
        _context = context;
        _scanCallback = scanCallback;
        _completeCallback = completeCallback;
        _errorCallback = errorCallback;

        dispatch_source_set_event_handler(_timer, ^{
            [self stopTimeoutReached];
        });
        dispatch_source_set_timer(
            _timer, dispatch_walltime(nullptr, timeout * NSEC_PER_MSEC), DISPATCH_TIME_FOREVER, 50 * NSEC_PER_MSEC);
        dispatch_resume(_timer);
    }
    return self;
}

- (void)centralManager:(CBCentralManager *)central
    didDiscoverPeripheral:(CBPeripheral *)peripheral
        advertisementData:(NSDictionary *)advertisementData
                     RSSI:(NSNumber *)RSSI
{
    NSNumber * isConnectable = [advertisementData objectForKey:CBAdvertisementDataIsConnectable];

    if (![isConnectable boolValue]) {
        return;
    }

    NSDictionary * servicesData = [advertisementData objectForKey:CBAdvertisementDataServiceDataKey];
    for (CBUUID * serviceUUID in servicesData) {
        if (![serviceUUID isEqualTo:_shortServiceUUID]) {
            continue;
        }
        NSData * serviceData = [servicesData objectForKey:serviceUUID];

        NSUInteger length = [serviceData length];
        if (length != sizeof(chip::Ble::ChipBLEDeviceIdentificationInfo)) {
            ChipLogError(Ble, "Device has invalid advertisement data length.");
            break;
        }

        chip::Ble::ChipBLEDeviceIdentificationInfo data;
        memcpy(&data, [serviceData bytes], sizeof(data));

        _scanCallback(_context, [peripheral.identifier.UUIDString UTF8String], data.GetDeviceDiscriminator(), data.GetVendorId(),
            data.GetProductId());

        break;
    }
}

- (void)stopTimeoutReached
{
    ChipLogProgress(Ble, "Scan timeout reached.");

    _completeCallback(_context);
    _errorCallback(_context, CHIP_ERROR_TIMEOUT.AsInteger());

    dispatch_source_cancel(_timer);
    [self.centralManager stopScan];
    self.centralManager = nil;
}

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    switch (central.state) {
    case CBManagerStatePoweredOn:
        ChipLogProgress(Ble, "Central BLE Manager is on. Starting to scan.");
        [central scanForPeripheralsWithServices:@[ _shortServiceUUID ] options:nil];
        break;
    default:
        ChipLogError(Ble, "CBManagerState is NOT ON. Unable to scan.");
        break;
    }
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral
{
}

@end

extern "C" void * pychip_ble_scanner_start(PyObject * context, void * adapter, uint32_t timeout,
    DeviceScannedCallback scanCallback, ScanCompleteCallback completeCallback, ScanErrorCallback errorCallback)
{
    // NOTE: adapter is ignored as it does not apply to mac

    ChipDeviceBleScanner * scanner = [[ChipDeviceBleScanner alloc] initWithContext:context
                                                                      scanCallback:scanCallback
                                                                  completeCallback:completeCallback
                                                                     errorCallback:errorCallback
                                                                         timeoutMs:timeout];

    return (__bridge_retained void *) (scanner);
}

extern "C" void pychip_ble_scanner_delete(void * scanner)
{
    CFRelease((CFTypeRef) scanner);
}
