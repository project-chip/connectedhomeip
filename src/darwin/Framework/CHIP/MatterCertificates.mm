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

#import "MatterCertificates.h"
#import "CHIPError_Internal.h"
#import "CHIPOperationalCredentialsDelegate.h"
#import "CHIPP256KeypairBridge.h"
#import "SpanUtils.h"

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CHIPMem.h>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;

// RAII helper for doing MemoryInit/MemoryShutdown, just in case the underlying
// Matter APIs we are using use Platform::Memory.  MemoryInit/MemoryShutdown are
// refcounted, so it's OK if we use AutoPlatformMemory after MemoryInit has
// already happened elsewhere.
struct AutoPlatformMemory {
  AutoPlatformMemory() { Platform::MemoryInit(); }
  ~AutoPlatformMemory() { Platform::MemoryShutdown(); }
};

@implementation MatterCertificates

+ (nullable NSData *)generateRootCertificate:(id<CHIPKeypair>)keypair
                                    issuerId:(nullable NSNumber *)issuerId
                                    fabricId:(nullable NSNumber *)fabricId
                                       error:(NSError * __autoreleasing *)error
{
    NSLog(@"Generating root certificate");

    AutoPlatformMemory platformMemory;

    NSData * rootCert = nil;
    CHIP_ERROR err = CHIPOperationalCredentialsDelegate::GenerateRootCertificate(keypair, issuerId, fabricId, &rootCert);
    if (error) {
        *error = [CHIPError errorForCHIPErrorCode:err];
    }

    if (err != CHIP_NO_ERROR) {
        NSLog(@"Generating root certificate failed: %s", ErrorStr(err));
    }

    return rootCert;
}

+ (nullable NSData *)generateIntermediateCertificate:(id<CHIPKeypair>)rootKeypair
                                     rootCertificate:(NSData *)rootCertificate
                               intermediatePublicKey:(SecKeyRef)intermediatePublicKey
                                            issuerId:(nullable NSNumber *)issuerId
                                            fabricId:(nullable NSNumber *)fabricId
                                               error:(NSError * __autoreleasing *)error
{
    NSLog(@"Generating intermediate certificate");

    AutoPlatformMemory platformMemory;

    NSData * intermediate = nil;
    CHIP_ERROR err = CHIPOperationalCredentialsDelegate::GenerateIntermediateCertificate(
        rootKeypair, rootCertificate, intermediatePublicKey, issuerId, fabricId, &intermediate);
    if (error) {
        *error = [CHIPError errorForCHIPErrorCode:err];
    }

    if (err != CHIP_NO_ERROR) {
        NSLog(@"Generating intermediate certificate failed: %s", ErrorStr(err));
    }

    return intermediate;
}

+ (BOOL)keypairMatchesCertificate:(NSData *)certificate keypair:(id<CHIPKeypair>)keypair
{
    P256PublicKey keypairPubKey;
    CHIP_ERROR err = CHIPP256KeypairBridge::MatterPubKeyFromSecKeyRef(keypair.pubkey, &keypairPubKey);
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

@end
