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
#import "NSDataSpanConversion.h"

#import <Security/SecKey.h>
#include <string>

#import "CHIPKeypair.h"
#import "CHIPLogging.h"

using namespace chip::Crypto;

CHIP_ERROR CHIPP256KeypairBridge::Init(id<CHIPKeypair> keypair)
{
    if (![keypair respondsToSelector:@selector(ECDSA_sign_message_DER:)]
        && ![keypair respondsToSelector:@selector(ECDSA_sign_message_raw:)]) {
        // Not a valid CHIPKeypair implementation.
        NSLog(@"Keypair does not support message signing");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mKeypair = keypair;
    return setPubkey();
}

CHIP_ERROR CHIPP256KeypairBridge::Initialize()
{
    if (!HasKeypair()) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Our keypair is always initialized.
    return CHIP_NO_ERROR;
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

CHIP_ERROR CHIPP256KeypairBridge::NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const
{
    if (!HasKeypair()) {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR CHIPP256KeypairBridge::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const
{
    if (!HasKeypair()) {
        CHIP_LOG_ERROR("ECDSA sign msg failure: no keypair to sign with.");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    NSData * msgData = [NSData dataWithBytes:msg length:msg_length];
    NSData * signature;
    if ([mKeypair respondsToSelector:@selector(ECDSA_sign_message_DER:)]) {
        signature = [mKeypair ECDSA_sign_message_DER:msgData];
        if (!signature) {
            CHIP_LOG_ERROR("ECDSA sign msg failure: no signature returned");
            return CHIP_ERROR_INTERNAL;
        }

        uint8_t buf[kP256_ECDSA_Signature_Length_Raw];
        chip::MutableByteSpan rawSignature(buf);

        CHIP_ERROR err = EcdsaAsn1SignatureToRaw(kP256_FE_Length, AsByteSpan(signature), rawSignature);
        if (err != CHIP_NO_ERROR) {
            CHIP_LOG_ERROR("Converting ASN.1 DER signature to raw form failed: %s", chip::ErrorStr(err));
            return err;
        }

        signature = AsData(rawSignature);
        if (!signature) {
            CHIP_LOG_ERROR("Failed to create NSData for raw signature");
            return CHIP_ERROR_INTERNAL;
        }
    } else {
        signature = [mKeypair ECDSA_sign_message_raw:msgData];
        if (!signature) {
            CHIP_LOG_ERROR("ECDSA sign msg failure: no signature returned");
            return CHIP_ERROR_INTERNAL;
        }
    }
    if (signature.length > out_signature.Capacity()) {
        CHIP_LOG_ERROR("ECDSA sign msg failure: unexpected signature size %llu vs %llu", static_cast<uint64_t>(signature.length),
            static_cast<uint64_t>(out_signature.Capacity()));
        return CHIP_ERROR_NO_MEMORY;
    }
    out_signature.SetLength(signature.length);
    std::memcpy(out_signature.Bytes(), signature.bytes, signature.length);
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

CHIP_ERROR CHIPP256KeypairBridge::setPubkey() { return MatterPubKeyFromSecKeyRef([mKeypair pubkey], &mPubkey); }

CHIP_ERROR CHIPP256KeypairBridge::MatterPubKeyFromSecKeyRef(SecKeyRef pubkeyRef, P256PublicKey * matterPubKey)
{
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
    *matterPubKey = pubkeyBytes;

    return CHIP_NO_ERROR;
}
