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

NS_ASSUME_NONNULL_BEGIN

typedef void (^MTRNOCChainGenerationCompleteHandler)(NSData * operationalCertificate, NSData * intermediateCertificate,
    NSData * rootCertificate, NSData * _Nullable ipk, NSNumber * _Nullable adminSubject, NSError * __autoreleasing * error);

@protocol MTRNOCChainIssuer <NSObject>
@required

/**
 * @brief When a MTRNOCChainIssuer is set for the MTRDeviceController, then onNOCChainGenerationNeeded will be
 * called when the NOC CSR needs to be signed. This allows for custom credentials issuer
 * implementations, for example, when a proprietary cloud API will perform the CSR signing.

 * The commissioning workflow will stop upon the onNOCChainGenerationNeeded callback and
 * resume once onNOCChainGenerationComplete is called

 * The following fields MUST be passed to onNOCChainGenerationComplete with non-nil values:
 * rootCertificate, intermediateCertificate, operationalCertificate.
 * If ipk and adminSubject are passed, then they will be used in
 * the AddNOC command sent to the commissionee. If they are not passed, then the values
 * provided in the MTRDeviceController initialization will be used.
 *
 * All csr and attestation fields are provided to allow for custom attestestation checks.
 */
- (void)onNOCChainGenerationNeeded:(CSRInfo *)csrInfo
                   attestationInfo:(AttestationInfo *)attestationInfo
      onNOCChainGenerationComplete:(MTRNOCChainGenerationCompleteHandler)onNOCChainGenerationComplete;

@end

NS_ASSUME_NONNULL_END
