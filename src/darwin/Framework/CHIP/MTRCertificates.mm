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

#import "MTRCertificates.h"
#import "MTRError_Internal.h"
#import "MTRFramework.h"
#import "MTRLogging_Internal.h"
#import "MTROperationalCredentialsDelegate.h"
#import "MTRP256KeypairBridge.h"
#import "NSDataSpanConversion.h"

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;

@implementation MTRCertificates

+ (void)initialize
{
    MTRFrameworkInit();
}

+ (MTRCertificateDERBytes _Nullable)createRootCertificate:(id<MTRKeypair>)keypair
                                                 issuerID:(NSNumber * _Nullable)issuerID
                                                 fabricID:(NSNumber * _Nullable)fabricID
                                           validityPeriod:(NSDateInterval *)validityPeriod
                                                    error:(NSError * __autoreleasing *)error
{
    MTR_LOG("Generating root certificate");
    NSData * rootCert = nil;
    CHIP_ERROR err
        = MTROperationalCredentialsDelegate::GenerateRootCertificate(keypair, issuerID, fabricID, validityPeriod, &rootCert);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:err];
    }

    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Generating root certificate failed: %s", ErrorStr(err));
    }

    return rootCert;
}

+ (MTRCertificateDERBytes _Nullable)createRootCertificate:(id<MTRKeypair>)keypair
                                                 issuerID:(NSNumber * _Nullable)issuerID
                                                 fabricID:(NSNumber * _Nullable)fabricID
                                                    error:(NSError * __autoreleasing *)error
{
    auto * validityPeriod = [[NSDateInterval alloc] initWithStartDate:[NSDate now] endDate:[NSDate distantFuture]];
    return [self createRootCertificate:keypair issuerID:issuerID fabricID:fabricID validityPeriod:validityPeriod error:error];
}

+ (MTRCertificateDERBytes _Nullable)createIntermediateCertificate:(id<MTRKeypair>)rootKeypair
                                                  rootCertificate:(MTRCertificateDERBytes)rootCertificate
                                            intermediatePublicKey:(SecKeyRef)intermediatePublicKey
                                                         issuerID:(NSNumber * _Nullable)issuerID
                                                         fabricID:(NSNumber * _Nullable)fabricID
                                                   validityPeriod:(NSDateInterval *)validityPeriod
                                                            error:(NSError * __autoreleasing *)error
{
    MTR_LOG("Generating intermediate certificate");
    NSData * intermediate = nil;
    CHIP_ERROR err = MTROperationalCredentialsDelegate::GenerateIntermediateCertificate(
        rootKeypair, rootCertificate, intermediatePublicKey, issuerID, fabricID, validityPeriod, &intermediate);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:err];
    }

    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Generating intermediate certificate failed: %s", ErrorStr(err));
    }

    return intermediate;
}

+ (MTRCertificateDERBytes _Nullable)createIntermediateCertificate:(id<MTRKeypair>)rootKeypair
                                                  rootCertificate:(MTRCertificateDERBytes)rootCertificate
                                            intermediatePublicKey:(SecKeyRef)intermediatePublicKey
                                                         issuerID:(NSNumber * _Nullable)issuerID
                                                         fabricID:(NSNumber * _Nullable)fabricID
                                                            error:(NSError * __autoreleasing *)error
{
    auto * validityPeriod = [[NSDateInterval alloc] initWithStartDate:[NSDate now] endDate:[NSDate distantFuture]];
    return [self createIntermediateCertificate:rootKeypair
                               rootCertificate:rootCertificate
                         intermediatePublicKey:intermediatePublicKey
                                      issuerID:issuerID
                                      fabricID:fabricID
                                validityPeriod:validityPeriod
                                         error:error];
}

+ (MTRCertificateDERBytes _Nullable)createOperationalCertificate:(id<MTRKeypair>)signingKeypair
                                              signingCertificate:(MTRCertificateDERBytes)signingCertificate
                                            operationalPublicKey:(SecKeyRef)operationalPublicKey
                                                        fabricID:(NSNumber *)fabricID
                                                          nodeID:(NSNumber *)nodeID
                                           caseAuthenticatedTags:(NSSet<NSNumber *> * _Nullable)caseAuthenticatedTags
                                                  validityPeriod:(NSDateInterval *)validityPeriod
                                                           error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    MTR_LOG("Generating operational certificate");
    NSData * opcert = nil;
    CHIP_ERROR err = MTROperationalCredentialsDelegate::GenerateOperationalCertificate(
        signingKeypair, signingCertificate, operationalPublicKey, fabricID, nodeID, caseAuthenticatedTags, validityPeriod, &opcert);
    if (error) {
        *error = [MTRError errorForCHIPErrorCode:err];
    }

    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Generating operational certificate failed: %s", ErrorStr(err));
    }

    return opcert;
}

+ (MTRCertificateDERBytes _Nullable)createOperationalCertificate:(id<MTRKeypair>)signingKeypair
                                              signingCertificate:(MTRCertificateDERBytes)signingCertificate
                                            operationalPublicKey:(SecKeyRef)operationalPublicKey
                                                        fabricID:(NSNumber *)fabricID
                                                          nodeID:(NSNumber *)nodeID
                                           caseAuthenticatedTags:(NSSet<NSNumber *> * _Nullable)caseAuthenticatedTags
                                                           error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    auto * validityPeriod = [[NSDateInterval alloc] initWithStartDate:[NSDate now] endDate:[NSDate distantFuture]];
    return [self createOperationalCertificate:signingKeypair
                           signingCertificate:signingCertificate
                         operationalPublicKey:operationalPublicKey
                                     fabricID:fabricID
                                       nodeID:nodeID
                        caseAuthenticatedTags:caseAuthenticatedTags
                               validityPeriod:validityPeriod
                                        error:error];
}

+ (BOOL)keypair:(id<MTRKeypair>)keypair matchesCertificate:(NSData *)certificate
{
    P256PublicKey keypairPubKey;
    CHIP_ERROR err = MTRP256KeypairBridge::MatterPubKeyFromSecKeyRef(keypair.publicKey, &keypairPubKey);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Can't extract public key from keypair: %s", ErrorStr(err));
        return NO;
    }
    P256PublicKeySpan keypairKeySpan(keypairPubKey.ConstBytes());

    P256PublicKey certPubKey;
    err = ExtractPubkeyFromX509Cert(AsByteSpan(certificate), certPubKey);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Can't extract public key from certificate: %s", ErrorStr(err));
        return NO;
    }
    P256PublicKeySpan certKeySpan(certPubKey.ConstBytes());

    return certKeySpan.data_equal(keypairKeySpan);
}

+ (BOOL)isCertificate:(MTRCertificateDERBytes)certificate1 equalTo:(MTRCertificateDERBytes)certificate2
{
    P256PublicKey pubKey1;
    CHIP_ERROR err = ExtractPubkeyFromX509Cert(AsByteSpan(certificate1), pubKey1);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Can't extract public key from first certificate: %s", ErrorStr(err));
        return NO;
    }
    P256PublicKeySpan keySpan1(pubKey1.ConstBytes());

    P256PublicKey pubKey2;
    err = ExtractPubkeyFromX509Cert(AsByteSpan(certificate2), pubKey2);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Can't extract public key from second certificate: %s", ErrorStr(err));
        return NO;
    }
    P256PublicKeySpan keySpan2(pubKey1.ConstBytes());

    if (!keySpan1.data_equal(keySpan2)) {
        return NO;
    }

    ChipDN subject1;
    err = ExtractSubjectDNFromX509Cert(AsByteSpan(certificate1), subject1);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Can't extract subject DN from first certificate: %s", ErrorStr(err));
        return NO;
    }

    ChipDN subject2;
    err = ExtractSubjectDNFromX509Cert(AsByteSpan(certificate2), subject2);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Can't extract subject DN from second certificate: %s", ErrorStr(err));
        return NO;
    }

    return subject1.IsEqual(subject2);
}

+ (NSData * _Nullable)createCertificateSigningRequest:(id<MTRKeypair>)keypair
                                                error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    MTRP256KeypairBridge keypairBridge;
    CHIP_ERROR err = CHIP_NO_ERROR;
    do {
        err = keypairBridge.Init(keypair);
        if (err != CHIP_NO_ERROR) {
            break;
        }

        uint8_t buf[kMIN_CSR_Buffer_Size];
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
    if (errorCode != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("convertX509Certificate: %s", errorCode.AsString());
        return nil;
    }

    MTR_LOG_DEBUG("convertX509Certificate: Success");

    return AsData(chipCertBytes);
}

+ (MTRCertificateDERBytes _Nullable)convertMatterCertificate:(MTRCertificateTLVBytes)matterCertificate
{
    chip::ByteSpan tlvCertBytes = AsByteSpan(matterCertificate);

    uint8_t derCertBuffer[chip::Controller::kMaxCHIPDERCertLength];
    chip::MutableByteSpan derCertBytes(derCertBuffer);

    CHIP_ERROR errorCode = chip::Credentials::ConvertChipCertToX509Cert(tlvCertBytes, derCertBytes);

    if (errorCode != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("convertMatterCertificate: %s", chip::ErrorStr(errorCode));
        return nil;
    }

    return AsData(derCertBytes);
}

+ (NSData * _Nullable)publicKeyFromCSR:(MTRCSRDERBytes)certificateSigningRequest
                                 error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    auto requestSpan = AsByteSpan(certificateSigningRequest);
    P256PublicKey publicKey;
    CHIP_ERROR err = VerifyCertificateSigningRequest(requestSpan.data(), requestSpan.size(), publicKey);
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("publicKeyFromCSR: %s", chip::ErrorStr(err));
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:err];
        }
        return nil;
    }

    P256PublicKeySpan publicKeySpan(publicKey.ConstBytes());
    return AsData(publicKeySpan);
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
    NSSet<NSNumber *> * tags = nil;
    if (caseAuthenticatedTags != nil) {
        tags = [NSSet setWithArray:caseAuthenticatedTags];
    }
    return [MTRCertificates createOperationalCertificate:signingKeypair
                                      signingCertificate:signingCertificate
                                    operationalPublicKey:operationalPublicKey
                                                fabricID:fabricId
                                                  nodeID:nodeId
                                   caseAuthenticatedTags:tags
                                                   error:error];
}

+ (nullable NSData *)generateCertificateSigningRequest:(id<MTRKeypair>)keypair
                                                 error:(NSError * __autoreleasing _Nullable * _Nullable)error
{
    return [MTRCertificates createCertificateSigningRequest:keypair error:error];
}

@end
