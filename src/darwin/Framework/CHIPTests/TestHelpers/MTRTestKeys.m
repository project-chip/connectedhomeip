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

#import "MTRTestKeys.h"
#import <Security/SecKey.h>

@interface MTRTestKeys ()
@property (readonly) SecKeyRef privateKey;
@property (readonly) SecKeyRef publicKey;
@end

@implementation MTRTestKeys

@synthesize publicKey = _publicKey, privateKey = _privateKey, ipk = _ipk;

- (NSData *)publicKeyData
{
    return (__bridge_transfer NSData *) SecKeyCopyExternalRepresentation([self publicKey], nil);
}

- (SecKeyRef)copyPublicKey
{
    CFRetain(_publicKey);
    return _publicKey;
}

- (instancetype)init
{
    if (!(self = [super init])) {
        return nil;
    }

    // Generate an IPK.  For now, hardcoded to 16 bytes until the
    // framework exposes this constant.
    const size_t ipk_size = 16;
    NSMutableData * ipkData = [NSMutableData dataWithLength:ipk_size];
    if (ipkData == nil) {
        return nil;
    }

    int status = SecRandomCopyBytes(kSecRandomDefault, ipk_size, [ipkData mutableBytes]);
    if (status != errSecSuccess) {
        NSLog(@"Failed to generate IPK");
        return nil;
    }
    _ipk = ipkData;

    // Generate a keypair.  For now harcoded to 256 bits until the framework exposes this constant.
    const size_t keySizeInBits = 256;
    const NSDictionary * keygenParams = @{
        (__bridge NSString *) kSecAttrKeyClass : (__bridge NSString *) kSecAttrKeyClassPrivate,
        (__bridge NSString *) kSecAttrKeyType : (__bridge NSString *) kSecAttrKeyTypeECSECPrimeRandom,
        (__bridge NSString *) kSecAttrKeySizeInBits : @(keySizeInBits),
        (__bridge NSString *) kSecAttrIsPermanent : @(NO)
    };

    CFErrorRef cfError = NULL;
    _privateKey = SecKeyCreateRandomKey(
        (__bridge CFDictionaryRef) keygenParams,
        &cfError);

    if (!_privateKey) {
        NSLog(@"Failed to generate private key: %@", (__bridge_transfer NSError *) cfError);
        return nil;
    }

    _publicKey = SecKeyCopyPublicKey(_privateKey);

    _signatureCount = 0;

    return self;
}

- (NSData *)signMessageECDSA_DER:(NSData *)message
{
    ++_signatureCount;

    CFErrorRef cfError = NULL;
    CFDataRef cfData = SecKeyCreateSignature(
        _privateKey,
        kSecKeyAlgorithmECDSASignatureMessageX962SHA256,
        (__bridge CFDataRef) message,
        &cfError);

    if (!cfData) {
        NSLog(@"Failed to sign cert: %@", (__bridge_transfer NSError *) cfError);
        return nil;
    }

    return (__bridge_transfer NSData *) cfData;
}

- (void)dealloc
{
    if (_publicKey) {
        CFRelease(_publicKey);
    }

    if (_privateKey) {
        CFRelease(_privateKey);
    }
}
@end
