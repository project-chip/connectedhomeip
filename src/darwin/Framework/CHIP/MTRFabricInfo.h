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

/**
 * An object that represents a fabric that the MTRDeviceControllerFactory knows
 * about (and hence one that could have a controller created for it via
 * createControllerOnExistingFabric).
 */

#import <Foundation/Foundation.h>
#import <Matter/MTRDefines.h>

NS_ASSUME_NONNULL_BEGIN

MTR_NEWLY_AVAILABLE
@interface MTRFabricInfo : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Root public key for the fabric.
 */
@property (readonly, nonatomic) NSData * rootPublicKey MTR_NEWLY_AVAILABLE;

/**
 * Vendor identifier for the fabric.
 */
@property (readonly, nonatomic) NSNumber * vendorID MTR_NEWLY_AVAILABLE;

/**
 * Fabric identifier (scoped to the root public key) for the fabric.
 */
@property (readonly, nonatomic) NSNumber * fabricID MTR_NEWLY_AVAILABLE;

/**
 * Node identifier for the given node on the fabric.
 */
@property (readonly, nonatomic) NSNumber * nodeID MTR_NEWLY_AVAILABLE;

/**
 * The string label for the fabric.  May be empty.
 */
@property (readonly, nonatomic) NSString * label MTR_NEWLY_AVAILABLE;

/**
 * The root certificate for the fabric.  This might be nil if a root
 * certificate is not available (e.g. if this is information about some remote
 * node that we don't have root certificate information for).
 */
@property (readonly, nonatomic, nullable) MTRCertificateDERBytes rootCertificate MTR_NEWLY_AVAILABLE;

/**
 * The same root certificate as rootCertificate, in Matter TLV format.
 */
@property (readonly, nonatomic, nullable) MTRCertificateTLVBytes rootCertificateTLV MTR_NEWLY_AVAILABLE;

/**
 * The intermediate certificate for the node.  This might be nil if there is
 * no intermediate certificate, or if the node is not on a fabric we have
 * access to.
 */
@property (readonly, nonatomic, nullable) MTRCertificateDERBytes intermediateCertificate MTR_NEWLY_AVAILABLE;

/**
 * The same intermediate certificate as intermediateCertificate, in Matter TLV format.
 */
@property (readonly, nonatomic, nullable) MTRCertificateTLVBytes intermediateCertificateTLV MTR_NEWLY_AVAILABLE;

/**
 * The operational certificate for the node.  This might be nil if
 * the node is not on a fabric we have access to.
 */
@property (readonly, nonatomic, nullable) MTRCertificateDERBytes operationalCertificate MTR_NEWLY_AVAILABLE;

/**
 * The same operational certificate as operationalCertificate, in Matter TLV format.
 */
@property (readonly, nonatomic, nullable) MTRCertificateTLVBytes operationalCertificateTLV MTR_NEWLY_AVAILABLE;

/**
 * The fabric index which identifies the fabric on the node.
 */
@property (readonly, nonatomic) NSNumber * fabricIndex MTR_NEWLY_AVAILABLE;

@end

NS_ASSUME_NONNULL_END
