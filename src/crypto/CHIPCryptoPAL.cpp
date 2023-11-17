/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

/**
 *    @file
 *      Platform agnostic implementation of CHIP crypto algorithms
 */

#include "CHIPCryptoPAL.h"
#include <lib/asn1/ASN1.h>
#include <lib/asn1/ASN1Macros.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/BufferReader.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <stdint.h>
#include <string.h>

using chip::ByteSpan;
using chip::MutableByteSpan;
using chip::Encoding::BufferWriter;
using chip::Encoding::LittleEndian::Reader;

using namespace chip::ASN1;

namespace chip {
namespace Crypto {
namespace {

constexpr uint8_t kIntegerTag         = 0x02u;
constexpr uint8_t kSeqTag             = 0x30u;
constexpr size_t kMinSequenceOverhead = 1 /* tag */ + 1 /* length */ + 1 /* actual data or second length byte*/;

/**
 * @brief Utility to convert DER-encoded INTEGER into a raw integer buffer in big-endian order
 *        with leading zeroes if the output buffer is larger than needed.
 * @param[in] reader Reader instance from which the input will be read
 * @param[out] raw_integer_out Buffer to receive the DER-encoded integer
 * @return CHIP_ERROR_INVALID_ARGUMENT or CHIP_ERROR_BUFFER_TOO_SMALL on error, CHIP_NO_ERROR otherwise
 */
CHIP_ERROR ReadDerUnsignedIntegerIntoRaw(Reader & reader, MutableByteSpan raw_integer_out)
{
    uint8_t cur_byte = 0;

    ReturnErrorOnFailure(reader.Read8(&cur_byte).StatusCode());

    // We expect first tag to be INTEGER
    VerifyOrReturnError(cur_byte == kIntegerTag, CHIP_ERROR_INVALID_ARGUMENT);

    // Read the length
    size_t integer_len = 0;
    ReturnErrorOnFailure(chip::Crypto::ReadDerLength(reader, integer_len));

    // Clear the destination buffer, so we can blit the unsigned value into place
    memset(raw_integer_out.data(), 0, raw_integer_out.size());

    // Check for pseudo-zero to mark unsigned value
    // This means we have too large an integer (should be at most 1 byte too large), it's invalid
    ReturnErrorCodeIf(integer_len > (raw_integer_out.size() + 1), CHIP_ERROR_INVALID_ARGUMENT);

    if (integer_len == (raw_integer_out.size() + 1u))
    {
        // Means we had a 0x00 byte stuffed due to MSB being high in original integer
        ReturnErrorOnFailure(reader.Read8(&cur_byte).StatusCode());

        // The extra byte must be a leading zero
        VerifyOrReturnError(cur_byte == 0, CHIP_ERROR_INVALID_ARGUMENT);
        --integer_len;
    }

    // We now have the rest of the tag that is a "minimal length" unsigned integer.
    // Blit it at the correct offset, since the order we use is MSB first for
    // both ASN.1 and EC curve raw points.
    size_t offset = raw_integer_out.size() - integer_len;
    return reader.ReadBytes(raw_integer_out.data() + offset, integer_len).StatusCode();
}

CHIP_ERROR ConvertIntegerRawToDerInternal(const ByteSpan & raw_integer, MutableByteSpan & out_der_integer,
                                          bool include_tag_and_length)
{
    if (raw_integer.empty() || out_der_integer.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    Reader reader(raw_integer);
    BufferWriter writer(out_der_integer);

    bool needs_leading_zero_byte = false;

    uint8_t cur_byte = 0;
    while ((reader.Remaining() > 0) && (reader.Read8(&cur_byte).StatusCode() == CHIP_NO_ERROR) && (cur_byte == 0))
    {
        // Omit all leading zeros
    }

    if ((cur_byte & 0x80u) != 0)
    {
        // If overall MSB (from leftmost byte) is set, we will need to push out a zero to avoid it being
        // considered a negative number.
        needs_leading_zero_byte = true;
    }

    // The + 1 is to account for the last consumed byte of the loop to skip leading zeros
    size_t length = reader.Remaining() + 1 + (needs_leading_zero_byte ? 1 : 0);

    if (length > 127)
    {
        // We do not support length over more than 1 bytes.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (include_tag_and_length)
    {
        // Put INTEGER tag
        writer.Put(kIntegerTag);

        // Put length over 1 byte (i.e. MSB clear)
        writer.Put(static_cast<uint8_t>(length));
    }

    // If leading zero or no more bytes remaining, must ensure we start with at least a zero byte
    if (needs_leading_zero_byte)
    {
        writer.Put(static_cast<uint8_t>(0u));
    }

    // Put first consumed byte from last read iteration of leading zero suppression
    writer.Put(cur_byte);

    // Fill the rest from the input in order
    while (reader.Read8(&cur_byte).StatusCode() == CHIP_NO_ERROR)
    {
        // Emit all other bytes as-is
        writer.Put(cur_byte);
    }

    size_t actually_written = 0;
    if (!writer.Fit(actually_written))
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    out_der_integer = out_der_integer.SubSpan(0, actually_written);

    return CHIP_NO_ERROR;
}

/**
 * @brief Find a 4 uppercase hex digit hex value after a prefix string. Used to implement
 *        fallback CN VID/PID encoding for PAA/PAI/DAC.
 *
 * @param[in] buffer - buffer in which to find the substring.
 * @param[in] prefix - prefix to match, which must be followed by 4 uppercase hex characters
 * @param[out] out_hex_value - on CHIP_NO_ERROR return, this will be the 16-bit hex value decoded.
 * @return CHIP_NO_ERROR on success, CHIP_ERROR_NOT_FOUND if not detected and
 *         CHIP_ERROR_WRONG_CERT_DN if we saw the prefix but no valid hex string.
 */
CHIP_ERROR Find16BitUpperCaseHexAfterPrefix(const ByteSpan & buffer, const char * prefix, uint16_t & out_hex_value)
{
    chip::CharSpan prefix_span = chip::CharSpan::fromCharString(prefix);

    bool found_prefix_at_least_once = false;

    // Scan string from left to right, to find the desired full matching substring.
    //
    // IMPORTANT NOTE: We are trying to find the equivalent of prefix + [0-9A-F]{4}.
    // The appearance of the full prefix, but not followed by the hex value, must
    // be detected, as it is illegal if there isn't a valid prefix within the string.
    // This is why we first check for the prefix and then maybe check for the hex
    // value, rather than doing a single check of making sure there is enough space
    // for both.
    for (size_t start_idx = 0; start_idx < buffer.size(); start_idx++)
    {
        const uint8_t * cursor = buffer.data() + start_idx;
        size_t remaining       = buffer.size() - start_idx;

        if (remaining < prefix_span.size())
        {
            // We can't possibly match prefix if not enough bytes left.
            break;
        }

        // Try to match the prefix at current position.
        if (memcmp(cursor, prefix_span.data(), prefix_span.size()) != 0)
        {
            // Did not find prefix, move to next position.
            continue;
        }

        // Otherwise, found prefix, skip to possible hex value.
        found_prefix_at_least_once = true;
        cursor += prefix_span.size();
        remaining -= prefix_span.size();

        constexpr size_t expected_hex_len = HEX_ENCODED_LENGTH(sizeof(uint16_t));
        if (remaining < expected_hex_len)
        {
            // We can't possibly match the hex values if not enough bytes left.
            break;
        }

        char hex_buf[expected_hex_len];
        memcpy(&hex_buf[0], cursor, sizeof(hex_buf));

        if (Encoding::UppercaseHexToUint16(&hex_buf[0], sizeof(hex_buf), out_hex_value) != 0)
        {
            // Found first full valid match, return success, out_hex_value already updated.
            return CHIP_NO_ERROR;
        }

        // Otherwise, did not find what we were looking for, try next position until exhausted.
    }

    return found_prefix_at_least_once ? CHIP_ERROR_WRONG_CERT_DN : CHIP_ERROR_NOT_FOUND;
}

} // namespace

using HKDF_sha_crypto = HKDF_sha;

CHIP_ERROR Spake2p::InternalHash(const uint8_t * in, size_t in_len)
{
    const uint64_t u64_len = in_len;

    uint8_t lb[8];
    lb[0] = static_cast<uint8_t>((u64_len >> 0) & 0xff);
    lb[1] = static_cast<uint8_t>((u64_len >> 8) & 0xff);
    lb[2] = static_cast<uint8_t>((u64_len >> 16) & 0xff);
    lb[3] = static_cast<uint8_t>((u64_len >> 24) & 0xff);
    lb[4] = static_cast<uint8_t>((u64_len >> 32) & 0xff);
    lb[5] = static_cast<uint8_t>((u64_len >> 40) & 0xff);
    lb[6] = static_cast<uint8_t>((u64_len >> 48) & 0xff);
    lb[7] = static_cast<uint8_t>((u64_len >> 56) & 0xff);

    ReturnErrorOnFailure(Hash(lb, sizeof(lb)));
    if (in != nullptr)
    {
        ReturnErrorOnFailure(Hash(in, in_len));
    }

    return CHIP_NO_ERROR;
}

Spake2p::Spake2p(size_t _fe_size, size_t _point_size, size_t _hash_size)
{
    fe_size    = _fe_size;
    point_size = _point_size;
    hash_size  = _hash_size;

    Kca = &Kcab[0];
    Kcb = &Kcab[hash_size / 2];
    Ka  = &Kae[0];
    Ke  = &Kae[hash_size / 2];

    M  = nullptr;
    N  = nullptr;
    G  = nullptr;
    X  = nullptr;
    Y  = nullptr;
    L  = nullptr;
    Z  = nullptr;
    V  = nullptr;
    w0 = nullptr;
    w1 = nullptr;
    xy = nullptr;

    order  = nullptr;
    tempbn = nullptr;
}

CHIP_ERROR Spake2p::Init(const uint8_t * context, size_t context_len)
{
    if (state != CHIP_SPAKE2P_STATE::PREINIT)
    {
        Clear();
    }

    ReturnErrorOnFailure(InitImpl());
    ReturnErrorOnFailure(PointLoad(spake2p_M_p256, sizeof(spake2p_M_p256), M));
    ReturnErrorOnFailure(PointLoad(spake2p_N_p256, sizeof(spake2p_N_p256), N));
    ReturnErrorOnFailure(InternalHash(context, context_len));

    state = CHIP_SPAKE2P_STATE::INIT;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::WriteMN()
{
    ReturnErrorOnFailure(InternalHash(spake2p_M_p256, sizeof(spake2p_M_p256)));
    ReturnErrorOnFailure(InternalHash(spake2p_N_p256, sizeof(spake2p_N_p256)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::BeginVerifier(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                  size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                                  size_t Lin_len)
{
    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::INIT, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(InternalHash(peer_identity, peer_identity_len));
    ReturnErrorOnFailure(InternalHash(my_identity, my_identity_len));
    ReturnErrorOnFailure(WriteMN());
    ReturnErrorOnFailure(FELoad(w0in, w0in_len, w0));
    ReturnErrorOnFailure(PointLoad(Lin, Lin_len, L));

    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::VERIFIER;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::BeginProver(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                                size_t w1in_len)
{
    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::INIT, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(InternalHash(my_identity, my_identity_len));
    ReturnErrorOnFailure(InternalHash(peer_identity, peer_identity_len));
    ReturnErrorOnFailure(WriteMN());
    ReturnErrorOnFailure(FELoad(w0in, w0in_len, w0));
    ReturnErrorOnFailure(FELoad(w1in, w1in_len, w1));

    state = CHIP_SPAKE2P_STATE::STARTED;
    role  = CHIP_SPAKE2P_ROLE::PROVER;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::ComputeRoundOne(const uint8_t * pab, size_t pab_len, uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    void * MN        = nullptr; // Choose M if a prover, N if a verifier
    void * XY        = nullptr; // Choose X if a prover, Y if a verifier

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::STARTED, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(*out_len >= point_size, error = CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(FEGenerate(xy));

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        MN = M;
        XY = X;
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        MN = N;
        XY = Y;
    }
    VerifyOrExit(MN != nullptr, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(XY != nullptr, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(error = PointAddMul(XY, G, xy, MN, w0));
    SuccessOrExit(error = PointWrite(XY, out, *out_len));

    state = CHIP_SPAKE2P_STATE::R1;
    error = CHIP_NO_ERROR;
exit:
    *out_len = point_size;
    return error;
}

CHIP_ERROR Spake2p::ComputeRoundTwo(const uint8_t * in, size_t in_len, uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    MutableByteSpan out_span{ out, *out_len };
    uint8_t point_buffer[kMAX_Point_Length];
    void * MN        = nullptr; // Choose N if a prover, M if a verifier
    void * XY        = nullptr; // Choose Y if a prover, X if a verifier
    uint8_t * Kcaorb = nullptr; // Choose Kca if a prover, Kcb if a verifier

    VerifyOrExit(*out_len >= hash_size, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(state == CHIP_SPAKE2P_STATE::R1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(in_len == point_size, error = CHIP_ERROR_INTERNAL);

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        SuccessOrExit(error = PointWrite(X, point_buffer, point_size));
        SuccessOrExit(error = InternalHash(point_buffer, point_size));
        SuccessOrExit(error = InternalHash(in, in_len));

        MN     = N;
        XY     = Y;
        Kcaorb = Kca;
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        SuccessOrExit(error = InternalHash(in, in_len));
        SuccessOrExit(error = PointWrite(Y, point_buffer, point_size));
        SuccessOrExit(error = InternalHash(point_buffer, point_size));

        MN     = M;
        XY     = X;
        Kcaorb = Kcb;
    }
    VerifyOrExit(MN != nullptr, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(XY != nullptr, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(error = PointLoad(in, in_len, XY));
    SuccessOrExit(error = PointIsValid(XY));
    SuccessOrExit(error = FEMul(tempbn, xy, w0));
    SuccessOrExit(error = PointInvert(MN));
    SuccessOrExit(error = PointAddMul(Z, XY, xy, MN, tempbn));
    SuccessOrExit(error = PointCofactorMul(Z));

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        SuccessOrExit(error = FEMul(tempbn, w1, w0));
        SuccessOrExit(error = PointAddMul(V, XY, w1, MN, tempbn));
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        SuccessOrExit(error = PointMul(V, L, xy));
    }

    SuccessOrExit(error = PointCofactorMul(V));
    SuccessOrExit(error = PointWrite(Z, point_buffer, point_size));
    SuccessOrExit(error = InternalHash(point_buffer, point_size));

    SuccessOrExit(error = PointWrite(V, point_buffer, point_size));
    SuccessOrExit(error = InternalHash(point_buffer, point_size));

    SuccessOrExit(error = FEWrite(w0, point_buffer, fe_size));
    SuccessOrExit(error = InternalHash(point_buffer, fe_size));

    SuccessOrExit(error = GenerateKeys());

    SuccessOrExit(error = Mac(Kcaorb, hash_size / 2, in, in_len, out_span));
    VerifyOrExit(out_span.size() == hash_size, error = CHIP_ERROR_INTERNAL);

    state = CHIP_SPAKE2P_STATE::R2;
    error = CHIP_NO_ERROR;
exit:
    *out_len = hash_size;
    return error;
}

CHIP_ERROR Spake2p::GenerateKeys()
{
    static const uint8_t info_keyconfirm[16] = { 'C', 'o', 'n', 'f', 'i', 'r', 'm', 'a', 't', 'i', 'o', 'n', 'K', 'e', 'y', 's' };

    MutableByteSpan Kae_span{ &Kae[0], sizeof(Kae) };

    ReturnErrorOnFailure(HashFinalize(Kae_span));
    ReturnErrorOnFailure(KDF(Ka, hash_size / 2, nullptr, 0, info_keyconfirm, sizeof(info_keyconfirm), Kcab, hash_size));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::KeyConfirm(const uint8_t * in, size_t in_len)
{
    uint8_t point_buffer[kP256_Point_Length];
    void * XY        = nullptr; // Choose X if a prover, Y if a verifier
    uint8_t * Kcaorb = nullptr; // Choose Kcb if a prover, Kca if a verifier

    VerifyOrReturnError(state == CHIP_SPAKE2P_STATE::R2, CHIP_ERROR_INTERNAL);

    if (role == CHIP_SPAKE2P_ROLE::PROVER)
    {
        XY     = X;
        Kcaorb = Kcb;
    }
    else if (role == CHIP_SPAKE2P_ROLE::VERIFIER)
    {
        XY     = Y;
        Kcaorb = Kca;
    }
    VerifyOrReturnError(XY != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(Kcaorb != nullptr, CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(PointWrite(XY, point_buffer, point_size));

    CHIP_ERROR err = MacVerify(Kcaorb, hash_size / 2, in, in_len, point_buffer, point_size);
    if (err == CHIP_ERROR_INTERNAL)
    {
        ChipLogError(SecureChannel, "Failed to verify peer's MAC. This can happen when setup code is incorrect.");
    }
    ReturnErrorOnFailure(err);

    state = CHIP_SPAKE2P_STATE::KC;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p::GetKeys(uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    VerifyOrExit(state == CHIP_SPAKE2P_STATE::KC, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(*out_len >= hash_size / 2, error = CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(out, Ke, hash_size / 2);
    error = CHIP_NO_ERROR;
exit:
    *out_len = hash_size / 2;
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitImpl()
{
    ReturnErrorOnFailure(sha256_hash_ctx.Begin());
    ReturnErrorOnFailure(InitInternal());
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::Hash(const uint8_t * in, size_t in_len)
{
    ReturnErrorOnFailure(sha256_hash_ctx.AddData(ByteSpan{ in, in_len }));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::HashFinalize(MutableByteSpan & out_span)
{
    ReturnErrorOnFailure(sha256_hash_ctx.Finish(out_span));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::KDF(const uint8_t * ikm, const size_t ikm_len, const uint8_t * salt,
                                              const size_t salt_len, const uint8_t * info, const size_t info_len, uint8_t * out,
                                              size_t out_len)
{
    HKDF_sha_crypto mHKDF;

    ReturnErrorOnFailure(mHKDF.HKDF_SHA256(ikm, ikm_len, salt, salt_len, info, info_len, out, out_len));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::ComputeW0(uint8_t * w0out, size_t * w0_len, const uint8_t * w0sin, size_t w0sin_len)
{
    ReturnErrorOnFailure(FELoad(w0sin, w0sin_len, w0));
    ReturnErrorOnFailure(FEWrite(w0, w0out, *w0_len));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2pVerifier::Serialize(MutableByteSpan & outSerialized) const
{
    VerifyOrReturnError(outSerialized.size() >= kSpake2p_VerifierSerialized_Length, CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(&outSerialized.data()[0], mW0, sizeof(mW0));
    memcpy(&outSerialized.data()[sizeof(mW0)], mL, sizeof(mL));

    outSerialized.reduce_size(kSpake2p_VerifierSerialized_Length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2pVerifier::Deserialize(const ByteSpan & inSerialized)
{
    VerifyOrReturnError(inSerialized.size() >= kSpake2p_VerifierSerialized_Length, CHIP_ERROR_INVALID_ARGUMENT);

    memcpy(mW0, &inSerialized.data()[0], sizeof(mW0));
    memcpy(mL, &inSerialized.data()[sizeof(mW0)], sizeof(mL));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2pVerifier::Generate(uint32_t pbkdf2IterCount, const ByteSpan & salt, uint32_t setupPin)
{
    uint8_t serializedWS[kSpake2p_WS_Length * 2] = { 0 };
    ReturnErrorOnFailure(ComputeWS(pbkdf2IterCount, salt, setupPin, serializedWS, sizeof(serializedWS)));

    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t len;

    // Create local Spake2+ object for w0 and L computations.
    Spake2p_P256_SHA256_HKDF_HMAC spake2p;
    uint8_t context[kSHA256_Hash_Length] = { 0 };
    SuccessOrExit(err = spake2p.Init(context, sizeof(context)));

    // Compute w0
    len = sizeof(mW0);
    SuccessOrExit(err = spake2p.ComputeW0(mW0, &len, &serializedWS[0], kSpake2p_WS_Length));
    VerifyOrExit(len == sizeof(mW0), err = CHIP_ERROR_INTERNAL);

    // Compute L
    len = sizeof(mL);
    SuccessOrExit(err = spake2p.ComputeL(mL, &len, &serializedWS[kSpake2p_WS_Length], kSpake2p_WS_Length));
    VerifyOrExit(len == sizeof(mL), err = CHIP_ERROR_INTERNAL);

exit:
    spake2p.Clear();
    return err;
}

CHIP_ERROR Spake2pVerifier::ComputeWS(uint32_t pbkdf2IterCount, const ByteSpan & salt, uint32_t setupPin, uint8_t * ws,
                                      uint32_t ws_len)
{
    PBKDF2_sha256 pbkdf2;
    uint8_t littleEndianSetupPINCode[sizeof(uint32_t)];
    Encoding::LittleEndian::Put32(littleEndianSetupPINCode, setupPin);

    ReturnErrorCodeIf(salt.size() < kSpake2p_Min_PBKDF_Salt_Length || salt.size() > kSpake2p_Max_PBKDF_Salt_Length,
                      CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(pbkdf2IterCount < kSpake2p_Min_PBKDF_Iterations || pbkdf2IterCount > kSpake2p_Max_PBKDF_Iterations,
                      CHIP_ERROR_INVALID_ARGUMENT);

    return pbkdf2.pbkdf2_sha256(littleEndianSetupPINCode, sizeof(littleEndianSetupPINCode), salt.data(), salt.size(),
                                pbkdf2IterCount, ws_len, ws);
}

CHIP_ERROR ReadDerLength(Reader & reader, size_t & length)
{
    length = 0;

    uint8_t cur_byte = 0;
    ReturnErrorOnFailure(reader.Read8(&cur_byte).StatusCode());

    if ((cur_byte & (1u << 7)) == 0)
    {
        // 7 bit length, the rest of the byte is the length.
        length = cur_byte & 0x7Fu;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    // Did not early return: > 7 bit length, the number of bytes of the length is provided next.
    uint8_t length_bytes = cur_byte & 0x7Fu;
    VerifyOrReturnError((length_bytes >= 1) && (length_bytes <= sizeof(size_t)), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(reader.HasAtLeast(length_bytes), CHIP_ERROR_BUFFER_TOO_SMALL);

    for (uint8_t i = 0; i < length_bytes; i++)
    {
        uint8_t cur_length_byte = 0;
        err                     = reader.Read8(&cur_length_byte).StatusCode();
        if (err != CHIP_NO_ERROR)
            break;

        // Cannot have zero padding on multi-byte lengths in DER, so first
        // byte must always be > 0.
        if ((i == 0) && (cur_length_byte == 0))
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        length <<= 8;
        length |= cur_length_byte;
    }

    // Single-byte long length cannot be < 128: DER always encodes on smallest size
    // possible, so length zero should have been a single byte short length.
    if ((length_bytes == 1) && (length < 128))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConvertIntegerRawToDerWithoutTag(const ByteSpan & raw_integer, MutableByteSpan & out_der_integer)
{
    return ConvertIntegerRawToDerInternal(raw_integer, out_der_integer, /* include_tag_and_length = */ false);
}

CHIP_ERROR ConvertIntegerRawToDer(const ByteSpan & raw_integer, MutableByteSpan & out_der_integer)
{
    return ConvertIntegerRawToDerInternal(raw_integer, out_der_integer, /* include_tag_and_length = */ true);
}

CHIP_ERROR EcdsaRawSignatureToAsn1(size_t fe_length_bytes, const ByteSpan & raw_sig, MutableByteSpan & out_asn1_sig)
{
    VerifyOrReturnError(fe_length_bytes > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(raw_sig.size() == (2u * fe_length_bytes), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_asn1_sig.size() >= (raw_sig.size() + kMax_ECDSA_X9Dot62_Asn1_Overhead), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Write both R an S integers past the overhead, we will shift them back later if we only needed 2 size bytes.
    uint8_t * cursor = out_asn1_sig.data() + kMinSequenceOverhead;
    size_t remaining = out_asn1_sig.size() - kMinSequenceOverhead;

    size_t integers_length = 0;

    // Write R (first `fe_length_bytes` block of raw signature)
    {
        MutableByteSpan out_der_integer(cursor, remaining);
        ReturnErrorOnFailure(ConvertIntegerRawToDer(raw_sig.SubSpan(0, fe_length_bytes), out_der_integer));
        VerifyOrReturnError(out_der_integer.size() <= remaining, CHIP_ERROR_INTERNAL);

        integers_length += out_der_integer.size();
        remaining -= out_der_integer.size();
        cursor += out_der_integer.size();
    }

    // Write S (second `fe_length_bytes` block of raw signature)
    {
        MutableByteSpan out_der_integer(cursor, remaining);
        ReturnErrorOnFailure(ConvertIntegerRawToDer(raw_sig.SubSpan(fe_length_bytes, fe_length_bytes), out_der_integer));
        VerifyOrReturnError(out_der_integer.size() <= remaining, CHIP_ERROR_INTERNAL);
        integers_length += out_der_integer.size();
    }

    // We only support outputs that would use 1 or 2 bytes of DER length after the SEQUENCE tag
    VerifyOrReturnError(integers_length <= UINT8_MAX, CHIP_ERROR_INVALID_ARGUMENT);

    // We now know the length of both variable sized integers in the sequence, so we
    // can write the tag and length.
    BufferWriter writer(out_asn1_sig);

    // Put SEQUENCE tag
    writer.Put(kSeqTag);

    // Put the length over 1 or two bytes depending on case
    constexpr uint8_t kExtendedLengthMarker = 0x80u;
    if (integers_length > 127u)
    {
        writer.Put(static_cast<uint8_t>(kExtendedLengthMarker | 1)); // Length is extended length, over 1 subsequent byte
        writer.Put(static_cast<uint8_t>(integers_length));
    }
    else
    {
        // Length is directly in the first byte with MSB clear if <= 127.
        writer.Put(static_cast<uint8_t>(integers_length));
    }

    // Put the contents of the integers previously serialized in the buffer.
    // The writer.Put is memmove-safe, so the shifting will happen from the read
    // of the same buffer where the write is taking place.
    writer.Put(out_asn1_sig.data() + kMinSequenceOverhead, integers_length);

    size_t actually_written = 0;
    VerifyOrReturnError(writer.Fit(actually_written), CHIP_ERROR_BUFFER_TOO_SMALL);

    out_asn1_sig = out_asn1_sig.SubSpan(0, actually_written);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EcdsaAsn1SignatureToRaw(size_t fe_length_bytes, const ByteSpan & asn1_sig, MutableByteSpan & out_raw_sig)
{
    VerifyOrReturnError(fe_length_bytes > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(asn1_sig.size() > kMinSequenceOverhead, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Output raw signature is <r,s> both of which are of fe_length_bytes (see SEC1).
    VerifyOrReturnError(out_raw_sig.size() >= (2u * fe_length_bytes), CHIP_ERROR_BUFFER_TOO_SMALL);

    Reader reader(asn1_sig);

    // Make sure we have a starting Sequence
    uint8_t tag = 0;
    ReturnErrorOnFailure(reader.Read8(&tag).StatusCode());
    VerifyOrReturnError(tag == kSeqTag, CHIP_ERROR_INVALID_ARGUMENT);

    // Read length of sequence
    size_t tag_len = 0;
    ReturnErrorOnFailure(ReadDerLength(reader, tag_len));

    // Length of sequence must match what is left of signature
    VerifyOrReturnError(tag_len == reader.Remaining(), CHIP_ERROR_INVALID_ARGUMENT);

    // Can now clear raw signature integers r,s one by one
    uint8_t * raw_cursor = out_raw_sig.data();

    // Read R
    ReturnErrorOnFailure(ReadDerUnsignedIntegerIntoRaw(reader, MutableByteSpan{ raw_cursor, fe_length_bytes }));

    raw_cursor += fe_length_bytes;

    // Read S
    ReturnErrorOnFailure(ReadDerUnsignedIntegerIntoRaw(reader, MutableByteSpan{ raw_cursor, fe_length_bytes }));

    out_raw_sig = out_raw_sig.SubSpan(0, (2u * fe_length_bytes));

    return CHIP_NO_ERROR;
}

CHIP_ERROR AES_CTR_crypt(const uint8_t * input, size_t input_length, const Aes128KeyHandle & key, const uint8_t * nonce,
                         size_t nonce_length, uint8_t * output)
{
    // Discard tag portion of CCM to apply only CTR mode encryption/decryption.
    constexpr size_t kTagLen = Crypto::kAES_CCM128_Tag_Length;
    uint8_t tag[kTagLen];

    return AES_CCM_encrypt(input, input_length, nullptr, 0, key, nonce, nonce_length, output, tag, kTagLen);
}

CHIP_ERROR GenerateCompressedFabricId(const Crypto::P256PublicKey & root_public_key, uint64_t fabric_id,
                                      MutableByteSpan & out_compressed_fabric_id)
{
    VerifyOrReturnError(root_public_key.IsUncompressed(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_compressed_fabric_id.size() >= kCompressedFabricIdentifierSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Ensure proper endianness for Fabric ID (i.e. big-endian as it appears in certificates)
    uint8_t fabric_id_as_big_endian_salt[kCompressedFabricIdentifierSize];
    chip::Encoding::BigEndian::Put64(&fabric_id_as_big_endian_salt[0], fabric_id);

    // Compute Compressed fabric reference per spec pseudocode
    //   CompressedFabricIdentifier =
    //     CHIP_Crypto_KDF(
    //       inputKey := TargetOperationalRootPublicKey,
    //       salt:= TargetOperationalFabricID,
    //       info := CompressedFabricInfo,
    //       len := 64)
    //
    // NOTE: len=64 bits is implied by output buffer size when calling HKDF_sha::HKDF_SHA256.

    constexpr uint8_t kCompressedFabricInfo[16] = /* "CompressedFabric" */
        { 0x43, 0x6f, 0x6d, 0x70, 0x72, 0x65, 0x73, 0x73, 0x65, 0x64, 0x46, 0x61, 0x62, 0x72, 0x69, 0x63 };
    HKDF_sha hkdf;

    // Must drop uncompressed point form format specifier (first byte), per spec method
    ByteSpan input_key_span(root_public_key.ConstBytes() + 1, root_public_key.Length() - 1);

    CHIP_ERROR status = hkdf.HKDF_SHA256(
        input_key_span.data(), input_key_span.size(), &fabric_id_as_big_endian_salt[0], sizeof(fabric_id_as_big_endian_salt),
        &kCompressedFabricInfo[0], sizeof(kCompressedFabricInfo), out_compressed_fabric_id.data(), kCompressedFabricIdentifierSize);

    // Resize output to final bounds on success
    if (status == CHIP_NO_ERROR)
    {
        out_compressed_fabric_id = out_compressed_fabric_id.SubSpan(0, kCompressedFabricIdentifierSize);
    }

    return status;
}

CHIP_ERROR GenerateCompressedFabricId(const Crypto::P256PublicKey & rootPublicKey, uint64_t fabricId, uint64_t & compressedFabricId)
{
    uint8_t allocated[sizeof(fabricId)];
    MutableByteSpan span(allocated);
    ReturnErrorOnFailure(GenerateCompressedFabricId(rootPublicKey, fabricId, span));
    // Decode compressed fabric ID accounting for endianness, as GenerateCompressedFabricId()
    // returns a binary buffer and is agnostic of usage of the output as an integer type.
    compressedFabricId = Encoding::BigEndian::Get64(allocated);
    return CHIP_NO_ERROR;
}

/* Operational Group Key Group, Security Info: "GroupKey v1.0" */
static const uint8_t kGroupSecurityInfo[] = { 0x47, 0x72, 0x6f, 0x75, 0x70, 0x4b, 0x65, 0x79, 0x20, 0x76, 0x31, 0x2e, 0x30 };

/* Group Key Derivation Function, Info: "GroupKeyHash" ” */
static const uint8_t kGroupKeyHashInfo[]  = { 0x47, 0x72, 0x6f, 0x75, 0x70, 0x4b, 0x65, 0x79, 0x48, 0x61, 0x73, 0x68 };
static const uint8_t kGroupKeyHashSalt[0] = {};

/*
    OperationalGroupKey =
        Crypto_KDF
        (
            InputKey = Epoch Key,
            Salt = CompressedFabricIdentifier,
            Info = "GroupKey v1.0",
            Length = CRYPTO_SYMMETRIC_KEY_LENGTH_BITS
        )
*/
CHIP_ERROR DeriveGroupOperationalKey(const ByteSpan & epoch_key, const ByteSpan & compressed_fabric_id, MutableByteSpan & out_key)
{
    VerifyOrReturnError(Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES == epoch_key.size(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES <= out_key.size(), CHIP_ERROR_INVALID_ARGUMENT);

    Crypto::HKDF_sha crypto;
    return crypto.HKDF_SHA256(epoch_key.data(), epoch_key.size(), compressed_fabric_id.data(), compressed_fabric_id.size(),
                              kGroupSecurityInfo, sizeof(kGroupSecurityInfo), out_key.data(),
                              Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
}

/*
    GKH = Crypto_KDF (
        InputKey = OperationalGroupKey,
        Salt = [],
        Info = "GroupKeyHash",
        Length = 16)
*/
CHIP_ERROR DeriveGroupSessionId(const ByteSpan & operational_key, uint16_t & session_id)
{
    VerifyOrReturnError(Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES == operational_key.size(), CHIP_ERROR_INVALID_ARGUMENT);
    Crypto::HKDF_sha crypto;
    uint8_t out_key[sizeof(uint16_t)];

    ReturnErrorOnFailure(crypto.HKDF_SHA256(operational_key.data(), operational_key.size(), kGroupKeyHashSalt,
                                            sizeof(kGroupKeyHashSalt), kGroupKeyHashInfo, sizeof(kGroupKeyHashInfo), out_key,
                                            sizeof(out_key)));
    session_id = Encoding::BigEndian::Get16(out_key);
    return CHIP_NO_ERROR;
}

/* Operational Group Key Group, PrivacyKey Info: "PrivacyKey" */
static const uint8_t kGroupPrivacyInfo[] = { 'P', 'r', 'i', 'v', 'a', 'c', 'y', 'K', 'e', 'y' };

/*
    PrivacyKey =
         Crypto_KDF
         (
            InputKey = EncryptionKey,
            Salt = [],
            Info = "PrivacyKey",
            Length = CRYPTO_SYMMETRIC_KEY_LENGTH_BITS
         )
*/
CHIP_ERROR DeriveGroupPrivacyKey(const ByteSpan & encryption_key, MutableByteSpan & out_key)
{
    VerifyOrReturnError(Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES == encryption_key.size(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES <= out_key.size(), CHIP_ERROR_INVALID_ARGUMENT);

    constexpr ByteSpan null_span = ByteSpan();

    Crypto::HKDF_sha crypto;
    return crypto.HKDF_SHA256(encryption_key.data(), encryption_key.size(), null_span.data(), null_span.size(), kGroupPrivacyInfo,
                              sizeof(kGroupPrivacyInfo), out_key.data(), Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
}

CHIP_ERROR DeriveGroupOperationalCredentials(const ByteSpan & epoch_key, const ByteSpan & compressed_fabric_id,
                                             GroupOperationalCredentials & operational_credentials)
{
    MutableByteSpan encryption_key(operational_credentials.encryption_key);
    MutableByteSpan privacy_key(operational_credentials.privacy_key);

    ReturnErrorOnFailure(Crypto::DeriveGroupOperationalKey(epoch_key, compressed_fabric_id, encryption_key));
    ReturnErrorOnFailure(Crypto::DeriveGroupSessionId(encryption_key, operational_credentials.hash));
    ReturnErrorOnFailure(Crypto::DeriveGroupPrivacyKey(encryption_key, privacy_key));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExtractVIDPIDFromAttributeString(DNAttrType attrType, const ByteSpan & attr,
                                            AttestationCertVidPid & vidpidFromMatterAttr, AttestationCertVidPid & vidpidFromCNAttr)
{
    ReturnErrorCodeIf(attrType == DNAttrType::kUnspecified, CHIP_NO_ERROR);
    ReturnErrorCodeIf(attr.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    if (attrType == DNAttrType::kMatterVID || attrType == DNAttrType::kMatterPID)
    {
        uint16_t matterAttr;
        VerifyOrReturnError(attr.size() == kVIDandPIDHexLength, CHIP_ERROR_WRONG_CERT_DN);
        VerifyOrReturnError(Encoding::UppercaseHexToUint16(reinterpret_cast<const char *>(attr.data()), attr.size(), matterAttr) ==
                                sizeof(matterAttr),
                            CHIP_ERROR_WRONG_CERT_DN);

        if (attrType == DNAttrType::kMatterVID)
        {
            // Not more than one VID attribute can be present.
            ReturnErrorCodeIf(vidpidFromMatterAttr.mVendorId.HasValue(), CHIP_ERROR_WRONG_CERT_DN);
            vidpidFromMatterAttr.mVendorId.SetValue(static_cast<VendorId>(matterAttr));
        }
        else
        {
            // Not more than one PID attribute can be present.
            ReturnErrorCodeIf(vidpidFromMatterAttr.mProductId.HasValue(), CHIP_ERROR_WRONG_CERT_DN);
            vidpidFromMatterAttr.mProductId.SetValue(matterAttr);
        }
    }
    // Otherwise, it is a CommonName attribute.
    else if (!vidpidFromCNAttr.Initialized())
    {
        ByteSpan attr_source_span{ attr };
        if (attr_source_span.size() > chip::Crypto::kMax_CommonNameAttr_Length)
        {
            attr_source_span.reduce_size(chip::Crypto::kMax_CommonNameAttr_Length);
        }

        // Try to find a valid Vendor ID encoded in fallback method.
        uint16_t vid   = 0;
        CHIP_ERROR err = Find16BitUpperCaseHexAfterPrefix(attr_source_span, kVIDPrefixForCNEncoding, vid);
        if (err == CHIP_NO_ERROR)
        {
            vidpidFromCNAttr.mVendorId.SetValue(static_cast<VendorId>(vid));
        }
        else if (err != CHIP_ERROR_NOT_FOUND)
        {
            // This indicates a bad/ambiguous format.
            return err;
        }

        // Try to find a valid Product ID encoded in fallback method.
        uint16_t pid = 0;
        err          = Find16BitUpperCaseHexAfterPrefix(attr_source_span, kPIDPrefixForCNEncoding, pid);
        if (err == CHIP_NO_ERROR)
        {
            vidpidFromCNAttr.mProductId.SetValue(pid);
        }
        else if (err != CHIP_ERROR_NOT_FOUND)
        {
            // This indicates a bad/ambiguous format.
            return err;
        }
    }

    return CHIP_NO_ERROR;
}

// Generates the to-be-signed portion of a PKCS#10 CSR (`CertificationRequestInformation`)
// that contains the
static CHIP_ERROR GenerateCertificationRequestInformation(ASN1Writer & writer, const Crypto::P256PublicKey & pubkey)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    /**
     *
     *  CertificationRequestInfo ::=
     *     SEQUENCE {
     *        version       INTEGER { v1(0) } (v1,...),
     *        subject       Name,
     *        subjectPKInfo SubjectPublicKeyInfo{{ PKInfoAlgorithms }},
     *        attributes    [0] Attributes{{ CRIAttributes }}
     * }
     */
    ASN1_START_SEQUENCE
    {
        ASN1_ENCODE_INTEGER(0); // version INTEGER { v1(0) }

        // subject Name
        ASN1_START_SEQUENCE
        {
            ASN1_START_SET
            {
                ASN1_START_SEQUENCE
                {
                    // Any subject, placeholder is good, since this
                    // is going to usually be ignored
                    ASN1_ENCODE_OBJECT_ID(kOID_AttributeType_OrganizationalUnitName);
                    ASN1_ENCODE_STRING(kASN1UniversalTag_UTF8String, "CSA", static_cast<uint16_t>(strlen("CSA")));
                }
                ASN1_END_SEQUENCE;
            }
            ASN1_END_SET;
        }
        ASN1_END_SEQUENCE;

        // subjectPKInfo
        ASN1_START_SEQUENCE
        {
            ASN1_START_SEQUENCE
            {
                ASN1_ENCODE_OBJECT_ID(kOID_PubKeyAlgo_ECPublicKey);
                ASN1_ENCODE_OBJECT_ID(kOID_EllipticCurve_prime256v1);
            }
            ASN1_END_SEQUENCE;
            ReturnErrorOnFailure(writer.PutBitString(0, pubkey, static_cast<uint8_t>(pubkey.Length())));
        }
        ASN1_END_SEQUENCE;

        // attributes [0]
        ASN1_START_CONSTRUCTED(kASN1TagClass_ContextSpecific, 0)
        {
            // Using a plain empty attributes request
            ASN1_START_SEQUENCE
            {
                ASN1_ENCODE_OBJECT_ID(kOID_Extension_CSRRequest);
                ASN1_START_SET
                {
                    ASN1_START_SEQUENCE {}
                    ASN1_END_SEQUENCE;
                }
                ASN1_END_SET;
            }
            ASN1_END_SEQUENCE;
        }
        ASN1_END_CONSTRUCTED;
    }
    ASN1_END_SEQUENCE;
exit:
    return err;
}

CHIP_ERROR GenerateCertificateSigningRequest(const P256Keypair * keypair, MutableByteSpan & csr_span)
{
    VerifyOrReturnError(keypair != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(csr_span.size() >= kMIN_CSR_Buffer_Size, CHIP_ERROR_BUFFER_TOO_SMALL);

    // First pass: Generate the CertificatioRequestInformation inner
    // encoding one time, to sign it, before re-generating it within the
    // full ASN1 writer later, since it's easier than trying to
    // figure-out the span we need to sign of the overall object.
    P256ECDSASignature signature;

    {
        // The first pass will just generate a signature, so we can use the
        // output buffer as scratch to avoid needing more stack space. There
        // are no secrets here and the contents is not reused since all we
        // need is the signature which is already separately stored.
        ASN1Writer toBeSignedWriter;
        toBeSignedWriter.Init(csr_span);
        CHIP_ERROR err = GenerateCertificationRequestInformation(toBeSignedWriter, keypair->Pubkey());
        ReturnErrorOnFailure(err);

        size_t encodedLen = (uint16_t) toBeSignedWriter.GetLengthWritten();
        // This should not/will not happen
        if (encodedLen > csr_span.size())
        {
            return CHIP_ERROR_INTERNAL;
        }

        err = keypair->ECDSA_sign_msg(csr_span.data(), encodedLen, signature);
        ReturnErrorOnFailure(err);
    }

    // Second pass: Generate the entire CSR body, restarting a new write
    // of the CertificationRequestInformation (cheap) and adding the
    // signature.
    //
    // See RFC2986 for ASN.1 module, repeated here in snippets
    CHIP_ERROR err = CHIP_NO_ERROR;

    ASN1Writer writer;
    writer.Init(csr_span);

    ASN1_START_SEQUENCE
    {

        /*  CertificationRequestInfo ::=
         *     SEQUENCE {
         *        version       INTEGER { v1(0) } (v1,...),
         *        subject       Name,
         *        subjectPKInfo SubjectPublicKeyInfo{{ PKInfoAlgorithms }},
         *        attributes    [0] Attributes{{ CRIAttributes }}
         *     }
         */
        GenerateCertificationRequestInformation(writer, keypair->Pubkey());

        // algorithm  AlgorithmIdentifier
        ASN1_START_SEQUENCE
        {
            // See RFC5480 sec 2.1
            ASN1_ENCODE_OBJECT_ID(kOID_SigAlgo_ECDSAWithSHA256);
        }
        ASN1_END_SEQUENCE;

        // signature  BIT STRING --> ECDSA-with-SHA256 signature with P256 key with R,S integers format
        // (see RFC3279 sec 2.2.3 ECDSA Signature Algorithm)
        ASN1_START_BIT_STRING_ENCAPSULATED
        {
            // Convert raw signature to embedded signature
            FixedByteSpan<Crypto::kP256_ECDSA_Signature_Length_Raw> rawSig(signature.Bytes());

            uint8_t derInt[kP256_FE_Length + kEmitDerIntegerWithoutTagOverhead];

            // Ecdsa-Sig-Value ::= SEQUENCE
            ASN1_START_SEQUENCE
            {
                using P256IntegerSpan = FixedByteSpan<Crypto::kP256_FE_Length>;
                // r INTEGER
                {
                    MutableByteSpan derIntSpan(derInt, sizeof(derInt));
                    ReturnErrorOnFailure(ConvertIntegerRawToDerWithoutTag(P256IntegerSpan(rawSig.data()), derIntSpan));
                    ReturnErrorOnFailure(writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false,
                                                         derIntSpan.data(), static_cast<uint16_t>(derIntSpan.size())));
                }

                // s INTEGER
                {
                    MutableByteSpan derIntSpan(derInt, sizeof(derInt));
                    ReturnErrorOnFailure(
                        ConvertIntegerRawToDerWithoutTag(P256IntegerSpan(rawSig.data() + kP256_FE_Length), derIntSpan));
                    ReturnErrorOnFailure(writer.PutValue(kASN1TagClass_Universal, kASN1UniversalTag_Integer, false,
                                                         derIntSpan.data(), static_cast<uint16_t>(derIntSpan.size())));
                }
            }
            ASN1_END_SEQUENCE;
        }
        ASN1_END_ENCAPSULATED;
    }
    ASN1_END_SEQUENCE;

exit:
    // Update size of output buffer on success
    if (err == CHIP_NO_ERROR)
    {
        csr_span.reduce_size(writer.GetLengthWritten());
    }
    return err;
}

CHIP_ERROR VerifyCertificateSigningRequestFormat(const uint8_t * csr, size_t csr_length)
{
    // Ensure we have enough size to validate header, and that our assumptions are met
    // for some tag computations below. A csr_length > 65535 would never be seen in
    // practice.
    VerifyOrReturnError((csr_length >= 16) && (csr_length <= 65535), CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

    Reader reader(csr, csr_length);

    // Ensure we have an outermost SEQUENCE
    uint8_t seq_header = 0;
    ReturnErrorOnFailure(reader.Read8(&seq_header).StatusCode());
    VerifyOrReturnError(seq_header == kSeqTag, CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

    size_t seq_length = 0;
    VerifyOrReturnError(ReadDerLength(reader, seq_length) == CHIP_NO_ERROR, CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);
    // Ensure that outer length matches sequence length + tag overhead, otherwise
    // we have trailing garbage
    size_t header_overhead = (seq_length <= 127) ? 2 : ((seq_length <= 255) ? 3 : 4);
    VerifyOrReturnError(csr_length == (seq_length + header_overhead), CHIP_ERROR_UNSUPPORTED_CERT_FORMAT);

    return CHIP_NO_ERROR;
}

} // namespace Crypto
} // namespace chip
