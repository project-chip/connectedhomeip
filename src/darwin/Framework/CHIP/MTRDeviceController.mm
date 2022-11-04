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
#import <os/lock.h>

#import "MTRDeviceController.h"

#import "MTRBaseDevice_Internal.h"
#import "MTRCommissioningParameters.h"
#import "MTRControllerFactory_Internal.h"
#import "MTRDeviceControllerStartupParams.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRDevicePairingDelegateBridge.h"
#import "MTRDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTRKeypair.h"
#import "MTRLogging.h"
#import "MTROperationalCredentialsDelegate.h"
#import "MTRP256KeypairBridge.h"
#import "MTRPersistentStorageDelegateBridge.h"
#import "MTRSetupPayload.h"
#import "NSDataSpanConversion.h"
#import <setup_payload/ManualSetupPayloadGenerator.h>
#import <setup_payload/SetupPayload.h>
#import <zap-generated/MTRBaseClusters.h>

#import "MTRDeviceAttestationDelegateBridge.h"
#import "MTRDeviceConnectionBridge.h"

#include <platform/CHIPDeviceBuildConfig.h>

#include <controller/CHIPDeviceController.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/CommissioningWindowOpener.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/attestation_verifier/DacOnlyPartialAttestationVerifier.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <platform/PlatformManager.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <system/SystemClock.h>

static NSString * const kErrorCommissionerInit = @"Init failure while initializing a commissioner";
static NSString * const kErrorIPKInit = @"Init failure while initializing IPK";
static NSString * const kErrorSigningKeypairInit = @"Init failure while creating signing keypair bridge";
static NSString * const kErrorOperationalCredentialsInit = @"Init failure while creating operational credentials delegate";
static NSString * const kErrorOperationalKeypairInit = @"Init failure while creating operational keypair bridge";
static NSString * const kErrorPairingInit = @"Init failure while creating a pairing delegate";
static NSString * const kErrorPartialDacVerifierInit = @"Init failure while creating a partial DAC verifier";
static NSString * const kErrorPairDevice = @"Failure while pairing the device";
static NSString * const kErrorUnpairDevice = @"Failure while unpairing the device";
static NSString * const kErrorStopPairing = @"Failure while trying to stop the pairing process";
static NSString * const kErrorOpenPairingWindow = @"Open Pairing Window failed";
static NSString * const kErrorGetPairedDevice = @"Failure while trying to retrieve a paired device";
static NSString * const kErrorNotRunning = @"Controller is not running. Call startup first.";
static NSString * const kInfoStackShutdown = @"Shutting down the Matter Stack";
static NSString * const kErrorSetupCodeGen = @"Generating Manual Pairing Code failed";
static NSString * const kErrorGenerateNOC = @"Generating operational certificate failed";
static NSString * const kErrorKeyAllocation = @"Generating new operational key failed";
static NSString * const kErrorCSRValidation = @"Extracting public key from CSR failed";
static NSString * const kErrorGetCommissionee = @"Failure obtaining device being commissioned";
static NSString * const kErrorGetAttestationChallenge = @"Failure getting attestation challenge";

@interface MTRDeviceController ()

// queue used to serialize all work performed by the MTRDeviceController
@property (atomic, readonly) dispatch_queue_t chipWorkQueue;

@property (readonly) chip::Controller::DeviceCommissioner * cppCommissioner;
@property (readonly) chip::Credentials::PartialDACVerifier * partialDACVerifier;
@property (readonly) MTRDevicePairingDelegateBridge * pairingDelegateBridge;
@property (readonly) MTROperationalCredentialsDelegate * operationalCredentialsDelegate;
@property (readonly) MTRP256KeypairBridge signingKeypairBridge;
@property (readonly) MTRP256KeypairBridge operationalKeypairBridge;
@property (readonly) MTRDeviceAttestationDelegateBridge * deviceAttestationDelegateBridge;
@property (readonly) MTRControllerFactory * factory;
@property (readonly) NSMutableDictionary * nodeIDToDeviceMap;
@property (readonly) os_unfair_lock deviceMapLock; // protects nodeIDToDeviceMap
@end

@implementation MTRDeviceController

- (instancetype)initWithFactory:(MTRControllerFactory *)factory queue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        _chipWorkQueue = queue;
        _factory = factory;
        _deviceMapLock = OS_UNFAIR_LOCK_INIT;
        _nodeIDToDeviceMap = [NSMutableDictionary dictionary];

        _pairingDelegateBridge = new MTRDevicePairingDelegateBridge();
        if ([self checkForInitError:(_pairingDelegateBridge != nullptr) logMsg:kErrorPairingInit]) {
            return nil;
        }

        _partialDACVerifier = new chip::Credentials::PartialDACVerifier();
        if ([self checkForInitError:(_partialDACVerifier != nullptr) logMsg:kErrorPartialDacVerifierInit]) {
            return nil;
        }

        _operationalCredentialsDelegate = new MTROperationalCredentialsDelegate();
        if ([self checkForInitError:(_operationalCredentialsDelegate != nullptr) logMsg:kErrorOperationalCredentialsInit]) {
            return nil;
        }
        _operationalCredentialsDelegate->setChipWorkQueue(_chipWorkQueue);
    }
    return self;
}

- (BOOL)isRunning
{
    return self.cppCommissioner != nullptr;
}

- (void)shutdown
{
    if (_cppCommissioner == nullptr) {
        // Already shut down.
        return;
    }

    [self cleanupAfterStartup];
}

// Clean up from a state where startup was called.
- (void)cleanupAfterStartup
{
    [_factory controllerShuttingDown:self];
}

// Part of cleanupAfterStartup that has to interact with the Matter work queue
// in a very specific way that only MTRControllerFactory knows about.
- (void)shutDownCppController
{
    if (_cppCommissioner) {
        auto * commissionerToShutDown = _cppCommissioner;
        // Flag ourselves as not running before we start shutting down
        // _cppCommissioner, so we're not in a state where we claim to be
        // running but are actually partially shut down.
        _cppCommissioner = nullptr;
        commissionerToShutDown->Shutdown();
        delete commissionerToShutDown;
        if (_operationalCredentialsDelegate != nil) {
            _operationalCredentialsDelegate->SetDeviceCommissioner(nullptr);
        }
    }
}

- (void)deinitFromFactory
{
    [self cleanup];
}

// Clean up any members we might have allocated.
- (void)cleanup
{
    VerifyOrDie(_cppCommissioner == nullptr);

    [self clearDeviceAttestationDelegateBridge];

    if (_operationalCredentialsDelegate) {
        delete _operationalCredentialsDelegate;
        _operationalCredentialsDelegate = nullptr;
    }

    if (_partialDACVerifier) {
        delete _partialDACVerifier;
        _partialDACVerifier = nullptr;
    }

    if (_pairingDelegateBridge) {
        delete _pairingDelegateBridge;
        _pairingDelegateBridge = nullptr;
    }
}

- (BOOL)startup:(MTRDeviceControllerStartupParamsInternal *)startupParams
{
    __block BOOL commissionerInitialized = NO;
    if ([self isRunning]) {
        MTR_LOG_ERROR("Unexpected duplicate call to startup");
        return NO;
    }

    dispatch_sync(_chipWorkQueue, ^{
        if ([self isRunning]) {
            return;
        }

        if (startupParams.vendorID == nil || [startupParams.vendorID unsignedShortValue] == chip::VendorId::Common) {
            // Shouldn't be using the "standard" vendor ID for actual devices.
            MTR_LOG_ERROR("%@ is not a valid vendorID to initialize a device controller with", startupParams.vendorID);
            return;
        }

        if (startupParams.operationalCertificate == nil && startupParams.nodeID == nil) {
            MTR_LOG_ERROR("Can't start a controller if we don't know what node id it is");
            return;
        }

        if ([startupParams keypairsMatchCertificates] == NO) {
            MTR_LOG_ERROR("Provided keypairs do not match certificates");
            return;
        }

        if (startupParams.operationalCertificate != nil && startupParams.operationalKeypair == nil
            && (!startupParams.fabricIndex.HasValue()
                || !startupParams.keystore->HasOpKeypairForFabric(startupParams.fabricIndex.Value()))) {
            MTR_LOG_ERROR("Have no operational keypair for our operational certificate");
            return;
        }

        CHIP_ERROR errorCode = CHIP_ERROR_INCORRECT_STATE;

        // create a MTRP256KeypairBridge here and pass it to the operationalCredentialsDelegate
        chip::Crypto::P256Keypair * signingKeypair = nullptr;
        if (startupParams.nocSigner) {
            errorCode = _signingKeypairBridge.Init(startupParams.nocSigner);
            if ([self checkForStartError:errorCode logMsg:kErrorSigningKeypairInit]) {
                return;
            }
            signingKeypair = &_signingKeypairBridge;
        }
        errorCode = _operationalCredentialsDelegate->Init(_factory.storageDelegateBridge, signingKeypair, startupParams.ipk,
            startupParams.rootCertificate, startupParams.intermediateCertificate);
        if ([self checkForStartError:errorCode logMsg:kErrorOperationalCredentialsInit]) {
            return;
        }

        _cppCommissioner = new chip::Controller::DeviceCommissioner();
        if (_cppCommissioner == nullptr) {
            [self checkForStartError:CHIP_ERROR_NO_MEMORY logMsg:kErrorCommissionerInit];
            return;
        }

        // nocBuffer might not be used, but if it is it needs to live
        // long enough (until after we are done using
        // commissionerParams).
        uint8_t nocBuffer[chip::Controller::kMaxCHIPDERCertLength];

        chip::Controller::SetupParams commissionerParams;

        commissionerParams.pairingDelegate = _pairingDelegateBridge;

        _operationalCredentialsDelegate->SetDeviceCommissioner(_cppCommissioner);

        commissionerParams.operationalCredentialsDelegate = _operationalCredentialsDelegate;

        commissionerParams.controllerRCAC = _operationalCredentialsDelegate->RootCertSpan();
        commissionerParams.controllerICAC = _operationalCredentialsDelegate->IntermediateCertSpan();

        if (startupParams.operationalKeypair != nil) {
            errorCode = _operationalKeypairBridge.Init(startupParams.operationalKeypair);
            if ([self checkForStartError:errorCode logMsg:kErrorOperationalKeypairInit]) {
                return;
            }
            commissionerParams.operationalKeypair = &_operationalKeypairBridge;
            commissionerParams.hasExternallyOwnedOperationalKeypair = true;
        }

        if (startupParams.operationalCertificate) {
            commissionerParams.controllerNOC = AsByteSpan(startupParams.operationalCertificate);
        } else {
            chip::MutableByteSpan noc(nocBuffer);

            if (commissionerParams.operationalKeypair != nullptr) {
                errorCode = _operationalCredentialsDelegate->GenerateNOC(startupParams.nodeID.unsignedLongLongValue,
                    startupParams.fabricID.unsignedLongLongValue, chip::kUndefinedCATs,
                    commissionerParams.operationalKeypair->Pubkey(), noc);

                if ([self checkForStartError:errorCode logMsg:kErrorGenerateNOC]) {
                    return;
                }
            } else {
                // Generate a new random keypair.
                uint8_t csrBuffer[chip::Crypto::kMAX_CSR_Length];
                chip::MutableByteSpan csr(csrBuffer);
                errorCode = startupParams.fabricTable->AllocatePendingOperationalKey(startupParams.fabricIndex, csr);
                if ([self checkForStartError:errorCode logMsg:kErrorKeyAllocation]) {
                    return;
                }

                chip::Crypto::P256PublicKey pubKey;
                errorCode = VerifyCertificateSigningRequest(csr.data(), csr.size(), pubKey);
                if ([self checkForStartError:errorCode logMsg:kErrorCSRValidation]) {
                    return;
                }

                errorCode = _operationalCredentialsDelegate->GenerateNOC(startupParams.nodeID.unsignedLongLongValue,
                    startupParams.fabricID.unsignedLongLongValue, chip::kUndefinedCATs, pubKey, noc);

                if ([self checkForStartError:errorCode logMsg:kErrorGenerateNOC]) {
                    return;
                }
            }
            commissionerParams.controllerNOC = noc;
        }
        commissionerParams.controllerVendorId = static_cast<chip::VendorId>([startupParams.vendorID unsignedShortValue]);
        commissionerParams.deviceAttestationVerifier = _factory.deviceAttestationVerifier;

        auto & factory = chip::Controller::DeviceControllerFactory::GetInstance();

        errorCode = factory.SetupCommissioner(commissionerParams, *_cppCommissioner);
        if ([self checkForStartError:errorCode logMsg:kErrorCommissionerInit]) {
            return;
        }

        chip::FabricIndex fabricIdx = _cppCommissioner->GetFabricIndex();

        uint8_t compressedIdBuffer[sizeof(uint64_t)];
        chip::MutableByteSpan compressedId(compressedIdBuffer);
        errorCode = _cppCommissioner->GetCompressedFabricIdBytes(compressedId);
        if ([self checkForStartError:errorCode logMsg:kErrorIPKInit]) {
            return;
        }

        errorCode = chip::Credentials::SetSingleIpkEpochKey(
            _factory.groupData, fabricIdx, _operationalCredentialsDelegate->GetIPK(), compressedId);
        if ([self checkForStartError:errorCode logMsg:kErrorIPKInit]) {
            return;
        }

        commissionerInitialized = YES;
    });

    if (commissionerInitialized == NO) {
        [self cleanupAfterStartup];
    }

    return commissionerInitialized;
}

- (NSNumber *)controllerNodeID
{
    if (![self isRunning]) {
        MTR_LOG_ERROR("A controller has no node id if it has not been started");
        return nil;
    }
    __block NSNumber * nodeID;
    dispatch_sync(_chipWorkQueue, ^{
        if (![self isRunning]) {
            MTR_LOG_ERROR("A controller has no node id if it has not been started");
            nodeID = nil;
        } else {
            nodeID = @(_cppCommissioner->GetNodeId());
        }
    });
    return nodeID;
}

- (BOOL)setupCommissioningSessionWithPayload:(MTRSetupPayload *)payload
                                   newNodeID:(NSNumber *)newNodeID
                                       error:(NSError * __autoreleasing *)error
{
    VerifyOrReturnValue([self checkIsRunning:error], NO);

    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        // Try to get a QR code if possible (because it has a better
        // discriminator, etc), then fall back to manual code if that fails.
        NSString * pairingCode = [payload qrCodeString:nil];
        if (pairingCode == nil) {
            pairingCode = [payload manualEntryCode];
        }
        if (pairingCode == nil) {
            success = ![self checkForError:CHIP_ERROR_INVALID_ARGUMENT logMsg:kErrorSetupCodeGen error:error];
            return;
        }

        chip::NodeId nodeId = [newNodeID unsignedLongLongValue];
        _operationalCredentialsDelegate->SetDeviceID(nodeId);
        CHIP_ERROR errorCode = self.cppCommissioner->EstablishPASEConnection(nodeId, [pairingCode UTF8String]);
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });

    return success;
}

- (BOOL)pairDevice:(uint64_t)deviceID
     discriminator:(uint16_t)discriminator
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    VerifyOrReturnValue([self checkIsRunning:error], NO);

    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        std::string manualPairingCode;
        chip::SetupPayload payload;
        payload.discriminator.SetLongValue(discriminator);
        payload.setUpPINCode = setupPINCode;

        auto errorCode = chip::ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualPairingCode);
        success = ![self checkForError:errorCode logMsg:kErrorSetupCodeGen error:error];
        VerifyOrReturn(success);

        _operationalCredentialsDelegate->SetDeviceID(deviceID);
        errorCode = self.cppCommissioner->EstablishPASEConnection(deviceID, manualPairingCode.c_str());
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });

    return success;
}

- (BOOL)pairDevice:(uint64_t)deviceID
           address:(NSString *)address
              port:(uint16_t)port
      setupPINCode:(uint32_t)setupPINCode
             error:(NSError * __autoreleasing *)error
{
    VerifyOrReturnValue([self checkIsRunning:error], NO);

    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        chip::Inet::IPAddress addr;
        chip::Inet::IPAddress::FromString([address UTF8String], addr);
        chip::Transport::PeerAddress peerAddress = chip::Transport::PeerAddress::UDP(addr, port);

        _operationalCredentialsDelegate->SetDeviceID(deviceID);

        auto params = chip::RendezvousParameters().SetSetupPINCode(setupPINCode).SetPeerAddress(peerAddress);
        auto errorCode = self.cppCommissioner->EstablishPASEConnection(deviceID, params);
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });

    return success;
}

- (BOOL)pairDevice:(uint64_t)deviceID onboardingPayload:(NSString *)onboardingPayload error:(NSError * __autoreleasing *)error
{
    VerifyOrReturnValue([self checkIsRunning:error], NO);

    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        _operationalCredentialsDelegate->SetDeviceID(deviceID);
        auto errorCode = self.cppCommissioner->EstablishPASEConnection(deviceID, [onboardingPayload UTF8String]);
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });
    return success;
}

- (BOOL)commissionDevice:(uint64_t)deviceID
     commissioningParams:(MTRCommissioningParameters *)commissioningParams
                   error:(NSError * __autoreleasing *)error
{
    VerifyOrReturnValue([self checkIsRunning:error], NO);

    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        chip::Controller::CommissioningParameters params;
        if (commissioningParams.csrNonce) {
            params.SetCSRNonce(AsByteSpan(commissioningParams.csrNonce));
        }
        if (commissioningParams.attestationNonce) {
            params.SetAttestationNonce(AsByteSpan(commissioningParams.attestationNonce));
        }
        if (commissioningParams.threadOperationalDataset) {
            params.SetThreadOperationalDataset(AsByteSpan(commissioningParams.threadOperationalDataset));
        }
        if (commissioningParams.wifiSSID) {
            chip::ByteSpan ssid = AsByteSpan(commissioningParams.wifiSSID);
            chip::ByteSpan credentials;
            if (commissioningParams.wifiCredentials != nil) {
                credentials = AsByteSpan(commissioningParams.wifiCredentials);
            }
            chip::Controller::WiFiCredentials wifiCreds(ssid, credentials);
            params.SetWiFiCredentials(wifiCreds);
        }
        if (commissioningParams.deviceAttestationDelegate) {
            [self clearDeviceAttestationDelegateBridge];

            chip::Optional<uint16_t> timeoutSecs;
            if (commissioningParams.failSafeExpiryTimeout) {
                timeoutSecs
                    = chip::MakeOptional(static_cast<uint16_t>([commissioningParams.failSafeExpiryTimeout unsignedIntValue]));
            }
            BOOL shouldWaitAfterDeviceAttestation = NO;
            if ([commissioningParams.deviceAttestationDelegate
                    respondsToSelector:@selector(deviceAttestation:completedForDevice:attestationDeviceInfo:error:)]) {
                shouldWaitAfterDeviceAttestation = YES;
            }
            _deviceAttestationDelegateBridge = new MTRDeviceAttestationDelegateBridge(
                self, commissioningParams.deviceAttestationDelegate, _chipWorkQueue, timeoutSecs, shouldWaitAfterDeviceAttestation);
            params.SetDeviceAttestationDelegate(_deviceAttestationDelegateBridge);
        }

        _operationalCredentialsDelegate->SetDeviceID(deviceID);
        auto errorCode = self.cppCommissioner->Commission(deviceID, params);
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });
    return success;
}

- (BOOL)continueCommissioningDevice:(void *)device
           ignoreAttestationFailure:(BOOL)ignoreAttestationFailure
                              error:(NSError * __autoreleasing *)error
{
    VerifyOrReturnValue([self checkIsRunning:error], NO);

    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        auto lastAttestationResult = _deviceAttestationDelegateBridge
            ? _deviceAttestationDelegateBridge->attestationVerificationResult()
            : chip::Credentials::AttestationVerificationResult::kSuccess;

        auto deviceProxy = static_cast<chip::DeviceProxy *>(device);
        auto errorCode = self.cppCommissioner->ContinueCommissioningAfterDeviceAttestation(deviceProxy,
            ignoreAttestationFailure ? chip::Credentials::AttestationVerificationResult::kSuccess : lastAttestationResult);
        success = ![self checkForError:errorCode logMsg:kErrorPairDevice error:error];
    });
    return success;
}

- (BOOL)stopDevicePairing:(uint64_t)deviceID error:(NSError * __autoreleasing *)error
{
    VerifyOrReturnValue([self checkIsRunning:error], NO);

    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        _operationalCredentialsDelegate->ResetDeviceID();
        auto errorCode = self.cppCommissioner->StopPairing(deviceID);
        success = ![self checkForError:errorCode logMsg:kErrorStopPairing error:error];
    });

    return success;
}

- (MTRBaseDevice *)getDeviceBeingCommissioned:(uint64_t)deviceId error:(NSError * __autoreleasing *)error
{
    VerifyOrReturnValue([self checkIsRunning:error], nil);

    __block MTRBaseDevice * device;
    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        chip::CommissioneeDeviceProxy * deviceProxy;
        auto errorCode = self->_cppCommissioner->GetDeviceBeingCommissioned(deviceId, &deviceProxy);
        success = ![self checkForError:errorCode logMsg:kErrorStopPairing error:error];
        device = [[MTRBaseDevice alloc] initWithPASEDevice:deviceProxy controller:self];
    });
    VerifyOrReturnValue(success, nil);

    return device;
}

- (BOOL)getBaseDevice:(uint64_t)deviceID queue:(dispatch_queue_t)queue completion:(MTRDeviceConnectionCallback)completion
{
    // Will get removed once deviceWithNodeID exists.
    return [self getBaseDevice:deviceID queue:queue completionHandler:completion];
}

- (MTRDevice *)deviceForNodeID:(NSNumber *)nodeID
{
    os_unfair_lock_lock(&_deviceMapLock);
    MTRDevice * deviceToReturn = self.nodeIDToDeviceMap[nodeID];
    if (!deviceToReturn) {
        deviceToReturn = [[MTRDevice alloc] initWithNodeID:nodeID controller:self];
        self.nodeIDToDeviceMap[nodeID] = deviceToReturn;
    }
    os_unfair_lock_unlock(&_deviceMapLock);

    return deviceToReturn;
}

- (void)removeDevice:(MTRDevice *)device
{
    os_unfair_lock_lock(&_deviceMapLock);
    MTRDevice * deviceToRemove = self.nodeIDToDeviceMap[device.nodeID];
    if (deviceToRemove == device) {
        self.nodeIDToDeviceMap[device.nodeID] = nil;
    } else {
        MTR_LOG_ERROR("Error: Cannot remove device %p with nodeID %llu", device, device.nodeID.unsignedLongLongValue);
    }
    os_unfair_lock_unlock(&_deviceMapLock);
}

- (BOOL)openPairingWindow:(uint64_t)deviceID duration:(NSUInteger)duration error:(NSError * __autoreleasing *)error
{
    VerifyOrReturnValue([self checkIsRunning:error], NO);

    if (duration > UINT16_MAX) {
        MTR_LOG_ERROR("Error: Duration %tu is too large. Max value %d", duration, UINT16_MAX);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return NO;
    }

    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        auto errorCode = chip::Controller::AutoCommissioningWindowOpener::OpenBasicCommissioningWindow(
            self.cppCommissioner, deviceID, chip::System::Clock::Seconds16(static_cast<uint16_t>(duration)));
        success = ![self checkForError:errorCode logMsg:kErrorOpenPairingWindow error:error];
    });

    return success;
}

- (NSString *)openPairingWindowWithPIN:(uint64_t)deviceID
                              duration:(NSUInteger)duration
                         discriminator:(NSUInteger)discriminator
                              setupPIN:(NSUInteger)setupPIN
                                 error:(NSError * __autoreleasing *)error
{
    __block NSString * rv = nil;

    VerifyOrReturnValue([self checkIsRunning:error], rv);

    if (duration > UINT16_MAX) {
        MTR_LOG_ERROR("Error: Duration %tu is too large. Max value %d", duration, UINT16_MAX);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return rv;
    }

    if (discriminator > 0xfff) {
        MTR_LOG_ERROR("Error: Discriminator %tu is too large. Max value %d", discriminator, 0xfff);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return rv;
    }

    if (!chip::CanCastTo<uint32_t>(setupPIN) || !chip::SetupPayload::IsValidSetupPIN(static_cast<uint32_t>(setupPIN))) {
        MTR_LOG_ERROR("Error: Setup pin %lu is not valid", static_cast<unsigned long>(setupPIN));
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_INTEGER_VALUE];
        }
        return rv;
    }

    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        chip::SetupPayload setupPayload;
        auto errorCode = chip::Controller::AutoCommissioningWindowOpener::OpenCommissioningWindow(self.cppCommissioner, deviceID,
            chip::System::Clock::Seconds16(static_cast<uint16_t>(duration)), chip::Crypto::kSpake2p_Min_PBKDF_Iterations,
            static_cast<uint16_t>(discriminator), chip::MakeOptional(static_cast<uint32_t>(setupPIN)), chip::NullOptional,
            setupPayload);

        auto success = ![self checkForError:errorCode logMsg:kErrorOpenPairingWindow error:error];
        VerifyOrReturn(success);

        chip::ManualSetupPayloadGenerator generator(setupPayload);
        std::string outCode;

        if (generator.payloadDecimalStringRepresentation(outCode) == CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Setup code is %s", outCode.c_str());
            rv = [NSString stringWithCString:outCode.c_str() encoding:[NSString defaultCStringEncoding]];
        } else {
            MTR_LOG_ERROR("Failed to get decimal setup code");
        }
    });

    return rv;
}

- (void)setPairingDelegate:(id<MTRDevicePairingDelegate>)delegate queue:(dispatch_queue_t)queue
{
    VerifyOrReturn([self checkIsRunning]);

    dispatch_async(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning]);

        self->_pairingDelegateBridge->setDelegate(delegate, queue);
    });
}

- (void)setNocChainIssuer:(id<MTRNOCChainIssuer>)nocChainIssuer queue:(dispatch_queue_t)queue
{
    VerifyOrReturn([self checkIsRunning]);

    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning]);

        if (nocChainIssuer != nil) {
            self->_operationalCredentialsDelegate->SetNocChainIssuer(nocChainIssuer, queue);
            self->_cppCommissioner->SetDeviceAttestationVerifier(_partialDACVerifier);
        } else {
            self->_cppCommissioner->SetDeviceAttestationVerifier(chip::Credentials::GetDeviceAttestationVerifier());
        }
    });
}

- (nullable NSData *)computePaseVerifier:(uint32_t)setupPincode iterations:(uint32_t)iterations salt:(NSData *)salt
{
    chip::Spake2pVerifier verifier;
    CHIP_ERROR err = verifier.Generate(iterations, AsByteSpan(salt), setupPincode);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("computePaseVerifier generation failed: %s", chip::ErrorStr(err));
        return nil;
    }

    uint8_t serializedBuffer[chip::Crypto::kSpake2p_VerifierSerialized_Length];
    chip::MutableByteSpan serializedBytes(serializedBuffer);
    err = verifier.Serialize(serializedBytes);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("computePaseVerifier serialization failed: %s", chip::ErrorStr(err));
        return nil;
    }

    return AsData(serializedBytes);
}

- (NSData * _Nullable)attestationChallengeForDeviceID:(NSNumber *)deviceID
{
    VerifyOrReturnValue([self checkIsRunning], nil);

    __block NSData * attestationChallenge;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning]);

        chip::CommissioneeDeviceProxy * deviceProxy;
        auto errorCode = self.cppCommissioner->GetDeviceBeingCommissioned([deviceID unsignedLongLongValue], &deviceProxy);
        auto success = ![self checkForError:errorCode logMsg:kErrorGetCommissionee error:nil];
        VerifyOrReturn(success);

        uint8_t challengeBuffer[chip::Crypto::kAES_CCM128_Key_Length];
        chip::ByteSpan challenge(challengeBuffer);

        errorCode = deviceProxy->GetAttestationChallenge(challenge);
        success = ![self checkForError:errorCode logMsg:kErrorGetAttestationChallenge error:nil];
        VerifyOrReturn(success);

        attestationChallenge = AsData(challenge);
    });

    return attestationChallenge;
}

- (BOOL)checkForInitError:(BOOL)condition logMsg:(NSString *)logMsg
{
    if (condition) {
        return NO;
    }

    MTR_LOG_ERROR("Error: %@", logMsg);

    [self cleanup];

    return YES;
}

- (void)clearDeviceAttestationDelegateBridge
{
    if (_deviceAttestationDelegateBridge) {
        delete _deviceAttestationDelegateBridge;
        _deviceAttestationDelegateBridge = nullptr;
    }
}

- (BOOL)checkForStartError:(CHIP_ERROR)errorCode logMsg:(NSString *)logMsg
{
    if (CHIP_NO_ERROR == errorCode) {
        return NO;
    }

    MTR_LOG_ERROR("Error(%" CHIP_ERROR_FORMAT "): %@", errorCode.Format(), logMsg);

    return YES;
}

- (BOOL)checkForError:(CHIP_ERROR)errorCode logMsg:(NSString *)logMsg error:(NSError * __autoreleasing *)error
{
    if (CHIP_NO_ERROR == errorCode) {
        return NO;
    }

    MTR_LOG_ERROR("Error(%" CHIP_ERROR_FORMAT "): %s", errorCode.Format(), [logMsg UTF8String]);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:errorCode];
    }

    return YES;
}

- (BOOL)checkIsRunning
{
    return [self checkIsRunning:nil];
}

- (BOOL)checkIsRunning:(NSError * __autoreleasing *)error
{
    if ([self isRunning]) {
        return YES;
    }

    MTR_LOG_ERROR("Error: %s", [kErrorNotRunning UTF8String]);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE];
    }

    return NO;
}

- (BOOL)_deviceBeingCommissionedOverBLE:(uint64_t)deviceID
{
    VerifyOrReturnValue([self checkIsRunning], NO);

    chip::CommissioneeDeviceProxy * deviceProxy;
    auto errorCode = self->_cppCommissioner->GetDeviceBeingCommissioned(deviceID, &deviceProxy);
    VerifyOrReturnValue(errorCode == CHIP_NO_ERROR, NO);

    return deviceProxy->GetDeviceTransportType() == chip::Transport::Type::kBle;
}

- (BOOL)getSessionForNode:(chip::NodeId)nodeID completion:(MTRInternalDeviceConnectionCallback)completion
{
    if (![self checkIsRunning]) {
        return NO;
    }

    dispatch_async(_chipWorkQueue, ^{
        NSError * error;
        if (![self checkIsRunning:&error]) {
            completion(nullptr, chip::NullOptional, error);
            return;
        }

        auto connectionBridge = new MTRDeviceConnectionBridge(completion);

        // MTRDeviceConnectionBridge always delivers errors async via
        // completion.
        connectionBridge->connect(self->_cppCommissioner, nodeID);
    });

    return YES;
}

- (BOOL)getSessionForCommissioneeDevice:(chip::NodeId)deviceID completion:(MTRInternalDeviceConnectionCallback)completion
{
    if (![self checkIsRunning]) {
        return NO;
    }

    dispatch_async(_chipWorkQueue, ^{
        NSError * error;
        if (![self checkIsRunning:&error]) {
            completion(nullptr, chip::NullOptional, error);
            return;
        }

        chip::CommissioneeDeviceProxy * deviceProxy;
        CHIP_ERROR err = self->_cppCommissioner->GetDeviceBeingCommissioned(deviceID, &deviceProxy);
        if (err != CHIP_NO_ERROR) {
            completion(nullptr, chip::NullOptional, [MTRError errorForCHIPErrorCode:err]);
            return;
        }

        chip::Optional<chip::SessionHandle> session = deviceProxy->GetSecureSession();
        if (!session.HasValue() || !session.Value()->AsSecureSession()->IsPASESession()) {
            completion(nullptr, chip::NullOptional, [MTRError errorForCHIPErrorCode:CHIP_ERROR_INCORRECT_STATE]);
            return;
        }

        completion(deviceProxy->GetExchangeManager(), session, nil);
    });

    return YES;
}

- (void)asyncDispatchToMatterQueue:(void (^)(chip::Controller::DeviceCommissioner *))block
                      errorHandler:(void (^)(NSError *))errorHandler
{
    {
        NSError * error;
        if (![self checkIsRunning:&error]) {
            errorHandler(error);
            return;
        }
    }

    dispatch_async(_chipWorkQueue, ^{
        NSError * error;
        if (![self checkIsRunning:&error]) {
            errorHandler(error);
            return;
        }

        block(self.cppCommissioner);
    });
}

@end

@implementation MTRDeviceController (InternalMethods)

- (chip::FabricIndex)fabricIndex
{
    if (!_cppCommissioner) {
        return chip::kUndefinedFabricIndex;
    }

    return _cppCommissioner->GetFabricIndex();
}

- (CHIP_ERROR)isRunningOnFabric:(chip::FabricTable *)fabricTable
                    fabricIndex:(chip::FabricIndex)fabricIndex
                      isRunning:(BOOL *)isRunning
{
    if (![self isRunning]) {
        *isRunning = NO;
        return CHIP_NO_ERROR;
    }

    const chip::FabricInfo * otherFabric = fabricTable->FindFabricWithIndex(fabricIndex);
    if (!otherFabric) {
        // Should not happen...
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (_cppCommissioner->GetFabricId() != otherFabric->GetFabricId()) {
        *isRunning = NO;
        return CHIP_NO_ERROR;
    }

    chip::Crypto::P256PublicKey ourRootPublicKey, otherRootPublicKey;
    ReturnErrorOnFailure(_cppCommissioner->GetRootPublicKey(ourRootPublicKey));
    ReturnErrorOnFailure(fabricTable->FetchRootPubkey(otherFabric->GetFabricIndex(), otherRootPublicKey));

    *isRunning = (ourRootPublicKey.Matches(otherRootPublicKey));
    return CHIP_NO_ERROR;
}

- (void)invalidateCASESessionForNode:(chip::NodeId)nodeID;
{
    if (![self checkIsRunning]) {
        return;
    }

    dispatch_sync(_chipWorkQueue, ^{
        if (![self checkIsRunning]) {
            return;
        }

        auto sessionMgr = self->_cppCommissioner->SessionMgr();
        VerifyOrDie(sessionMgr != nullptr);

        sessionMgr->MarkSessionsAsDefunct(
            self->_cppCommissioner->GetPeerScopedId(nodeID), chip::MakeOptional(chip::Transport::SecureSession::Type::kCASE));
    });
}
@end

@implementation MTRDeviceController (Deprecated)

- (NSNumber *)controllerNodeId
{
    return self.controllerNodeID;
}

- (nullable NSData *)fetchAttestationChallengeForDeviceId:(uint64_t)deviceId
{
    return [self attestationChallengeForDeviceID:@(deviceId)];
}

- (BOOL)getBaseDevice:(uint64_t)deviceID queue:(dispatch_queue_t)queue completionHandler:(MTRDeviceConnectionCallback)completion
{
    NSError * error;
    if (![self checkIsRunning:&error]) {
        dispatch_async(queue, ^{
            completion(nil, error);
        });
        return NO;
    }

    // We know getSessionForNode will return YES here, since we already checked
    // that we are running.
    return [self getSessionForNode:deviceID
                        completion:^(chip::Messaging::ExchangeManager * _Nullable exchangeManager,
                            const chip::Optional<chip::SessionHandle> & session, NSError * _Nullable error) {
                            // Create an MTRBaseDevice for the node id involved, now that our
                            // CASE session is primed.  We don't actually care about the session
                            // information here.
                            dispatch_async(queue, ^{
                                MTRBaseDevice * device;
                                if (error == nil) {
                                    device = [[MTRBaseDevice alloc] initWithNodeID:@(deviceID) controller:self];
                                } else {
                                    device = nil;
                                }
                                completion(device, error);
                            });
                        }];
}

@end
