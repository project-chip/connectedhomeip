/**
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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

#import "DataSourceCompat.h"

#import "MCCastingApp.h"
#import "MCCommissionableData.h"
#import "MCCryptoUtils.h"
#import "MCDeviceAttestationCredentials.h"
#import "MCErrorUtils.h"

#include <app/data-model/ListLargeSystemExtensions.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <lib/support/CHIPMem.h>
#include <platform/PlatformManager.h>

@interface DataSourceCompat ()

@property (nonatomic, strong) dispatch_queue_t clientQueue;
@property (nonatomic, strong) AppParameters * appParameters;
@property (nonatomic, strong) DeviceAttestationCredentialsHolder * dacHolder;

@end

@implementation DataSourceCompat

- (instancetype)initWithClientQueue:(dispatch_queue_t)clientQueue
{
    self = [super init];
    if (self) {
        _clientQueue = clientQueue;
    }
    return self;
}

- (dispatch_queue_t _Nonnull)clientQueue
{
    return _clientQueue;
}

- (void)setAppParameters:(AppParameters *)appParameters
{
    _appParameters = appParameters;
}

- (void)setDacHolder:(DeviceAttestationCredentialsHolder *)dacHolder
{
    _dacHolder = dacHolder;
}

- (NSData *)castingAppDidReceiveRequestForRotatingDeviceIdUniqueId:(id)sender
{
    ChipLogProgress(AppServer, "DataSourceCompat::castingAppDidReceiveRequestForRotatingDeviceIdUniqueId called");
    if (self.appParameters != nil) {
        return self.appParameters.rotatingDeviceIdUniqueId;
    }
    return nil;
}

- (MCCommissionableData *)castingAppDidReceiveRequestForCommissionableData:(id)sender
{
    ChipLogProgress(AppServer, "DataSourceCompat::castingAppDidReceiveRequestForCommissionableData called");
    if (self.appParameters != nil) {
        return [[MCCommissionableData alloc] initWithPasscode:self.appParameters.onboardingPayload.setupPasscode discriminator:self.appParameters.onboardingPayload.setupDiscriminator spake2pIterationCount:self.appParameters.spake2pIterationCount spake2pVerifier:self.appParameters.spake2pVerifierBase64 spake2pSalt:self.appParameters.spake2pSaltBase64];
    }
    return nil;
}

- (MCDeviceAttestationCredentials *)castingAppDidReceiveRequestForDeviceAttestationCredentials:(id)sender
{
    ChipLogProgress(AppServer, "DataSourceCompat::castingAppDidReceiveRequestForDeviceAttestationCredentials called");
    if (self.dacHolder != nil) {
        return [[MCDeviceAttestationCredentials alloc] initWithCertificationDeclaration:[self.dacHolder getCertificationDeclaration] firmwareInformation:[self.dacHolder getFirmwareInformation] deviceAttestationCert:[self.dacHolder getDeviceAttestationCert] productAttestationIntermediateCert:[self.dacHolder getProductAttestationIntermediateCert]];
    }
    return nil;
}

- (MatterError *)castingApp:(id)sender didReceiveRequestToSignCertificateRequest:(NSData *)csrData outRawSignature:(NSData * _Nonnull * _Nonnull)outRawSignature
{
    ChipLogProgress(AppServer, "DataSourceCompat::castingApp didReceiveRequestToSignCertificateRequest called");
    if (self.dacHolder != nil) {
        // Get the private SecKey from dacHolder
        SecKeyRef privateSecKey = [self.dacHolder getDeviceAttestationCertPrivateKeyRef];
        if (privateSecKey == nil) {
            ChipLogError(AppServer, "DataSourceCompat::castingApp didReceiveRequestToSignCertificateRequest No privateSecKey found");
            return [MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INVALID_ARGUMENT];
        }

        // Sign csrData to get asn1SignatureData
        CFErrorRef error = nil;
        CFDataRef asn1SignatureData = SecKeyCreateSignature(privateSecKey, kSecKeyAlgorithmECDSASignatureMessageX962SHA256, (__bridge CFDataRef) csrData, &error);
        if (error != nil) {
            ChipLogError(AppServer, "DataSourceCompat::castingApp didReceiveRequestToSignCertificateRequest failed to sign message, err: %@", error);
            return [MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INVALID_ARGUMENT];
        }
        if (asn1SignatureData == nil) {
            ChipLogError(AppServer, "DataSourceCompat::castingApp didReceiveRequestToSignCertificateRequest failed to sign message, asn1SignatureData is nil");
            return [MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INVALID_ARGUMENT];
        }

        // Convert ASN.1 DER signature to SEC1 raw format
        return [MCCryptoUtils ecdsaAsn1SignatureToRawWithFeLengthBytes:32 asn1Signature:asn1SignatureData
                                                       outRawSignature:outRawSignature];
    }

    return [MCErrorUtils MatterErrorFromChipError:CHIP_ERROR_INCORRECT_STATE];
}
@end
