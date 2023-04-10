/**
 *
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

#import "MTRCSRInfo.h"
#import "MTRFramework.h"
#import "MTRLogging_Internal.h"
#import "NSDataSpanConversion.h"

#include <credentials/DeviceAttestationConstructor.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

static CHIP_ERROR ExtractCSRAndNonce(MTRTLVBytes csrElementsTLV, chip::ByteSpan & csr, chip::ByteSpan & nonce)
{
    // We don't care about vendor_reserved*.
    chip::ByteSpan vendor_reserved1, vendor_reserved2, vendor_reserved3;
    CHIP_ERROR err = chip::Credentials::DeconstructNOCSRElements(
        AsByteSpan(csrElementsTLV), csr, nonce, vendor_reserved1, vendor_reserved2, vendor_reserved3);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Failed to parse csrElementsTLV %@: %" CHIP_ERROR_FORMAT, csrElementsTLV, err.Format());
    }
    return err;
}

NS_ASSUME_NONNULL_BEGIN

@implementation MTROperationalCSRInfo : NSObject

+ (void)initialize
{
    // Needed for some of our init* methods.
    MTRFrameworkInit();
}

- (instancetype)initWithCSR:(MTRCSRDERBytes)csr
                   csrNonce:(NSData *)csrNonce
             csrElementsTLV:(MTRTLVBytes)csrElementsTLV
       attestationSignature:(NSData *)attestationSignature;
{
    if (self = [super init]) {
        _csr = csr;
        _csrNonce = csrNonce;
        _csrElementsTLV = csrElementsTLV;
        _attestationSignature = attestationSignature;
    }
    return self;
}

- (instancetype)initWithCSRNonce:(NSData *)csrNonce
                  csrElementsTLV:(MTRTLVBytes)csrElementsTLV
            attestationSignature:(NSData *)attestationSignature
{
    chip::ByteSpan csr;
    {
        // We don't care about the nonce.
        chip::ByteSpan ignoredNonce;
        VerifyOrReturnValue(ExtracCSRAndNonce(csrElementsTLV, csr, ignoredNonce) == CHIP_NO_ERROR, nil);
    }

    return [self initWithCSR:AsData(csr) csrNonce:csrNonce csrElementsTLV:csrElementsTLV attestationSignature:attestationSignature];
}

- (instancetype)initWithCSRElementsTLV:(MTRTLVBytes)csrElementsTLV attestationSignature:(NSData *)attestationSignature
{
    chip::ByteSpan csr, csrNonce;
    VerifyOrReturnValue(ExtracCSRAndNonce(csrElementsTLV, csr, csrNonce) == CHIP_NO_ERROR, nil);

    return [self initWithCSR:AsData(csr)
                    csrNonce:AsData(csrNonce)
              csrElementsTLV:csrElementsTLV
        attestationSignature:attestationSignature];
}

- (instancetype)initWithCSRResponseParams:(MTROperationalCredentialsClusterCSRResponseParams *)responseParams
{
    return [self initWithCSRElementsTLV:responseParams.nocsrElements attestationSignature:responseParams.attestationSignature];
}
@end

@implementation CSRInfo : NSObject

- (instancetype)initWithNonce:(NSData *)nonce
                     elements:(NSData *)elements
            elementsSignature:(NSData *)elementsSignature
                          csr:(NSData *)csr
{
    if (self = [super init]) {
        _nonce = nonce;
        _elements = elements;
        _elementsSignature = elementsSignature;
        _csr = csr;
    }
    return self;
}
@end

NS_ASSUME_NONNULL_END
