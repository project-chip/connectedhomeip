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
#import "CHIPError_Internal.h"
#import "CHIPKeypair.h"
#import "CHIPLogging.h"
#import "CHIPOperationalCredentialsDelegate.h"
#import "CHIPP256KeypairBridge.h"
#import "CHIPPersistentStorageDelegateBridge.h"
#import "CHIPSetupPayload.h"
#import <zap-generated/CHIPClustersObjc.h>

#import "CHIPDeviceConnectionBridge.h"

#include <platform/CHIPDeviceBuildConfig.h>

#include <controller/CHIPDeviceController.h>
#include <lib/support/CHIPMem.h>
#include <platform/PlatformManager.h>

static const char * const CHIP_COMMISSIONER_DEVICE_ID_KEY = "com.zigbee.chip.commissioner.device_id";

static NSString * const kErrorMemoryInit = @"Init Memory failure";
static NSString * const kErrorCommissionerInit = @"Init failure while initializing a commissioner";
static NSString * const kErrorOperationalCredentialsInit = @"Init failure while creating operational credentials delegate";
static NSString * const kErrorPairingInit = @"Init failure while creating a pairing delegate";
static NSString * const kErrorPersistentStorageInit = @"Init failure while creating a persistent storage delegate";
static NSString * const kErrorPairDevice = @"Failure while pairing the device";
static NSString * const kErrorUnpairDevice = @"Failure while unpairing the device";
static NSString * const kErrorStopPairing = @"Failure while trying to stop the pairing process";
static NSString * const kErrorGetPairedDevice = @"Failure while trying to retrieve a paired device";
static NSString * const kErrorNotRunning = @"Controller is not running. Call startup first.";
static NSString * const kInfoStackShutdown = @"Shutting down the CHIP Stack";

@interface CHIPDeviceController ()

// queue used to serialize all work performed by the CHIPDeviceController
@property (atomic, readonly) dispatch_queue_t chipWorkQueue;

@property (readonly) chip::Controller::DeviceCommissioner * cppCommissioner;
@property (readonly) CHIPDevicePairingDelegateBridge * pairingDelegateBridge;
@property (readonly) CHIPPersistentStorageDelegateBridge * persistentStorageDelegateBridge;
@property (readonly) CHIPOperationalCredentialsDelegate * operationalCredentialsDelegate;
@property (readonly) CHIPP256KeypairBridge keypairBridge;
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

        _chipWorkQueue = chip::DeviceLayer::PlatformMgrImpl().GetWorkQueue();

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

        _operationalCredentialsDelegate = new CHIPOperationalCredentialsDelegate();
        if ([self checkForInitError:(_operationalCredentialsDelegate != nullptr) logMsg:kErrorOperationalCredentialsInit]) {
            return nil;
        }
    }
    return self;
}

- (BOOL)isRunning
{
    return self.cppCommissioner != nullptr;
}

- (BOOL)shutdown
{
    dispatch_async(_chipWorkQueue, ^{
        if (self->_cppCommissioner) {
            CHIP_LOG_DEBUG("%@", kInfoStackShutdown);
            self->_cppCommissioner->Shutdown();
            delete self->_cppCommissioner;
            self->_cppCommissioner = nullptr;
        }
    });

    // StopEventLoopTask will block until blocks are executed
    chip::DeviceLayer::PlatformMgrImpl().StopEventLoopTask();

    return YES;
}

- (BOOL)startup:(_Nullable id<CHIPPersistentStorageDelegate>)storageDelegate
       vendorId:(uint16_t)vendorId
      nocSigner:(id<CHIPKeypair>)nocSigner
{
    chip::DeviceLayer::PlatformMgrImpl().StartEventLoopTask();

    __block BOOL commissionerInitialized = NO;
    if ([self isRunning]) {
        CHIP_LOG_DEBUG("Ignoring duplicate call to startup, Controller already started...");
        return YES;
    }

    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            commissionerInitialized = YES;
            return;
        }

        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;

        _persistentStorageDelegateBridge->setFrameworkDelegate(storageDelegate);

        // create a CHIPP256KeypairBridge here and pass it to the operationalCredentialsDelegate
        std::unique_ptr<chip::Crypto::CHIPP256KeypairNativeBridge> nativeBridge;
        if (nocSigner != nil) {
            _keypairBridge.Init(nocSigner);
            nativeBridge.reset(new chip::Crypto::CHIPP256KeypairNativeBridge(_keypairBridge));
        }
        errorCode = _operationalCredentialsDelegate->init(_persistentStorageDelegateBridge, std::move(nativeBridge));
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorOperationalCredentialsInit]) {
            return;
        }

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

        chip::Controller::CommissionerInitParams params;

        params.storageDelegate = _persistentStorageDelegateBridge;
        params.mDeviceAddressUpdateDelegate = _pairingDelegateBridge;
        params.pairingDelegate = _pairingDelegateBridge;

        params.operationalCredentialsDelegate = _operationalCredentialsDelegate;

        chip::Crypto::P256Keypair ephemeralKey;
        errorCode = ephemeralKey.Initialize();
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
            return;
        }

        NSMutableData * nocBuffer = [[NSMutableData alloc] initWithLength:chip::Controller::kMaxCHIPDERCertLength];
        chip::MutableByteSpan noc((uint8_t *) [nocBuffer mutableBytes], chip::Controller::kMaxCHIPDERCertLength);

        NSMutableData * rcacBuffer = [[NSMutableData alloc] initWithLength:chip::Controller::kMaxCHIPDERCertLength];
        chip::MutableByteSpan rcac((uint8_t *) [rcacBuffer mutableBytes], chip::Controller::kMaxCHIPDERCertLength);

        chip::MutableByteSpan icac;

        errorCode = _operationalCredentialsDelegate->GenerateNOCChainAfterValidation(
            _localDeviceId, 0, ephemeralKey.Pubkey(), rcac, icac, noc);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
            return;
        }

        params.ephemeralKeypair = &ephemeralKey;
        params.controllerRCAC = rcac;
        params.controllerICAC = icac;
        params.controllerNOC = noc;
        params.controllerVendorId = vendorId;

        errorCode = _cppCommissioner->Init(params);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
            return;
        }

        commissionerInitialized = YES;
    });

    return commissionerInitialized;
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
    uint16_t deviceIdLength = sizeof(_localDeviceId);
    if (CHIP_NO_ERROR
        != _persistentStorageDelegateBridge->SyncGetKeyValue(CHIP_COMMISSIONER_DEVICE_ID_KEY, &_localDeviceId, deviceIdLength)) {
        _localDeviceId = arc4random();
        _localDeviceId = _localDeviceId << 32 | arc4random();
        CHIP_LOG_ERROR("Assigned %llx node ID to the controller", _localDeviceId);

        _persistentStorageDelegateBridge->SyncSetKeyValue(CHIP_COMMISSIONER_DEVICE_ID_KEY, &_localDeviceId, sizeof(_localDeviceId));
    } else {
        CHIP_LOG_ERROR("Found %llx node ID for the controller", _localDeviceId);
    }
    return [NSNumber numberWithUnsignedLongLong:_localDeviceId];
}

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
          csrNonce:(nullable NSData *)csrNonce
             error:(NSError * __autoreleasing *)error
{
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    __block BOOL success = NO;
    if (![self isRunning]) {
        success = ![self checkForError:errorCode logMsg:kErrorNotRunning error:error];
        return success;
    }
    dispatch_sync(_chipWorkQueue, ^{
        chip::RendezvousParameters params
            = chip::RendezvousParameters().SetSetupPINCode(setupPINCode).SetDiscriminator(discriminator);

        if (csrNonce != nil) {
            params = params.SetCSRNonce(chip::ByteSpan((const uint8_t *) csrNonce.bytes, csrNonce.length));
        }

        if ([self isRunning]) {
            _operationalCredentialsDelegate->SetDeviceID(deviceID);
            errorCode = self.cppCommissioner->PairDevice(deviceID, params);
        }
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });

    return success;
}

- (BOOL)pairDevice:(uint64_t)deviceID
           address:(NSString *)address
              port:(uint16_t)port
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    __block BOOL success = NO;
    if (![self isRunning]) {
        success = ![self checkForError:errorCode logMsg:kErrorNotRunning error:error];
        return success;
    }
    dispatch_sync(_chipWorkQueue, ^{
        chip::Inet::IPAddress addr;
        chip::Inet::IPAddress::FromString([address UTF8String], addr);
        chip::Transport::PeerAddress peerAddress = chip::Transport::PeerAddress::UDP(addr, port);

        chip::RendezvousParameters params = chip::RendezvousParameters()
                                                .SetSetupPINCode(setupPINCode)
                                                .SetDiscriminator(discriminator)
                                                .SetPeerAddress(peerAddress);
        if ([self isRunning]) {
            _operationalCredentialsDelegate->SetDeviceID(deviceID);
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
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    __block BOOL success = NO;
    if (![self isRunning]) {
        success = ![self checkForError:errorCode logMsg:kErrorNotRunning error:error];
        return success;
    }
    dispatch_sync(_chipWorkQueue, ^{
        chip::Controller::SerializedDevice serializedTestDevice;
        chip::Inet::IPAddress addr;
        chip::Inet::IPAddress::FromString([address UTF8String], addr);

        if ([self isRunning]) {
            _operationalCredentialsDelegate->SetDeviceID(deviceID);
            errorCode = _cppCommissioner->PairTestDeviceWithoutSecurity(
                deviceID, chip::Transport::PeerAddress::UDP(addr, port), serializedTestDevice);
        }
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });

    return success;
}

- (BOOL)pairDevice:(uint64_t)deviceID
        onboardingPayload:(NSString *)onboardingPayload
    onboardingPayloadType:(CHIPOnboardingPayloadType)onboardingPayloadType
                    error:(NSError * __autoreleasing *)error
{
    BOOL didSucceed = NO;
    CHIPSetupPayload * setupPayload = [CHIPOnboardingPayloadParser setupPayloadForOnboardingPayload:onboardingPayload
                                                                                             ofType:onboardingPayloadType
                                                                                              error:error];
    if (setupPayload) {
        uint16_t discriminator = setupPayload.discriminator.unsignedShortValue;
        uint32_t setupPINCode = setupPayload.setUpPINCode.unsignedIntValue;
        _operationalCredentialsDelegate->SetDeviceID(deviceID);
        didSucceed = [self pairDevice:deviceID discriminator:discriminator setupPINCode:setupPINCode csrNonce:nil error:error];
    } else {
        CHIP_LOG_ERROR("Failed to create CHIPSetupPayload for pairing with error %@", *error);
    }
    return didSucceed;
}

- (BOOL)unpairDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    __block BOOL success = NO;
    if (![self isRunning]) {
        success = ![self checkForError:errorCode logMsg:kErrorNotRunning error:error];
        return success;
    }
    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            errorCode = self.cppCommissioner->UnpairDevice(deviceID);
        }
        success = ![self checkForError:errorCode logMsg:kErrorUnpairDevice error:error];
    });

    return success;
}

- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    __block BOOL success = NO;
    if (![self isRunning]) {
        success = ![self checkForError:errorCode logMsg:kErrorNotRunning error:error];
        return success;
    }
    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            _operationalCredentialsDelegate->ResetDeviceID();
            errorCode = self.cppCommissioner->StopPairing(deviceID);
        }
        success = ![self checkForError:errorCode logMsg:kErrorStopPairing error:error];
    });

    return success;
}

- (BOOL)isDevicePaired:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    __block BOOL paired = NO;
    if (![self isRunning]) {
        [self checkForError:CHIP_ERROR_INCORRECT_STATE logMsg:kErrorNotRunning error:error];
        return paired;
    }
    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            paired = self.cppCommissioner->DoesDevicePairingExist(chip::PeerId().SetNodeId(deviceID));
        }
    });

    return paired;
}

- (BOOL)getConnectedDevice:(uint64_t)deviceID
                     queue:(dispatch_queue_t)queue
         completionHandler:(CHIPDeviceConnectionCallback)completionHandler
{
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    if (![self isRunning]) {
        NSError * error;
        [self checkForError:errorCode logMsg:kErrorNotRunning error:&error];
        dispatch_async(queue, ^{
            completionHandler(nil, error);
        });
        return NO;
    }

    dispatch_async(_chipWorkQueue, ^{
        if ([self isRunning]) {
            CHIPDeviceConnectionBridge * connectionBridge = new CHIPDeviceConnectionBridge(completionHandler, queue);
            errorCode = connectionBridge->connect(self->_cppCommissioner, deviceID);
        }

        NSError * error;
        if ([self checkForError:errorCode logMsg:kErrorGetPairedDevice error:&error]) {
            // Errors are propagated to the caller through completionHandler.
            // No extra error handling is needed here.
            return;
        }
    });

    return YES;
}

- (CHIPDevice *)getPairedDevice:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    __block CHIPDevice * chipDevice = nil;
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    if (![self isRunning]) {
        [self checkForError:errorCode logMsg:kErrorNotRunning error:error];
        return chipDevice;
    }
    dispatch_sync(_chipWorkQueue, ^{
        chip::Controller::Device * device = nullptr;

        if ([self isRunning]) {
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
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    if (![self isRunning]) {
        [self checkForError:errorCode logMsg:kErrorNotRunning error:nil];
        return;
    }
    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            errorCode = self.cppCommissioner->UpdateDevice(deviceID);
            CHIP_LOG_ERROR("Update device address returned: %s", chip::ErrorStr(errorCode));
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

    CHIP_LOG_ERROR("Error(%s): %s", chip::ErrorStr(errorCode), [logMsg UTF8String]);
    if (error) {
        *error = [CHIPError errorForCHIPErrorCode:errorCode];
    }

    return YES;
}

@end
