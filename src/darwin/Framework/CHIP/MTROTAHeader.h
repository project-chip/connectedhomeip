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

#import <Foundation/Foundation.h>

/**
 * A representation of an OTA image header as defined in the Matter
 * specification's "Over-the-Air (OTA) Software Update File Format" section.
 */

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, MTROTAImageDigestType) {
    MTROTAImageDigestTypeSha256 = 1,
    MTROTAImageDigestTypeSha256_128,
    MTROTAImageDigestTypeSha256_120,
    MTROTAImageDigestTypeSha256_96,
    MTROTAImageDigestTypeSha256_64,
    MTROTAImageDigestTypeSha256_32,
    MTROTAImageDigestTypeSha384,
    MTROTAImageDigestTypeSha512,
    MTROTAImageDigestTypeSha3_224,
    MTROTAImageDigestTypeSha3_256,
    MTROTAImageDigestTypeSha3_384,
    MTROTAImageDigestTypeSha3_512,
};

@interface MTROTAHeader : NSObject

/**
 * The identifier of the vendor whose product this image is meant for.
 *
 * This field can be compared to the vendor id received in the Query Image
 * command to determine whether an image matches.
 *
 * This field may be 0, in which case the image might apply to products from
 * more than one vendor.  If it's nonzero, it must match the vendor id in Query
 * Image for this image to be considered.
 */
@property (nonatomic, copy) NSNumber * vendorID;
/**
 * The identifier of the specific product the image is meant for.  May be 0, if
 * the image might apply to more than one product.  This is allowed, but not
 * required, to be matched against the product id received in Query Image.
 */
@property (nonatomic, copy) NSNumber * productID;
/**
 * The size of the actual image payload, which follows the header in the OTA
 * file.
 */
@property (nonatomic, copy) NSNumber * payloadSize;
/**
 * The version of the software contained in this image.  This is the version the
 * OTA requestor will be updated to if this image is installed.  This can be
 * used to determine whether this image is newer than what the requestor is
 * currently running, by comparing it to the SoftwareVersion in the Query Image
 * command.
 */
@property (nonatomic, copy) NSNumber * softwareVersion;
/**
 * Human-readable version of softwareVersion.  This must not be used for
 * deciding which versions are newer or older; use softwareVersion for that.
 */
@property (nonatomic, copy) NSString * softwareVersionString;
/**
 * If not nil a URL pointing to release notes for the software update
 * represented by the image.
 */
@property (nonatomic, copy, nullable) NSString * releaseNotesURL;
/**
 * A digest of the payload that follows the header.  Can be used to verify that
 * the payload is not truncated or corrupted.
 */
@property (nonatomic, copy) NSData * imageDigest;
/**
 * The specific algorithm that was used to compute imageDigest.
 */
@property (nonatomic, assign) MTROTAImageDigestType imageDigestType;
/**
 * If not nil, specifies the smallest software version that this update can be
 * applied on top of.  In that case, this value must be compared to the
 * SoftwareVersion in the QueryImage command to check whether this image is
 * valid for the OTA requestor.
 */
@property (nonatomic, copy, nullable) NSNumber * minApplicableVersion;
/**
 * If not nil, specifies the largest software version that this update can be
 * applied on top of.  In that case, this value must be compared to the
 * SoftwareVersion in the QueryImage command to check whether this image is
 * valid for the OTA requestor.
 */
@property (nonatomic, copy, nullable) NSNumber * maxApplicableVersion;

+ (nullable MTROTAHeader *)headerFromData:(NSData *)data error:(NSError * __autoreleasing *)error MTR_NEWLY_AVAILABLE;
@end

MTR_NEWLY_DEPRECATED("Please use [MTROTAHeader headerFromData]")
@interface MTROTAHeaderParser : NSObject
+ (nullable MTROTAHeader *)headerFromData:(NSData *)data error:(NSError * __autoreleasing *)error;
@end

NS_ASSUME_NONNULL_END
