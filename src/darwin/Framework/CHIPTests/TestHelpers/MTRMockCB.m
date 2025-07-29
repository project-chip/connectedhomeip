/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MTRMockCB.h"

#import "MTRDefines_Internal.h"

#import <XCTest/XCTest.h>
#import <objc/runtime.h>
#import <os/log.h>
#import <stdatomic.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRMockCBPeripheralDetails : NSObject

- (instancetype)initWithIdentifier:(NSUUID *)identifier;

@property (readonly, nonatomic, weak) MTRMockCB * mock;
@property (readonly, nonatomic, strong) NSUUID * identifier;
@property (readonly, nonatomic, copy) NSString * name;
@property (readonly, nonatomic, copy) NSDictionary<NSString *, id> * advertisementData;

@property (nonatomic, assign) CBPeripheralState state;
@property (nonatomic, nullable, copy) NSDictionary<NSString *, id> * extraAdvertisementData;
@property (nonatomic, copy) NSArray<CBMutableService *> * serviceDetails;

@end

@interface MTRMockCBCentralManager : NSObject

- (instancetype)_initWithMock:(MTRMockCB *)mock;
- (void)_didUpdateState;
- (void)_maybeDiscoverPeripheral:(MTRMockCBPeripheralDetails *)details;

#pragma mark - CBManager

@property (nonatomic, assign, readonly) CBManagerState state;
@property (nonatomic, assign, readonly) CBManagerAuthorization authorization;
@property (class, nonatomic, assign, readonly) CBManagerAuthorization authorization;

#pragma mark - CBCentralManager

@property (nonatomic, weak, nullable) id<CBCentralManagerDelegate> delegate;
@property (nonatomic, assign, readonly) BOOL isScanning;

+ (BOOL)supportsFeatures:(CBCentralManagerFeature)features;

- (instancetype)init;
- (instancetype)initWithDelegate:(nullable id<CBCentralManagerDelegate>)delegate
                           queue:(nullable dispatch_queue_t)queue;
- (instancetype)initWithDelegate:(nullable id<CBCentralManagerDelegate>)delegate
                           queue:(nullable dispatch_queue_t)queue
                         options:(nullable NSDictionary<NSString *, id> *)options;

- (NSArray<CBPeripheral *> *)retrievePeripheralsWithIdentifiers:(NSArray<NSUUID *> *)identifiers;
- (NSArray<CBPeripheral *> *)retrieveConnectedPeripheralsWithServices:(NSArray<CBUUID *> *)serviceUUIDs;
- (void)scanForPeripheralsWithServices:(nullable NSArray<CBUUID *> *)serviceUUIDs options:(nullable NSDictionary<NSString *, id> *)options;
- (void)stopScan;
- (void)connectPeripheral:(CBPeripheral *)peripheral options:(nullable NSDictionary<NSString *, id> *)options;
- (void)cancelPeripheralConnection:(CBPeripheral *)peripheral;
- (void)registerForConnectionEventsWithOptions:(nullable NSDictionary<CBConnectionEventMatchingOption, id> *)options;

@end

@interface MTRMockCBPeripheral : NSObject <NSCopying>

- (instancetype)_initWithDetails:(MTRMockCBPeripheralDetails *)details manager:(MTRMockCBCentralManager *)manager;

@property (readonly, strong, nonatomic) MTRMockCBCentralManager * manager; // not API, but used by BlePlatformDelegateImpl via KVC
@property (readonly, nonatomic) NSUInteger mtuLength; // not API, but used by BlePlatformDelegateImpl via KVC

#pragma mark - CBPeer

@property (readonly, nonatomic) NSUUID * identifier;

#pragma mark - CBPeripheral

@property (weak, nonatomic, nullable) id<CBPeripheralDelegate> delegate;
@property (retain, readonly, nullable) NSString * name;
@property (retain, readonly, nullable) NSNumber * RSSI;
@property (readonly) CBPeripheralState state;
@property (retain, readonly, nullable) NSArray<CBService *> * services;
@property (readonly) BOOL canSendWriteWithoutResponse;
@property (readonly) BOOL ancsAuthorized;

- (void)readRSSI;
- (void)discoverServices:(nullable NSArray<CBUUID *> *)serviceUUIDs;
- (void)discoverIncludedServices:(nullable NSArray<CBUUID *> *)includedServiceUUIDs forService:(CBService *)service;
- (void)discoverCharacteristics:(nullable NSArray<CBUUID *> *)characteristicUUIDs forService:(CBService *)service;
- (void)readValueForCharacteristic:(CBCharacteristic *)characteristic;
- (NSUInteger)maximumWriteValueLengthForType:(CBCharacteristicWriteType)type;
- (void)writeValue:(NSData *)data forCharacteristic:(CBCharacteristic *)characteristic type:(CBCharacteristicWriteType)type;
- (void)setNotifyValue:(BOOL)enabled forCharacteristic:(CBCharacteristic *)characteristic;
- (void)discoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic;
- (void)readValueForDescriptor:(CBDescriptor *)descriptor;
- (void)writeValue:(NSData *)data forDescriptor:(CBDescriptor *)descriptor;
- (void)openL2CAPChannel:(CBL2CAPPSM)PSM;

@end

static NSString * CBManagerStateAsString(CBManagerState state)
{
    switch (state) {
    case CBManagerStateUnknown:
        return @"CBManagerStateUnknown";
    case CBManagerStateResetting:
        return @"CBManagerStateResetting";
    case CBManagerStateUnsupported:
        return @"CBManagerStateUnsupported";
    case CBManagerStateUnauthorized:
        return @"CBManagerStateUnauthorized";
    case CBManagerStatePoweredOff:
        return @"CBManagerStatePoweredOff";
    case CBManagerStatePoweredOn:
        return @"CBManagerStatePoweredOn";
    }
    return [NSString stringWithFormat:@"CBManagerState(%ld)", (long) state];
}

@implementation MTRMockCB {
@package
    os_log_t _log;
    dispatch_queue_t _queue;
    os_block_t _invalidate;
    NSHashTable<MTRMockCBCentralManager *> * _managers;
    NSMutableDictionary<NSUUID *, MTRMockCBPeripheralDetails *> * _peripherals;
    CBManagerState _state;
}

static void InterceptClassMethod(__strong os_block_t * inOutCleanup, Class cls, SEL sel, id block)
{
    Method method = class_getClassMethod(cls, sel); // may return an inherited method
    if (!method) {
        NSString * reason = [NSString stringWithFormat:@"+[%@ %@] does not exist",
                                      NSStringFromClass(cls), NSStringFromSelector(sel)];
        @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:reason userInfo:nil];
    }
    IMP originalImp = method_getImplementation(method);

    // Try to add the method first, in case it came from a super class.
    // Note we need to pass the meta-class to class_addMethod().
    IMP newImp = imp_implementationWithBlock(block);
    if (class_addMethod(object_getClass(cls), sel, newImp, method_getTypeEncoding(method))) {
        method = class_getClassMethod(cls, sel); // look up again so we clean up the method we added
    } else {
        method_setImplementation(method, newImp);
    }

    os_block_t nextCleanup = *inOutCleanup;
    *inOutCleanup = ^{
        // This isn't 100% correct if we added an override of a super-class method, because
        // there is no API for removing a method. Instead we directly point it at the
        // inherited implementation; this is good enough for our purposes here.
        method_setImplementation(method, originalImp);
        imp_removeBlock(newImp); // otherwise the block might leak
        (void) block; // keep an obvious reference to avoid `leaks` false positives before cleanup
        nextCleanup();
    };
}

- (instancetype)init
{
    self = [super init];
    _log = os_log_create("com.csa.matter", "mock");

    static atomic_flag sInitialized = ATOMIC_FLAG_INIT;
    if (atomic_flag_test_and_set(&sInitialized)) {
        os_log_error(_log, "CoreBluetooth mocking is already enabled");
        return nil;
    }

    mtr_weakify(self);
    _invalidate = ^{
        mtr_strongify(self);
        self->_invalidate = nil;
        atomic_flag_clear(&sInitialized);
    };

    _queue = dispatch_queue_create("mock.cb", DISPATCH_QUEUE_SERIAL_WITH_AUTORELEASE_POOL);
    dispatch_queue_set_specific(_queue, (__bridge void *) self, @YES, nil); // mark our queue

    _managers = [NSHashTable weakObjectsHashTable];
    _peripherals = [[NSMutableDictionary alloc] init];
    _state = CBManagerStatePoweredOn;

    os_log(_log, "Enabling CoreBluetooth mocking");

    // Replace implementations of class methods we need to mock. We don't need to intercept
    // any instance methods directly, because we're returning a mock object from `alloc`.
    InterceptClassMethod(&_invalidate, CBCentralManager.class, @selector(alloc), ^id NS_RETURNS_RETAINED(void) {
        mtr_strongify(self);
        return self ? [[MTRMockCBCentralManager alloc] _initWithMock:self] : nil;
    });
    InterceptClassMethod(&_invalidate, CBCentralManager.class, @selector(supportsFeatures:), ^BOOL(CBCentralManagerFeature features) {
        return [MTRMockCBCentralManager supportsFeatures:features];
    });
    InterceptClassMethod(&_invalidate, CBManager.class, @selector(authorization), ^CBManagerAuthorization(void) {
        return [MTRMockCBCentralManager authorization];
    });

    return self;
}

- (void)sync:(void (^NS_NOESCAPE)(BOOL isValid))block
{
    // Allow `sync` to work like a recursive lock for convenience.
    if (dispatch_get_specific((__bridge void *) self) != NULL) {
        block(_invalidate != nil);
    } else {
        dispatch_sync(_queue, ^{
            block(_invalidate != nil);
        });
    }
}

- (void)reportApiViolation:(NSString *)format, ... NS_FORMAT_FUNCTION(1, 2)
{
    // For now treat these as fatal. We could instead report to a callback.
    va_list args;
    va_start(args, format);
    NSString * reason = [[NSString alloc] initWithFormat:format arguments:args];
    va_end(args);
    @throw [NSException exceptionWithName:NSInternalInconsistencyException reason:reason userInfo:nil];
}

- (BOOL)isValid
{
    __block BOOL result;
    [self sync:^(BOOL isValid) {
        result = isValid;
    }];
    return result;
}

- (void)reset
{
    [self sync:^(BOOL isValid) {
        [_peripherals removeAllObjects];
        _onScanForPeripheralsWithServicesOptions = nil;
        _onStopScan = nil;
        _onConnectPeripheralWithOptions = nil;
    }];
}

- (void)stopMocking
{
    [self sync:^(BOOL isValid) {
        if (isValid) {
            os_log(_log, "Disabling CoreBluetooth mocking");

            _invalidate();
            _invalidate = nil;

            NSArray<MTRMockCBCentralManager *> * managers = [_managers allObjects];
            _managers = nil;
            _peripherals = nil;

            if (_state != CBManagerStatePoweredOff) {
                _state = CBManagerStatePoweredOff;
                for (MTRMockCBCentralManager * manager in managers) {
                    [manager _didUpdateState];
                }
            }
        }
    }];
}

- (CBManagerState)state
{
    __block CBManagerState result;
    [self sync:^(BOOL isValid) {
        result = _state;
    }];
    return result;
}

- (void)setState:(CBManagerState)state
{
    [self sync:^(BOOL isValid) {
        if (isValid && state != _state) {
            _state = state;
            for (MTRMockCBCentralManager * manager in _managers) {
                [manager _didUpdateState];
            }
        }
    }];
}

- (void)addMockPeripheralWithIdentifier:(NSUUID *)identifier
                               services:(NSArray<CBMutableService *> *)services
                      advertisementData:(nullable NSDictionary<NSString *, id> *)advertisementData
{
    [self sync:^(BOOL isValid) {
        if (isValid) {
            MTRMockCBPeripheralDetails * details = _peripherals[identifier];
            if (!details) {
                details = [[MTRMockCBPeripheralDetails alloc] initWithIdentifier:identifier];
                _peripherals[identifier] = details;
            }
            details.serviceDetails = services;
            details.extraAdvertisementData = advertisementData;

            for (MTRMockCBCentralManager * manager in _managers) {
                [manager _maybeDiscoverPeripheral:details];
            }
        }
    }];
}

- (void)addMockPeripheralWithIdentifier:(NSUUID *)identifier
                           serviceUUIDs:(NSArray<CBUUID *> *)serviceUUIDs
                      advertisementData:(nullable NSDictionary<NSString *, id> *)advertisementData
{
    NSMutableArray<CBMutableService *> * services = [[NSMutableArray alloc] initWithCapacity:serviceUUIDs.count];
    BOOL primary = YES;
    for (CBUUID * uuid in serviceUUIDs) {
        [services addObject:[[CBMutableService alloc] initWithType:uuid primary:primary]];
        primary = NO;
    }
    [self addMockPeripheralWithIdentifier:identifier services:services advertisementData:advertisementData];
}

- (void)addMockCommissionableMatterDeviceWithIdentifier:(NSUUID *)identifier
                                               vendorID:(NSNumber *)vendorID
                                              productID:(NSNumber *)productID
                                          discriminator:(NSNumber *)discriminator
{
    // Note: CBUUID transparently expands 16 or 32 bit UUIDs to 128 bit as required,
    // however the current BLEConnectionDelegateImpl has its own comparison logic
    // that does not treat short and long UUIDs as equivalent and only works with
    // short UUIDs. The full UUID is "0000fff6-0000-1000-8000-00805f9b34fb".
    CBUUID * matterServiceUUID = [CBUUID UUIDWithString:@"fff6"];

    // See "Matter BLE Service Data payload format" in the spec, all fields little endian.
    const uint8_t serviceDataBytes[] = {
        0x00, // OpCode 0x00 (Commissionable)
        discriminator.unsignedIntValue & 0xff, (discriminator.unsignedIntValue >> 8) & 0xf,
        vendorID.unsignedIntValue & 0xff, (vendorID.unsignedIntValue >> 8) & 0xff,
        productID.unsignedIntValue & 0xff, (productID.unsignedIntValue >> 8) & 0xff,
        0x00, // Flags
    };
    NSData * matterServiceData = [NSData dataWithBytes:serviceDataBytes length:sizeof(serviceDataBytes)];

    CBMutableService * matterService = [[CBMutableService alloc] initWithType:matterServiceUUID primary:YES];
    matterService.characteristics = @[
        [[CBMutableCharacteristic alloc] initWithType:[CBUUID UUIDWithString:@"18ee2ef5-263d-4559-959f-4f9c429f9d11"]
                                           properties:CBCharacteristicPropertyWrite
                                                value:nil
                                          permissions:0],
        [[CBMutableCharacteristic alloc] initWithType:[CBUUID UUIDWithString:@"18ee2ef5-263d-4559-959f-4f9c429f9d12"]
                                           properties:CBCharacteristicPropertyIndicate
                                                value:nil
                                          permissions:0],
    ];

    [self addMockPeripheralWithIdentifier:identifier
                                 services:@[ matterService ]
                        advertisementData:@{ CBAdvertisementDataServiceDataKey : @ { matterServiceUUID : matterServiceData } }];
}

- (void)removeMockPeripheralWithIdentifier:(NSUUID *)identifier
{
    [self sync:^(BOOL isValid) {
        if (isValid) {
            _peripherals[identifier] = nil;
        }
    }];
}

@end

@implementation MTRMockCBPeripheralDetails

- (instancetype)initWithIdentifier:(NSUUID *)identifier
{
    self = [super init];
    _identifier = identifier;
    return self;
}

- (NSString *)name
{
    return _identifier.UUIDString;
}

- (NSDictionary<NSString *, id> *)advertisementData
{
    NSMutableDictionary * data = [[NSMutableDictionary alloc] init];
    data[CBAdvertisementDataLocalNameKey] = self.name;
    data[CBAdvertisementDataServiceUUIDsKey] = self.serviceUUIDs;
    data[CBAdvertisementDataIsConnectable] = @YES;
    if (_extraAdvertisementData) {
        [data addEntriesFromDictionary:_extraAdvertisementData];
    }
    return [data copy];
}

- (NSArray<CBUUID *> *)serviceUUIDs
{
    NSMutableArray<CBUUID *> * uuids = [[NSMutableArray alloc] initWithCapacity:_serviceDetails.count];
    for (CBMutableService * service in _serviceDetails) {
        [uuids addObject:service.UUID];
    }
    return uuids;
}

- (BOOL)matchesAnyServices:(nullable NSArray<CBUUID *> *)serviceUUIDs
{
    if (!serviceUUIDs) {
        return YES; // special case
    }
    NSSet<CBUUID *> * actual = [NSSet setWithArray:self.serviceUUIDs];
    NSSet<CBUUID *> * expected = [NSSet setWithArray:serviceUUIDs];
    return [actual intersectsSet:expected];
}

@end

@implementation MTRMockCBCentralManager {
@package
    MTRMockCB * _mock; // retain cycle (broken by stopMocking)
    BOOL _initialized;
    NSMapTable<NSUUID *, MTRMockCBPeripheral *> * _peripherals; // strong to weak

    id<CBCentralManagerDelegate> __weak _Nullable _delegate;
    dispatch_queue_t _delegateQueue;
    NSDictionary<NSString *, id> * _Nullable _options;

    NSArray<CBUUID *> * _Nullable _scanServiceUUIDs;
    NSDictionary<NSString *, id> * _Nullable _scanOptions;
}

- (instancetype)_initWithMock:(MTRMockCB *)mock
{
    self = [super init];
    _mock = mock;
    return self;
}

- (BOOL)isKindOfClass:(Class)aClass
{
    return [super isKindOfClass:aClass] || aClass == CBManager.class || aClass == CBCentralManager.class;
}

- (NSString *)description
{
    __block NSString * result;
    [_mock sync:^(BOOL isValid) {
        result = [NSString stringWithFormat:@"<%@ %p %@ %@>",
                           self.class, self, CBManagerStateAsString(self.state), isValid ? @"valid" : @"defunct"];
    }];
    return result;
}

#pragma mark - CBManager

+ (CBManagerAuthorization)authorization
{
    return CBManagerAuthorizationAllowedAlways;
}

- (CBManagerAuthorization)authorization
{
    return [[self class] authorization];
}

- (CBManagerState)state
{
    return _mock.state;
}

#pragma mark - CBCentralManager

+ (BOOL)supportsFeatures:(CBCentralManagerFeature)features
{
    return NO;
}

- (instancetype)init
{
    return [self initWithDelegate:nil queue:nil options:nil];
}

- (instancetype)initWithDelegate:(nullable id<CBCentralManagerDelegate>)delegate
                           queue:(nullable dispatch_queue_t)queue
{
    return [self initWithDelegate:delegate queue:queue options:nil];
}

- (instancetype)initWithDelegate:(nullable id<CBCentralManagerDelegate>)delegate
                           queue:(nullable dispatch_queue_t)queue
                         options:(nullable NSDictionary<NSString *, id> *)options
{
    XCTAssertFalse(_initialized);
    _initialized = YES;
    _peripherals = [NSMapTable strongToWeakObjectsMapTable];

    _delegate = delegate;
    _delegateQueue = queue ?: dispatch_get_main_queue();
    _options = options;

    [_mock sync:^(BOOL isValid) {
        if (isValid) {
            [_mock->_managers addObject:self];
            [self _didUpdateState];
        }
    }];
    return self;
}

- (NSArray<CBPeripheral *> *)retrievePeripheralsWithIdentifiers:(NSArray<NSUUID *> *)identifiers
{
    return nil;
}

- (NSArray<CBPeripheral *> *)retrieveConnectedPeripheralsWithServices:(NSArray<CBUUID *> *)serviceUUIDs
{
    return nil;
}

- (void)scanForPeripheralsWithServices:(nullable NSArray<CBUUID *> *)serviceUUIDs
                               options:(nullable NSDictionary<NSString *, id> *)options
{
    [_mock sync:^(BOOL isValid) {
        _scanServiceUUIDs = [serviceUUIDs copy];
        _scanOptions = [options copy];
        _isScanning = YES;

        if (isValid) {
            __auto_type callback = _mock.onScanForPeripheralsWithServicesOptions;
            if (callback) {
                callback(serviceUUIDs, options);
            }

            for (MTRMockCBPeripheralDetails * details in _mock->_peripherals.allValues) {
                [self _maybeDiscoverPeripheral:details];
            }
        }
    }];
}

- (void)stopScan
{
    [_mock sync:^(BOOL isValid) {
        _scanServiceUUIDs = nil;
        _scanOptions = nil;
        _isScanning = NO;

        if (isValid) {
            __auto_type callback = _mock.onStopScan;
            if (callback) {
                callback();
            }
        }
    }];
}

- (void)connectPeripheral:(CBPeripheral *)peripheral options:(nullable NSDictionary<NSString *, id> *)options
{
    [_mock sync:^(BOOL isValid) {
        if (isValid) {
            MTRMockCBPeripheralDetails * details = [self _activeDetailsForPeripheral:peripheral];
            if (!details || details.state != CBPeripheralStateDisconnected) {
                return;
            }

            details.state = CBPeripheralStateConnecting;
            __auto_type callback = _mock.onConnectPeripheralWithOptions;
            if (callback && callback(peripheral, options)) {
                details.state = CBPeripheralStateConnected;
                if ([_delegate respondsToSelector:@selector(centralManager:didConnectPeripheral:)]) {
                    os_log(_mock->_log, "%@ didConnectPeripheral %@", self, peripheral);
                    dispatch_async(_delegateQueue, ^{
                        [self->_delegate centralManager:(id) self
                                   didConnectPeripheral:peripheral];
                    });
                }
            }
        }
    }];
}

- (void)cancelPeripheralConnection:(CBPeripheral *)peripheral
{
}

- (void)registerForConnectionEventsWithOptions:(nullable NSDictionary<CBConnectionEventMatchingOption, id> *)options
{
}

#pragma mark - Internals

- (void)_didUpdateState
{
    os_log(_mock->_log, "%@ didUpdateState", self);
    dispatch_async(_delegateQueue, ^{
        [self->_delegate centralManagerDidUpdateState:(id) self];
    });
}

- (MTRMockCBPeripheral *)_peripheral:(MTRMockCBPeripheralDetails *)details
{
    // MTRMockCBPeripheralDetails is managed by the MTRMockCB and is valid for
    // all (MTRMock)CBCentralManager instances, but (MTRMock)CBPeripheral is
    // specific to each central manager, cache them in our weak map table.
    NSUUID * identifier = details.identifier;
    MTRMockCBPeripheral * peripheral = [_peripherals objectForKey:identifier];
    if (!peripheral) {
        peripheral = [[MTRMockCBPeripheral alloc] _initWithDetails:details manager:self];
        [_peripherals setObject:peripheral forKey:identifier];
    }
    return peripheral;
}

- (nullable MTRMockCBPeripheralDetails *)_activeDetailsForPeripheral:(nullable CBPeripheral *)peripheral
{
    if (!peripheral) {
        [_mock reportApiViolation:@"Passed a nil CBPeripheral"];
        return nil;
    }

    if (![peripheral isKindOfClass:MTRMockCBPeripheral.class]) {
        [_mock reportApiViolation:@"Passed a foreign (non-mock) CBPeripheral %@", peripheral];
        return nil;
    }

    MTRMockCBPeripheral * mockPeripheral = (id) peripheral;
    MTRMockCBCentralManager * manager = mockPeripheral.manager;
    if (manager != self) {
        [_mock reportApiViolation:@"Passed a peripheral %@ belonging to manager %@ rather than %@",
               mockPeripheral, manager, self];
        return nil;
    }

    // Look up details via identifier to ensure the peripheral has not been removed
    return _mock->_peripherals[mockPeripheral.identifier];
}

- (void)_maybeDiscoverPeripheral:(MTRMockCBPeripheralDetails *)details
{
    if (_isScanning && [details matchesAnyServices:_scanServiceUUIDs] &&
        [_delegate respondsToSelector:@selector(centralManager:didDiscoverPeripheral:advertisementData:RSSI:)]) {
        MTRMockCBPeripheral * peripheral = [self _peripheral:details];
        NSDictionary<NSString *, id> * advertisementData = details.advertisementData;
        os_log(_mock->_log, "%@ didDiscoverPeripheral %@", self, peripheral);
        dispatch_async(_delegateQueue, ^{
            [self->_delegate centralManager:(id) self
                      didDiscoverPeripheral:(id) peripheral
                          advertisementData:advertisementData
                                       RSSI:@127 /* Reserved for "RSSI not available" */];
        });
    }
}

@end

@implementation MTRMockCBPeripheral {
    MTRMockCBPeripheralDetails * _details;
    MTRMockCBCentralManager * _manager;
}

- (instancetype)_initWithDetails:(MTRMockCBPeripheralDetails *)details manager:(MTRMockCBCentralManager *)manager
{
    self = [super init];
    _details = details;
    _manager = manager;
    return self;
}

- (BOOL)isKindOfClass:(Class)aClass
{
    return [super isKindOfClass:aClass] || aClass == CBPeer.class || aClass == CBPeripheral.class;
}

- (NSString *)description
{
    __block NSString * result;
    [_manager->_mock sync:^(BOOL isValid) {
        result = [NSString stringWithFormat:@"<%@ %p %@ %@>", self.class, self, self.identifier, isValid ? @"valid" : @"defunct"];
    }];
    return result;
}

- (NSUInteger)mtuLength
{
    return 15;
}

#pragma mark - CBPeer <NSCopying>

- (BOOL)isEqual:(id)object
{
    return [object class] == [self class] && [((MTRMockCBPeripheral *) object).identifier isEqual:self.identifier];
}

- (NSUInteger)hash
{
    return self.identifier.hash;
}

- (id)copyWithZone:(nullable NSZone *)zone
{
    return self; // identifier is not mutable (and this is what CBPeer does)
}

- (NSUUID *)identifier
{
    return _details.identifier;
}

#pragma mark - CBPeripheral

- (nullable NSString *)name
{
    return _details.identifier.UUIDString;
}

- (nullable NSNumber *)RSSI
{
    return nil;
}

- (CBPeripheralState)state
{
    return _details.state;
}

- (nullable NSArray<CBService *> *)services
{
    // Just return the CBMutableService instances directly for now.
    // They don't hold the expected back reference to our peripheral,
    // but this is good enough for our current mocking needs.
    return _details.serviceDetails;
}

- (BOOL)canSendWriteWithoutResponse
{
    return YES; // ?
}

- (BOOL)ancsAuthorized
{
    return NO;
}

- (void)readRSSI
{
}

- (void)discoverServices:(nullable NSArray<CBUUID *> *)serviceUUIDs
{
    [_manager->_mock sync:^(BOOL isValid) {
        if (isValid &&
            [_details matchesAnyServices:serviceUUIDs] &&
            [_delegate respondsToSelector:@selector(peripheral:didDiscoverServices:)]) {
            os_log(_manager->_mock->_log, "%@ didDiscoverServices", self);
            dispatch_async(_manager->_delegateQueue, ^{
                [self->_delegate peripheral:(id) self didDiscoverServices:nil];
            });
        }
    }];
}

- (void)discoverIncludedServices:(nullable NSArray<CBUUID *> *)includedServiceUUIDs forService:(CBService *)service
{
}

- (void)discoverCharacteristics:(nullable NSArray<CBUUID *> *)characteristicUUIDs forService:(CBService *)service
{
    // We don't actually keep track of what characteristics have been discovered
    [_manager->_mock sync:^(BOOL isValid) {
        if (isValid && [_delegate respondsToSelector:@selector(peripheral:didDiscoverCharacteristicsForService:error:)]) {
            os_log(_manager->_mock->_log, "%@ didDiscoverCharacteristicsForService %@", self, service);
            dispatch_async(_manager->_delegateQueue, ^{
                [self->_delegate peripheral:(id) self didDiscoverCharacteristicsForService:service error:nil];
            });
        }
    }];
}

- (void)readValueForCharacteristic:(CBCharacteristic *)characteristic
{
}

- (NSUInteger)maximumWriteValueLengthForType:(CBCharacteristicWriteType)type
{
    return 512;
}

- (void)writeValue:(NSData *)data forCharacteristic:(CBCharacteristic *)characteristic type:(CBCharacteristicWriteType)type
{
    [_manager->_mock sync:^(BOOL isValid) {
        if (isValid) {
            __auto_type callback = _manager->_mock.onWriteValueForCharacteristic;
            if (callback) {
                callback((id) self, data, (id) characteristic, type);
            }
        }
    }];
}

- (void)setNotifyValue:(BOOL)enabled forCharacteristic:(CBCharacteristic *)characteristic
{
}

- (void)discoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic
{
}

- (void)readValueForDescriptor:(CBDescriptor *)descriptor
{
}

- (void)writeValue:(NSData *)data forDescriptor:(CBDescriptor *)descriptor
{
}

- (void)openL2CAPChannel:(CBL2CAPPSM)PSM
{
}

@end

NS_ASSUME_NONNULL_END
