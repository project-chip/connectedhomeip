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

#import "FabricKeys.h"

#import <Security/SecKey.h>

@interface FabricKeys ()
@property (readonly) SecKeyRef privateKey;
@property (readonly) SecKeyRef publicKey;
@end

static const NSString * MTRIPKKeyChainLabel = @"matter-tool.nodeopcerts.IPK:0";
static const NSString * MTRCAKeyChainLabel = @"matter-tool.nodeopcerts.CA:0";

@implementation FabricKeys

+ (NSDictionary *)ipkParams
{
    return @{
        (__bridge NSString *) kSecClass : (__bridge NSString *) kSecClassKey,
        (__bridge NSString *) kSecAttrApplicationLabel : MTRIPKKeyChainLabel,
        (__bridge NSString *) kSecAttrKeyClass : (__bridge NSString *) kSecAttrKeyClassSymmetric,
    };
}

+ (NSData *)loadIPK
{
    NSMutableDictionary * query = [[NSMutableDictionary alloc] initWithDictionary:[FabricKeys ipkParams]];
    query[(__bridge NSString *) kSecReturnData] = @(YES);

    // The CFDataRef we get from SecItemCopyMatching allocates its buffer in a
    // way that zeroes it when deallocated.
    CFDataRef keyDataRef;
    OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef) query, (CFTypeRef *) &keyDataRef);
    if (status != errSecSuccess || keyDataRef == nil) {
        NSLog(@"Did not find IPK in the keychain");
        return nil;
    }

    NSLog(@"Found an existing IPK in the keychain");
    NSData * keyData = CFBridgingRelease(keyDataRef);

    return [[NSData alloc] initWithBase64EncodedData:keyData options:0];
}

+ (NSData *)generateIPK
{
    NSMutableDictionary * query = [[NSMutableDictionary alloc] initWithDictionary:[FabricKeys ipkParams]];

    // First, delete any existing item, since otherwise trying to add the new item
    // later will fail.  Ignore delete failure, since we might not have had the
    // item at all.
    SecItemDelete((__bridge CFDictionaryRef) query);

    // Generate an IPK.  For now, hardcoded to 16 bytes until the
    // framework exposes this constant.
    const size_t ipk_size = 16;
    NSMutableData * ipkData = [NSMutableData dataWithLength:ipk_size];
    if (ipkData == nil) {
        return nil;
    }

    int status = SecRandomCopyBytes(kSecRandomDefault, ipk_size, [ipkData mutableBytes]);
    if (status != errSecSuccess) {
        NSLog(@"Failed to generate IPK : %d", status);
        return nil;
    }

    query[(__bridge NSString *) kSecValueData] = [ipkData base64EncodedDataWithOptions:0];

    OSStatus addStatus = SecItemAdd((__bridge CFDictionaryRef) query, NULL);
    if (addStatus != errSecSuccess) {
        NSLog(@"Failed to store IPK : %d", addStatus);
        return nil;
    }

    return ipkData;
}

+ (NSDictionary *)privateKeyParams
{
    return @{
        (__bridge NSString *) kSecClass : (__bridge NSString *) kSecClassKey,
        (__bridge NSString *) kSecAttrApplicationLabel : MTRCAKeyChainLabel,
        // We're storing a base-64 encoding of some opaque thing that represents
        // our keypair.  It's not really a public or private key; claim it's a
        // symmetric key.
        (__bridge NSString *) kSecAttrKeyClass : (__bridge NSString *) kSecAttrKeyClassSymmetric,
    };
}

+ (NSDictionary *)privateKeyCreationParams
{
    // For now harcoded to 256 bits until the framework exposes this constant.
    const size_t keySizeInBits = 256;

    return @{
        (__bridge NSString *) kSecAttrKeyClass : (__bridge NSString *) kSecAttrKeyClassPrivate,
        (__bridge NSString *) kSecAttrKeyType : (__bridge NSString *) kSecAttrKeyTypeECSECPrimeRandom,
        (__bridge NSString *) kSecAttrKeySizeInBits : @(keySizeInBits),
        (__bridge NSString *) kSecAttrIsPermanent : @(NO)
    };
}

+ (SecKeyRef)loadCAPrivateKey
{
    NSMutableDictionary * query = [[NSMutableDictionary alloc] initWithDictionary:[FabricKeys privateKeyParams]];
    query[(__bridge NSString *) kSecReturnData] = @(YES);

    // The CFDataRef we get from SecItemCopyMatching allocates its buffer in a
    // way that zeroes it when deallocated.
    CFDataRef keyDataRef;
    OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef) query, (CFTypeRef *) &keyDataRef);
    if (status != errSecSuccess || keyDataRef == nil) {
        NSLog(@"Did not find CA key in the keychain");
        return NULL;
    }

    NSLog(@"Found an existing CA key in the keychain");
    NSData * encodedKey = CFBridgingRelease(keyDataRef);

    NSData * keyData = [[NSData alloc] initWithBase64EncodedData:encodedKey options:0];
    if (keyData == nil) {
        NSLog(@"Could not base64-decode CA key");
        return NULL;
    }

    CFErrorRef cfError = NULL;
    SecKeyRef key = SecKeyCreateWithData(
        (__bridge CFDataRef) keyData,
        (__bridge CFDictionaryRef)[FabricKeys privateKeyCreationParams],
        &cfError);

    if (!key) {
        NSLog(@"Could not reconstruct private key %@", (__bridge_transfer NSError *) cfError);
        return NULL;
    }

    return key;
}

+ (SecKeyRef)generateCAPrivateKey
{
    NSMutableDictionary * query = [[NSMutableDictionary alloc] initWithDictionary:[FabricKeys privateKeyParams]];

    // First, delete any existing item, since otherwise trying to add the new item
    // later will fail.  Ignore delete failure, since we might not have had the
    // item at all.
    SecItemDelete((__bridge CFDictionaryRef) query);

    CFErrorRef cfError = NULL;
    SecKeyRef key = SecKeyCreateRandomKey(
        (__bridge CFDictionaryRef)[FabricKeys privateKeyCreationParams],
        &cfError);

    if (!key) {
        NSLog(@"Could not generate private key: %@", (__bridge_transfer NSError *) cfError);
        return NULL;
    }

    NSData * keyData = (__bridge_transfer NSData *) SecKeyCopyExternalRepresentation(key, &cfError);
    if (!keyData) {
        NSLog(@"Could not get key external representation: %@", (__bridge_transfer NSError *) cfError);
        CFRelease(key);
        return NULL;
    }

    query[(__bridge NSString *) kSecValueData] = [keyData base64EncodedDataWithOptions:0];

    OSStatus status = SecItemAdd((__bridge CFDictionaryRef) query, NULL);
    if (status != errSecSuccess) {
        NSLog(@"Failed to store private key : %d", status);
        CFRelease(key);
        return NULL;
    }

    return key;
}

- (instancetype)init
{
    if (!(self = [super init])) {
        return nil;
    }

    if (!(_ipk = [FabricKeys loadIPK])) {
        if (!(_ipk = [FabricKeys generateIPK])) {
            return nil;
        }
    }

    if (!(_privateKey = [FabricKeys loadCAPrivateKey])) {
        if (!(_privateKey = [FabricKeys generateCAPrivateKey])) {
            return nil;
        }
    }

    _publicKey = SecKeyCopyPublicKey(_privateKey);
    return self;
}

- (NSData *)signMessageECDSA_DER:(NSData *)message
{
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
