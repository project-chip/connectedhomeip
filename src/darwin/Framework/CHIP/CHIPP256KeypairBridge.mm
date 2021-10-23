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

#import "CHIPP256KeypairBridge.h"

#import <CommonCrypto/CommonDigest.h>
#import <Security/SecKey.h>
#include <string>

#import "CHIPKeypair.h"
#import "CHIPLogging.h"

using namespace chip::Crypto;

CHIP_ERROR CHIPP256KeypairBridge::Init(id<CHIPKeypair> keypair)
{
    mKeypair = keypair;
    return setPubkey();
}

CHIP_ERROR CHIPP256KeypairBridge::Initialize()
{
    if (!HasKeypair()) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return ([mKeypair initialize]) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR CHIPP256KeypairBridge::Serialize(P256SerializedKeypair & output) const
{
    if (!HasKeypair()) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR CHIPP256KeypairBridge::Deserialize(P256SerializedKeypair & input)
{
    if (!HasKeypair()) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR CHIPP256KeypairBridge::NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length)
{
    if (!HasKeypair()) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR CHIPP256KeypairBridge::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature)
{
    if (!HasKeypair()) {
        CHIP_LOG_ERROR("ECDSA sign msg failure: no keypair to sign with.");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    NSData * msgData = [NSData dataWithBytes:msg length:msg_length];
    NSMutableData * hashedData = [NSMutableData dataWithLength:CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(msgData.bytes, (CC_LONG) msgData.length, (unsigned char *) hashedData.mutableBytes);
    CHIP_LOG_DEBUG("Generated Msg hash, signing hash now");
    NSData * signature = [mKeypair ECDSA_sign_hash:hashedData];
    if (!signature) {
        CHIP_LOG_ERROR("ECDSA sign msg failure: no signature returned");
        return CHIP_ERROR_INTERNAL;
    }
    if (signature.length > out_signature.Capacity()) {
        CHIP_LOG_ERROR("ECDSA sign msg failure: unexpected signature size %tu vs %tu ", signature.length, out_signature.Capacity());
        return CHIP_ERROR_NO_MEMORY;
    }
    out_signature.SetLength(signature.length);
    std::memcpy(out_signature, signature.bytes, signature.length);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPP256KeypairBridge::ECDSA_sign_hash(const uint8_t * hash, size_t hash_length, P256ECDSASignature & out_signature)
{
    if (!HasKeypair()) {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    NSData * hashData = [NSData dataWithBytes:hash length:hash_length];
    NSData * signature = [mKeypair ECDSA_sign_hash:hashData];
    if (!signature) {
        CHIP_LOG_ERROR("ECDSA sign hash failure: no signature returned");
        return CHIP_ERROR_INTERNAL;
    }
    if (signature.length > out_signature.Capacity()) {
        CHIP_LOG_ERROR(
            "ECDSA sign hash failure: unexpected signature size %tu vs %tu ", signature.length, out_signature.Capacity());
        return CHIP_ERROR_NO_MEMORY;
    }
    out_signature.SetLength(signature.length);
    std::memcpy(out_signature, signature.bytes, signature.length);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPP256KeypairBridge::ECDH_derive_secret(
    const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    if (!HasKeypair()) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR CHIPP256KeypairBridge::setPubkey()
{
    SecKeyRef pubkeyRef = [mKeypair pubkey];
    if (!pubkeyRef) {
        CHIP_LOG_ERROR("Unable to initialize Pubkey");
        return CHIP_ERROR_INTERNAL;
    }

    NSData * pubkeyData = (__bridge_transfer NSData *) SecKeyCopyExternalRepresentation(pubkeyRef, nil);
    if (!pubkeyData) {
        CHIP_LOG_ERROR("Unable to copy external representation for pubkey ref, cannot initialize pubkey");
        return CHIP_ERROR_INTERNAL;
    }
    if (pubkeyData.length != kP256_PublicKey_Length) {
        CHIP_LOG_ERROR("Unexpected pubkey length, cannot initialize pubkey");
        return CHIP_ERROR_INTERNAL;
    }
    chip::FixedByteSpan<kP256_PublicKey_Length> pubkeyBytes((const uint8_t *) pubkeyData.bytes);
    mPubkey = P256PublicKey(pubkeyBytes);

    return CHIP_NO_ERROR;
}
