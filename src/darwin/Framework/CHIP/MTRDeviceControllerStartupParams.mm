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
#import "MTRConversion.h"
#import "MTRDeviceControllerStartupParams_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRP256KeypairBridge.h"
#import "NSDataSpanConversion.h"

#import <Matter/MTRDeviceControllerStorageDelegate.h>

#include <controller/OperationalCredentialsDelegate.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>
#include <lib/core/PeerId.h>

using namespace chip;

static CHIP_ERROR ExtractNodeIDFabricIDFromNOC(
    MTRCertificateDERBytes noc, NSNumber * __autoreleasing * nodeID, NSNumber * __autoreleasing * fabricID)
{
    // ExtractNodeIdFabricIdFromOpCert needs a TLV-encoded opcert, not a DER-encoded one.
    auto * tlvNOC = [MTRCertificates convertX509Certificate:noc];
    if (tlvNOC == nil) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ByteSpan nocSpan = AsByteSpan(tlvNOC);

    FabricId certFabricID = kUndefinedFabricId;
    NodeId certNodeID = kUndefinedNodeId;
    CHIP_ERROR err = Credentials::ExtractNodeIdFabricIdFromOpCert(nocSpan, &certNodeID, &certFabricID);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Unable to extract node ID and fabric ID from operational certificate: %s", err.AsString());
        return err;
    }
    *nodeID = @(certNodeID);
    *fabricID = @(certFabricID);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ExtractFabricIDFromNOC(MTRCertificateDERBytes noc, NSNumber * __autoreleasing * fabricID)
{
    NSNumber * ignored;
    return ExtractNodeIDFabricIDFromNOC(noc, &ignored, fabricID);
}

static CHIP_ERROR ExtractNodeIDFromNOC(MTRCertificateDERBytes noc, NSNumber * __autoreleasing * nodeID)
{
    NSNumber * ignored;
    return ExtractNodeIDFabricIDFromNOC(noc, nodeID, &ignored);
}

@implementation MTRDeviceControllerStartupParams

- (instancetype)initWithIPK:(NSData *)ipk fabricID:(NSNumber *)fabricID nocSigner:(id<MTRKeypair>)nocSigner
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
    _uniqueIdentifier = [NSUUID UUID];

    return self;
}

- (instancetype)initWithIPK:(NSData *)ipk
         operationalKeypair:(id<MTRKeypair>)operationalKeypair
     operationalCertificate:(MTRCertificateDERBytes)operationalCertificate
    intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
            rootCertificate:(MTRCertificateDERBytes)rootCertificate
{
    if (!(self = [super init])) {
        return nil;
    }

    { // Scope for temporary
        NSNumber * fabricID;
        CHIP_ERROR err = ExtractFabricIDFromNOC(operationalCertificate, &fabricID);
        if (err != CHIP_NO_ERROR) {
            return nil;
        }
        _fabricID = fabricID;
    }

    _operationalKeypair = operationalKeypair;
    _operationalCertificate = [operationalCertificate copy];
    _intermediateCertificate = [intermediateCertificate copy];
    _rootCertificate = [rootCertificate copy];
    _ipk = [ipk copy];
    _uniqueIdentifier = [NSUUID UUID];

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
    _caseAuthenticatedTags = params.caseAuthenticatedTags;
    _rootCertificate = params.rootCertificate;
    _intermediateCertificate = params.intermediateCertificate;
    _operationalCertificate = params.operationalCertificate;
    _operationalKeypair = params.operationalKeypair;
    _operationalCertificateIssuer = params.operationalCertificateIssuer;
    _operationalCertificateIssuerQueue = params.operationalCertificateIssuerQueue;
    _uniqueIdentifier = params.uniqueIdentifier;

    return self;
}

- (instancetype)initWithParameters:(MTRDeviceControllerParameters *)params error:(CHIP_ERROR &)error
{
    if (!(self = [super init])) {
        error = CHIP_ERROR_INCORRECT_STATE;
        return nil;
    }

    if (![params isKindOfClass:[MTRDeviceControllerExternalCertificateParameters class]]) {
        MTR_LOG_ERROR("Unexpected subclass of MTRDeviceControllerParameters");
        error = CHIP_ERROR_INVALID_ARGUMENT;
        return nil;
    }

    _nocSigner = nil;

    NSNumber * fabricID;
    error = ExtractFabricIDFromNOC(params.operationalCertificate, &fabricID);
    if (error != CHIP_NO_ERROR) {
        return nil;
    }
    _fabricID = fabricID;

    _ipk = params.ipk;
    _vendorID = params.vendorID;
    // Note: Since we have an operationalCertificate, we do not need a nodeID as
    // part of our params; it will not be used.  Don't even initialize it, to
    // avoid confusion about that.
    //
    // We don't really use the fabricID for anything either, but we promise to
    // have a non-nil one, which is why we set it above.
    _nodeID = nil;
    _caseAuthenticatedTags = nil;
    _rootCertificate = params.rootCertificate;
    _intermediateCertificate = params.intermediateCertificate;
    _operationalCertificate = params.operationalCertificate;
    _operationalKeypair = params.operationalKeypair;
    _operationalCertificateIssuer = params.operationalCertificateIssuer;
    _operationalCertificateIssuerQueue = params.operationalCertificateIssuerQueue;
    _uniqueIdentifier = params.uniqueIdentifier;

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
        MTR_LOG_ERROR("Failed to convert Matter certificate to X.509 DER: %s", ErrorStr(err));
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
    return [self initWithIPK:ipk fabricID:@(fabricId) nocSigner:nocSigner];
}

- (instancetype)initWithOperationalKeypair:(id<MTRKeypair>)operationalKeypair
                    operationalCertificate:(MTRCertificateDERBytes)operationalCertificate
                   intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
                           rootCertificate:(MTRCertificateDERBytes)rootCertificate
                                       ipk:(NSData *)ipk
{
    return [self initWithIPK:ipk
             operationalKeypair:operationalKeypair
         operationalCertificate:operationalCertificate
        intermediateCertificate:intermediateCertificate
                rootCertificate:rootCertificate];
}

@end

@implementation MTRDeviceControllerAbstractParameters
- (instancetype)_initInternal
{
    return [super init];
}
@end

constexpr NSUInteger kDefaultConcurrentSubscriptionPoolSize = 300;

@implementation MTRDeviceControllerParameters
- (instancetype)initWithStorageDelegate:(id<MTRDeviceControllerStorageDelegate>)storageDelegate
                   storageDelegateQueue:(dispatch_queue_t)storageDelegateQueue
                       uniqueIdentifier:(NSUUID *)uniqueIdentifier
                                    ipk:(NSData *)ipk
                               vendorID:(NSNumber *)vendorID
                     operationalKeypair:(id<MTRKeypair>)operationalKeypair
                 operationalCertificate:(MTRCertificateDERBytes)operationalCertificate
                intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
                        rootCertificate:(MTRCertificateDERBytes)rootCertificate
{
    if (!(self = [super _initInternal])) {
        return nil;
    }

    _productAttestationAuthorityCertificates = nil;
    _certificationDeclarationCertificates = nil;
    _shouldAdvertiseOperational = NO;

    _ipk = ipk;
    _vendorID = vendorID;
    _rootCertificate = rootCertificate;
    _intermediateCertificate = intermediateCertificate;
    _operationalCertificate = operationalCertificate;
    _operationalKeypair = operationalKeypair;

    _operationalCertificateIssuer = nil;
    _operationalCertificateIssuerQueue = nil;
    _storageDelegate = storageDelegate;
    _storageDelegateQueue = storageDelegateQueue;
    _uniqueIdentifier = uniqueIdentifier;

    _concurrentSubscriptionEstablishmentsAllowedOnThread = kDefaultConcurrentSubscriptionPoolSize;

    return self;
}

- (void)setOperationalCertificateIssuer:(id<MTROperationalCertificateIssuer>)operationalCertificateIssuer
                                  queue:(dispatch_queue_t)queue
{
    _operationalCertificateIssuer = operationalCertificateIssuer;
    _operationalCertificateIssuerQueue = queue;
}

- (void)setOTAProviderDelegate:(id<MTROTAProviderDelegate>)otaProviderDelegate queue:(dispatch_queue_t)queue
{
    _otaProviderDelegate = otaProviderDelegate;
    _otaProviderDelegateQueue = queue;
}

@end

@implementation MTRDeviceControllerExternalCertificateParameters
- (instancetype)initWithStorageDelegate:(id<MTRDeviceControllerStorageDelegate>)storageDelegate
                   storageDelegateQueue:(dispatch_queue_t)storageDelegateQueue
                       uniqueIdentifier:(NSUUID *)uniqueIdentifier
                                    ipk:(NSData *)ipk
                               vendorID:(NSNumber *)vendorID
                     operationalKeypair:(id<MTRKeypair>)operationalKeypair
                 operationalCertificate:(MTRCertificateDERBytes)operationalCertificate
                intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
                        rootCertificate:(MTRCertificateDERBytes)rootCertificate
{
    return [super initWithStorageDelegate:storageDelegate
                     storageDelegateQueue:storageDelegateQueue
                         uniqueIdentifier:uniqueIdentifier
                                      ipk:ipk
                                 vendorID:vendorID
                       operationalKeypair:operationalKeypair
                   operationalCertificate:operationalCertificate
                  intermediateCertificate:intermediateCertificate
                          rootCertificate:rootCertificate];
}
@end

@implementation MTRDeviceControllerStartupParamsInternal

- (instancetype)initWithParams:(MTRDeviceControllerStartupParams *)params
{
    if (!(self = [super initWithParams:params])) {
        return nil;
    }

    _storageDelegate = nil;
    _storageDelegateQueue = nil;

    if (self.nocSigner == nil && self.rootCertificate == nil) {
        MTR_LOG_ERROR("nocSigner and rootCertificate are both nil; no public key available to identify the fabric");
        return nil;
    }

    if (self.operationalCertificate != nil && self.nodeID != nil) {
        MTR_LOG_ERROR("nodeID must be nil if operationalCertificate is not nil");
        return nil;
    }

    if (self.caseAuthenticatedTags != nil && self.nodeID == nil) {
        MTR_LOG_ERROR("caseAuthenticatedTags must be nil if nodeID is nil");
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
            advertiseOperational:(BOOL)advertiseOperational
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
    _advertiseOperational = advertiseOperational;
    _allowMultipleControllersPerFabric = NO;

    return self;
}

- (instancetype)initForExistingFabric:(FabricTable *)fabricTable
                          fabricIndex:(FabricIndex)fabricIndex
                             keystore:(chip::Crypto::OperationalKeystore *)keystore
                 advertiseOperational:(BOOL)advertiseOperational
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

        // Make sure to preserve caseAuthenticatedTags from the existing certificate.
        uint8_t nocBuf[Credentials::kMaxCHIPCertLength];
        MutableByteSpan noc(nocBuf);
        CHIP_ERROR err = fabricTable->FetchNOCCert(fabric->GetFabricIndex(), noc);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Failed to get existing NOC: %s", ErrorStr(err));
            return nil;
        }

        if (self.operationalKeypair == nil) {
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

        CATValues cats;
        err = Credentials::ExtractCATsFromOpCert(noc, cats);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Failed to extract existing CATs: %s", ErrorStr(err));
            return nil;
        }

        auto tagCount = cats.GetNumTagsPresent();
        if (tagCount > 0) {
            self.caseAuthenticatedTags = CATValuesToSet(cats);
        } else {
            self.caseAuthenticatedTags = nil;
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
                [MTRCertificates isCertificate:oldIntermediateCert
                                       equalTo:self.intermediateCertificate]
                    == NO))) {
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
    _advertiseOperational = advertiseOperational;
    _allowMultipleControllersPerFabric = NO;

    return self;
}

- (instancetype)initForNewController:(MTRDeviceController *)controller
                         fabricTable:(chip::FabricTable *)fabricTable
                            keystore:(chip::Crypto::OperationalKeystore *)keystore
                advertiseOperational:(BOOL)advertiseOperational
                              params:(MTRDeviceControllerParameters *)params
                               error:(CHIP_ERROR &)error
{
    if (!(self = [super initWithParameters:params error:error])) {
        return nil;
    }

    Crypto::P256PublicKey pubKey;
    error = ExtractPubkeyFromX509Cert(AsByteSpan(self.rootCertificate), pubKey);
    if (error != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Can't extract public key from root certificate: %s", error.AsString());
        return nil;
    }

    NSNumber * nodeID;
    error = ExtractNodeIDFromNOC(self.operationalCertificate, &nodeID);
    if (error != CHIP_NO_ERROR) {
        // Already logged.
        return nil;
    }

    if (fabricTable->FindIdentity(pubKey, self.fabricID.unsignedLongLongValue, nodeID.unsignedLongLongValue)) {
        MTR_LOG_ERROR("Trying to start a controller identity that is already running");
        error = CHIP_ERROR_INVALID_ARGUMENT;
        return nil;
    }

    auto * oldNOCTLV = [controller.controllerDataStore fetchLastLocallyUsedNOC];
    if (oldNOCTLV != nil) {
        ByteSpan oldNOCSpan = AsByteSpan(oldNOCTLV);

        FabricId ignored = kUndefinedFabricId;
        NodeId oldNodeID = kUndefinedNodeId;
        CHIP_ERROR err = Credentials::ExtractNodeIdFabricIdFromOpCert(oldNOCSpan, &oldNodeID, &ignored);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Unable to extract node ID and fabric ID from old operational certificate: %s", err.AsString());
            return nil;
        }

        CATValues oldCATs;
        err = Credentials::ExtractCATsFromOpCert(oldNOCSpan, oldCATs);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Failed to extract CATs from old operational certificate: %s", err.AsString());
            return nil;
        }

        auto * tlvNOC = [MTRCertificates convertX509Certificate:self.operationalCertificate];
        if (tlvNOC == nil) {
            return nil;
        }

        ByteSpan nocSpan = AsByteSpan(tlvNOC);
        CATValues newCATs;
        err = Credentials::ExtractCATsFromOpCert(nocSpan, newCATs);
        if (err != CHIP_NO_ERROR) {
            MTR_LOG_ERROR("Failed to extract CATs from new operational certificate: %s", err.AsString());
            return nil;
        }

        if (nodeID.unsignedLongLongValue != oldNodeID || oldCATs != newCATs) {
            // Our NOC has changed in a way that would affect ACL checks.  Clear
            // out our session resumption storage, because resuming those CASE
            // sessions will end up doing ACL checks against our old NOC.
            MTR_LOG("Node ID or CATs changed.  Clearing CASE resumption storage.");
            [controller.controllerDataStore clearAllResumptionInfo];
        }
    }

    _fabricTable = fabricTable;
    _keystore = keystore;
    _advertiseOperational = advertiseOperational;
    _allowMultipleControllersPerFabric = YES;
    _storageDelegate = params.storageDelegate;
    _storageDelegateQueue = params.storageDelegateQueue;
    _productAttestationAuthorityCertificates = params.productAttestationAuthorityCertificates;
    _certificationDeclarationCertificates = params.certificationDeclarationCertificates;

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
