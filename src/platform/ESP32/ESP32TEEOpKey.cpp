/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

// The feature depends only on ESP-TEE secure storage: keys are generated there and all
// private-key operations (CSR self-signature, CASE signatures) use the raw secure-storage
// ECDSA sign primitive. It deliberately avoids the mbedTLS TEE-pk wrapper. These sources are
// compiled only when chip_enable_esp32_tee (set from CONFIG_SECURE_ENABLE_TEE) is set.

#include <platform/ESP32/ESP32TEEOpKey.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <esp_idf_version.h>
#include <esp_tee_sec_storage.h>

#include <cstring>

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

constexpr size_t kP256FieldLength = 32; // secp256r1 coordinate / scalar length

esp_tee_sec_storage_key_cfg_t MakeKeyCfg(const char * keyId)
{
    esp_tee_sec_storage_key_cfg_t cfg = {};
    cfg.id                            = keyId;
    cfg.type                          = ESP_SEC_STG_KEY_ECDSA_SECP256R1;
    cfg.flags                         = SEC_STORAGE_FLAG_NONE;
    return cfg;
}

} // namespace

CHIP_ERROR ESP32TEEOpKeyGenerate(const char * keyId)
{
    VerifyOrReturnError(keyId != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    esp_tee_sec_storage_key_cfg_t cfg = MakeKeyCfg(keyId);
    esp_err_t err                     = esp_tee_sec_storage_gen_key(&cfg);
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL, ChipLogError(Crypto, "gen_key(%s) failed: %d", keyId, err));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32TEEOpKeyGetPublicKey(const char * keyId, Crypto::P256PublicKey & outPublicKey)
{
    VerifyOrReturnError(keyId != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    esp_tee_sec_storage_key_cfg_t cfg     = MakeKeyCfg(keyId);
    esp_tee_sec_storage_ecdsa_pubkey_t pk = {};
    esp_err_t err                         = esp_tee_sec_storage_ecdsa_get_pubkey(&cfg, &pk);
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INVALID_KEY_ID, ChipLogError(Crypto, "get_pubkey(%s) failed: %d", keyId, err));

    // P256PublicKey holds the uncompressed point: 0x04 || X || Y.
    uint8_t * dst = outPublicKey.Bytes();
    dst[0]        = 0x04;
    memcpy(dst + 1, pk.pub_x, kP256FieldLength);
    memcpy(dst + 1 + kP256FieldLength, pk.pub_y, kP256FieldLength);
    return CHIP_NO_ERROR;
}

namespace {

// Minimal DER INTEGER encoding of a 32-byte big-endian value into @p out (needs >= 35 bytes:
// tag + length + optional 0x00 sign-padding + up to 32 magnitude bytes). Strips leading zero
// bytes and prepends 0x00 when the top bit is set, so the value stays positive.
size_t EncodeDerInteger(const uint8_t * val, uint8_t * out)
{
    size_t i = 0;
    while ((i < kP256FieldLength - 1) && (val[i] == 0))
    {
        i++;
    }
    const size_t magLen = kP256FieldLength - i;
    const bool pad      = (val[i] & 0x80) != 0;
    out[0]              = 0x02; // INTEGER
    out[1]              = static_cast<uint8_t>(magLen + (pad ? 1 : 0));
    size_t p            = 2;
    if (pad)
    {
        out[p++] = 0x00;
    }
    memcpy(out + p, val + i, magLen);
    return p + magLen;
}

} // namespace

// Builds a PKCS#10 CSR without the mbedTLS pk wrapper: the public key comes from the TEE and
// the CertificationRequestInfo is signed with the same raw secure-storage ECDSA primitive used
// for CASE. For a P-256 key every ASN.1 element except the key and signature is fixed-shape.
CHIP_ERROR ESP32TEEOpKeyNewCSR(const char * keyId, MutableByteSpan & outCsr)
{
    VerifyOrReturnError(keyId != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    Crypto::P256PublicKey pub; // uncompressed: 0x04 || X || Y (65 bytes)
    ReturnErrorOnFailure(ESP32TEEOpKeyGetPublicKey(keyId, pub));
    VerifyOrReturnError(pub.Length() == 1 + 2 * kP256FieldLength, CHIP_ERROR_INTERNAL);

    // SubjectPublicKeyInfo for P-256: fixed 26-byte prefix + the 65-byte public key.
    static const uint8_t kSpkiPrefix[] = { 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01,
                                           0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00 };
    // Subject Name "O=CSR" (matches the software keystore; mbedTLS rejects an empty subject).
    static const uint8_t kSubject[] = { 0x30, 0x0e, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03,
                                        0x55, 0x04, 0x0a, 0x0c, 0x03, 0x43, 0x53, 0x52 };
    static const uint8_t kVersion[] = { 0x02, 0x01, 0x00 }; // INTEGER 0
    static const uint8_t kAttrs[]   = { 0xa0, 0x00 };       // attributes [0] IMPLICIT, empty
    static const uint8_t kSigAlg[]  = {
        0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02
    }; // ecdsa-with-SHA256

    // CertificationRequestInfo body (its content, without the outer SEQUENCE header).
    uint8_t body[128];
    size_t n = 0;
    memcpy(body + n, kVersion, sizeof(kVersion));
    n += sizeof(kVersion);
    memcpy(body + n, kSubject, sizeof(kSubject));
    n += sizeof(kSubject);
    memcpy(body + n, kSpkiPrefix, sizeof(kSpkiPrefix));
    n += sizeof(kSpkiPrefix);
    memcpy(body + n, pub.ConstBytes(), pub.Length());
    n += pub.Length();
    memcpy(body + n, kAttrs, sizeof(kAttrs));
    n += sizeof(kAttrs);
    VerifyOrReturnError(n < 0x80, CHIP_ERROR_INTERNAL); // fixed size (112) fits a 1-byte length

    uint8_t cri[130];
    cri[0] = 0x30;
    cri[1] = static_cast<uint8_t>(n);
    memcpy(cri + 2, body, n);
    const size_t criLen = n + 2;

    // Sign the CertificationRequestInfo inside the TEE.
    Crypto::P256ECDSASignature rawSig;
    ReturnErrorOnFailure(ESP32TEEOpKeySign(keyId, ByteSpan(cri, criLen), rawSig));
    VerifyOrReturnError(rawSig.Length() == 2 * kP256FieldLength, CHIP_ERROR_INTERNAL);

    // signature: BIT STRING wrapping ECDSA-Sig-Value ::= SEQUENCE { INTEGER r, INTEGER s }.
    // A DER INTEGER for a 32-byte value can be up to 35 bytes (tag + len + sign pad + 32).
    uint8_t rEnc[kP256FieldLength + 3];
    uint8_t sEnc[kP256FieldLength + 3];
    const size_t rl     = EncodeDerInteger(rawSig.ConstBytes(), rEnc);
    const size_t sl     = EncodeDerInteger(rawSig.ConstBytes() + kP256FieldLength, sEnc);
    const size_t seqLen = rl + sl;
    VerifyOrReturnError(seqLen < 0x80, CHIP_ERROR_INTERNAL);

    uint8_t sig[8 + 2 * (kP256FieldLength + 3)];
    size_t b = 0;
    sig[b++] = 0x03;                                 // BIT STRING
    sig[b++] = static_cast<uint8_t>(seqLen + 2 + 1); // SEQUENCE header (2) + unused-bits byte (1)
    sig[b++] = 0x00;                                 // 0 unused bits
    sig[b++] = 0x30;                                 // SEQUENCE (ECDSA-Sig-Value)
    sig[b++] = static_cast<uint8_t>(seqLen);
    memcpy(sig + b, rEnc, rl);
    b += rl;
    memcpy(sig + b, sEnc, sl);
    b += sl;

    // CertificationRequest ::= SEQUENCE { CertificationRequestInfo, sigAlg, signature }.
    const size_t contentLen = criLen + sizeof(kSigAlg) + b;
    VerifyOrReturnError(contentLen <= 0xff, CHIP_ERROR_INTERNAL);
    const size_t hdrLen = (contentLen < 0x80) ? 2 : 3; // 2-byte length prefix once content >= 128
    VerifyOrReturnError(outCsr.size() >= hdrLen + contentLen, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t * p = outCsr.data();
    *p++        = 0x30;
    if (hdrLen == 3)
    {
        *p++ = 0x81;
    }
    *p++ = static_cast<uint8_t>(contentLen);
    memcpy(p, cri, criLen);
    p += criLen;
    memcpy(p, kSigAlg, sizeof(kSigAlg));
    p += sizeof(kSigAlg);
    memcpy(p, sig, b);

    outCsr.reduce_size(hdrLen + contentLen);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32TEEOpKeySign(const char * keyId, ByteSpan message, Crypto::P256ECDSASignature & outSignature)
{
    VerifyOrReturnError(keyId != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!message.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[Crypto::kSHA256_Hash_Length] = {};
    ReturnErrorOnFailure(Crypto::Hash_SHA256(message.data(), message.size(), digest));

    esp_tee_sec_storage_key_cfg_t cfg    = MakeKeyCfg(keyId);
    esp_tee_sec_storage_ecdsa_sign_t sig = {};
    esp_err_t err                        = esp_tee_sec_storage_ecdsa_sign(&cfg, digest, sizeof(digest), &sig);
    VerifyOrReturnError(err == ESP_OK, CHIP_ERROR_INTERNAL, ChipLogError(Crypto, "ecdsa_sign(%s) failed: %d", keyId, err));

    // P256ECDSASignature is the raw R || S (2 * 32 bytes). IDF v6.0 merged the separate
    // sign_r/sign_s fields into a single signature[] holding R||S packed at the field length.
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(6, 0, 0)
    memcpy(outSignature.Bytes(), sig.signature, 2 * kP256FieldLength);
#else
    memcpy(outSignature.Bytes(), sig.sign_r, kP256FieldLength);
    memcpy(outSignature.Bytes() + kP256FieldLength, sig.sign_s, kP256FieldLength);
#endif
    return outSignature.SetLength(2 * kP256FieldLength);
}

CHIP_ERROR ESP32TEEOpKeyRemove(const char * keyId)
{
    VerifyOrReturnError(keyId != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    esp_err_t err = esp_tee_sec_storage_clear_key(keyId);
    // Treat "not present" as success so Remove is idempotent.
    VerifyOrReturnError(err == ESP_OK || err == ESP_ERR_NOT_FOUND, CHIP_ERROR_INTERNAL,
                        ChipLogError(Crypto, "clear_key(%s) failed: %d", keyId, err));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32TEEOpKeySelfTest()
{
    static const char * kId = "mtr-op-selftest";
    CHIP_ERROR err          = CHIP_NO_ERROR;

    LogErrorOnFailure(ESP32TEEOpKeyRemove(kId)); // start clean, best-effort

    Crypto::P256PublicKey pub;
    uint8_t csrBuf[Crypto::kMIN_CSR_Buffer_Size];
    MutableByteSpan csr(csrBuf);
    Crypto::P256PublicKey csrPub;
    Crypto::P256ECDSASignature sig;
    const uint8_t msg[] = "esp-tee-op-key-selftest";

    VerifyOrExit((err = ESP32TEEOpKeyGenerate(kId)) == CHIP_NO_ERROR, ChipLogError(Crypto, "selftest: gen failed"));
    VerifyOrExit((err = ESP32TEEOpKeyGetPublicKey(kId, pub)) == CHIP_NO_ERROR, ChipLogError(Crypto, "selftest: pubkey failed"));

    // Raw TEE sign path (this is what CASE / SignWithOpKeypair uses) — test it first.
    VerifyOrExit((err = ESP32TEEOpKeySign(kId, ByteSpan(msg, sizeof(msg)), sig)) == CHIP_NO_ERROR,
                 ChipLogError(Crypto, "selftest: sign failed"));
    VerifyOrExit((err = pub.ECDSA_validate_msg_signature(msg, sizeof(msg), sig)) == CHIP_NO_ERROR,
                 ChipLogError(Crypto, "selftest: signature verify failed"));
    ChipLogProgress(Crypto, "selftest: sign+verify OK");

    VerifyOrExit((err = ESP32TEEOpKeyNewCSR(kId, csr)) == CHIP_NO_ERROR, ChipLogError(Crypto, "selftest: CSR failed"));
    // Verifies the CSR self-signature AND recovers the embedded public key.
    VerifyOrExit((err = Crypto::VerifyCertificateSigningRequest(csr.data(), csr.size(), csrPub)) == CHIP_NO_ERROR,
                 ChipLogError(Crypto, "selftest: CSR verify failed"));
    VerifyOrExit(csrPub.Matches(pub), err = CHIP_ERROR_INTERNAL; ChipLogError(Crypto, "selftest: CSR pubkey mismatch"));

    ChipLogProgress(Crypto, "TEE operational-key self-test PASSED (gen/pubkey/sign/CSR all verified in TEE)");

exit:
    LogErrorOnFailure(ESP32TEEOpKeyRemove(kId)); // best-effort cleanup
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
