/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <string.h>
#include <support/BufferReader.h>
#include <support/BufferWriter.h>
#include <support/CodeUtils.h>

using chip::Encoding::BufferWriter;
using chip::Encoding::LittleEndian::Reader;

namespace {

constexpr uint8_t kIntegerTag         = 0x02u;
constexpr uint8_t kSeqTag             = 0x30u;
constexpr size_t kMinSequenceOverhead = 1 /* tag */ + 1 /* length */ + 1 /* actual data or second length byte*/;

CHIP_ERROR ReadDerLength(Reader & reader, uint8_t & length)
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

    // Did not early return: > 7 bit length, the number of bytes of the length is provided next.
    uint8_t length_bytes = cur_byte & 0x7Fu;

    if ((length_bytes > 1) || !reader.HasAtLeast(length_bytes))
    {
        // We only support lengths of 0..255 over 2 bytes
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        // Next byte has length 0..255.
        return reader.Read8(&length).StatusCode();
    }
}

CHIP_ERROR ReadDerUnsignedIntegerIntoRaw(Reader & reader, uint8_t * raw_integer_out, size_t raw_integer_length)
{
    uint8_t cur_byte = 0;

    ReturnErrorOnFailure(reader.Read8(&cur_byte).StatusCode());

    // We expect first tag to be INTEGER
    VerifyOrReturnError(cur_byte == kIntegerTag, CHIP_ERROR_INVALID_ARGUMENT);

    // Read the length
    uint8_t integer_len = 0;
    ReturnErrorOnFailure(ReadDerLength(reader, integer_len));

    // Clear the destination buffer, so we can blit the unsigned value into place
    memset(raw_integer_out, 0, raw_integer_length);

    // Check for pseudo-zero to mark unsigned value
    // This means we have too large an integer (should be at most 1 byte too large), it's invalid
    ReturnErrorCodeIf(integer_len > (raw_integer_length + 1), CHIP_ERROR_INVALID_ARGUMENT);

    if (integer_len == (raw_integer_length + 1u))
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
    size_t offset = raw_integer_length - integer_len;
    return reader.ReadBytes(&raw_integer_out[offset], integer_len).StatusCode();
}

size_t EmitDerIntegerFromRaw(const uint8_t * raw_integer, size_t raw_integer_length_bytes, uint8_t * out_der_integer,
                             size_t out_der_integer_size)
{
    if ((raw_integer == nullptr) || (raw_integer_length_bytes == 0))
    {
        return 0;
    }

    Reader reader(raw_integer, static_cast<uint16_t>(raw_integer_length_bytes));
    BufferWriter writer(out_der_integer, static_cast<uint16_t>(out_der_integer_size));

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

    // The + 1 is to account for the last consummed byte of the loop to skip leading zeros
    size_t length = reader.Remaining() + 1 + (needs_leading_zero_byte ? 1 : 0);

    if (length > 127)
    {
        // We do not support length over more than 1 bytes.
        return 0;
    }

    // Put INTEGER tag
    writer.Put(kIntegerTag);

    // Put length over 1 byte (i.e. MSB clear)
    writer.Put(static_cast<uint8_t>(length));

    // If leading zero or no more bytes remaining, must ensure we start we at least a zero byte
    if (needs_leading_zero_byte)
    {
        writer.Put(static_cast<uint8_t>(0u));
    }

    // Put first consummed byte from last read iteration of leading zero suppression
    writer.Put(cur_byte);

    // Fill the rest from the input in order
    while ((reader.Remaining() > 0) && (reader.Read8(&cur_byte).StatusCode() == CHIP_NO_ERROR))
    {
        // Emit all other bytes as-is
        writer.Put(cur_byte);
    }

    size_t actually_written = 0;
    if (!writer.Fit(actually_written))
    {
        // Somehow it was too big, return 0 for error.
        return 0;
    }

    return actually_written;
}

} // namespace

namespace chip {
namespace Crypto {

#ifdef ENABLE_HSM_HKDF
using HKDF_sha_crypto = HKDF_shaHSM;
#else
using HKDF_sha_crypto = HKDF_sha;
#endif

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
    state = CHIP_SPAKE2P_STATE::PREINIT;

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

    SuccessOrExit(error = Mac(Kcaorb, hash_size / 2, in, in_len, out));

    state = CHIP_SPAKE2P_STATE::R2;
    error = CHIP_NO_ERROR;
exit:
    *out_len = hash_size;
    return error;
}

CHIP_ERROR Spake2p::GenerateKeys()
{
    static const uint8_t info_keyconfirm[16] = { 'C', 'o', 'n', 'f', 'i', 'r', 'm', 'a', 't', 'i', 'o', 'n', 'K', 'e', 'y', 's' };

    ReturnErrorOnFailure(HashFinalize(Kae));
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
    ReturnErrorOnFailure(MacVerify(Kcaorb, hash_size / 2, in, in_len, point_buffer, point_size));

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
    ReturnErrorOnFailure(sha256_hash_ctx.AddData(in, in_len));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::HashFinalize(uint8_t * out)
{
    ReturnErrorOnFailure(sha256_hash_ctx.Finish(out));
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

CHIP_ERROR EcdsaRawSignatureToAsn1(size_t fe_length_bytes, const uint8_t * raw_sig, size_t raw_sig_length, uint8_t * out_asn1_sig,
                                   size_t out_asn1_sig_length, size_t & out_asn1_sig_actual_length)
{
    VerifyOrReturnError(fe_length_bytes > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(raw_sig != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(raw_sig_length == (2u * fe_length_bytes), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_asn1_sig != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_asn1_sig_length >= (raw_sig_length + kMax_ECDSA_X9Dot62_Asn1_Overhead), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Write both R an S integers past the overhead, we will shift them back later if we only needed 2 size bytes.
    uint8_t * cursor = &out_asn1_sig[kMinSequenceOverhead];
    size_t remaining = out_asn1_sig_length - kMinSequenceOverhead;

    size_t integers_length = 0;

    // Write R (first `fe_length_bytes` block of raw signature)
    size_t integer_length = EmitDerIntegerFromRaw(&raw_sig[0], fe_length_bytes, cursor, remaining);
    VerifyOrReturnError(integer_length != 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(integer_length < remaining, CHIP_ERROR_INTERNAL);
    remaining -= integer_length;
    integers_length += integer_length;
    cursor += integer_length;

    // Write S (second `fe_length_bytes` block of raw signature)
    integer_length = EmitDerIntegerFromRaw(&raw_sig[fe_length_bytes], fe_length_bytes, cursor, remaining);
    VerifyOrReturnError(integer_length != 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(integer_length <= remaining, CHIP_ERROR_INTERNAL);
    integers_length += integer_length;

    // We only support outputs that would use 1 or 2 bytes of DER length after the SEQUENCE tag
    VerifyOrReturnError(integers_length <= UINT8_MAX, CHIP_ERROR_INVALID_ARGUMENT);

    // We now know the length of both variable sized integers in the sequence, so we
    // can write the tag and length.
    BufferWriter writer(out_asn1_sig, static_cast<uint16_t>(out_asn1_sig_length));

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
    writer.Put(&out_asn1_sig[kMinSequenceOverhead], integers_length);

    size_t actually_written = 0;
    VerifyOrReturnError(writer.Fit(actually_written), CHIP_ERROR_BUFFER_TOO_SMALL);

    out_asn1_sig_actual_length = actually_written;
    return CHIP_NO_ERROR;
}

CHIP_ERROR EcdsaAsn1SignatureToRaw(size_t fe_length_bytes, const uint8_t * asn1_sig, size_t asn1_sig_length, uint8_t * out_raw_sig,
                                   size_t out_raw_sig_length)
{
    VerifyOrReturnError(fe_length_bytes > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(asn1_sig != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(asn1_sig_length > kMinSequenceOverhead, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Output raw signature is <r,s> both of which are of fe_length_bytes (see SEC1).
    VerifyOrReturnError(out_raw_sig_length >= (2u * fe_length_bytes), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(out_raw_sig != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    chip::Encoding::LittleEndian::Reader reader(asn1_sig, static_cast<uint16_t>(asn1_sig_length));

    // Make sure we have a starting Sequence
    uint8_t tag = 0;
    ReturnErrorOnFailure(reader.Read8(&tag).StatusCode());
    VerifyOrReturnError(tag == kSeqTag, CHIP_ERROR_INVALID_ARGUMENT);

    // Read length of sequence
    uint8_t tag_len = 0;
    ReturnErrorOnFailure(ReadDerLength(reader, tag_len));

    // Length of sequence must match what is left of signature
    VerifyOrReturnError(tag_len == reader.Remaining(), CHIP_ERROR_INVALID_ARGUMENT);

    // Can now clear raw signature integers r,s one by one
    uint8_t * raw_cursor = out_raw_sig;

    // Read R
    ReturnErrorOnFailure(ReadDerUnsignedIntegerIntoRaw(reader, raw_cursor, fe_length_bytes));

    raw_cursor += fe_length_bytes;

    // Read S
    ReturnErrorOnFailure(ReadDerUnsignedIntegerIntoRaw(reader, raw_cursor, fe_length_bytes));

    return CHIP_NO_ERROR;
}

} // namespace Crypto
} // namespace chip
