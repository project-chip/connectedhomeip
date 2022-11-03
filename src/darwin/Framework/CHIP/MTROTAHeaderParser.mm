/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTROTAHeaderParser.h"

#import "MTRError.h"
#import "MTRError_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <lib/core/OTAImageHeader.h>

@implementation MTROTAHeader
@end

@implementation MTROTAHeaderParser
+ (nullable MTROTAHeader *)headerFromData:(NSData *)data error:(NSError * __autoreleasing *)error
{
    chip::OTAImageHeaderParser parser;

    parser.Init();

    if (!parser.IsInitialized()) {
        *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil];
        return nil;
    }

    chip::ByteSpan buffer = AsByteSpan(data);
    chip::OTAImageHeader header;
    CHIP_ERROR err = parser.AccumulateAndDecode(buffer, header);
    if (err != CHIP_NO_ERROR) {
        *error = [MTRError errorForCHIPErrorCode:err];
        parser.Clear();
        return nil;
    }

    auto headerObj = [MTROTAHeader new];
    headerObj.vendorID = @(header.mVendorId);
    headerObj.productID = @(header.mProductId);
    headerObj.payloadSize = @(header.mPayloadSize);
    headerObj.softwareVersion = @(header.mSoftwareVersion);
    headerObj.softwareVersionString = AsString(header.mSoftwareVersionString);
    headerObj.releaseNotesURL = AsString(header.mReleaseNotesURL);
    headerObj.imageDigest = AsData(header.mImageDigest);
    headerObj.imageDigestType = static_cast<MTROTAImageDigestType>(chip::to_underlying(header.mImageDigestType));

    if (header.mMinApplicableVersion.HasValue()) {
        headerObj.minApplicableVersion = @(header.mMinApplicableVersion.Value());
    }

    if (header.mMaxApplicableVersion.HasValue()) {
        headerObj.maxApplicableVersion = @(header.mMaxApplicableVersion.Value());
    }

    parser.Clear();
    return headerObj;
}
@end
