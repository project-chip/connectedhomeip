#import "CHIPToolKeypair.h"
#import <Matter/Matter.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/asn1/ASN1.h>
#include <stddef.h>

#define CHIPPlugin_CAKeyTag "com.apple.matter.commissioner.ca.issuer.id"
#define Public_KeySize "256"

static NSString * const kCHIPToolKeychainLabel = @"Chip Tool Keypair";
static NSString * const kOperationalCredentialsIssuerKeypairStorage = @"ChipToolOpCredsCAKey";
static NSString * const kOperationalCredentialsIPK = @"ChipToolOpCredsIPK";

@implementation CHIPToolKeypair {
    chip::Crypto::P256Keypair _mKeyPair;
    chip::Crypto::P256Keypair _mIssuer;
    NSData * _ipk;
    uint32_t _mNow;
    SecKeyRef _mPublicKey;
}

- (instancetype)init
{
    if (self = [super init]) {
        _mNow = 0;
    }
    return self;
}

- (BOOL)initialize
{
    return _mKeyPair.Initialize(chip::Crypto::ECPKeyTarget::ECDSA) == CHIP_NO_ERROR;
}

- (NSData *)signMessageECDSA_RAW:(NSData *)message
{
    chip::Crypto::P256ECDSASignature signature;
    NSData * out_signature;
    CHIP_ERROR signing_error = _mKeyPair.ECDSA_sign_msg((const uint8_t *) [message bytes], (size_t)[message length], signature);
    if (signing_error != CHIP_NO_ERROR)
        return nil;
    out_signature = [NSData dataWithBytes:signature.Bytes() length:signature.Length()];
    return out_signature;
}

- (SecKeyRef)publicKey
{
    if (_mPublicKey == nil) {
        chip::Crypto::P256PublicKey publicKey = _mKeyPair.Pubkey();
        NSData * publicKeyNSData = [NSData dataWithBytes:publicKey.Bytes() length:publicKey.Length()];
        NSDictionary * attributes = @{
            (__bridge NSString *) kSecAttrKeyClass : (__bridge NSString *) kSecAttrKeyClassPublic,
            (NSString *) kSecAttrKeyType : (NSString *) kSecAttrKeyTypeECSECPrimeRandom,
            (NSString *) kSecAttrKeySizeInBits : @Public_KeySize,
            (NSString *) kSecAttrLabel : kCHIPToolKeychainLabel,
            (NSString *) kSecAttrApplicationTag : @CHIPPlugin_CAKeyTag,
        };
        _mPublicKey = SecKeyCreateWithData((__bridge CFDataRef) publicKeyNSData, (__bridge CFDictionaryRef) attributes, nullptr);
    }
    return _mPublicKey;
}

- (CHIP_ERROR)Deserialize:(chip::Crypto::P256SerializedKeypair &)input
{
    return _mKeyPair.Deserialize(input);
}

- (CHIP_ERROR)Serialize:(chip::Crypto::P256SerializedKeypair &)output
{
    return _mKeyPair.Serialize(output);
}

- (NSData *)getIPK
{
    return _ipk;
}

- (CHIP_ERROR)createOrLoadKeys:(CHIPToolPersistentStorageDelegate *)storage
{
    chip::ASN1::ASN1UniversalTime effectiveTime;
    chip::Crypto::P256SerializedKeypair serializedKey;
    NSData * value;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initializing the default start validity to start of 2021. The default validity duration is 10 years.
    CHIP_ZERO_AT(effectiveTime);
    effectiveTime.Year = 2021;
    effectiveTime.Month = 1;
    effectiveTime.Day = 1;
    ReturnErrorOnFailure(chip::Credentials::ASN1ToChipEpochTime(effectiveTime, _mNow));

    value = [storage storageDataForKey:kOperationalCredentialsIssuerKeypairStorage];
    err = [self initSerializedKeyFromValue:value serializedKey:serializedKey];

    if (err != CHIP_NO_ERROR) {
        // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
        if (![self initialize]) {
            return CHIP_ERROR_INTERNAL;
        }
        ReturnErrorOnFailure([self Serialize:serializedKey]);

        NSData * valueData = [NSData dataWithBytes:serializedKey.Bytes() length:serializedKey.Length()];
        [storage setStorageData:valueData forKey:kOperationalCredentialsIssuerKeypairStorage];
    } else {
        ReturnErrorOnFailure([self Deserialize:serializedKey]);
    }

    NSData * ipk = [storage storageDataForKey:kOperationalCredentialsIPK];
    if (ipk == nil) {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if (err != CHIP_NO_ERROR) {
        uint8_t tempIPK[chip::Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];

        ReturnLogErrorOnFailure(chip::Crypto::DRBG_get_bytes(tempIPK, sizeof(tempIPK)));

        _ipk = [NSData dataWithBytes:tempIPK length:sizeof(tempIPK)];
        [storage setStorageData:_ipk forKey:kOperationalCredentialsIPK];
    } else {
        _ipk = ipk;
    }

    return CHIP_NO_ERROR;
}

- (CHIP_ERROR)initSerializedKeyFromValue:(NSData *)value serializedKey:(chip::Crypto::P256SerializedKeypair &)serializedKey
{
    if (value == nil) {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    if (serializedKey.Capacity() < [value length]) {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    memcpy(serializedKey.Bytes(), [value bytes], [value length]);
    serializedKey.SetLength([value length]);
    return CHIP_NO_ERROR;
}

- (void)dealloc
{
    if (_mPublicKey) {
        CFRelease(_mPublicKey);
    }
}

@end
