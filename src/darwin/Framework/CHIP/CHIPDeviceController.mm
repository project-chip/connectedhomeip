/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#import "CHIPDeviceController.h"

#import "CHIPDevicePairingDelegateBridge.h"
#import "CHIPDevice_Internal.h"
#import "CHIPError.h"
#import "CHIPLogging.h"
#import "CHIPPersistentStorageDelegateBridge.h"

#include <controller/CHIPDeviceController.h>
#include <support/CHIPMem.h>

static const char * const CHIP_SELECT_QUEUE = "com.zigbee.chip.select";
static const char * const CHIP_COMMISSIONER_DEVICE_ID_KEY = "com.zigbee.chip.commissioner.device.id";

static NSString * const kErrorMemoryInit = @"Init Memory failure";
static NSString * const kErrorCommissionerCreate = @"Init failure while creating a commissioner";
static NSString * const kErrorCommissionerInit = @"Init failure while initializing a commissioner";
static NSString * const kErrorPairingInit = @"Init failure while creating a pairing delegate";
static NSString * const kErrorPersistentStorageInit = @"Init failure while creating a persistent storage delegate";
static NSString * const kErrorNetworkDispatchQueueInit = @"Init failure while initializing a dispatch queue for the network events";
static NSString * const kErrorPairDevice = @"Failure while pairing the device";
static NSString * const kErrorUnpairDevice = @"Failure while unpairing the device";
static NSString * const kErrorStopPairing = @"Failure while trying to stop the pairing process";
static NSString * const kErrorGetPairedDevice = @"Failure while trying to retrieve a paired device";
static NSString * const kInfoStackShutdown = @"Shutting down the CHIP Stack";

@interface CHIPDeviceController ()

@property (nonatomic, readonly, strong, nonnull) NSRecursiveLock * lock;

// queue used to call select on the system and inet layer fds., remove this with NW Framework.
// primarily used to not block the work queue
@property (atomic, readonly) dispatch_queue_t chipSelectQueue;

@property (readonly) chip::Controller::DeviceCommissioner * cppCommissioner;
@property (readonly) CHIPDevicePairingDelegateBridge * pairingDelegateBridge;
@property (readonly) CHIPPersistentStorageDelegateBridge * persistentStorageDelegateBridge;
@property (readonly) chip::NodeId localDeviceId;

@end

@implementation CHIPDeviceController

+ (CHIPDeviceController *)sharedController
{
    static CHIPDeviceController * controller = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // initialize the device controller
        controller = [[CHIPDeviceController alloc] init];
    });
    return controller;
}

- (instancetype)init
{
    if (self = [super init]) {
        CHIP_ERROR errorCode = CHIP_NO_ERROR;

        _lock = [[NSRecursiveLock alloc] init];

        errorCode = chip::Platform::MemoryInit();
        if ([self checkForInitError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorMemoryInit]) {
            return nil;
        }

        _pairingDelegateBridge = new CHIPDevicePairingDelegateBridge();
        if ([self checkForInitError:(_pairingDelegateBridge != nullptr) logMsg:kErrorPairingInit]) {
            return nil;
        }

        _persistentStorageDelegateBridge = new CHIPPersistentStorageDelegateBridge();
        if ([self checkForInitError:(_persistentStorageDelegateBridge != nullptr) logMsg:kErrorPersistentStorageInit]) {
            return nil;
        }

    }
    return self;
}

- (BOOL)isRunning
{
    [self.lock lock];
    BOOL commissionerInitialzied = _cppCommissioner != nullptr;
    [self.lock unlock];
    return commissionerInitialzied;
}

- (BOOL)shutdown
{
    [self.lock lock];
    if (_cppCommissioner) {
        CHIP_LOG_DEBUG("%@", kInfoStackShutdown);
        _cppCommissioner->Shutdown();
        delete _cppCommissioner;
        _cppCommissioner = nullptr;
    }
    [self.lock unlock];
    return YES;
}

- (BOOL)startup:(id<CHIPPersistentStorageDelegate>)storageDelegate queue:(nonnull dispatch_queue_t)queue
{
    if ([self isRunning])
    {
        return YES;
    }

    CHIP_ERROR errorCode = CHIP_NO_ERROR;
    [self.lock lock];
    _cppCommissioner = new chip::Controller::DeviceCommissioner();
    [self.lock unlock];
    if ([self checkForStartError:(_cppCommissioner != nullptr) logMsg:kErrorCommissionerCreate]) {
        return NO;
    }

    [self.lock lock];
    _persistentStorageDelegateBridge->setFrameworkDelegate(storageDelegate, queue);
    [self.lock unlock];
    [self getControllerNodeId];

    [self.lock lock];
    errorCode = _cppCommissioner->Init(_localDeviceId, _persistentStorageDelegateBridge, _pairingDelegateBridge);
    [self.lock unlock];
    if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
        return NO;
    }

    [self.lock lock];
    _chipSelectQueue = dispatch_queue_create(CHIP_SELECT_QUEUE, DISPATCH_QUEUE_SERIAL);
    [self.lock unlock];
    if ([self checkForStartError:(_chipSelectQueue != nil) logMsg:kErrorNetworkDispatchQueueInit]) {
        return NO;
    }

    // Start the IO pump
    dispatch_async(_chipSelectQueue, ^() {
        [self.lock lock];
        if (self.cppCommissioner)
        {
            self.cppCommissioner->ServiceEvents();
        }
        [self.lock unlock];
    });

    return YES;
}

- (NSNumber *)getControllerNodeId
{
    uint16_t idStringLen = 32;
    char deviceIdString[idStringLen];
    if (CHIP_NO_ERROR
        != _persistentStorageDelegateBridge->GetKeyValue(CHIP_COMMISSIONER_DEVICE_ID_KEY, deviceIdString, idStringLen)) {
        _localDeviceId = arc4random();
        _localDeviceId = _localDeviceId << 32 | arc4random();
        CHIP_LOG_ERROR("Assigned %llx node ID to the controller", _localDeviceId);
        _persistentStorageDelegateBridge->SetKeyValue(
            CHIP_COMMISSIONER_DEVICE_ID_KEY, [[NSString stringWithFormat:@"%llx", _localDeviceId] UTF8String]);
    } else {
        NSScanner * scanner = [NSScanner scannerWithString:[NSString stringWithUTF8String:deviceIdString]];
        [scanner scanHexLongLong:&_localDeviceId];
        CHIP_LOG_ERROR("Found %llx node ID for the controller", _localDeviceId);
    }
    return [NSNumber numberWithUnsignedLongLong:_localDeviceId];
}

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    [self.lock lock];
    chip::RendezvousParameters params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode).SetDiscriminator(discriminator);
    CHIP_ERROR err = self.cppCommissioner->PairDevice(deviceID, params);
    [self.lock unlock];

    return ![self checkForError:err logMsg:kErrorPairDevice error:error];
}

- (BOOL)unpairDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;

    [self.lock lock];
    if (self.cppCommissioner != nullptr)
    {
        errorCode = self.cppCommissioner->UnpairDevice(deviceID);
    }
    [self.lock unlock];

    return ![self checkForError:errorCode logMsg:kErrorUnpairDevice error:error];
}

- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;

    [self.lock lock];
    if (self.cppCommissioner != nullptr)
    {
        errorCode = self.cppCommissioner->StopPairing(deviceID);
    }
    [self.lock unlock];

    return ![self checkForError:errorCode logMsg:kErrorStopPairing error:error];
}

- (CHIPDevice *)getPairedDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    chip::Controller::Device * device = nil;

    [self.lock lock];
    if (self.cppCommissioner != nullptr)
    {
        errorCode = self.cppCommissioner->GetDevice(deviceID, &device);
    }
    [self.lock unlock];

    if ([self checkForError:errorCode logMsg:kErrorGetPairedDevice error:error]) {
        return nil;
    }

    return [[CHIPDevice alloc] initWithDevice:device];
}

- (void)setPairingDelegate:(id<CHIPDevicePairingDelegate>)delegate queue:(dispatch_queue_t)queue
{
    [self.lock lock];
    _pairingDelegateBridge->setDelegate(delegate, queue);
    [self.lock unlock];
}

- (void)sendWiFiCredentials:(NSString *)ssid password:(NSString *)password
{
    [self.lock lock];
    _pairingDelegateBridge->SendWiFiCredentials(ssid, password);
    [self.lock unlock];
}

- (void)sendThreadCredentials:(NSData *)threadDataSet
{
    [self.lock lock];
    _pairingDelegateBridge->SendThreadCredentials(threadDataSet);
    [self.lock unlock];
}

- (BOOL)checkForInitError:(BOOL)condition logMsg:(NSString *)logMsg
{
    if (condition) {
        return NO;
    }

    CHIP_LOG_ERROR("Error: %@", logMsg);

    if (_cppCommissioner) {
        delete _cppCommissioner;
        _cppCommissioner = NULL;
    }

    if (_pairingDelegateBridge) {
        delete _pairingDelegateBridge;
        _pairingDelegateBridge = NULL;
    }

    if (_persistentStorageDelegateBridge) {
        delete _persistentStorageDelegateBridge;
        _persistentStorageDelegateBridge = NULL;
    }

    return YES;
}

- (BOOL)checkForStartError:(BOOL)condition logMsg:(NSString *)logMsg
{
    if (condition) {
        return NO;
    }

    CHIP_LOG_ERROR("Error: %@", logMsg);

    if (_cppCommissioner) {
        delete _cppCommissioner;
        _cppCommissioner = NULL;
    }

    return YES;
}

- (BOOL)checkForError:(CHIP_ERROR)errorCode logMsg:(NSString *)logMsg error:(NSError * __autoreleasing *)error
{
    if (CHIP_NO_ERROR == errorCode) {
        return NO;
    }

    CHIP_LOG_ERROR("Error(%d): %@, %@", errorCode, [CHIPError errorForCHIPErrorCode:errorCode], logMsg);
    if (error) {
        *error = [CHIPError errorForCHIPErrorCode:errorCode];
    }

    return YES;
}

@end
