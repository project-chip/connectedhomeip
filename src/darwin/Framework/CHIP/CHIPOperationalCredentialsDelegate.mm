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

#include <core/CHIPTLV.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/TimeUtils.h>
#include <support/PersistentStorageMacros.h>
#include <support/SafeInt.h>

constexpr const char kOperationalCredentialsRootCertificateStorage[] = "MatterCARootCert";

using namespace chip;
using namespace TLV;

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

    CHIP_LOG_ERROR("CHIPOperationalCredentialsDelegate::init returning %s", chip::ErrorStr(err));
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

CHIP_ERROR CHIPOperationalCredentialsDelegate::LoadKeysFromKeyChain()
{
    const NSDictionary * query = @{
        (id) kSecClass : (id) kSecClassGenericPassword,
        (id) kSecAttrService : kCHIPCAKeyChainLabel,
        (id) kSecAttrSynchronizable : @YES,
        (id) kSecReturnData : @YES,
    };

    CFDataRef keyDataRef;
    OSStatus status = SecItemCopyMatching((CFDictionaryRef) query, (CFTypeRef *) &keyDataRef);
    if (status == errSecItemNotFound || keyDataRef == nil) {
        CHIP_LOG_ERROR("Did not find an existing key in the keychain");
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_LOG_ERROR("Found an existing keypair in the keychain");
    NSData * keyData = CFBridgingRelease(keyDataRef);

    NSData * keypairData = [[NSData alloc] initWithBase64EncodedData:keyData options:0];

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

CHIP_ERROR CHIPOperationalCredentialsDelegate::GenerateKeys()
{
    CHIP_LOG_ERROR("Generating keys for the CA");
    CHIP_ERROR errorCode = mIssuerKey.Initialize();
    if (errorCode != CHIP_NO_ERROR) {
        return errorCode;
    }

    chip::Crypto::P256SerializedKeypair serializedKey;
    errorCode = mIssuerKey.Serialize(serializedKey);

    NSData * keypairData = [NSData dataWithBytes:serializedKey.Bytes() length:serializedKey.Length()];

    const NSDictionary * addParams = @{
        (id) kSecClass : (id) kSecClassGenericPassword,
        (id) kSecAttrService : kCHIPCAKeyChainLabel,
        (id) kSecAttrSynchronizable : @YES,
        (id) kSecValueData : [keypairData base64EncodedDataWithOptions:0],
    };

    OSStatus status = SecItemAdd((__bridge CFDictionaryRef) addParams, NULL);
    // TODO: Enable SecItemAdd for Darwin unit tests
    if (status != errSecSuccess && !isRunningTests()) {
        NSLog(@"Failed in storing key : %d", status);
        return CHIP_ERROR_INTERNAL;
    }

    NSLog(@"Stored the keys");
    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::DeleteKeys()
{
    CHIP_LOG_ERROR("Deleting current CA keys");
    OSStatus status = noErr;

    const NSDictionary * deleteParams = @{
        (id) kSecClass : (id) kSecClassGenericPassword,
        (id) kSecAttrService : kCHIPCAKeyChainLabel,
        (id) kSecAttrSynchronizable : @YES,
    };

    status = SecItemDelete((__bridge CFDictionaryRef) deleteParams);
    if (status != errSecSuccess) {
        NSLog(@"Failed in deleting key : %d", status);
        return CHIP_ERROR_INTERNAL;
    }

    NSLog(@"Deleted the key");
    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPOperationalCredentialsDelegate::GenerateNOCChain(const chip::ByteSpan & csrElements,
    const chip::ByteSpan & attestationSignature, const chip::ByteSpan & DAC, const chip::ByteSpan & PAI, const chip::ByteSpan & PAA,
    chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> * onCompletion)
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

    chip::NodeId assignedId;
    if (mNodeIdRequested) {
        assignedId = mNextRequestedNodeId;
        mNodeIdRequested = false;
    } else {
        if (mDeviceBeingPaired == chip::kUndefinedNodeId) {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        assignedId = mDeviceBeingPaired;
    }

    chip::Credentials::X509CertRequestParams noc_request
        = { 1, mIssuerId, validityStart, validityEnd, true, mNextFabricId, true, assignedId };

    TLVReader reader;
    reader.Init(csrElements.data(), static_cast<uint32_t>(csrElements.size()));

    if (reader.GetType() == kTLVType_NotSpecified) {
        ReturnErrorOnFailure(reader.Next());
    }

    VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_WRONG_TLV_TYPE);
    VerifyOrReturnError(reader.GetTag() == AnonymousTag, CHIP_ERROR_UNEXPECTED_TLV_ELEMENT);

    TLVType containerType;
    ReturnErrorOnFailure(reader.EnterContainer(containerType));
    ReturnErrorOnFailure(reader.Next(kTLVType_ByteString, TLV::ContextTag(1)));

    ByteSpan csr(reader.GetReadPoint(), reader.GetLength());
    reader.ExitContainer(containerType);

    chip::Crypto::P256PublicKey pubkey;
    ReturnErrorOnFailure(chip::Crypto::VerifyCertificateSigningRequest(csr.data(), csr.size(), pubkey));

    NSMutableData * nocBuffer = [[NSMutableData alloc] initWithLength:chip::Controller::kMaxCHIPDERCertLength];
    uint8_t * noc = (uint8_t *) [nocBuffer mutableBytes];
    chip::MutableByteSpan nocCert(noc, chip::Controller::kMaxCHIPDERCertLength);

    ReturnErrorOnFailure(chip::Credentials::NewNodeOperationalX509Cert(
        noc_request, chip::Credentials::CertificateIssuerLevel::kIssuerIsRootCA, pubkey, mIssuerKey, nocCert));

    NSMutableData * rcacBuffer = [[NSMutableData alloc] initWithLength:chip::Controller::kMaxCHIPDERCertLength];
    uint16_t rcacLen = chip::Controller::kMaxCHIPDERCertLength;
    uint8_t * rcac = (uint8_t *) [rcacBuffer mutableBytes];
    MutableByteSpan rootCert(rcac, rcacLen);

    CHIP_ERROR err = CHIP_NO_ERROR;
    PERSISTENT_KEY_OP(mNextFabricId, kOperationalCredentialsRootCertificateStorage, key,
        err = mStorage->SyncGetKeyValue(key, rootCert.data(), rcacLen));

    if (err != CHIP_NO_ERROR) {
        chip::Credentials::X509CertRequestParams rcac_request
            = { 0, mIssuerId, validityStart, validityEnd, true, mNextFabricId, false, 0 };
        ReturnErrorOnFailure(chip::Credentials::NewRootX509Cert(rcac_request, mIssuerKey, rootCert));

        VerifyOrReturnError(CanCastTo<uint16_t>(rootCert.size()), CHIP_ERROR_INVALID_ARGUMENT);
        PERSISTENT_KEY_OP(mNextFabricId, kOperationalCredentialsRootCertificateStorage, key,
            err = mStorage->SyncSetKeyValue(key, rootCert.data(), static_cast<uint16_t>(rootCert.size())));
        ReturnErrorOnFailure(err);
    } else {
        rootCert.reduce_size(rootCertBufLen);
    }

    onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, nocCert, chip::ByteSpan(), rootCert);

    return CHIP_NO_ERROR;
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
