/**
 *    Copyright (c) 2023 Project CHIP Authors
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

#import "MTRFabricInfo_Internal.h"
#import "MTRLogging_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"
#import <Matter/MTRCertificates.h>

using namespace chip;

typedef CHIP_ERROR (FabricTable::*CertGetter)(FabricIndex, MutableByteSpan &) const;

static bool FetchCerts(MTRCertificateDERBytes __strong & cert, MTRCertificateTLVBytes __strong & tlvCert,
    const FabricTable & fabricTable, uint8_t fabricIndex, CertGetter getter, const char * certType)
{
    uint8_t certBuffer[Credentials::kMaxCHIPCertLength];
    MutableByteSpan certSpan(certBuffer);
    CHIP_ERROR err = (fabricTable.*getter)(fabricIndex, certSpan);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Failed to fetch %s certificate for fabric index %x: %s", certType, fabricIndex, err.AsString());
        return false;
    }

    if (getter == &FabricTable::FetchICACert && certSpan.size() == 0) {
        cert = nil;
        tlvCert = nil;
        return true;
    }

    tlvCert = AsData(certSpan);
    cert = [MTRCertificates convertMatterCertificate:tlvCert];
    if (cert == nil) {
        MTR_LOG_ERROR("Failed to convert %s certificate to x509 encoding", certType);
        return false;
    }

    return true;
}

@implementation MTRFabricInfo
- (instancetype)initWithFabricTable:(const FabricTable &)fabricTable fabricInfo:(const FabricInfo &)fabricInfo
{
    if (!(self = [super init])) {
        return nil;
    }

    Crypto::P256PublicKey publicKey;
    CHIP_ERROR err = fabricInfo.FetchRootPubkey(publicKey);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Failed to fetch root public key for fabric index %x: %s", fabricInfo.GetFabricIndex(), err.AsString());
        return nil;
    }

    _rootPublicKey = AsData(ByteSpan(publicKey.ConstBytes(), publicKey.Length()));

    _vendorID = @(fabricInfo.GetVendorId());
    _fabricID = @(fabricInfo.GetFabricId());
    _nodeID = @(fabricInfo.GetNodeId());
    _label = AsString(fabricInfo.GetFabricLabel());

    if (!FetchCerts(
            _rootCertificate, _rootCertificateTLV, fabricTable, fabricInfo.GetFabricIndex(), &FabricTable::FetchRootCert, "root")) {
        return nil;
    }
    if (!FetchCerts(_intermediateCertificate, _intermediateCertificateTLV, fabricTable, fabricInfo.GetFabricIndex(),
            &FabricTable::FetchICACert, "intermediate")) {
        return nil;
    }
    if (!FetchCerts(_operationalCertificate, _operationalCertificateTLV, fabricTable, fabricInfo.GetFabricIndex(),
            &FabricTable::FetchNOCCert, "operational")) {
        return nil;
    }

    _fabricIndex = @(fabricInfo.GetFabricIndex());

    return self;
}

@end
