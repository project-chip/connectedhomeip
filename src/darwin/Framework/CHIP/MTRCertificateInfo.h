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

#import <Foundation/Foundation.h>
#import <Matter/MTRCertificates.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDistinguishedNameInfo;

/**
 * Exposes Matter-specific information from an operational X.509 certificate.
 *
 * Note: This class does not support parsing certificates related to Device Attestation.
 */
NS_SWIFT_SENDABLE
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRCertificateInfo : NSObject <NSCopying>

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

/**
 * Initializes the receiver with an operational certificate in Matter TLV format.
 */
- (nullable instancetype)initWithTLVBytes:(MTRCertificateTLVBytes)bytes;

@property (readonly) MTRDistinguishedNameInfo * issuer;
@property (readonly) MTRDistinguishedNameInfo * subject;

@property (readonly) NSDate * notBefore;
@property (readonly) NSDate * notAfter;

@end

/**
 * Represents the Matter-specific components of an X.509 Distinguished Name.
 */
NS_SWIFT_SENDABLE
API_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRDistinguishedNameInfo : NSObject <NSCopying>

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

/**
 * The Node ID contained in the DN, if any.
 */
@property (readonly, nullable) NSNumber * nodeID;

/**
 * The Fabric ID contained in the DN, if any.
 */
@property (readonly, nullable) NSNumber * fabricID;

/**
 * The `RCAC` ID contained in the DN, if any.
 */
@property (readonly, nullable) NSNumber * rootCACertificateID;

/**
 * The `ICAC` ID contained in the DN, if any.
 */
@property (readonly, nullable) NSNumber * intermediateCACertificateID;

/**
 * The set of CASE Authenticated Tags contained in the DN.
 */
@property (readonly) NSSet<NSNumber *> * caseAuthenticatedTags;

@end

NS_ASSUME_NONNULL_END
