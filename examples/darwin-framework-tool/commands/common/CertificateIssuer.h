/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#import <Matter/Matter.h>

#import "ControllerStorage.h"

NS_ASSUME_NONNULL_BEGIN

@interface CertificateIssuer : NSObject <MTROperationalCertificateIssuer>
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)new NS_UNAVAILABLE;

+ (CertificateIssuer *)sharedInstance;

- (void)startWithStorage:(id<MTRStorage>)storage
                   error:(NSError * _Nullable __autoreleasing * _Nonnull)error;

- (id<MTRKeypair>)issueOperationalKeypairWithControllerStorage:(ControllerStorage *)storage error:(NSError * _Nullable __autoreleasing * _Nonnull)error;

- (MTRCertificateDERBytes _Nullable)issueOperationalCertificateForNodeID:(NSNumber *)nodeID
                                                                fabricID:(NSNumber *)fabricID
                                                               publicKey:(SecKeyRef)publicKey
                                                                   error:(NSError * _Nullable __autoreleasing * _Nonnull)error;

@property (nonatomic, readonly) MTRCertificateDERBytes rootCertificate;
@property (nonatomic, readonly) id<MTRKeypair> signingKey;
@property (nonatomic, readonly) NSData * ipk;

@property (nonatomic, nullable) NSNumber * fabricID;
@property (nonatomic, nullable) NSNumber * nextNodeID;
@property (nonatomic, readonly) BOOL shouldSkipAttestationCertificateValidation;

@end

NS_ASSUME_NONNULL_END
