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

@interface CHIPDeviceController ()

@property (nonatomic, readonly, strong, nonnull) NSRecursiveLock * lock;

// queue used to call select on the system and inet layer fds., remove this with NW Framework.
// primarily used to not block the work queue
@property (atomic, readonly) dispatch_queue_t chipSelectQueue;

@property (readonly) chip::Controller::DeviceCommissioner * cppCommissioner;
@property (readonly) CHIPDevicePairingDelegateBridge * pairingDelegateBridge;
@property (readonly) CHIPPersistentStorageDelegateBridge * persistentStorageDelegateBridge;

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

        _cppCommissioner = new chip::Controller::DeviceCommissioner();
        if ([self checkForInitError:(_cppCommissioner != nullptr) logMsg:kErrorCommissionerCreate]) {
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

        chip::NodeId localDeviceId = 0;
        uint16_t idStringLen = 32;
        char deviceIdString[idStringLen];
        if (CHIP_NO_ERROR
            != _persistentStorageDelegateBridge->GetKeyValue(CHIP_COMMISSIONER_DEVICE_ID_KEY, deviceIdString, idStringLen)) {
            localDeviceId = arc4random();
            localDeviceId = localDeviceId << 32 | arc4random();
            CHIP_LOG_ERROR("Assigned %llx node ID to the controller", localDeviceId);
            _persistentStorageDelegateBridge->SetKeyValue(
                CHIP_COMMISSIONER_DEVICE_ID_KEY, [[NSString stringWithFormat:@"%llx", localDeviceId] UTF8String]);
        } else {
            NSScanner * scanner = [NSScanner scannerWithString:[NSString stringWithUTF8String:deviceIdString]];
            [scanner scanHexLongLong:&localDeviceId];
            CHIP_LOG_ERROR("Found %llx node ID for the controller", localDeviceId);
        }

        errorCode = _cppCommissioner->Init(localDeviceId, _persistentStorageDelegateBridge, _pairingDelegateBridge);
        if ([self checkForInitError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
            return nil;
        }

        _chipSelectQueue = dispatch_queue_create(CHIP_SELECT_QUEUE, DISPATCH_QUEUE_SERIAL);
        if ([self checkForInitError:(_chipSelectQueue != nil) logMsg:kErrorNetworkDispatchQueueInit]) {
            return nil;
        }

        // Start the IO pump
        dispatch_async(_chipSelectQueue, ^() {
            self.cppCommissioner->ServiceEvents();
        });
    }
    return self;
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
    CHIP_ERROR errorCode = CHIP_NO_ERROR;

    [self.lock lock];
    errorCode = self.cppCommissioner->UnpairDevice(deviceID);
    [self.lock unlock];

    return ![self checkForError:errorCode logMsg:kErrorUnpairDevice error:error];
}

- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR errorCode = CHIP_NO_ERROR;

    [self.lock lock];
    errorCode = self.cppCommissioner->StopPairing(deviceID);
    [self.lock unlock];

    return ![self checkForError:errorCode logMsg:kErrorStopPairing error:error];
}

- (CHIPDevice *)getPairedDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR errorCode = CHIP_NO_ERROR;
    chip::Controller::Device * device = nil;

    [self.lock lock];
    errorCode = self.cppCommissioner->GetDevice(deviceID, &device);
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

- (void)setPersistentStorageDelegate:(id<CHIPPersistentStorageDelegate>)delegate queue:(dispatch_queue_t)queue
{
    [self.lock lock];
    _persistentStorageDelegateBridge->setFrameworkDelegate(delegate, queue);
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
