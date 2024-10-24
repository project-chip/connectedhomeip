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

#import "CertificateIssuer.h"
#import "CHIPToolKeypair.h"

#include <lib/support/logging/CHIPLogging.h>

constexpr const uint32_t kIssuerId = 12345678;

@interface CertificateIssuer ()
- (MTRCertificateDERBytes _Nullable)issueOperationalCertificateForNodeID:(NSNumber *)nodeID
                                                                fabricID:(NSNumber *)fabricID
                                                               publicKey:(SecKeyRef)publicKey
                                                                   error:(NSError * _Nullable __autoreleasing * _Nonnull)error;
@end

@implementation CertificateIssuer

+ (CertificateIssuer *)sharedInstance
{
    static CertificateIssuer * certificateIssuer = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        certificateIssuer = [[CertificateIssuer alloc] init];
    });
    return certificateIssuer;
}

- (instancetype)init
{
    if (!(self = [super init])) {
        return nil;
    }

    _rootCertificate = nil;
    _ipk = nil;
    _signingKey = nil;
    _fabricID = nil;
    _nextNodeID = nil;
    _shouldSkipAttestationCertificateValidation = NO;

    return self;
}

- (void)startWithStorage:(id<MTRStorage>)storage
                   error:(NSError * _Nullable __autoreleasing * _Nonnull)error
{
    __auto_type * signingKey = [[CHIPToolKeypair alloc] init];

    __auto_type err = [signingKey createOrLoadKeys:storage];
    if (CHIP_NO_ERROR != err) {
        *error = [NSError errorWithDomain:@"Error" code:0 userInfo:@{ @"reason" : @"Error creating or loading keys" }];
        return;
    }

    __auto_type * rootCertificate = [MTRCertificates createRootCertificate:signingKey issuerID:@(kIssuerId) fabricID:nil error:error];
    if (nil == rootCertificate) {
        *error = [NSError errorWithDomain:@"Error" code:0 userInfo:@{ @"reason" : @"Error creating root certificate" }];
        return;
    }

    _rootCertificate = rootCertificate;
    _signingKey = signingKey;
    _ipk = [signingKey getIPK];
}

- (id<MTRKeypair>)issueOperationalKeypairWithControllerStorage:(ControllerStorage *)storage error:(NSError * _Nullable __autoreleasing * _Nonnull)error
{
    __auto_type * keypair = [[CHIPToolKeypair alloc] init];

    __auto_type err = [keypair createOrLoadKeys:storage];
    if (CHIP_NO_ERROR != err) {
        *error = [NSError errorWithDomain:@"Error" code:0 userInfo:@{ @"reason" : @"Error creating or loading keys" }];
        return nil;
    }

    return keypair;
}

- (void)issueOperationalCertificateForRequest:(MTROperationalCSRInfo *)csrInfo
                              attestationInfo:(MTRDeviceAttestationInfo *)attestationInfo
                                   controller:(MTRDeviceController *)controller
                                   completion:(void (^)(MTROperationalCertificateChain * _Nullable info,
                                                  NSError * _Nullable error))completion
{
    NSError * error = nil;

    if (self.nextNodeID == nil) {
        error = [NSError errorWithDomain:@"Error" code:0 userInfo:@{ @"reason" : @"nextNodeID is nil" }];
        completion(nil, error);
        return;
    }

    __auto_type * csr = csrInfo.csr;
    __auto_type * rawPublicKey = [MTRCertificates publicKeyFromCSR:csr error:&error];
    if (error != nil) {
        completion(nil, error);
        return;
    }

    NSDictionary * attributes = @{
        (__bridge NSString *) kSecAttrKeyType : (__bridge NSString *) kSecAttrKeyTypeECSECPrimeRandom,
        (__bridge NSString *) kSecAttrKeyClass : (__bridge NSString *) kSecAttrKeyClassPublic
    };
    CFErrorRef keyCreationError = NULL;
    SecKeyRef publicKey
        = SecKeyCreateWithData((__bridge CFDataRef) rawPublicKey, (__bridge CFDictionaryRef) attributes, &keyCreationError);

    __auto_type * operationalCert = [self issueOperationalCertificateForNodeID:self.nextNodeID
                                                                      fabricID:self.fabricID
                                                                     publicKey:publicKey
                                                                         error:&error];

    // Release no-longer-needed key before we do anything else.
    CFRelease(publicKey);

    if (error != nil) {
        completion(nil, error);
        return;
    }

    __auto_type * certChain = [[MTROperationalCertificateChain alloc] initWithOperationalCertificate:operationalCert
                                                                             intermediateCertificate:nil
                                                                                     rootCertificate:self.rootCertificate
                                                                                        adminSubject:nil];
    completion(certChain, nil);
}

- (MTRCertificateDERBytes _Nullable)issueOperationalCertificateForNodeID:(NSNumber *)nodeID
                                                                fabricID:(NSNumber *)fabricID
                                                               publicKey:(SecKeyRef)publicKey
                                                                   error:(NSError * _Nullable __autoreleasing * _Nonnull)error
{
    __auto_type * operationalCert = [MTRCertificates createOperationalCertificate:self.signingKey
                                                               signingCertificate:self.rootCertificate
                                                             operationalPublicKey:publicKey
                                                                         fabricID:fabricID
                                                                           nodeID:nodeID
                                                            caseAuthenticatedTags:nil
                                                                            error:error];
    return operationalCert;
}

@end
