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
#import "MTRDeviceController.h"

#import "MTRBaseDevice_Internal.h"
#import "MTRCommissioningParameters.h"
#import "MTRControllerFactory_Internal.h"
#import "MTRDeviceControllerStartupParams.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRDevicePairingDelegateBridge.h"
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

@interface MTRDeviceController ()

// queue used to serialize all work performed by the MTRDeviceController
@property (atomic, readonly) dispatch_queue_t chipWorkQueue;

@property (readonly) chip::Controller::DeviceCommissioner * cppCommissioner;
@property (readonly) MTRDevicePairingDelegateBridge * pairingDelegateBridge;
@property (readonly) MTROperationalCredentialsDelegate * operationalCredentialsDelegate;
@property (readonly) MTRP256KeypairBridge signingKeypairBridge;
@property (readonly) MTRP256KeypairBridge operationalKeypairBridge;
@property (readonly) MTRDeviceAttestationDelegateBridge * deviceAttestationDelegateBridge;
@property (readonly) MTRControllerFactory * factory;
@end

@implementation MTRDeviceController

- (instancetype)initWithFactory:(MTRControllerFactory *)factory queue:(dispatch_queue_t)queue
{
    if (self = [super init]) {
        _chipWorkQueue = queue;
        _factory = factory;

        _pairingDelegateBridge = new MTRDevicePairingDelegateBridge();
        if ([self checkForInitError:(_pairingDelegateBridge != nullptr) logMsg:kErrorPairingInit]) {
            return nil;
        }

        _operationalCredentialsDelegate = new MTROperationalCredentialsDelegate();
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
    [self cleanup];
}

// Part of cleanupAfterStartup that has to interact with the Matter work queue
// in a very specific way that only MTRControllerFactory knows about.
- (void)shutDownCppController
{
    if (_cppCommissioner) {
        _cppCommissioner->Shutdown();
        delete _cppCommissioner;
        _cppCommissioner = nullptr;
    }
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

        if (startupParams.vendorId == nil || [startupParams.vendorId unsignedShortValue] == chip::VendorId::Common) {
            // Shouldn't be using the "standard" vendor ID for actual devices.
            MTR_LOG_ERROR("%@ is not a valid vendorId to initialize a device controller with", startupParams.vendorId);
            return;
        }

        if (startupParams.operationalCertificate == nil && startupParams.nodeId == nil) {
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
            if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorSigningKeypairInit]) {
                return;
            }
            signingKeypair = &_signingKeypairBridge;
        }
        errorCode = _operationalCredentialsDelegate->Init(_factory.storageDelegateBridge, signingKeypair, startupParams.ipk,
            startupParams.rootCertificate, startupParams.intermediateCertificate);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorOperationalCredentialsInit]) {
            return;
        }

        _cppCommissioner = new chip::Controller::DeviceCommissioner();
        if ([self checkForStartError:(_cppCommissioner != nullptr) logMsg:kErrorCommissionerInit]) {
            return;
        }

        // nocBuffer might not be used, but if it is it needs to live
        // long enough (until after we are done using
        // commissionerParams).
        uint8_t nocBuffer[chip::Controller::kMaxCHIPDERCertLength];

        chip::Controller::SetupParams commissionerParams;

        commissionerParams.pairingDelegate = _pairingDelegateBridge;

        commissionerParams.operationalCredentialsDelegate = _operationalCredentialsDelegate;

        commissionerParams.controllerRCAC = _operationalCredentialsDelegate->RootCertSpan();
        commissionerParams.controllerICAC = _operationalCredentialsDelegate->IntermediateCertSpan();

        if (startupParams.operationalKeypair != nil) {
            errorCode = _operationalKeypairBridge.Init(startupParams.operationalKeypair);
            if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorOperationalKeypairInit]) {
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
                errorCode = _operationalCredentialsDelegate->GenerateNOC([startupParams.nodeId unsignedLongLongValue],
                    startupParams.fabricId, chip::kUndefinedCATs, commissionerParams.operationalKeypair->Pubkey(), noc);

                if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorGenerateNOC]) {
                    return;
                }
            } else {
                // Generate a new random keypair.
                uint8_t csrBuffer[chip::Crypto::kMAX_CSR_Length];
                chip::MutableByteSpan csr(csrBuffer);
                errorCode = startupParams.fabricTable->AllocatePendingOperationalKey(startupParams.fabricIndex, csr);
                if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorKeyAllocation]) {
                    return;
                }

                chip::Crypto::P256PublicKey pubKey;
                errorCode = VerifyCertificateSigningRequest(csr.data(), csr.size(), pubKey);
                if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCSRValidation]) {
                    return;
                }

                errorCode = _operationalCredentialsDelegate->GenerateNOC(
                    [startupParams.nodeId unsignedLongLongValue], startupParams.fabricId, chip::kUndefinedCATs, pubKey, noc);

                if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorGenerateNOC]) {
                    return;
                }
            }
            commissionerParams.controllerNOC = noc;
        }
        commissionerParams.controllerVendorId = static_cast<chip::VendorId>([startupParams.vendorId unsignedShortValue]);
        commissionerParams.deviceAttestationVerifier = _factory.deviceAttestationVerifier;

        auto & factory = chip::Controller::DeviceControllerFactory::GetInstance();

        errorCode = factory.SetupCommissioner(commissionerParams, *_cppCommissioner);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorCommissionerInit]) {
            return;
        }

        chip::FabricIndex fabricIdx = _cppCommissioner->GetFabricIndex();

        uint8_t compressedIdBuffer[sizeof(uint64_t)];
        chip::MutableByteSpan compressedId(compressedIdBuffer);
        errorCode = _cppCommissioner->GetCompressedFabricIdBytes(compressedId);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorIPKInit]) {
            return;
        }

        errorCode = chip::Credentials::SetSingleIpkEpochKey(
            _factory.groupData, fabricIdx, _operationalCredentialsDelegate->GetIPK(), compressedId);
        if ([self checkForStartError:(CHIP_NO_ERROR == errorCode) logMsg:kErrorIPKInit]) {
            return;
        }

        commissionerInitialized = YES;
    });

    if (commissionerInitialized == NO) {
        [self cleanupAfterStartup];
    }

    return commissionerInitialized;
}

- (NSNumber *)controllerNodeId
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
        payload.discriminator = discriminator;
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

- (BOOL)commissionDevice:(uint64_t)deviceId
     commissioningParams:(MTRCommissioningParameters *)commissioningParams
                   error:(NSError * __autoreleasing *)error
{
    VerifyOrReturnValue([self checkIsRunning:error], NO);

    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        chip::Controller::CommissioningParameters params;
        if (commissioningParams.CSRNonce) {
            params.SetCSRNonce(chip::ByteSpan((uint8_t *) commissioningParams.CSRNonce.bytes, commissioningParams.CSRNonce.length));
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
        if (commissioningParams.deviceAttestationDelegate) {
            [self clearDeviceAttestationDelegateBridge];

            chip::Optional<uint16_t> timeoutSecs;
            if (commissioningParams.failSafeExpiryTimeoutSecs) {
                timeoutSecs
                    = chip::MakeOptional(static_cast<uint16_t>([commissioningParams.failSafeExpiryTimeoutSecs unsignedIntValue]));
            }
            _deviceAttestationDelegateBridge = new MTRDeviceAttestationDelegateBridge(
                self, commissioningParams.deviceAttestationDelegate, _chipWorkQueue, timeoutSecs);
            params.SetDeviceAttestationDelegate(_deviceAttestationDelegateBridge);
        }

        _operationalCredentialsDelegate->SetDeviceID(deviceId);
        auto errorCode = self.cppCommissioner->Commission(deviceId, params);
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
        auto errorCode = self.cppCommissioner->ContinueCommissioningAfterDeviceAttestationFailure(deviceProxy,
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

    __block chip::CommissioneeDeviceProxy * deviceProxy;

    __block BOOL success = NO;
    dispatch_sync(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning:error]);

        auto errorCode = self->_cppCommissioner->GetDeviceBeingCommissioned(deviceId, &deviceProxy);
        success = ![self checkForError:errorCode logMsg:kErrorStopPairing error:error];
    });
    VerifyOrReturnValue(success, nil);

    return [[MTRBaseDevice alloc] initWithDevice:deviceProxy];
}

- (BOOL)getBaseDevice:(uint64_t)deviceID
                queue:(dispatch_queue_t)queue
    completionHandler:(MTRDeviceConnectionCallback)completionHandler
{
    NSError * error;
    if (![self checkIsRunning:&error]) {
        dispatch_async(queue, ^{
            completionHandler(nil, error);
        });
        return NO;
    }

    dispatch_async(_chipWorkQueue, ^{
        VerifyOrReturn([self checkIsRunning]);

        auto connectionBridge = new MTRDeviceConnectionBridge(completionHandler, queue);
        auto errorCode = connectionBridge->connect(self->_cppCommissioner, deviceID);
        if ([self checkForError:errorCode logMsg:kErrorGetPairedDevice error:nil]) {
            // Errors are propagated to the caller through completionHandler.
            // No extra error handling is needed here.
            return;
        }
    });

    return YES;
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

    setupPIN &= ((1 << chip::kSetupPINCodeFieldLengthInBits) - 1);

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
    dispatch_async(_chipWorkQueue, ^{
        self->_pairingDelegateBridge->setDelegate(delegate, queue);
    });
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

- (BOOL)checkForStartError:(BOOL)condition logMsg:(NSString *)logMsg
{
    if (condition) {
        return NO;
    }

    MTR_LOG_ERROR("Error: %@", logMsg);

    return YES;
}

- (BOOL)checkForError:(CHIP_ERROR)errorCode logMsg:(NSString *)logMsg error:(NSError * __autoreleasing *)error
{
    if (CHIP_NO_ERROR == errorCode) {
        return NO;
    }

    MTR_LOG_ERROR("Error(%s): %s", chip::ErrorStr(errorCode), [logMsg UTF8String]);
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

- (BOOL)_deviceBeingCommissionedOverBLE:(uint64_t)deviceId
{
    VerifyOrReturnValue([self checkIsRunning], NO);

    chip::CommissioneeDeviceProxy * deviceProxy;
    auto errorCode = self->_cppCommissioner->GetDeviceBeingCommissioned(deviceId, &deviceProxy);
    VerifyOrReturnValue(errorCode == CHIP_NO_ERROR, NO);

    return deviceProxy->GetDeviceTransportType() == chip::Transport::Type::kBle;
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

@end
