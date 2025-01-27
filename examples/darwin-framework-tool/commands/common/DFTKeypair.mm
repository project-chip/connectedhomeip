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

#import "DFTKeypair.h"

#import "CHIPCommandStorageDelegate.h"
#import "ControllerStorage.h"

#define CAKeyTag "com.apple.matter.commissioner.ca.issuer.id"
#define KeySize "256"
#define CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES 16

// Note: The following constants are used for storing keychain and operational credentials.
// To maintain backward compatibility with existing keychain entries and preferences,
// these constants still mention "Chip Tool".
static NSString * const kKeychainLabel = @"Chip Tool Keypair";
static NSString * const kOperationalCredentialsIssuerKeypairStorage = @"ChipToolOpCredsCAKey";
static NSString * const kOperationalCredentialsIPK = @"ChipToolOpCredsIPK";

// Error Descriptions
NSString * const kErrorDomain = @"Error";
NSString * const kErrorFailedToStoreKeypairData = @"Failed to store keypair data in storage.";
NSString * const kErrorInvalidIPKData = @"Invalid IPK data.";
NSString * const kErrorFailedToGenerateIPK = @"Failed to generate random bytes for IPK.";
NSString * const kErrorFailedToStoreIPK = @"Failed to store IPK in storage.";
NSString * const kPublicKeyRetrievalFailureReason = @"Failed to retrieve the public key from the private key. This may occur if the private key was created without storing the corresponding public key in the keychain, or if the system cannot reconstruct the public key.";

@interface DFTKeypair ()
@property (readonly) SecKeyRef privateKey;
@property (readonly) SecKeyRef publicKey;
@property (readonly) NSData * ipk;
@end

@implementation DFTKeypair
+ (instancetype)createKeypairWithStorage:(id)storage error:(NSError * _Nullable __autoreleasing *)error
{
    __auto_type * keypair = [[self alloc] init];
    if (![keypair setupKeys:storage error:error] || ![keypair setupIPK:storage error:error]) {
        return nil;
    }

    return keypair;
}

- (NSData *)signMessageECDSA_DER:(NSData *)message
{
    if (!_privateKey) {
        NSLog(@"Error: Private key is not available for signing.");
        return nil;
    }

    CFErrorRef cfError = NULL;
    CFDataRef signatureData = SecKeyCreateSignature(_privateKey,
        kSecKeyAlgorithmECDSASignatureMessageX962SHA256,
        (__bridge CFDataRef) message,
        &cfError);

    if (!signatureData) {
        NSError * error = (__bridge_transfer NSError *) cfError;
        NSLog(@"Error: Failed to sign message: %@", error.localizedDescription);
        return nil;
    }

    return (__bridge_transfer NSData *) signatureData;
}

- (SecKeyRef)copyPublicKey
{
    if (_publicKey) {
        CFRetain(_publicKey);
        return _publicKey;
    }

    return nil;
}

- (BOOL)setupKeys:(id)storage error:(NSError * _Nonnull __autoreleasing *)error
{
    __auto_type * keypairData = [self _getValueForKeyWithStorage:storage key:kOperationalCredentialsIssuerKeypairStorage];
    return keypairData ? [self loadKeys:keypairData error:error] : [self createKeys:storage error:error];
}

- (BOOL)loadKeys:(NSData *)keypairData error:(NSError * _Nonnull __autoreleasing *)error
{
    NSDictionary * const attributes = @{
        (NSString *) kSecAttrKeyClass : (NSString *) kSecAttrKeyClassPrivate,
        (NSString *) kSecAttrKeyType : (NSString *) kSecAttrKeyTypeECSECPrimeRandom,
        (NSString *) kSecAttrKeySizeInBits : @KeySize,
        (NSString *) kSecAttrLabel : kKeychainLabel,
        (NSString *) kSecAttrApplicationTag : [@CAKeyTag dataUsingEncoding:NSUTF8StringEncoding],
    };

    CFErrorRef cfError = NULL;
    __auto_type * privateKey = SecKeyCreateWithData((__bridge CFDataRef) keypairData, (__bridge CFDictionaryRef) attributes, &cfError);
    if (!privateKey) {
        *error = (__bridge_transfer NSError *) cfError;
        return NO;
    }

    __auto_type * publicKey = SecKeyCopyPublicKey(privateKey);
    if (!publicKey) {
        CFRelease(privateKey);
        *error = [NSError errorWithDomain:kErrorDomain code:0 userInfo:@{ NSLocalizedDescriptionKey : kPublicKeyRetrievalFailureReason }];
        return NO;
    }

    _privateKey = privateKey;
    _publicKey = publicKey;
    return YES;
}

- (BOOL)createKeys:(id)storage error:(NSError * _Nonnull __autoreleasing *)error
{
    NSDictionary * const attributes = @{
        (NSString *) kSecAttrKeyType : (NSString *) kSecAttrKeyTypeECSECPrimeRandom,
        (NSString *) kSecAttrKeySizeInBits : @KeySize,
        (NSString *) kSecAttrLabel : kKeychainLabel,
        (NSString *) kSecAttrApplicationTag : [@CAKeyTag dataUsingEncoding:NSUTF8StringEncoding],
    };

    CFErrorRef cfError = NULL;
    __auto_type * privateKey = SecKeyCreateRandomKey((__bridge CFDictionaryRef) attributes, &cfError);
    if (!privateKey) {
        *error = (__bridge_transfer NSError *) cfError;
        return NO;
    }

    __auto_type * publicKey = SecKeyCopyPublicKey(privateKey);
    if (!publicKey) {
        CFRelease(privateKey);
        *error = [NSError errorWithDomain:kErrorDomain code:0 userInfo:@{ NSLocalizedDescriptionKey : kPublicKeyRetrievalFailureReason }];
        return NO;
    }

    __auto_type * keypairData = (__bridge_transfer NSData *) SecKeyCopyExternalRepresentation(privateKey, &cfError);
    if (!keypairData) {
        CFRelease(privateKey);
        CFRelease(publicKey);
        *error = (__bridge_transfer NSError *) cfError;
        return NO;
    }

    if (![self _setValueForKeyWithStorage:storage key:kOperationalCredentialsIssuerKeypairStorage value:keypairData]) {
        CFRelease(privateKey);
        CFRelease(publicKey);
        *error = [NSError errorWithDomain:kErrorDomain code:0 userInfo:@{ NSLocalizedDescriptionKey : kErrorFailedToStoreKeypairData }];
        return NO;
    }

    _privateKey = privateKey;
    _publicKey = publicKey;
    return YES;
}

- (BOOL)setupIPK:(id)storage error:(NSError * _Nonnull __autoreleasing *)error
{
    __auto_type * ipk = [self _getValueForKeyWithStorage:storage key:kOperationalCredentialsIPK];
    return ipk ? [self loadIPK:ipk error:error] : [self createIPK:storage error:error];
}

- (BOOL)loadIPK:(NSData *)ipk error:(NSError * _Nonnull __autoreleasing *)error
{
    if (ipk.length != CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES) {
        *error = [NSError errorWithDomain:kErrorDomain code:0 userInfo:@{ NSLocalizedDescriptionKey : kErrorInvalidIPKData }];
        return NO;
    }

    _ipk = ipk;
    return YES;
}

- (BOOL)createIPK:(id)storage error:(NSError * _Nonnull __autoreleasing *)error
{
    uint8_t tempIPK[CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];

    if (errSecSuccess != SecRandomCopyBytes(kSecRandomDefault, (sizeof tempIPK) / (sizeof tempIPK[0]), &tempIPK[0])) {
        *error = [NSError errorWithDomain:kErrorDomain code:0 userInfo:@{ NSLocalizedDescriptionKey : kErrorFailedToGenerateIPK }];
        return NO;
    }

    __auto_type * ipk = [NSData dataWithBytes:tempIPK length:(sizeof tempIPK)];
    if (![self _setValueForKeyWithStorage:storage key:kOperationalCredentialsIPK value:ipk]) {
        *error = [NSError errorWithDomain:kErrorDomain code:0 userInfo:@{ NSLocalizedDescriptionKey : kErrorFailedToStoreIPK }];
        return NO;
    }

    _ipk = ipk;
    return YES;
}

- (NSData *)getIPK
{
    return _ipk;
}

- (NSData *)_getValueForKeyWithStorage:(id)storage key:(NSString *)key
{
    if ([storage isKindOfClass:[CHIPToolPersistentStorageDelegate class]]) {
        return [storage storageDataForKey:key];
    } else if ([storage isKindOfClass:[ControllerStorage class]]) {
        return [storage valueForKey:key];
    }
    return nil;
}

- (BOOL)_setValueForKeyWithStorage:(id)storage key:(NSString *)key value:(NSData *)value
{
    if ([storage isKindOfClass:[CHIPToolPersistentStorageDelegate class]]) {
        return [storage setStorageData:value forKey:key];
    } else if ([storage isKindOfClass:[ControllerStorage class]]) {
        [storage storeValue:value forKey:key];
        return YES;
    }
    return NO;
}

- (void)dealloc
{
    if (_privateKey) {
        CFRelease(_privateKey);
    }

    if (_publicKey) {
        CFRelease(_publicKey);
    }
}

@end
