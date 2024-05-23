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

#import "MCCryptoUtils.h"
#import "MCErrorUtils.h"

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

#include <Security/Security.h>

@implementation MCCryptoUtils

+ (MatterError *)ecdsaAsn1SignatureToRawWithFeLengthBytes:(NSUInteger)feLengthBytes asn1Signature:(CFDataRef)asn1Signature outRawSignature:(NSData **)outRawSignature
{
    // convert asn1Signature from CFDataRef to MutableByteSpan (asn1SignatureByteSpan)
    uint8_t asn1SignatureBytes[256];
    chip::MutableByteSpan asn1SignatureByteSpan = chip::MutableByteSpan(asn1SignatureBytes, sizeof(asn1SignatureBytes));
    size_t signatureLen = CFDataGetLength(asn1Signature);
    CFDataGetBytes(asn1Signature, CFRangeMake(0, signatureLen), asn1SignatureByteSpan.data());
    asn1SignatureByteSpan.reduce_size(signatureLen);

    // get a rawSignatureMutableByteSpan to pass to chip::Crypto::EcdsaAsn1SignatureToRaw
    uint8_t * rawSignatureBytes = new uint8_t[(*outRawSignature).length];
    chip::MutableByteSpan rawSignatureMutableByteSpan = chip::MutableByteSpan(rawSignatureBytes, (*outRawSignature).length);

    // convert ASN.1 DER signature to SEC1 raw format
    CHIP_ERROR err = chip::Crypto::EcdsaAsn1SignatureToRaw(feLengthBytes, chip::ByteSpan(asn1SignatureByteSpan.data(), asn1SignatureByteSpan.size()), rawSignatureMutableByteSpan);
    if (err != CHIP_NO_ERROR) {
        ChipLogError(AppServer, "chip::Crypto::EcdsaAsn1SignatureToRaw() failed");
        return [MCErrorUtils MatterErrorFromChipError:err];
    }

    // copy from rawSignatureMutableByteSpan into *outRawSignature
    *outRawSignature = [NSData dataWithBytes:rawSignatureMutableByteSpan.data() length:rawSignatureMutableByteSpan.size()];
    return MATTER_NO_ERROR;
}

@end
