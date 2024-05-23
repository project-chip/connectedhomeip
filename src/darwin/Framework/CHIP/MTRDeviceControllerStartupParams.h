/**
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
#import <Matter/MTROperationalCertificateIssuer.h>

NS_ASSUME_NONNULL_BEGIN

@protocol MTRKeypair;

MTR_AVAILABLE(ios(16.1), macos(13.0), watchos(9.1), tvos(16.1))
@interface MTRDeviceControllerStartupParams : NSObject

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

/**
 * Prepare to initialize a controller given a keypair to use for signing
 * operational certificates.
 *
 * A controller created from MTRDeviceControllerStartupParams initialized with
 * this method will be able to issue operational certificates to devices it
 * commissions, using nocSigner to sign them.

 * @param ipk The Identity Protection Key, must be 16 bytes in length
 * @param fabricID The fabric identifier, must be non-zero.
 */
- (instancetype)initWithIPK:(NSData *)ipk
                   fabricID:(NSNumber *)fabricID
                  nocSigner:(id<MTRKeypair>)nocSigner MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Prepare to initialize a controller that is not able to sign operational
 * certificates itself, and therefore needs to be provided with a complete
 * operational certificate chain.  This initialization method should be used
 * when none of the certificate-signing private keys are available locally.
 *
 * A controller created from MTRDeviceControllerStartupParams initialized with
 * this method will not be able to commission devices unless
 * operationalCertificateIssuer and operationalCertificateIssuerQueue are set.
 *
 * The fabric id and node id to use for the controller will be derived from the provided
 * operationalCertificate.
 *
 * @param ipk The Identity Protection Key, must be 16 bytes in length
 * @param intermediateCertificate may be nil if operationalCertificate is directly signed by rootCertificate.
 */
- (instancetype)initWithIPK:(NSData *)ipk
         operationalKeypair:(id<MTRKeypair>)operationalKeypair
     operationalCertificate:(MTRCertificateDERBytes)operationalCertificate
    intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
            rootCertificate:(MTRCertificateDERBytes)rootCertificate MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Keypair used to sign operational certificates.  This is the root CA keypair
 * if not using an intermediate CA, the intermediate CA's keypair otherwise.
 *
 * Allowed to be nil if this controller will not be issuing internally-generated
 * operational certificates.  In that case, the MTRDeviceControllerStartupParams
 * object must be initialized using
 * initWithIPK:operationalKeypair:operationalCertificate:intermediateCertificate:rootCertificate:
 * (to provide the operational credentials for t2he controller itself).
 */
@property (nonatomic, copy, readonly, nullable) id<MTRKeypair> nocSigner;

/**
 * Fabric id for the controller.  Must be set to a nonzero value.  This is
 * scoped by the root public key, which is determined as follows:
 *
 * * If a root certificate is provided, it is the public key of the root
 *   certificate.
 *
 * * If a root certificate is not provided, the root public key is the public
 *   key of the nocSigner keypair, since in this case we are not using an
 *   intermediate certificate.
 */
@property (nonatomic, copy, readonly) NSNumber * fabricID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * IPK to use for the controller's fabric.  Allowed to change from the last time
 * a controller was started on this fabric if a new IPK has been distributed to
 * all the devices the controller wants to interact with.
 */
@property (nonatomic, copy, readonly) NSData * ipk;

/**
 * Vendor ID (allocated by the Connectivity Standards Alliance) for
 * this controller.
 *
 * If not nil, must not be the "standard" vendor id (0).
 *
 * When creating a new fabric:
 *
 * * Must not be nil.
 *
 * When using an existing fabric:
 *
 * * Will override existing value if not nil. Otherwise existing value will be
 *   used.
 */
@property (nonatomic, copy, nullable) NSNumber * vendorID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * Node id for this controller.
 *
 * If operationalCertificate is not nil, must be nil.  The provided operational
 * certificate will be used as-is.
 *
 * If not nil, must be a valid Matter operational node id.
 *
 * If operationalCertificate is nil, nodeID and operationalKeypair are used to
 * determine an operational certificate, as follows:
 *
 * * When creating a new fabric:
 *
 * ** nodeID is allowed to be nil to indicate that a random node id should be
 *    generated.
 *
 * * When using an existing fabric:
 *
 * ** nodeID is allowed to be nil to indicate that the existing operational node
 *    id should be used.  The existing operational keys will also be used,
 *    unless operationalKeypair is provided.  The existing caseAuthenticatedTags
 *    will be used.
 *
 * ** If nodeID is not nil, a new operational certificate will be generated for
 *    the provided node id (even if that matches the existing node id), using
 *    either the operationalKeypair if that is provided or a new randomly
 *    generated operational key, and using the provided caseAuthenticatedTags.
 */
@property (nonatomic, copy, nullable) NSNumber * nodeID MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * CASE authenticated tags to use for this controller's operational certificate.
 *
 * Only allowed to be not nil if nodeID is not nil.  In particular, if
 * operationalCertificate is not nil, must be nil.  The provided operational
 * certificate will be used as-is.
 *
 * If not nil, must contain at most 3 numbers, which are expected to be 32-bit
 * unsigned Case Authenticated Tag values.
 */
@property (nonatomic, copy, nullable)
    NSSet<NSNumber *> * caseAuthenticatedTags MTR_AVAILABLE(ios(17.0), macos(14.0), watchos(10.0), tvos(17.0));

/**
 * Root certificate, in X.509 DER form, to use.
 *
 * Must not be nil if an intermediate CA is being used, to allow determination
 * of the root public key.
 *
 * If not nil, and if an intermediate CA is not being used, the public key of
 * this certificate must match the public key of nocSigner, if nocSigner is not
 * nil.
 *
 * When creating a new fabric:
 *
 * * May be nil if nocSigner is not nil and an intermediate CA is not being
 *   used.  In that case the nocSigner keypair, which is the keypair for the
 *   root certificate, will be used to generate and sign a root certificate,
 *   with a random issuer id.  In this case, the fabricID will be included in
 *   the root certificate's subject DN.
 *
 * When using an existing fabric:
 *
 * * May be nil if nocSigner is not nil and an intermediate CA is not being
 *   used.  In that case, the existing root certificate for the fabric will be
 *   used.
 *
 * * If not nil must satisfy the following properties:
 *
 *   1) The public key must match the public key of the existing root
 *      certificate.
 *   2) The subject DN must match the subject DN of the existing root
 *      certificate.
 */
@property (nonatomic, copy, nullable) MTRCertificateDERBytes rootCertificate;

/**
 * Intermediate certificate, in X.509 DER form, to use.
 *
 * If not nil, rootCertificate must not be nil, and the intermediate certificate
 * must be signed by rootCertificate.
 *
 * If not nil, and nocSigner is not nil, the public key of this certificate must
 * match the public key of nocSigner.
 *
 * When creating a new fabric:
 *
 * * Must not be nil if an intermediate CA is being used.
 *
 * * Must be nil if an intermediate CA is not being used.
 *
 * When using an existing fabric:
 *
 * * If not nil, will be used as the intermediate certificate for issuing
 *   operational certificates.
 *
 * * If nil:
 *
 *   * If nocSigner is not nil, there is an existing intermediate certificate,
 *     and it matches the nocSigner public key, the existing intermediate
 *     certificate will be used.
 *
 *   * Otherwise the fabric will not use an intermediate certificate.  This
 *     allows switching from using an intermediate CA to not using one.
 *
 */
@property (nonatomic, copy, nullable) MTRCertificateDERBytes intermediateCertificate;

/**
 * Operational certificate, in X.509 DER form, to use.
 *
 * If not nil, will be used as the operational certificate.  In this case
 * operationalKeypair must not be nil.
 *
 * If nil, an operational certificate will be determined as described in the
 * documentation for nodeID.
 */
@property (nonatomic, copy, readonly, nullable) MTRCertificateDERBytes operationalCertificate;

/**
 * Operational keypair to use.  If operationalCertificate is not nil, the public
 * key must match operationalCertificate.
 *
 * If not nil, and if operationalCertificate is nil, a new operational
 * certificate will be generated for the given operationalKeypair.  The node id
 * for that certificate will be determined as described in the documentation for
 * nodeID.
 */
@property (nonatomic, strong, nullable) id<MTRKeypair> operationalKeypair;

/**
 * The certificate issuer delegate to use for issuing operational certificates
 * when commmissioning devices.  Allowed to be nil if this controller either
 * does not issue operational certificates at all or internally generates the
 * certificates to be issued.  In the latter case, nocSigner must not be nil.
 */
@property (nonatomic, strong, nullable) id<MTROperationalCertificateIssuer> operationalCertificateIssuer MTR_AVAILABLE(
    ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

/**
 * The dispatch queue on which operationalCertificateIssuer should be called.
 * Allowed to be nil if and only if operationalCertificateIssuer is nil.
 */
@property (nonatomic, strong, nullable)
    dispatch_queue_t operationalCertificateIssuerQueue MTR_AVAILABLE(ios(16.4), macos(13.3), watchos(9.4), tvos(16.4));

@end

@interface MTRDeviceControllerStartupParams (Deprecated)

@property (nonatomic, assign, readonly) uint64_t fabricId MTR_DEPRECATED(
    "Please use fabricID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy, nullable) NSNumber * vendorId MTR_DEPRECATED(
    "Please use vendorID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
@property (nonatomic, copy, nullable)
    NSNumber * nodeId MTR_DEPRECATED("Please use nodeID", ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

- (instancetype)initWithSigningKeypair:(id<MTRKeypair>)nocSigner
                              fabricId:(uint64_t)fabricId
                                   ipk:(NSData *)ipk MTR_DEPRECATED("Please use initWithIPK:fabricID:nocSigner:", ios(16.1, 16.4),
                                           macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));
- (instancetype)initWithOperationalKeypair:(id<MTRKeypair>)operationalKeypair
                    operationalCertificate:(MTRCertificateDERBytes)operationalCertificate
                   intermediateCertificate:(MTRCertificateDERBytes _Nullable)intermediateCertificate
                           rootCertificate:(MTRCertificateDERBytes)rootCertificate
                                       ipk:(NSData *)ipk
    MTR_DEPRECATED("Please use initWithIPK:operationalKeypair:operationalCertificate:intermediateCertificate:rootCertificate:",
        ios(16.1, 16.4), macos(13.0, 13.3), watchos(9.1, 9.4), tvos(16.1, 16.4));

@end

NS_ASSUME_NONNULL_END
