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

#import <Matter/MTRAttestationInfo.h>
#import <Matter/MTRCSRInfo.h>
#import <Matter/MTRCertificates.h>

NS_ASSUME_NONNULL_BEGIN

@class MTRDeviceController;

/**
 * A representation of the operation certificate info for a node.
 *
 * A nil intermediateCertificate means there is no intermediate.
 *
 * adminSubject is passed to the device as part of the AddNOC command.  A nil
 * adminSubject means the node id of the relevant MTRDeviceController will be
 * used.
 */
MTR_NEWLY_AVAILABLE
@interface MTROperationalCertificateInfo : NSObject
@property (nonatomic, copy) MTRCertificateDERBytes operationalCertificate;
@property (nonatomic, copy, nullable) MTRCertificateDERBytes intermediateCertificate;
@property (nonatomic, copy) MTRCertificateDERBytes rootCertificate;
@property (nonatomic, copy, nullable) NSNumber * adminSubject;

- (instancetype)initWithOperationalCertificate:(MTRCertificateDERBytes)operationalCertificate
                       intermediateCertificate:(nullable MTRCertificateDERBytes)intermediateCertificate
                               rootCertificate:(MTRCertificateDERBytes)rootCertificate
                                  adminSubject:(nullable NSNumber *)adminSubject;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;
@end

typedef void (^MTROperationalCertificateIssuedHandler)(MTROperationalCertificateInfo * _Nullable info, NSError * _Nullable error);

MTR_NEWLY_AVAILABLE
@protocol MTROperationalCertificateIssuer
@required

/**
 * @brief When an MTROperationalCertificateIssuer is set for an
 * MTRDeviceController, it will be used to issue operational certificates as
 * needed during commissioning.
 *
 * Commissioning will pause when
 * issueOperationalCertificateForRequest:attestationInfo:completion: is called,
 * and resume when the completion is invoked with a non-nil
 * MTROperationalCertificateInfo.  When the completion is invoked with an error,
 * commissioning will fail.
 */
- (void)issueOperationalCertificateForRequest:(MTROperationalCSRInfo *)csrInfo
                              attestationInfo:(MTRAttestationInfo *)attestationInfo
                                   controller:(MTRDeviceController *)controller
                                   completion:(MTROperationalCertificateIssuedHandler)completion;

@end

MTR_NEWLY_DEPRECATED("Please use MTROperationalCertificateIssuedHandler")
typedef void (^MTRNOCChainGenerationCompleteHandler)(NSData * operationalCertificate, NSData * intermediateCertificate,
    NSData * rootCertificate, NSData * _Nullable ipk, NSNumber * _Nullable adminSubject, NSError * __autoreleasing * error);

MTR_NEWLY_DEPRECATED("Please use MTROperationalCertificateIssuer")
@protocol MTRNOCChainIssuer <NSObject>
@required

- (void)onNOCChainGenerationNeeded:(CSRInfo *)csrInfo
                   attestationInfo:(AttestationInfo *)attestationInfo
      onNOCChainGenerationComplete:(MTRNOCChainGenerationCompleteHandler)onNOCChainGenerationComplete;

@end

NS_ASSUME_NONNULL_END
