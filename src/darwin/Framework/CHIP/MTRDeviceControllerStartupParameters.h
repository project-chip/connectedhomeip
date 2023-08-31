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

#import <Matter/MTRDefines.h>
#import <Matter/MTRDeviceControllerStorageDelegate.h>

NS_ASSUME_NONNULL_BEGIN

MTR_NEWLY_AVAILABLE
@interface MTRDeviceControllerStartupParameters : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * The Product Attestation Authority certificates that are trusted to sign
 * device attestation information (and in particular to sign Product Attestation
 * Intermediate certificates, which then sign Device Attestation Certificates).
 *
 * Defaults to nil.
 */
@property (nonatomic, copy, nullable) NSArray<MTRCertificateDERBytes> * productAttestationAuthorityCertificates;

/**
 * The Certification Declaration certificates whose public keys correspond to
 * private keys that are trusted to sign certification declarations.  Defaults
 * to nil.
 *
 * These certificates are used in addition to, not replacing, the default set of
 * well-known certification declaration signing keys.
 */
@property (nonatomic, copy, nullable) NSArray<MTRCertificateDERBytes> * certificationDeclarationCertificates;

/**
 * Set an MTROperationalCertificateIssuer to call (on the provided queue) when
 * operational certificates need to be provided during commissioning.
 */
- (void)setOperationalCertificateIssuer:(id<MTROperationalCertificateIssuer>)operationalCertificateIssuer
                                  queue:(dispatch_queue_t)queue;

@end

MTR_NEWLY_AVAILABLE
@interface MTRDeviceControllerExternalCertificateStartupParameters : MTRDeviceControllerStartupParameters

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Prepare to initialize a controller that is not able to sign operational
 * certificates itself, and therefore needs to be provided with a complete
 * operational certificate chain.
 *
 * A controller created from MTRDeviceControllerStartupParams initialized with
 * this method will not be able to commission devices unless
 * operationalCertificateIssuer and operationalCertificateIssuerQueue are set.
 *
 * The fabric id and node id to use for the controller will be derived from the provided
 * operationalCertificate.
 *
 * @param storageDelegate The storage to use for the controller.  This will be
 *                        called into on storageDelegateQueue.
 *
 * @param storageDelegateQueue The queue for calls into storageDelegate.  See
 *                             MTRDeviceControllerStorageDelegate documentation
 *                             for the rules about what work is allowed to be
 *                             done on this queue.
 *
 * @param uniqueIdentifier The unique id to assign to the controller.
 *
 * @param vendorID The vendor ID (allocated by the Connectivity Standards Alliance) for
 *                 this controller. Must not be the "standard" vendor id (0).
 *
 * @param ipk The Identity Protection Key. Must be 16 bytes in length.
 *
 * @param intermediateCertificate Must be nil if operationalCertificate is
 *                                directly signed by rootCertificate.  Otherwise
 *                                must be the certificate that signed
 *                                operationalCertificate.
 */
- (instancetype)initWithStorageDelegate:(id<MTRDeviceControllerStorageDelegate>)storageDelegate
                   storageDelegateQueue:(dispatch_queue_t)storageDelegateQueue
                       uniqueIdentifier:(NSUUID *)uniqueIdentifier
                                    ipk:(NSData *)ipk
                               vendorID:(NSNumber *)vendorID
                     operationalKeypair:(id<MTRKeypair>)operationalKeypair
                 operationalCertificate:(MTRCertificateDERBytes)operationalCertificate
                intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
                        rootCertificate:(MTRCertificateDERBytes)rootCertificate;

@end

NS_ASSUME_NONNULL_END
