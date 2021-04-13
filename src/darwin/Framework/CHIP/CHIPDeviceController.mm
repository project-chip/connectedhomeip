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
#import "gen/CHIPClustersObjc.h"

#include <platform/CHIPDeviceBuildConfig.h>

#include <controller/CHIPDeviceController.h>
#include <support/CHIPMem.h>

static const char * const CHIP_CONTROLLER_QUEUE = "com.zigbee.chip.framework.controller.workqueue";
static const char * const CHIP_COMMISSIONER_DEVICE_ID_KEY = "com.zigbee.chip.commissioner.device.id";

static NSString * const kErrorMemoryInit = @"Init Memory failure";
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

// queue used to serialize all work performed by the CHIPDeviceController
@property (atomic, readonly) dispatch_queue_t chipWorkQueue;

@property (readonly) chip::Controller::DeviceCommissioner * cppCommissioner;
@property (readonly) CHIPDevicePairingDelegateBridge * pairingDelegateBridge;
@property (readonly) CHIPPersistentStorageDelegateBridge * persistentStorageDelegateBridge;
@property (readonly) chip::NodeId localDeviceId;
@property (readonly) uint16_t listenPort;
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

        _chipWorkQueue = dispatch_queue_create(CHIP_CONTROLLER_QUEUE, DISPATCH_QUEUE_SERIAL);
        if ([self checkForStartError:(_chipWorkQueue != nil) logMsg:kErrorNetworkDispatchQueueInit]) {
            return nil;
        }

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
    __block BOOL commissionerInitialized;
    dispatch_sync(_chipWorkQueue, ^{
        commissionerInitialized = [self _isRunning];
    });
    return commissionerInitialized;
}

- (BOOL)_isRunning
{
    return _cppCommissioner != nullptr;
}

- (BOOL)shutdown
{
    dispatch_sync(_chipWorkQueue, ^{
        if (_cppCommissioner) {
            CHIP_LOG_DEBUG("%@", kInfoStackShutdown);
            _cppCommissioner->Shutdown();
            delete _cppCommissioner;
            _cppCommissioner = nullptr;
        }
    });

    return YES;
}

- (BOOL)startup:(_Nullable id<CHIPPersistentStorageDelegate>)storageDelegate queue:(_Nullable dispatch_queue_t)queue
{
    __block BOOL commissionerInitialized = NO;
    dispatch_sync(_chipWorkQueue, ^{
        if ([self _isRunning]) {
            commissionerInitialized = YES;
            return;
        }

        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;

        _persistentStorageDelegateBridge->setFrameworkDelegate(storageDelegate, queue);
        // initialize NodeID if needed
        [self _getControllerNodeId];

        _cppCommissioner = new chip::Controller::DeviceCommissioner();
        if ([self checkForInitError:(_cppCommissioner != nullptr) logMsg:kErrorMemoryInit]) {
            return;
        }

        if (_listenPort) {
            errorCode = _cppCommissioner->SetUdpListenPort(_listenPort);
            if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
                return;
            }
        }

        chip::Controller::ControllerInitParams params {
            .storageDelegate = _persistentStorageDelegateBridge,
            .mDeviceAddressUpdateDelegate = _pairingDelegateBridge,
        };

        errorCode = _cppCommissioner->Init(_localDeviceId, params, _pairingDelegateBridge);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
            return;
        }

        // Start the IO pump
        self.cppCommissioner->ServiceEvents();
    });

    return YES;
}

- (NSNumber *)getControllerNodeId
{
    __block NSNumber * nodeID;
    dispatch_sync(_chipWorkQueue, ^{
        nodeID = [self _getControllerNodeId];
    });
    return nodeID;
}

- (NSNumber *)_getControllerNodeId
{
    uint16_t idStringLen = 32;
    char deviceIdString[idStringLen];
    if (CHIP_NO_ERROR
        != _persistentStorageDelegateBridge->SyncGetKeyValue(CHIP_COMMISSIONER_DEVICE_ID_KEY, deviceIdString, idStringLen)) {
        _localDeviceId = arc4random();
        _localDeviceId = _localDeviceId << 32 | arc4random();
        CHIP_LOG_ERROR("Assigned %llx node ID to the controller", _localDeviceId);
        _persistentStorageDelegateBridge->AsyncSetKeyValue(
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
    __block BOOL success;
    dispatch_sync(_chipWorkQueue, ^{
        chip::RendezvousParameters params
            = chip::RendezvousParameters().SetSetupPINCode(setupPINCode).SetDiscriminator(discriminator);
        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;

        if ([self _isRunning]) {
            errorCode = self.cppCommissioner->PairDevice(deviceID, params);
        }
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });

    return success;
}

- (BOOL)pairDeviceWithoutSecurity:(uint64_t)deviceID
                          address:(NSString *)address
                             port:(uint16_t)port
                            error:(NSError * __autoreleasing *)error
{
    __block BOOL success;
    dispatch_sync(_chipWorkQueue, ^{
        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
        chip::Controller::SerializedDevice serializedTestDevice;
        chip::Inet::IPAddress addr;
        chip::Inet::IPAddress::FromString([address UTF8String], addr);

        if ([self _isRunning]) {
            errorCode = _cppCommissioner->PairTestDeviceWithoutSecurity(
                deviceID, chip::Transport::PeerAddress::UDP(addr, port), serializedTestDevice);
        }
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });

    return success;
}

- (BOOL)unpairDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    __block BOOL success;
    dispatch_sync(_chipWorkQueue, ^{
        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;

        if ([self _isRunning]) {
            errorCode = self.cppCommissioner->UnpairDevice(deviceID);
        }
        success = ![self checkForError:errorCode logMsg:kErrorUnpairDevice error:error];
    });

    return success;
}

- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    __block BOOL success;
    dispatch_sync(_chipWorkQueue, ^{
        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;

        if ([self _isRunning]) {
            errorCode = self.cppCommissioner->StopPairing(deviceID);
        }
        success = ![self checkForError:errorCode logMsg:kErrorStopPairing error:error];
    });

    return success;
}

- (CHIPDevice *)getPairedDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    __block CHIPDevice * chipDevice = nil;
    dispatch_sync(_chipWorkQueue, ^{
        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
        chip::Controller::Device * device = nullptr;

        if ([self _isRunning]) {
            errorCode = self.cppCommissioner->GetDevice(deviceID, &device);
        }

        if ([self checkForError:errorCode logMsg:kErrorGetPairedDevice error:error]) {
            return;
        }

        chipDevice = [[CHIPDevice alloc] initWithDevice:device];
    });

    return chipDevice;
}

- (void)setListenPort:(uint16_t)port
{
    _listenPort = port;
}

- (void)updateDevice:(uint64_t)deviceID fabricId:(uint64_t)fabricId
{
    dispatch_sync(_chipWorkQueue, ^{
        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
        chip::Controller::Device * device = nullptr;

        if ([self _isRunning]) {
            errorCode = self.cppCommissioner->GetDevice(deviceID, &device);
        }

        if (errorCode != CHIP_NO_ERROR) {
            return;
        }

        errorCode = self.cppCommissioner->UpdateDevice(device, fabricId);

        if (errorCode != CHIP_NO_ERROR) {
            return;
        }
    });
}

- (void)setPairingDelegate:(id<CHIPDevicePairingDelegate>)delegate queue:(dispatch_queue_t)queue
{
    dispatch_async(_chipWorkQueue, ^{
        self->_pairingDelegateBridge->setDelegate(delegate, queue);
    });
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
