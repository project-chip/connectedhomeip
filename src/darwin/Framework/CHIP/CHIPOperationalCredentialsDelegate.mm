/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#import "CHIPOperationalCredentialsDelegate.h"

#import <Security/Security.h>

#include <Security/SecKey.h>

#import "CHIPLogging.h"

#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/TimeUtils.h>

static BOOL isRunningTests(void)
{
    NSDictionary * environment = [[NSProcessInfo processInfo] environment];
    return (environment[@"XCTestConfigurationFilePath"] != nil);
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::init(CHIPPersistentStorageDelegateBridge * storage)
{
    if (storage == nil) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mStorage = storage;

    CHIP_ERROR err = LoadKeysFromKeyChain();
    if (err != CHIP_NO_ERROR) {
        // Generate keys if keys could not be loaded
        err = GenerateKeys();
    }

    if (err == CHIP_NO_ERROR) {
        // If keys were loaded, or generated, let's get the certificate issuer ID

        // TODO - enable generating a random issuer ID and saving it in persistent storage
        // err = SetIssuerID(storage);
    }

    CHIP_LOG_ERROR("CHIPOperationalCredentialsDelegate::init returning %d", err);
    return err;
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::SetIssuerID(CHIPPersistentStorageDelegateBridge * storage)
{
    static const char * const CHIP_COMMISSIONER_CA_ISSUER_ID = "com.zigbee.chip.commissioner.ca.issuer.id";
    if (storage == nil) {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    uint16_t idStringLen = 16;
    char issuerIdString[idStringLen];
    if (CHIP_NO_ERROR != storage->SyncGetKeyValue(CHIP_COMMISSIONER_CA_ISSUER_ID, issuerIdString, idStringLen)) {
        mIssuerId = arc4random();
        CHIP_LOG_ERROR("Assigned %d certificate issuer ID to the commissioner", mIssuerId);
        storage->SyncSetKeyValue(CHIP_COMMISSIONER_CA_ISSUER_ID, &mIssuerId, sizeof(mIssuerId));
    } else {
        CHIP_LOG_ERROR("Found %d certificate issuer ID for the commissioner", mIssuerId);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::ConvertToP256Keypair(SecKeyRef keypair)
{
    NSData * keypairData = (__bridge_transfer NSData *) SecKeyCopyExternalRepresentation(keypair, nil);
    if (keypairData == nil) {
        NSLog(@"Failed in getting keypair data");
        return CHIP_ERROR_INTERNAL;
    }

    chip::Crypto::P256SerializedKeypair serialized;
    if ([keypairData length] != serialized.Capacity()) {
        NSLog(@"Keypair length %zu does not match expected length %zu", [keypairData length], serialized.Capacity());
        return CHIP_ERROR_INTERNAL;
    }

    std::memmove((uint8_t *) serialized, [keypairData bytes], [keypairData length]);
    serialized.SetLength([keypairData length]);

    CHIP_LOG_ERROR("Deserializing the key");
    return mIssuerKey.Deserialize(serialized);
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::LoadKeysFromKeyChain()
{
    SecKeyRef privateKey;

    const NSDictionary * query = @ {
        (id) kSecClass : (id) kSecClassKey,
        (id) kSecAttrKeyType : mKeyType,
        (id) kSecAttrKeySizeInBits : mKeySize,
        (id) kSecAttrLabel : kCHIPCAKeyLabel,
        (id) kSecAttrApplicationTag : kCHIPCAKeyTag,
        (id) kSecReturnRef : (id) kCFBooleanTrue
    };

    OSStatus status = SecItemCopyMatching((CFDictionaryRef) query, (CFTypeRef *) &privateKey);
    if (status == errSecItemNotFound || privateKey == nil) {
        CHIP_LOG_ERROR("Did not find an existing key in the keychain");
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_LOG_ERROR("Found an existing keypair in the keychain");
    return ConvertToP256Keypair(privateKey);
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::GenerateKeys()
{
    SecKeyRef publicKey;
    SecKeyRef privateKey;

    CHIP_LOG_ERROR("Generating keys for the CA");
    OSStatus status = noErr;

    const NSDictionary * keygenParams = @ {
        (id) kSecAttrKeyType : mKeyType,
        (id) kSecAttrKeySizeInBits : mKeySize,
        (id) kSecAttrLabel : kCHIPCAKeyLabel,
        (id) kSecAttrApplicationTag : kCHIPCAKeyTag,
    };

    status = SecKeyGeneratePair((__bridge CFDictionaryRef) keygenParams, &publicKey, &privateKey);
    if (status != noErr || publicKey == nil || privateKey == nil) {
        NSLog(@"Failed in keygen");
        return CHIP_ERROR_INTERNAL;
    }

    const NSDictionary * addParams = @ {
        (id) kSecClass : (id) kSecClassKey,
        (id) kSecAttrKeyType : mKeyType,
        (id) kSecAttrKeySizeInBits : mKeySize,
        (id) kSecAttrLabel : kCHIPCAKeyLabel,
        (id) kSecAttrApplicationTag : kCHIPCAKeyTag,
        (id) kSecValueRef : (__bridge id) privateKey,
    };

    status = SecItemAdd((__bridge CFDictionaryRef) addParams, NULL);
    // TODO: Enable SecItemAdd for Darwin unit tests
    if (status != errSecSuccess && !isRunningTests()) {
        NSLog(@"Failed in storing key : %d", status);
        return CHIP_ERROR_INTERNAL;
    }

    NSLog(@"Stored the keys");
    return ConvertToP256Keypair(privateKey);
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::DeleteKeys()
{
    CHIP_LOG_ERROR("Deleting current CA keys");
    OSStatus status = noErr;

    const NSDictionary * deleteParams = @ {
        (id) kSecClass : (id) kSecClassKey,
        (id) kSecAttrKeyType : mKeyType,
        (id) kSecAttrKeySizeInBits : mKeySize,
        (id) kSecAttrLabel : kCHIPCAKeyLabel,
        (id) kSecAttrApplicationTag : kCHIPCAKeyTag,
    };

    status = SecItemDelete((__bridge CFDictionaryRef) deleteParams);
    if (status != errSecSuccess) {
        NSLog(@"Failed in deleting key : %d", status);
        return CHIP_ERROR_INTERNAL;
    }

    NSLog(@"Deleted the key");
    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::GenerateNodeOperationalCertificate(const chip::PeerId & peerId,
    const chip::ByteSpan & csr, int64_t serialNumber, uint8_t * certBuf, uint32_t certBufSize, uint32_t & outCertLen)
{
    uint32_t validityStart, validityEnd;

    if (!ToChipEpochTime(0, validityStart)) {
        NSLog(@"Failed in computing certificate validity start date");
        return CHIP_ERROR_INTERNAL;
    }

    if (!ToChipEpochTime(kCertificateValiditySecs, validityEnd)) {
        NSLog(@"Failed in computing certificate validity end date");
        return CHIP_ERROR_INTERNAL;
    }

    chip::Credentials::X509CertRequestParams request
        = { serialNumber, mIssuerId, validityStart, validityEnd, true, peerId.GetFabricId(), true, peerId.GetNodeId() };

    chip::Crypto::P256PublicKey pubkey;
    CHIP_ERROR err = chip::Crypto::VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey);
    if (err != CHIP_NO_ERROR) {
        return err;
    }

    return chip::Credentials::NewNodeOperationalX509Cert(
        request, chip::Credentials::CertificateIssuerLevel::kIssuerIsRootCA, pubkey, mIssuerKey, certBuf, certBufSize, outCertLen);
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::GetRootCACertificate(
    chip::FabricId fabricId, uint8_t * certBuf, uint32_t certBufSize, uint32_t & outCertLen)
{
    // TODO: Don't generate root certificate unless there's none, or the current is expired.
    uint32_t validityStart, validityEnd;

    if (!ToChipEpochTime(0, validityStart)) {
        NSLog(@"Failed in computing certificate validity start date");
        return CHIP_ERROR_INTERNAL;
    }

    if (!ToChipEpochTime(kCertificateValiditySecs, validityEnd)) {
        NSLog(@"Failed in computing certificate validity end date");
        return CHIP_ERROR_INTERNAL;
    }

    chip::Credentials::X509CertRequestParams request = { 0, mIssuerId, validityStart, validityEnd, true, fabricId, false, 0 };

    return chip::Credentials::NewRootX509Cert(request, mIssuerKey, certBuf, certBufSize, outCertLen);
}

bool CHIPOperationalCredentialsDelegate::ToChipEpochTime(uint32_t offset, uint32_t & epoch)
{
    NSDate * date = [NSDate dateWithTimeIntervalSinceNow:offset];
    unsigned units = NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay | NSCalendarUnitHour | NSCalendarUnitMinute
        | NSCalendarUnitSecond;
    NSCalendar * calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    NSDateComponents * components = [calendar components:units fromDate:date];

    uint16_t year = static_cast<uint16_t>([components year]);
    uint8_t month = static_cast<uint8_t>([components month]);
    uint8_t day = static_cast<uint8_t>([components day]);
    uint8_t hour = static_cast<uint8_t>([components hour]);
    uint8_t minute = static_cast<uint8_t>([components minute]);
    uint8_t second = static_cast<uint8_t>([components second]);
    return chip::CalendarToChipEpochTime(year, month, day, hour, minute, second, epoch);
}
