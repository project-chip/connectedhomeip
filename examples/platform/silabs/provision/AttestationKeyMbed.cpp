#include "AttestationKeyMbed.h"
#include <lib/support/CodeUtils.h>
#include <platform/silabs/SilabsConfig.h>
#include <mbedtls/x509_csr.h>
#include <mbedtls/asn1.h>
#ifdef SIWX_917
#include <mbedtls/sha256.h>
#endif
#include <em_msc.h>
#include <stdio.h>
#include <string.h>


namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

using SilabsConfig = chip::DeviceLayer::Internal::SilabsConfig;


CHIP_ERROR AttestationKeyMbed::Import(const uint8_t * asn1, size_t size)
{
    VerifyOrReturnError(sizeof(mValue) >= size, CHIP_ERROR_INVALID_ARGUMENT);
    mSize = size;
    memcpy(mValue, asn1, size);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AttestationKeyMbed::Export(uint8_t * asn1, size_t max, size_t &size)
{
    VerifyOrReturnError(max >= mSize, CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(asn1, mValue, mSize);
    mSize = size;
    return CHIP_NO_ERROR;
}


CHIP_ERROR AttestationKeyMbed::GenerateCSR(uint16_t vid, uint16_t pid, const CharSpan &cn, MutableCharSpan & csr)
{
    mbedtls_pk_context key_ctx;
    mbedtls_x509write_csr csr_ctx;

    VerifyOrReturnError(nullptr != csr.data(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(csr.size() >= 512, CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509write_csr_init(&csr_ctx);

    // Subject name
    char subject_name[64] = { 0 };
    snprintf(subject_name, sizeof(subject_name), "CN=%s Mvid:%04X Mpid:%04X", cn.data(), vid, pid);
    int err = mbedtls_x509write_csr_set_subject_name(&csr_ctx, subject_name);
    VerifyOrReturnError(0 == err, CHIP_ERROR_INTERNAL);

    // Algorithm
    mbedtls_x509write_csr_set_md_alg(&csr_ctx, MBEDTLS_MD_SHA256);

    // Generate key
    mbedtls_pk_init(&key_ctx);
    mbedtls_pk_setup(&key_ctx, mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY));
    mbedtls_ecp_keypair *kp_ctx = mbedtls_pk_ec(key_ctx);
    int (*f_rng)(void *, unsigned char *, size_t) = nullptr;
    void *p_rng = nullptr;
    err = mbedtls_ecdsa_genkey(kp_ctx, MBEDTLS_ECP_DP_SECP256R1, f_rng, p_rng);
    mbedtls_x509write_csr_set_key(&csr_ctx, &key_ctx);

    // TODO: STORE KEY!

    // Generate CSR
    err = mbedtls_x509write_csr_pem(&csr_ctx, (uint8_t *) csr.data(), csr.size(), NULL, NULL);
    VerifyOrReturnError(0 == err, CHIP_ERROR_INTERNAL);
    csr.reduce_size(strlen(csr.data()) + 1);

    // Release
    mbedtls_x509write_csr_free(&csr_ctx);
    return CHIP_NO_ERROR;
}


struct Asn1Signature
{
public:
    Asn1Signature(uint8_t *value, size_t size) : asn1(value, size) {}

    CHIP_ERROR Unwrap(MutableByteSpan & out)
    {
        uint8_t *p = (uint8_t *)this->asn1.data();
        uint8_t *q = p + this->asn1.size();
        this->out = out.data();
        this->offset = 0;
        mbedtls_asn1_traverse_sequence_of(&p, q, 0, 0, 0, 0, Asn1Signature::Callback, this);
        out.reduce_size(this->offset);
        return CHIP_NO_ERROR;
    }

private:
    static int Callback(void *ctx, int tag, unsigned char *value, size_t size)
    {
        if(MBEDTLS_ASN1_INTEGER == tag)
        {
            Asn1Signature *sig = (Asn1Signature *)ctx;
            uint8_t *s = &sig->out[sig->offset];
            uint8_t *v = size > 32 ? &value[1] : &value[0];
            memcpy(s, v, 32);
            sig->offset += 32;
        }
        return 0;
    }

    const ByteSpan asn1;
    uint8_t *out = nullptr;
    size_t offset = 0;
};


CHIP_ERROR AttestationKeyMbed::SignMessage(const ByteSpan & message, MutableByteSpan & out_span)
{
    uint8_t hash[32];
    size_t hash_size = sizeof(hash);
    uint8_t signature[128] = { 0 };
    size_t signature_size = sizeof(signature);

    int(*f_rng)(void *, unsigned char *, size_t) = nullptr;
    void *p_rng = nullptr;
    mbedtls_pk_context ctx = { 0 };

    ChipLogProgress(DeviceLayer, "SignWithDeviceAttestationKey, key(%u)", (unsigned) mSize);

#ifdef SIWX_917
    mbedtls_sha256(message.data(), message.size(), hash, 0);
    VerifyOrReturnError(0 == mbedtls_pk_parse_key(&ctx, mValue, mSize, nullptr, 0), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(0 == mbedtls_pk_sign(&ctx, MBEDTLS_MD_SHA256, hash, hash_size, signature, &signature_size, f_rng, p_rng ), CHIP_ERROR_INTERNAL);
#else
    VerifyOrReturnError(0 == mbedtls_sha256(message.data(), message.size(), hash, 0), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(0 == mbedtls_pk_parse_key(&ctx, mValue, mSize, nullptr, 0, f_rng, p_rng), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(0 == mbedtls_pk_sign(&ctx, MBEDTLS_MD_SHA256, hash, hash_size, signature, sizeof(signature), &signature_size, f_rng, p_rng ), CHIP_ERROR_INTERNAL);
#endif

    Asn1Signature s(signature, signature_size);
    return s.Unwrap(out_span);
}


} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
