/**
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "MTRDeviceControllerStartupParams.h"
#import "MTRCertificates.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRLogging.h"
#import "MTRP256KeypairBridge.h"
#import "NSDataSpanConversion.h"

#include <controller/OperationalCredentialsDelegate.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>
#include <lib/core/PeerId.h>

using namespace chip;

@implementation MTRDeviceControllerStartupParams

- (instancetype)initWithSigningKeypair:(id<MTRKeypair>)nocSigner fabricID:(NSNumber *)fabricID ipk:(NSData *)ipk
{
    if (!(self = [super init])) {
        return nil;
    }

    if (!IsValidFabricId(fabricID.unsignedLongLongValue)) {
        MTR_LOG_ERROR("%llu is not a valid fabric id to initialize a device controller with", fabricID.unsignedLongLongValue);
        return nil;
    }

    _nocSigner = nocSigner;
    _fabricID = [fabricID copy];
    _ipk = [ipk copy];

    return self;
}

- (instancetype)initWithOperationalKeypair:(id<MTRKeypair>)operationalKeypair
                    operationalCertificate:(MTRCertificateDERBytes)operationalCertificate
                   intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
                           rootCertificate:(MTRCertificateDERBytes)rootCertificate
                                       ipk:(NSData *)ipk
{
    if (!(self = [super init])) {
        return nil;
    }

    { // Scope for temporaries
        // ExtractNodeIdFabricIdFromOpCert needs a TLV-encoded opcert, not a DER-encoded one.
        uint8_t tlvOpCertBuf[Credentials::kMaxCHIPCertLength];
        MutableByteSpan tlvOpCert(tlvOpCertBuf);
        CHIP_ERROR err = Credentials::ConvertX509CertToChipCert(AsByteSpan(operationalCertificate), tlvOpCert);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Unable to convert operational certificate to TLV: %s", ErrorStr(err));
            return nil;
        }

        FabricId fabricId = kUndefinedFabricId;
        NodeId unused = kUndefinedNodeId;
        err = Credentials::ExtractNodeIdFabricIdFromOpCert(tlvOpCert, &unused, &fabricId);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Unable to extract fabric id from operational certificate: %s", ErrorStr(err));
            return nil;
        }
        _fabricID = @(fabricId);
    }

    _operationalKeypair = operationalKeypair;
    _operationalCertificate = [operationalCertificate copy];
    _intermediateCertificate = [intermediateCertificate copy];
    _rootCertificate = [rootCertificate copy];
    _ipk = [ipk copy];

    return self;
}

- (instancetype)initWithParams:(MTRDeviceControllerStartupParams *)params
{
    if (!(self = [super init])) {
        return nil;
    }

    _nocSigner = params.nocSigner;
    _fabricID = params.fabricID;
    _ipk = params.ipk;
    _vendorID = params.vendorID;
    _nodeID = params.nodeID;
    _rootCertificate = params.rootCertificate;
    _intermediateCertificate = params.intermediateCertificate;
    _operationalCertificate = params.operationalCertificate;
    _operationalKeypair = params.operationalKeypair;

    return self;
}

@end

// Convert a ByteSpan representing a Matter TLV certificate into NSData holding
// a DER X.509 certificate.  Returns nil on failures.
static NSData * _Nullable MatterCertToX509Data(const ByteSpan & cert)
{
    uint8_t buf[Controller::kMaxCHIPDERCertLength];
    MutableByteSpan derCert(buf);
    CHIP_ERROR err = Credentials::ConvertChipCertToX509Cert(cert, derCert);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Failed do convert Matter certificate to X.509 DER: %s", ErrorStr(err));
        return nil;
    }

    return AsData(derCert);
}

@implementation MTRDeviceControllerStartupParams (Deprecated)

- (uint64_t)fabricId
{
    return self.fabricID.unsignedLongLongValue;
}

- (nullable NSNumber *)vendorId
{
    return self.vendorID;
}

- (void)setVendorId:(nullable NSNumber *)vendorId
{
    self.vendorID = vendorId;
}

- (nullable NSNumber *)nodeId
{
    return self.nodeID;
}

- (void)setNodeId:(nullable NSNumber *)nodeId
{
    self.nodeID = nodeId;
}

- (instancetype)initWithSigningKeypair:(id<MTRKeypair>)nocSigner fabricId:(uint64_t)fabricId ipk:(NSData *)ipk
{
    return [self initWithSigningKeypair:nocSigner fabricID:@(fabricId) ipk:ipk];
}

@end

@implementation MTRDeviceControllerStartupParamsInternal

- (instancetype)initWithParams:(MTRDeviceControllerStartupParams *)params
{
    if (!(self = [super initWithParams:params])) {
        return nil;
    }

    if (self.nocSigner == nil && self.rootCertificate == nil) {
        MTR_LOG_ERROR("nocSigner and rootCertificate are both nil; no public key available to identify the fabric");
        return nil;
    }

    if (self.operationalCertificate != nil && self.nodeID != nil) {
        MTR_LOG_ERROR("nodeID must be nil if operationalCertificate is not nil");
        return nil;
    }

    if (self.operationalCertificate != nil) {
        if (self.operationalKeypair == nil) {
            MTR_LOG_ERROR("Must have an operational keypair if an operational certificate is provided");
            return nil;
        }

        if (![MTRCertificates keypair:self.operationalKeypair matchesCertificate:self.operationalCertificate]) {
            MTR_LOG_ERROR("operationalKeypair public key does not match operationalCertificate");
            return nil;
        }
    }

    return self;
}

- (instancetype)initForNewFabric:(chip::FabricTable *)fabricTable
                        keystore:(chip::Crypto::OperationalKeystore *)keystore
                          params:(MTRDeviceControllerStartupParams *)params
{
    if (!(self = [self initWithParams:params])) {
        return nil;
    }

    if (self.nocSigner == nil && self.operationalCertificate == nil) {
        MTR_LOG_ERROR("No way to get an operational certificate: nocSigner and operationalCertificate are both nil");
        return nil;
    }

    if (self.operationalCertificate == nil && self.nodeID == nil) {
        // Just avoid setting the top bit, to avoid issues with node
        // ids outside the operational range.
        uint64_t nodeId = arc4random();
        nodeId = (nodeId << 31) | (arc4random() >> 1);
        self.nodeID = @(nodeId);
    }

    if (self.rootCertificate == nil) {
        NSError * error;
        self.rootCertificate = [MTRCertificates createRootCertificate:self.nocSigner
                                                             issuerID:nil
                                                             fabricID:self.fabricID
                                                                error:&error];
        if (error != nil || self.rootCertificate == nil) {
            MTR_LOG_ERROR("Failed to generate root certificate: %@", error);
            return nil;
        }
    }

    _fabricTable = fabricTable;
    _keystore = keystore;

    return self;
}

- (instancetype)initForExistingFabric:(FabricTable *)fabricTable
                          fabricIndex:(FabricIndex)fabricIndex
                             keystore:(chip::Crypto::OperationalKeystore *)keystore
                               params:(MTRDeviceControllerStartupParams *)params
{
    if (!(self = [self initWithParams:params])) {
        return nil;
    }

    const FabricInfo * fabric = fabricTable->FindFabricWithIndex(fabricIndex);

    if (self.vendorID == nil) {
        self.vendorID = @(fabric->GetVendorId());
    }

    BOOL usingExistingNOC = NO;
    if (self.operationalCertificate == nil && self.nodeID == nil) {
        self.nodeID = @(fabric->GetNodeId());

        if (self.operationalKeypair == nil) {
            uint8_t nocBuf[Credentials::kMaxCHIPCertLength];
            MutableByteSpan noc(nocBuf);
            CHIP_ERROR err = fabricTable->FetchNOCCert(fabric->GetFabricIndex(), noc);
            if (err != CHIP_NO_ERROR) {
                MTR_LOG_ERROR("Failed to get existing NOC: %s", ErrorStr(err));
                return nil;
            }
            self.operationalCertificate = MatterCertToX509Data(noc);
            if (self.operationalCertificate == nil) {
                MTR_LOG_ERROR("Failed to convert TLV NOC to DER X.509: %s", ErrorStr(err));
                return nil;
            }
            if (!keystore->HasOpKeypairForFabric(fabric->GetFabricIndex())) {
                MTR_LOG_ERROR("No existing operational key for fabric");
                return nil;
            }
        }

        usingExistingNOC = YES;
    }

    NSData * oldIntermediateCert = nil;
    {
        uint8_t icaBuf[Credentials::kMaxCHIPCertLength];
        MutableByteSpan icaCert(icaBuf);
        CHIP_ERROR err = fabricTable->FetchICACert(fabric->GetFabricIndex(), icaCert);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Failed to get existing intermediate certificate: %s", ErrorStr(err));
            return nil;
        }
        // There might not be an ICA cert for this fabric.
        if (!icaCert.empty()) {
            oldIntermediateCert = MatterCertToX509Data(icaCert);
            if (oldIntermediateCert == nil) {
                return nil;
            }
        }
    }

    if (self.nocSigner != nil && self.intermediateCertificate == nil && oldIntermediateCert != nil) {
        // It's possible that we are switching from using an ICA cert to not using
        // one.  We can detect this case by checking whether the provided nocSigner
        // matches the ICA cert.
        if ([MTRCertificates keypair:self.nocSigner matchesCertificate:oldIntermediateCert] == YES) {
            // Keep using the existing intermediate certificate.
            self.intermediateCertificate = oldIntermediateCert;
        }
        // else presumably the nocSigner matches the root (will be verified later)
        // and we are no longer using an intermediate.
    }

    // If we were planning to use our existing NOC from the fabric info but we are
    // changing from having an ICA to not having one, or changing from having one
    // to not having one, or changing the identity of our ICA, we need to generate
    // a new NOC.  But we can keep our existing operational keypair and node id;
    // nothing is forcing us to rotate those.
    if (usingExistingNOC == YES
        && ((oldIntermediateCert == nil) != (self.intermediateCertificate == nil)
            || ((oldIntermediateCert != nil) &&
                [MTRCertificates isCertificate:oldIntermediateCert equalTo:self.intermediateCertificate] == NO))) {
        self.operationalCertificate = nil;
    }

    NSData * oldRootCert;
    {
        uint8_t rootBuf[Credentials::kMaxCHIPCertLength];
        MutableByteSpan rootCert(rootBuf);
        CHIP_ERROR err = fabricTable->FetchRootCert(fabric->GetFabricIndex(), rootCert);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Failed to get existing root certificate: %s", ErrorStr(err));
            return nil;
        }
        oldRootCert = MatterCertToX509Data(rootCert);
        if (oldRootCert == nil) {
            return nil;
        }
    }

    if (self.rootCertificate == nil) {
        self.rootCertificate = oldRootCert;
    } else if ([MTRCertificates isCertificate:oldRootCert equalTo:self.rootCertificate] == NO) {
        MTR_LOG_ERROR("Root certificate identity does not match existing root certificate");
        return nil;
    }

    _fabricTable = fabricTable;
    _fabricIndex.Emplace(fabricIndex);
    _keystore = keystore;

    return self;
}

- (BOOL)keypairsMatchCertificates
{
    if (self.nocSigner != nil) {
        NSData * signingCert = self.intermediateCertificate;
        if (signingCert == nil) {
            signingCert = self.rootCertificate;
            if (signingCert == nil) {
                MTR_LOG_ERROR("No certificate to match nocSigner");
                return NO;
            }
        }

        if ([MTRCertificates keypair:self.nocSigner matchesCertificate:signingCert] == NO) {
            MTR_LOG_ERROR("Provided nocSigner does not match certificates");
            return NO;
        }
    }

    if (self.operationalCertificate != nil && self.operationalKeypair != nil) {
        if ([MTRCertificates keypair:self.operationalKeypair matchesCertificate:self.operationalCertificate] == NO) {
            MTR_LOG_ERROR("Provided operationalKeypair does not match operationalCertificate");
            return NO;
        }
    }

    return YES;
}

@end
