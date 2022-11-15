/**
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

#import "MTRCertificates.h"
#import "MTRError_Internal.h"
#import "MTRLogging.h"
#import "MTRMemory.h"
#import "MTROperationalCredentialsDelegate.h"
#import "MTRP256KeypairBridge.h"
#import "NSDataSpanConversion.h"

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;

@implementation MTRCertificates

+ (MTRCertificateDERBytes _Nullable)createRootCertificate:(id<MTRKeypair>)keypair
                                                 issuerID:(NSNumber * _Nullable)issuerID
                                                 fabricID:(NSNumber * _Nullable)fabricID
                                                    error:(NSError * __autoreleasing *)error
{
    NSLog(@"Generating root certificate");

    [MTRMemory ensureInit];

    NSData * rootCert = nil;
    CHIP_ERROR err = MTROperationalCredentialsDelegate::GenerateRootCertificate(keypair, issuerID, fabricID, &rootCert);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:err];
    }

    if (err != CHIP_NO_ERROR) {
        NSLog(@"Generating root certificate failed: %s", ErrorStr(err));
    }

    return rootCert;
}

+ (MTRCertificateDERBytes _Nullable)createIntermediateCertificate:(id<MTRKeypair>)rootKeypair
                                                  rootCertificate:(MTRCertificateDERBytes)rootCertificate
                                            intermediatePublicKey:(SecKeyRef)intermediatePublicKey
                                                         issuerID:(NSNumber * _Nullable)issuerID
                                                         fabricID:(NSNumber * _Nullable)fabricID
                                                            error:(NSError * __autoreleasing *)error
{
    NSLog(@"Generating intermediate certificate");

    [MTRMemory ensureInit];

    NSData * intermediate = nil;
    CHIP_ERROR err = MTROperationalCredentialsDelegate::GenerateIntermediateCertificate(
        rootKeypair, rootCertificate, intermediatePublicKey, issuerID, fabricID, &intermediate);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:err];
    }

    if (err != CHIP_NO_ERROR) {
        NSLog(@"Generating intermediate certificate failed: %s", ErrorStr(err));
    }

    return intermediate;
}

+ (MTRCertificateDERBytes _Nullable)createOperationalCertificate:(id<MTRKeypair>)signingKeypair
                                              signingCertificate:(MTRCertificateDERBytes)signingCertificate
                                            operationalPublicKey:(SecKeyRef)operationalPublicKey
                                                        fabricID:(NSNumber *)fabricID
                                                          nodeID:(NSNumber *)nodeID
                                           caseAuthenticatedTags:(NSArray<NSNumber *> * _Nullable)caseAuthenticatedTags
                                                           error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    NSLog(@"Generating operational certificate");

    [MTRMemory ensureInit];

    NSData * opcert = nil;
    CHIP_ERROR err = MTROperationalCredentialsDelegate::GenerateOperationalCertificate(
        signingKeypair, signingCertificate, operationalPublicKey, fabricID, nodeID, caseAuthenticatedTags, &opcert);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:err];
    }

    if (err != CHIP_NO_ERROR) {
        NSLog(@"Generating operational certificate failed: %s", ErrorStr(err));
    }

    return opcert;
}

+ (BOOL)keypair:(id<MTRKeypair>)keypair matchesCertificate:(NSData *)certificate
{
    [MTRMemory ensureInit];

    P256PublicKey keypairPubKey;
    CHIP_ERROR err = MTRP256KeypairBridge::MatterPubKeyFromSecKeyRef(keypair.publicKey, &keypairPubKey);
    if (err != CHIP_NO_ERROR) {
        NSLog(@"Can't extract public key from keypair: %s", ErrorStr(err));
        return NO;
    }
    P256PublicKeySpan keypairKeySpan(keypairPubKey.ConstBytes());

    P256PublicKey certPubKey;
    err = ExtractPubkeyFromX509Cert(AsByteSpan(certificate), certPubKey);
    if (err != CHIP_NO_ERROR) {
        NSLog(@"Can't extract public key from certificate: %s", ErrorStr(err));
        return NO;
    }
    P256PublicKeySpan certKeySpan(certPubKey.ConstBytes());

    return certKeySpan.data_equal(keypairKeySpan);
}

+ (BOOL)isCertificate:(MTRCertificateDERBytes)certificate1 equalTo:(MTRCertificateDERBytes)certificate2
{
    [MTRMemory ensureInit];

    P256PublicKey pubKey1;
    CHIP_ERROR err = ExtractPubkeyFromX509Cert(AsByteSpan(certificate1), pubKey1);
    if (err != CHIP_NO_ERROR) {
        NSLog(@"Can't extract public key from first certificate: %s", ErrorStr(err));
        return NO;
    }
    P256PublicKeySpan keySpan1(pubKey1.ConstBytes());

    P256PublicKey pubKey2;
    err = ExtractPubkeyFromX509Cert(AsByteSpan(certificate2), pubKey2);
    if (err != CHIP_NO_ERROR) {
        NSLog(@"Can't extract public key from second certificate: %s", ErrorStr(err));
        return NO;
    }
    P256PublicKeySpan keySpan2(pubKey1.ConstBytes());

    if (!keySpan1.data_equal(keySpan2)) {
        return NO;
    }

    ChipDN subject1;
    err = ExtractSubjectDNFromX509Cert(AsByteSpan(certificate1), subject1);
    if (err != CHIP_NO_ERROR) {
        NSLog(@"Can't extract subject DN from first certificate: %s", ErrorStr(err));
        return NO;
    }

    ChipDN subject2;
    err = ExtractSubjectDNFromX509Cert(AsByteSpan(certificate2), subject2);
    if (err != CHIP_NO_ERROR) {
        NSLog(@"Can't extract subject DN from second certificate: %s", ErrorStr(err));
        return NO;
    }

    return subject1.IsEqual(subject2);
}

+ (NSData * _Nullable)createCertificateSigningRequest:(id<MTRKeypair>)keypair
                                                error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    [MTRMemory ensureInit];

    MTRP256KeypairBridge keypairBridge;
    CHIP_ERROR err = CHIP_NO_ERROR;
    do {
        err = keypairBridge.Init(keypair);
        if (err != CHIP_NO_ERROR) {
            break;
        }

        uint8_t buf[kMAX_CSR_Length];
        MutableByteSpan csr(buf);
        err = GenerateCertificateSigningRequest(&keypairBridge, csr);
        if (err != CHIP_NO_ERROR) {
            break;
        }

        return AsData(csr);
    } while (0);

    if (error) {
        *error = [MTRError errorForCHIPErrorCode:err];
    }
    return nil;
}

+ (MTRCertificateTLVBytes _Nullable)convertX509Certificate:(MTRCertificateDERBytes)x509Certificate
{

    chip::ByteSpan x509CertBytes = AsByteSpan(x509Certificate);

    uint8_t chipCertBuffer[chip::Credentials::kMaxCHIPCertLength];
    chip::MutableByteSpan chipCertBytes(chipCertBuffer);

    CHIP_ERROR errorCode = chip::Credentials::ConvertX509CertToChipCert(x509CertBytes, chipCertBytes);
    MTR_LOG_ERROR("ConvertX509CertToChipCert: %{public}s", chip::ErrorStr(errorCode));

    if (errorCode != CHIP_NO_ERROR)
        return nil;

    return AsData(chipCertBytes);
}

@end

@implementation MTRCertificates (Deprecated)

+ (nullable NSData *)generateRootCertificate:(id<MTRKeypair>)keypair
                                    issuerId:(nullable NSNumber *)issuerId
                                    fabricId:(nullable NSNumber *)fabricId
                                       error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    return [MTRCertificates createRootCertificate:keypair issuerID:issuerId fabricID:fabricId error:error];
}

+ (nullable NSData *)generateIntermediateCertificate:(id<MTRKeypair>)rootKeypair
                                     rootCertificate:(NSData *)rootCertificate
                               intermediatePublicKey:(SecKeyRef)intermediatePublicKey
                                            issuerId:(nullable NSNumber *)issuerId
                                            fabricId:(nullable NSNumber *)fabricId
                                               error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    return [MTRCertificates createIntermediateCertificate:rootKeypair
                                          rootCertificate:rootCertificate
                                    intermediatePublicKey:intermediatePublicKey
                                                 issuerID:issuerId
                                                 fabricID:fabricId
                                                    error:error];
}

+ (nullable NSData *)generateOperationalCertificate:(id<MTRKeypair>)signingKeypair
                                 signingCertificate:(NSData *)signingCertificate
                               operationalPublicKey:(SecKeyRef)operationalPublicKey
                                           fabricId:(NSNumber *)fabricId
                                             nodeId:(NSNumber *)nodeId
                              caseAuthenticatedTags:(NSArray<NSNumber *> * _Nullable)caseAuthenticatedTags
                                              error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    return [MTRCertificates createOperationalCertificate:signingKeypair
                                      signingCertificate:signingCertificate
                                    operationalPublicKey:operationalPublicKey
                                                fabricID:fabricId
                                                  nodeID:nodeId
                                   caseAuthenticatedTags:caseAuthenticatedTags
                                                   error:error];
}

+ (nullable NSData *)generateCertificateSigningRequest:(id<MTRKeypair>)keypair
                                                 error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    return [MTRCertificates createCertificateSigningRequest:keypair error:error];
}

@end
