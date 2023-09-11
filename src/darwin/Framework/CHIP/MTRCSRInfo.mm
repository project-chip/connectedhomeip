/**
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

- (instancetype)_initWithValidatedCSR:(MTRCSRDERBytes)csr
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

- (instancetype)initWithCSR:(MTRCSRDERBytes)csr
                   csrNonce:(NSData *)csrNonce
             csrElementsTLV:(MTRTLVBytes)csrElementsTLV
       attestationSignature:(NSData *)attestationSignature;
{
    return [self _initWithValidatedCSR:csr
                              csrNonce:csrNonce
                        csrElementsTLV:csrElementsTLV
                  attestationSignature:attestationSignature];
}

- (nullable instancetype)initWithCSRNonce:(NSData *)csrNonce
                           csrElementsTLV:(MTRTLVBytes)csrElementsTLV
                     attestationSignature:(NSData *)attestationSignature
{
    chip::ByteSpan csr, extractedNonce;
    VerifyOrReturnValue(ExtractCSRAndNonce(csrElementsTLV, csr, extractedNonce) == CHIP_NO_ERROR, nil);

    if (!extractedNonce.data_equal(AsByteSpan(csrNonce))) {
        MTR_LOG_ERROR("Provided CSR nonce does not match provided csrElementsTLV");
        return nil;
    }

    return [self _initWithValidatedCSR:AsData(csr)
                              csrNonce:csrNonce
                        csrElementsTLV:csrElementsTLV
                  attestationSignature:attestationSignature];
}

- (nullable instancetype)initWithCSRElementsTLV:(MTRTLVBytes)csrElementsTLV attestationSignature:(NSData *)attestationSignature
{
    chip::ByteSpan csr, csrNonce;
    VerifyOrReturnValue(ExtractCSRAndNonce(csrElementsTLV, csr, csrNonce) == CHIP_NO_ERROR, nil);

    return [self _initWithValidatedCSR:AsData(csr)
                              csrNonce:AsData(csrNonce)
                        csrElementsTLV:csrElementsTLV
                  attestationSignature:attestationSignature];
}

- (nullable instancetype)initWithCSRResponseParams:(MTROperationalCredentialsClusterCSRResponseParams *)responseParams
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
