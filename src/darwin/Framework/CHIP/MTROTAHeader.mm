/**
 *
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTROTAHeader.h"

#import "MTRError.h"
#import "MTRError_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <lib/core/OTAImageHeader.h>

@implementation MTROTAHeader
- (instancetype)initWithData:(NSData *)data
{
    if (!(self = [super init])) {
        return nil;
    }

    chip::OTAImageHeaderParser parser;

    parser.Init();

    if (!parser.IsInitialized()) {
        return nil;
    }

    chip::ByteSpan buffer = AsByteSpan(data);
    chip::OTAImageHeader header;
    CHIP_ERROR err = parser.AccumulateAndDecode(buffer, header);
    if (err != CHIP_NO_ERROR) {
        parser.Clear();
        return nil;
    }

    _vendorID = @(header.mVendorId);
    _productID = @(header.mProductId);
    _payloadSize = @(header.mPayloadSize);
    _softwareVersion = @(header.mSoftwareVersion);
    _softwareVersionString = AsString(header.mSoftwareVersionString);
    _releaseNotesURL = AsString(header.mReleaseNotesURL);
    _imageDigest = AsData(header.mImageDigest);
    _imageDigestType = static_cast<MTROTAImageDigestType>(chip::to_underlying(header.mImageDigestType));

    if (header.mMinApplicableVersion.HasValue()) {
        _minApplicableVersion = @(header.mMinApplicableVersion.Value());
    } else {
        _minApplicableVersion = nil;
    }

    if (header.mMaxApplicableVersion.HasValue()) {
        _maxApplicableVersion = @(header.mMaxApplicableVersion.Value());
    } else {
        _maxApplicableVersion = nil;
    }

    parser.Clear();
    return self;
}
@end

@implementation MTROTAHeaderParser

+ (nullable MTROTAHeader *)headerFromData:(NSData *)data error:(NSError * __autoreleasing *)error
{
    auto * header = [[MTROTAHeader alloc] initWithData:data];
    if (header == nil && error != nil) {
        *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
    }
    return header;
}

@end
