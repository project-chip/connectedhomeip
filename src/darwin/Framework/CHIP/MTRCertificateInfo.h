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
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDistinguishedNameInfo;

/**
 * Exposes Matter-specific information from an operational X.509 certificate.
 *
 * Note: This class does not support parsing certificates related to Device Attestation.
 */
NS_SWIFT_SENDABLE
MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRCertificateInfo : NSObject <NSCopying>

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

/**
 * Initializes the receiver with an operational certificate in Matter TLV format.
 *
 * This can be a node operational certificate, a Matter intermediate
 * certificate, or a Matter root certificate.
 */
- (nullable instancetype)initWithTLVBytes:(MTRCertificateTLVBytes)bytes;

/**
 * The Distinguished Name of the issuer of the certificate.
 *
 * For a node operational certificate, the issuer will match the subject of the
 * root certificate or intermediate certificate that represents the entity that
 * issued the node operational certificate.
 *
 * For an intermediate certificate, the issuer will match the subject of the
 * root certificate.
 *
 * Matter root certificates are self-signed, i.e. the issuer and the subject are
 * the same.
 */
@property (readonly) MTRDistinguishedNameInfo * issuer;

/**
 * The Distinguished Name of the entity represented by the certificate.
 */
@property (readonly) MTRDistinguishedNameInfo * subject;

@property (readonly) NSDate * notBefore;
@property (readonly) NSDate * notAfter;

@end

/**
 * Represents the Matter-specific components of an X.509 Distinguished Name.
 */
NS_SWIFT_SENDABLE
MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4))
@interface MTRDistinguishedNameInfo : NSObject <NSCopying>

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

/**
 * The Node ID contained in the DN, if any.  Will be non-nil for the subject of
 * a valid node operational certificate.
 */
@property (readonly, nullable) NSNumber * nodeID;

/**
 * The Fabric ID contained in the DN, if any.  Will be non-nil for the subject
 * of a valid node operational certificate, and may be non-nil for the subject
 * of a valid intermediate or root certificate.
 */
@property (readonly, nullable) NSNumber * fabricID;

/**
 * The `RCAC` ID contained in the DN, if any.  Will be non-nil for the subject
 * of a valid root certificate.
 */
@property (readonly, nullable) NSNumber * rootCACertificateID;

/**
 * The `ICAC` ID contained in the DN, if any.  Will be non-nil for the subject
 * of a valid intermediate certificate.
 */
@property (readonly, nullable) NSNumber * intermediateCACertificateID;

/**
 * The set of CASE Authenticated Tags contained in the DN.  Maybe be non-empty for
 * the subject of a valid node operational certificate.
 */
@property (readonly) NSSet<NSNumber *> * caseAuthenticatedTags;

@end

NS_ASSUME_NONNULL_END
