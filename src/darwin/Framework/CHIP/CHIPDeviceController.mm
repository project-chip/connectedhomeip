/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#import "CHIPCommissioningParameters.h"
#import "CHIPControllerAccessControl.h"
#import "CHIPDevicePairingDelegateBridge.h"
#import "CHIPDevice_Internal.h"
#import "CHIPError_Internal.h"
#import "CHIPKeypair.h"
#import "CHIPLogging.h"
#import "CHIPOperationalCredentialsDelegate.h"
#import "CHIPP256KeypairBridge.h"
#import "CHIPPersistentStorageDelegateBridge.h"
#import "CHIPSetupPayload.h"
#import <setup_payload/ManualSetupPayloadGenerator.h>
#import <setup_payload/SetupPayload.h>
#import <zap-generated/CHIPClustersObjc.h>

#import "CHIPDeviceConnectionBridge.h"

#include <platform/CHIPDeviceBuildConfig.h>

#include <controller/CHIPDeviceController.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/CommissioningWindowOpener.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/PlatformManager.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <system/SystemClock.h>

static const char * const CHIP_COMMISSIONER_DEVICE_ID_KEY = "com.zigbee.chip.commissioner.device_id";

static NSString * const kErrorMemoryInit = @"Init Memory failure";
static NSString * const kErrorKVSInit = @"Init Key Value Store failure";
static NSString * const kErrorCommissionerInit = @"Init failure while initializing a commissioner";
static NSString * const kErrorGroupProviderInit = @"Init failure while initializing group data provider";
static NSString * const kErrorIPKInit = @"Init failure while initializing IPK";
static NSString * const kErrorOperationalCredentialsInit = @"Init failure while creating operational credentials delegate";
static NSString * const kErrorPairingInit = @"Init failure while creating a pairing delegate";
static NSString * const kErrorPersistentStorageInit = @"Init failure while creating a persistent storage delegate";
static NSString * const kErrorPairDevice = @"Failure while pairing the device";
static NSString * const kErrorUnpairDevice = @"Failure while unpairing the device";
static NSString * const kErrorStopPairing = @"Failure while trying to stop the pairing process";
static NSString * const kErrorGetPairedDevice = @"Failure while trying to retrieve a paired device";
static NSString * const kErrorNotRunning = @"Controller is not running. Call startup first.";
static NSString * const kInfoStackShutdown = @"Shutting down the CHIP Stack";
static NSString * const kErrorSetupCodeGen = @"Generating Manual Pairing Code failed";

@interface CHIPDeviceController ()

// queue used to serialize all work performed by the CHIPDeviceController
@property (atomic, readonly) dispatch_queue_t chipWorkQueue;

@property (readonly) chip::Controller::DeviceCommissioner * cppCommissioner;
// We use TestPersistentStorageDelegate just to get an in-memory store to back
// our group data provider impl.  We initialize this store correctly on every
// controller startup, so don't need to actually persist it.
@property (readonly) chip::TestPersistentStorageDelegate * groupStorageDelegate;
@property (readonly) chip::Credentials::GroupDataProviderImpl * groupDataProvider;
@property (readonly) CHIPDevicePairingDelegateBridge * pairingDelegateBridge;
@property (readonly) CHIPPersistentStorageDelegateBridge * persistentStorageDelegateBridge;
@property (readonly) CHIPOperationalCredentialsDelegate * operationalCredentialsDelegate;
@property (readonly) CHIPP256KeypairBridge keypairBridge;
@property (readonly) chip::NodeId localDeviceId;
@property (readonly) uint16_t listenPort;
@property (readonly) const char * kvsPath;
@end

// TODO Replace Shared Controller with a Controller Factory Singleton
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
        _kvsPath = nullptr;

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

        _groupStorageDelegate = new chip::TestPersistentStorageDelegate();
        if ([self checkForInitError:(_groupStorageDelegate != nullptr) logMsg:kErrorGroupProviderInit]) {
            return nil;
        }

        // For now default args are fine, since we are just using this for the IPK.
        _groupDataProvider = new chip::Credentials::GroupDataProviderImpl();
        if ([self checkForInitError:(_groupDataProvider != nullptr) logMsg:kErrorGroupProviderInit]) {
            return nil;
        }

        _groupDataProvider->SetStorageDelegate(_groupStorageDelegate);
        errorCode = _groupDataProvider->Init();
        if ([self checkForInitError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorGroupProviderInit]) {
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
            chip::FabricIndex fabricIdx;
            CHIP_ERROR err = self->_cppCommissioner->GetFabricIndex(&fabricIdx);
            if (err == CHIP_NO_ERROR && self->_groupDataProvider) {
                // Clear out out group keys for this fabric index, just in case
                // we get a different fabric index assigned when we are started
                // again, since _groupDataProvider lives across shutdown/startup
                // cycles.
                self->_groupDataProvider->RemoveGroupKeys(fabricIdx);
            }
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
    return [self startup:storageDelegate vendorId:vendorId nocSigner:nocSigner ipk:nil];
}
- (BOOL)startup:(_Nullable id<CHIPPersistentStorageDelegate>)storageDelegate
       vendorId:(uint16_t)vendorId
      nocSigner:(id<CHIPKeypair>)nocSigner
            ipk:(NSData * _Nullable)ipk
{
    if (vendorId == chip::VendorId::Common) {
        // Shouldn't be using the "standard" vendor ID for actual devices.
        CHIP_LOG_ERROR("%d is not a valid vendorId to initialize a device controller with", vendorId);
        return NO;
    }

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

        [CHIPControllerAccessControl init];

        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;

        _persistentStorageDelegateBridge->setFrameworkDelegate(storageDelegate);
        // create a CHIPP256KeypairBridge here and pass it to the operationalCredentialsDelegate
        std::unique_ptr<chip::Crypto::CHIPP256KeypairNativeBridge> nativeBridge;
        if (nocSigner != nil) {
            _keypairBridge.Init(nocSigner);
            nativeBridge.reset(new chip::Crypto::CHIPP256KeypairNativeBridge(_keypairBridge));
        }
        errorCode = _operationalCredentialsDelegate->init(_persistentStorageDelegateBridge, std::move(nativeBridge), ipk);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorOperationalCredentialsInit]) {
            return;
        }

        // initialize NodeID if needed
        [self _getControllerNodeId];

        _cppCommissioner = new chip::Controller::DeviceCommissioner();
        if ([self checkForStartError:(_cppCommissioner != nullptr) logMsg:kErrorCommissionerInit]) {
            return;
        }

        chip::Controller::FactoryInitParams params;
        chip::Controller::SetupParams commissionerParams;

        if (_listenPort) {
            params.listenPort = _listenPort;
        }
        params.enableServerInteractions = true;

        // Initialize device attestation verifier
        // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
        const chip::Credentials::AttestationTrustStore * testingRootStore = chip::Credentials::GetTestAttestationTrustStore();
        chip::Credentials::SetDeviceAttestationVerifier(chip::Credentials::GetDefaultDACVerifier(testingRootStore));

        params.groupDataProvider = _groupDataProvider;
        params.fabricIndependentStorage = _persistentStorageDelegateBridge;
        commissionerParams.storageDelegate = _persistentStorageDelegateBridge;
        commissionerParams.pairingDelegate = _pairingDelegateBridge;

        commissionerParams.operationalCredentialsDelegate = _operationalCredentialsDelegate;

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
            _localDeviceId, /* fabricId = */ 1, chip::kUndefinedCATs, ephemeralKey.Pubkey(), rcac, icac, noc);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
            return;
        }

        commissionerParams.operationalKeypair = &ephemeralKey;
        commissionerParams.controllerRCAC = rcac;
        commissionerParams.controllerICAC = icac;
        commissionerParams.controllerNOC = noc;
        commissionerParams.controllerVendorId = vendorId;

        if (_kvsPath != nullptr) {
            errorCode = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(_kvsPath);
            if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorKVSInit]) {
                return;
            }
        }

        // TODO Replace Shared Controller with a Controller Factory Singleton
        auto & factory = chip::Controller::DeviceControllerFactory::GetInstance();
        errorCode = factory.Init(params);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
            return;
        }

        errorCode = factory.SetupCommissioner(commissionerParams, *_cppCommissioner);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
            return;
        }

        chip::FabricIndex fabricIdx = 0;
        errorCode = _cppCommissioner->GetFabricIndex(&fabricIdx);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorIPKInit]) {
            return;
        }

        uint8_t compressedIdBuffer[sizeof(uint64_t)];
        chip::MutableByteSpan compressedId(compressedIdBuffer);
        errorCode = _cppCommissioner->GetFabricInfo()->GetCompressedId(compressedId);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorIPKInit]) {
            return;
        }

        errorCode = chip::Credentials::SetSingleIpkEpochKey(
            _groupDataProvider, fabricIdx, _operationalCredentialsDelegate->GetIPK(), compressedId);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorIPKInit]) {
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
             error:(NSError * __autoreleasing *)error
{
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    __block BOOL success = NO;
    if (![self isRunning]) {
        success = ![self checkForError:errorCode logMsg:kErrorNotRunning error:error];
        return success;
    }
    dispatch_sync(_chipWorkQueue, ^{
        std::string manualPairingCode;
        chip::SetupPayload payload;
        payload.discriminator = discriminator;
        payload.setUpPINCode = setupPINCode;

        errorCode = chip::ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualPairingCode);
        success = ![self checkForError:errorCode logMsg:kErrorSetupCodeGen error:error];
        if (!success) {
            return;
        }
        if ([self isRunning]) {
            _operationalCredentialsDelegate->SetDeviceID(deviceID);
            errorCode = self.cppCommissioner->EstablishPASEConnection(deviceID, manualPairingCode.c_str());
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

- (BOOL)pairDevice:(uint64_t)deviceID onboardingPayload:(NSString *)onboardingPayload error:(NSError * __autoreleasing *)error
{
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    __block BOOL success = NO;
    if (![self isRunning]) {
        success = ![self checkForError:errorCode logMsg:kErrorNotRunning error:error];
        return success;
    }
    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            _operationalCredentialsDelegate->SetDeviceID(deviceID);
            errorCode = self.cppCommissioner->EstablishPASEConnection(deviceID, [onboardingPayload UTF8String]);
        }
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });
    return success;
}

- (BOOL)commissionDevice:(uint64_t)deviceId
     commissioningParams:(CHIPCommissioningParameters *)commissioningParams
                   error:(NSError * __autoreleasing *)error
{
    __block CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    __block BOOL success = NO;
    if (![self isRunning]) {
        success = ![self checkForError:errorCode logMsg:kErrorNotRunning error:error];
        return success;
    }
    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            chip::Controller::CommissioningParameters params;
            if (commissioningParams.CSRNonce) {
                params.SetCSRNonce(
                    chip::ByteSpan((uint8_t *) commissioningParams.CSRNonce.bytes, commissioningParams.CSRNonce.length));
            }
            if (commissioningParams.attestationNonce) {
                params.SetAttestationNonce(chip::ByteSpan(
                    (uint8_t *) commissioningParams.attestationNonce.bytes, commissioningParams.attestationNonce.length));
            }
            if (commissioningParams.threadOperationalDataset) {
                params.SetThreadOperationalDataset(chip::ByteSpan((uint8_t *) commissioningParams.threadOperationalDataset.bytes,
                    commissioningParams.threadOperationalDataset.length));
            }
            if (commissioningParams.wifiSSID && commissioningParams.wifiCredentials) {
                chip::ByteSpan ssid((uint8_t *) commissioningParams.wifiSSID.bytes, commissioningParams.wifiSSID.length);
                chip::ByteSpan credentials(
                    (uint8_t *) commissioningParams.wifiCredentials.bytes, commissioningParams.wifiCredentials.length);
                chip::Controller::WiFiCredentials wifiCreds(ssid, credentials);
                params.SetWiFiCredentials(wifiCreds);
            }

            _operationalCredentialsDelegate->SetDeviceID(deviceId);
            errorCode = self.cppCommissioner->Commission(deviceId, params);
        }
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
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

- (CHIPDevice *)getDeviceBeingCommissioned:(uint64_t)deviceId error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    if (![self isRunning]) {
        [self checkForError:errorCode logMsg:kErrorNotRunning error:error];
        return nil;
    }

    chip::CommissioneeDeviceProxy * deviceProxy;
    errorCode = self->_cppCommissioner->GetDeviceBeingCommissioned(deviceId, &deviceProxy);
    if (errorCode != CHIP_NO_ERROR) {
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:errorCode];
        }
        return nil;
    }
    return [[CHIPDevice alloc] initWithDevice:deviceProxy];
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

- (BOOL)openPairingWindow:(uint64_t)deviceID duration:(NSUInteger)duration error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (duration > UINT16_MAX) {
        CHIP_LOG_ERROR("Error: Duration %tu is too large. Max value %d", duration, UINT16_MAX);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return NO;
    }

    err = chip::Controller::AutoCommissioningWindowOpener::OpenBasicCommissioningWindow(
        self.cppCommissioner, deviceID, chip::System::Clock::Seconds16(static_cast<uint16_t>(duration)));

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%s): Open Pairing Window failed", chip::ErrorStr(err));
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return NO;
    }

    return YES;
}

- (NSString *)openPairingWindowWithPIN:(uint64_t)deviceID
                              duration:(NSUInteger)duration
                         discriminator:(NSUInteger)discriminator
                              setupPIN:(NSUInteger)setupPIN
                                 error:(NSError * __autoreleasing *)error
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (duration > UINT16_MAX) {
        CHIP_LOG_ERROR("Error: Duration %tu is too large. Max value %d", duration, UINT16_MAX);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return nil;
    }

    if (discriminator > 0xfff) {
        CHIP_LOG_ERROR("Error: Discriminator %tu is too large. Max value %d", discriminator, 0xfff);
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return nil;
    }

    setupPIN &= ((1 << chip::kSetupPINCodeFieldLengthInBits) - 1);

    chip::SetupPayload setupPayload;
    err = chip::Controller::AutoCommissioningWindowOpener::OpenCommissioningWindow(self.cppCommissioner, deviceID,
        chip::System::Clock::Seconds16(static_cast<uint16_t>(duration)), chip::Crypto::kSpake2p_Min_PBKDF_Iterations,
        static_cast<uint16_t>(discriminator), chip::MakeOptional(static_cast<uint32_t>(setupPIN)), chip::NullOptional,
        setupPayload);

    if (err != CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Error(%s): Open Pairing Window failed", chip::ErrorStr(err));
        if (error) {
            *error = [CHIPError errorForCHIPErrorCode:err];
        }
        return nil;
    }

    chip::ManualSetupPayloadGenerator generator(setupPayload);
    std::string outCode;

    if (generator.payloadDecimalStringRepresentation(outCode) == CHIP_NO_ERROR) {
        CHIP_LOG_ERROR("Setup code is %s", outCode.c_str());
    } else {
        CHIP_LOG_ERROR("Failed to get decimal setup code");
        return nil;
    }

    return [NSString stringWithCString:outCode.c_str() encoding:[NSString defaultCStringEncoding]];
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

- (void)setKeyValueStoreManagerPath:(const char *)kvsPath
{
    _kvsPath = kvsPath;
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

    if (_groupDataProvider) {
        _groupDataProvider->Finish();
        delete _groupDataProvider;
        _groupDataProvider = NULL;
    }

    if (_groupStorageDelegate) {
        delete _groupStorageDelegate;
        _groupStorageDelegate = NULL;
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
        _cppCommissioner->Shutdown();
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

- (void)dealloc
{
}

- (BOOL)deviceBeingCommissionedOverBLE:(uint64_t)deviceId
{
    CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;
    if (![self isRunning]) {
        [self checkForError:errorCode logMsg:kErrorNotRunning error:nil];
        return NO;
    }

    chip::CommissioneeDeviceProxy * deviceProxy;
    errorCode = self->_cppCommissioner->GetDeviceBeingCommissioned(deviceId, &deviceProxy);
    if (errorCode != CHIP_NO_ERROR) {
        return NO;
    }

    return deviceProxy->GetDeviceTransportType() == chip::Transport::Type::kBle;
}

@end
