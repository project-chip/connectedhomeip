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

#import "MTROTAHeader.h"

#import "MTRError.h"
#import "MTRError_Internal.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <lib/core/OTAImageHeader.h>

@implementation MTROTAHeader
+ (nullable MTROTAHeader *)headerFromData:(NSData *)data error:(NSError * __autoreleasing *)error
{
    chip::OTAImageHeaderParser parser;

    parser.Init();

    if (!parser.IsInitialized()) {
        if (error != nil) {
            *error = [NSError errorWithDomain:MTRErrorDomain code:MTRErrorCodeGeneralError userInfo:nil];
        }
        return nil;
    }

    chip::ByteSpan buffer = AsByteSpan(data);
    chip::OTAImageHeader header;
    CHIP_ERROR err = parser.AccumulateAndDecode(buffer, header);
    if (err != CHIP_NO_ERROR) {
        if (error != nil) {
            *error = [MTRError errorForCHIPErrorCode:err];
        }
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

@implementation MTROTAHeaderParser

+ (nullable MTROTAHeader *)headerFromData:(NSData *)data error:(NSError * __autoreleasing *)error
{
    return [MTROTAHeader headerFromData:data error:error];
}

@end
