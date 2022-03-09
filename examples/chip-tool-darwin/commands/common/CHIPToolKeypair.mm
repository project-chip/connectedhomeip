#import "CHIPToolKeypair.h"
#include <CHIP/CHIP.h>
#import <CHIP/CHIPKeypair.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/asn1/ASN1.h>
#include <lib/support/Base64.h>
#include <stddef.h>
#include <string>

#define CHIPPlugin_CAKeyTag "com.apple.matter.commissioner.ca.issuer.id"
#define Public_KeySize "256"

static NSString * const kCHIPToolKeychainLabel = @"Chip Tool Keypair";
static NSString * const kOperationalCredentialsIssuerKeypairStorage = @"ChipToolOpCredsCAKey";

std::string StringToBase64(const std::string & value)
{
    std::unique_ptr<char[]> buffer(new char[BASE64_ENCODED_LEN(value.length())]);

    uint32_t len = chip::Base64Encode32(
        reinterpret_cast<const uint8_t *>(value.data()), static_cast<uint32_t>(value.length()), buffer.get());
    if (len == UINT32_MAX) {
        return "";
    }

    return std::string(buffer.get(), len);
}

std::string Base64ToString(const std::string & b64Value)
{
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[BASE64_MAX_DECODED_LEN(b64Value.length())]);

    uint32_t len = chip::Base64Decode32(b64Value.data(), static_cast<uint32_t>(b64Value.length()), buffer.get());
    if (len == UINT32_MAX) {
        return "";
    }

    return std::string(reinterpret_cast<const char *>(buffer.get()), len);
}

@interface CHIPToolKeypair ()
@property (nonatomic) chip::Crypto::P256Keypair mKeyPair;
@property (nonatomic) chip::Crypto::P256Keypair mIssuer;
@property (atomic) uint32_t mNow;
@end

@implementation CHIPToolKeypair
- (instancetype)init
{
    if (self = [super init]) {
        _mNow = 0;
    }
    return self;
}

- (BOOL)initialize
{
    return _mKeyPair.Initialize() == CHIP_NO_ERROR;
}

- (NSData *)ECDSA_sign_hash:(NSData *)hash
{
    chip::Crypto::P256ECDSASignature signature;
    NSData * out_signature;
    CHIP_ERROR signing_error = _mKeyPair.ECDSA_sign_hash((const uint8_t *) [hash bytes], (const size_t)[hash length], signature);
    if (signing_error != CHIP_NO_ERROR)
        return nil;
    out_signature = [NSData dataWithBytes:signature.Bytes() length:signature.Length()];
    return out_signature;
}

- (SecKeyRef)pubkey
{
    chip::Crypto::P256PublicKey publicKey = _mKeyPair.Pubkey();
    NSData * publicKeyNSData = [NSData dataWithBytes:publicKey.Bytes() length:publicKey.Length()];
    NSDictionary * attributes = @{
        (__bridge NSString *) kSecAttrKeyClass : (__bridge NSString *) kSecAttrKeyClassPublic,
        (NSString *) kSecAttrKeyType : (NSString *) kSecAttrKeyTypeECSECPrimeRandom,
        (NSString *) kSecAttrKeySizeInBits : @Public_KeySize,
        (NSString *) kSecAttrLabel : kCHIPToolKeychainLabel,
        (NSString *) kSecAttrApplicationTag : @CHIPPlugin_CAKeyTag,
    };
    return SecKeyCreateWithData((__bridge CFDataRef) publicKeyNSData, (__bridge CFDictionaryRef) attributes, NULL);
}

- (CHIP_ERROR)Deserialize:(chip::Crypto::P256SerializedKeypair &)input
{
    return _mKeyPair.Deserialize(input);
}

- (CHIP_ERROR)Serialize:(chip::Crypto::P256SerializedKeypair &)output
{
    return _mKeyPair.Serialize(output);
}

- (CHIP_ERROR)createOrLoadKeys:(CHIPToolPersistentStorageDelegate *)storage
{
    chip::ASN1::ASN1UniversalTime effectiveTime;
    chip::Crypto::P256SerializedKeypair serializedKey;
    NSString * value;
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initializing the default start validity to start of 2021. The default validity duration is 10 years.
    CHIP_ZERO_AT(effectiveTime);
    effectiveTime.Year = 2021;
    effectiveTime.Month = 1;
    effectiveTime.Day = 1;
    ReturnErrorOnFailure(chip::Credentials::ASN1ToChipEpochTime(effectiveTime, _mNow));

    uint16_t keySize = static_cast<uint16_t>(serializedKey.Capacity());

    value = [storage CHIPGetKeyValue:kOperationalCredentialsIssuerKeypairStorage];
    err = [self decodeNSStringWithValue:value serializedKey:serializedKey];
    serializedKey.SetLength(keySize);

    if (err != CHIP_NO_ERROR) {
        // Storage doesn't have an existing keypair. Let's create one and add it to the storage.
        if (![self initialize]) {
            return CHIP_ERROR_INTERNAL;
        }
        ReturnErrorOnFailure([self Serialize:serializedKey]);

        keySize = static_cast<uint16_t>(serializedKey.Capacity());
        std::string serializeString(serializedKey.Bytes(), serializedKey.Bytes() + serializedKey.Capacity());
        std::string base64Value = StringToBase64(serializeString);
        NSString * valueString = [NSString stringWithUTF8String:base64Value.c_str()];
        [storage CHIPSetKeyValue:kOperationalCredentialsIssuerKeypairStorage value:valueString];
    } else {
        ReturnErrorOnFailure([self Deserialize:serializedKey]);
    }
    return err;
}

- (CHIP_ERROR)decodeNSStringWithValue:(NSString *)value serializedKey:(chip::Crypto::P256SerializedKeypair &)serializedKey
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t keySize = static_cast<uint16_t>(serializedKey.Capacity());
    if (value != nil) {
        std::string decoded = Base64ToString([value UTF8String]);

        if (decoded.length() > UINT16_MAX) {
            err = CHIP_ERROR_BUFFER_TOO_SMALL;
        } else {
            if (serializedKey != nullptr) {
                memcpy(serializedKey, decoded.data(), std::min<size_t>(decoded.length(), keySize));
                if (keySize < decoded.length()) {
                    err = CHIP_ERROR_BUFFER_TOO_SMALL;
                }
            } else {
                err = CHIP_ERROR_NO_MEMORY;
            }
            keySize = static_cast<uint16_t>(decoded.length());
        }
    } else {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    serializedKey.SetLength(keySize);
    return err;
}

@end
