/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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
 *          KVS Implementation for SE05x
 *          The functions are used to read the NFC commissioned data from SE05x.
 */

#include <platform/nxp/crypto/se05x/CHIPCryptoPALHsm_se05x_utils.h>
#include <platform/nxp/crypto/se05x/kvs_utilities/CHIPCryptoPALHsm_se05x_readClusters.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVReader.h>
#include <lib/core/TLVTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

#include <stdio.h>
#include <string.h>

#define NXP_CRYPTO_KEY_MAGIC                                                                                                       \
    {                                                                                                                              \
        0xA5, 0xA6, 0xB5, 0xB6, 0xA5, 0xA6, 0xB5, 0xB6                                                                             \
    }

#define NIST256_HEADER_OFFSET 26

#define VERSION_8_4 ((8) << (8 * 3) | (4) << (8 * 2) | (32) << (8 * 1))

#define VERSION_8_12 ((8) << (8 * 3) | (12) << (8 * 2) | (32) << (8 * 1))

#define VERSION_8_18 ((8) << (8 * 3) | (18) << (8 * 2) | (32) << (8 * 1))

#define MAKE_SE05X_NETWORK_ID(keyID) (0x7FFF3400 + keyID)
#define SE051H_COMM_PARAMETERS 0x7FFF3002

using namespace chip;
using namespace chip::TLV;

static uint8_t g_fabric_index  = 0;
static uint32_t g_noc_chain_id = 0;
static uint32_t g_root_cert_id = 0;
/* For IPK calculation */
FabricId g_fabric_id                                                 = kUndefinedFabricId;
uint8_t g_p256_root_public_key[chip::Crypto::kP256_PublicKey_Length] = { 0 };

/*
 * The function will create a reference key for the NIST 256 key created in the secure element.
 * The reference key can be used to refer the key used in secure element for ECDSA operations.
 * The contents of the reference key will be < Public Key + Magic number + key id >
 */
static CHIP_ERROR se05x_create_refkey(const uint8_t * pubKeyBuf, size_t pubKeyBufLen, uint32_t keyId, uint8_t * outBuf,
                                      size_t * outLen)
{
    const uint8_t header[]         = { 0x15, 0x24, 0x00, 0x01, 0x30, 0x01, 0x61 };
    const uint8_t se05x_magic_no[] = NXP_CRYPTO_KEY_MAGIC;
    size_t offset                  = 0;

    VerifyOrReturnError(pubKeyBuf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outBuf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outLen != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(pubKeyBufLen != chip::Crypto::kP256_PublicKey_Length, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError((offset + sizeof(header)) <= *outLen, CHIP_ERROR_INTERNAL);
    memcpy(&outBuf[offset], header, sizeof(header));
    offset += sizeof(header);

    VerifyOrReturnError((offset + (pubKeyBufLen - NIST256_HEADER_OFFSET)) <= *outLen, CHIP_ERROR_INTERNAL);
    memcpy(&outBuf[offset], pubKeyBuf + 16, pubKeyBufLen - NIST256_HEADER_OFFSET);
    offset += pubKeyBufLen - NIST256_HEADER_OFFSET;

    VerifyOrReturnError((SIZE_MAX - sizeof(se05x_magic_no)) >= (offset), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError((offset + sizeof(se05x_magic_no)) <= *outLen, CHIP_ERROR_INTERNAL);
    memcpy(&outBuf[offset], se05x_magic_no, sizeof(se05x_magic_no));
    offset += sizeof(se05x_magic_no);

    uint8_t keyIdBytes[] = { static_cast<uint8_t>((keyId >> 24) & 0xFF), static_cast<uint8_t>((keyId >> 16) & 0xFF),
                             static_cast<uint8_t>((keyId >> 8) & 0xFF), static_cast<uint8_t>(keyId & 0xFF) };

    VerifyOrReturnError((SIZE_MAX - sizeof(keyIdBytes)) >= (offset), CHIP_ERROR_INTERNAL);
    VerifyOrReturnError((offset + sizeof(keyIdBytes)) <= *outLen, CHIP_ERROR_INTERNAL);
    memcpy(&outBuf[offset], keyIdBytes, sizeof(keyIdBytes));
    offset += sizeof(keyIdBytes);
    memset(&outBuf[offset], 0x00, 20);
    offset += 20;
    outBuf[offset++] = 0x18;
    *outLen          = offset;

    return CHIP_NO_ERROR;
}

uint8_t se05x_get_fabric_id()
{
    return g_fabric_index;
}

CHIP_ERROR se05x_is_nfc_commissioning_done()
{
    CHIP_ERROR status = CHIP_NO_ERROR;
    std::vector<uint8_t> op_cred_buff(512);
    size_t op_cred_buff_len = op_cred_buff.size();

    status = se05x_get_certificate(SE051H_OP_CRED_CLUSTER_ID, op_cred_buff.data(), &op_cred_buff_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    if (op_cred_buff.data()[op_cred_buff_len - 1] == 0x00)
    {
        return CHIP_ERROR_INTERNAL;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR se05x_read_node_operational_keypair(uint8_t * op_key_ref_key, size_t * op_key_ref_key_len)
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    VerifyOrReturnError(op_key_ref_key != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(op_key_ref_key_len != NULL, CHIP_ERROR_INTERNAL);

    std::vector<uint8_t> pubKeyBuf(256);
    size_t pubKeyBufLen = pubKeyBuf.size();

    status = se05x_get_certificate(SE051H_NODE_OP_KEY_ID, pubKeyBuf.data(), &pubKeyBufLen);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    status = se05x_create_refkey(pubKeyBuf.data(), pubKeyBufLen, SE051H_NODE_OP_KEY_ID, op_key_ref_key, op_key_ref_key_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_node_oper_cert(uint8_t * noc_buf, size_t * noc_buf_len)
{
    CHIP_ERROR status      = CHIP_NO_ERROR;
    uint64_t id_64b        = 0;
    TLVType outertype      = kTLVType_NotSpecified;
    TLVType outerContainer = kTLVType_NotSpecified;
    TLVReader reader;

    // Get SE05x applet version for version-specific handling
    sss_se05x_session_t * se05x_session = reinterpret_cast<sss_se05x_session_t *>(&gex_sss_chip_ctx.session);
    VerifyOrReturnError(se05x_session != nullptr, CHIP_ERROR_INTERNAL);

    // Validate input parameters
    VerifyOrReturnError(noc_buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(noc_buf_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*noc_buf_len > 0, CHIP_ERROR_INVALID_ARGUMENT);

    const size_t original_buf_len = *noc_buf_len;
    size_t buf_len                = original_buf_len;

    noc_buf[0] = 0x15;
    buf_len    = original_buf_len - 1;

    // Read operational credential cluster data
    status = se05x_get_certificate(SE051H_OP_CRED_CLUSTER_ID, noc_buf + 1, &buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);
    VerifyOrReturnError(buf_len > 0, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(buf_len < (original_buf_len - 3), CHIP_ERROR_INTERNAL);

    noc_buf[buf_len + 1] = 0x18;
    buf_len              = buf_len + 2;

    // Parse TLV structure to extract NOC chain ID
    reader.Init(noc_buf, buf_len);

    // Navigate to the cluster data structure
    ReturnErrorOnFailure(reader.Next(kTLVType_Structure, AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerContainer));

    const uint32_t applet_version = se05x_session->s_ctx.applet_version;

    // SE05x applet versions > 8.18 use CommonTag format with 16-bit tag values
    // instead of ContextTag with 8-bit values
    if (applet_version <= VERSION_8_18)
    {
        ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFF)));
        ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFD)));
        ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFC)));
        ReturnErrorOnFailure(reader.Next(kTLVType_List, ContextTag(0xFB)));
        ReturnErrorOnFailure(reader.Next(kTLVType_List, ContextTag(0xF9)));
        ReturnErrorOnFailure(reader.Next(kTLVType_List, ContextTag(0xF8)));
    }
    else
    {
        ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, CommonTag(0xFFFF)));
        ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, CommonTag(0xFFFD)));
        ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, CommonTag(0xFFFC)));
        ReturnErrorOnFailure(reader.Next(kTLVType_List, CommonTag(0xFFFB)));
        ReturnErrorOnFailure(reader.Next(kTLVType_List, CommonTag(0xFFF9)));
        ReturnErrorOnFailure(reader.Next(kTLVType_List, CommonTag(0xFFF8)));
    }

    // Extract NOC chain ID
    ReturnErrorOnFailure(reader.Next(kTLVType_List, ContextTag(0x00)));
    ReturnErrorOnFailure(reader.EnterContainer(outertype));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.Get(id_64b));

    g_noc_chain_id = static_cast<uint32_t>(id_64b >> 32);
    g_noc_chain_id = ((g_noc_chain_id >> 24) & 0x000000FF) | ((g_noc_chain_id >> 8) & 0x0000FF00) |
        ((g_noc_chain_id << 8) & 0x00FF0000) | ((g_noc_chain_id << 24) & 0xFF000000);

    ChipLogDetail(Crypto, "SE05x: Node Operational certificate chain id: 0x%08" PRIX32, g_noc_chain_id);
    ReturnErrorOnFailure(reader.ExitContainer(outertype));

    // Extract root certificate ID
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(0x02)));
    ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(0x03)));
    ReturnErrorOnFailure(reader.Next(kTLVType_List, ContextTag(0x04)));
    ReturnErrorOnFailure(reader.EnterContainer(outertype));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.Get(id_64b));

    g_root_cert_id = static_cast<uint32_t>(id_64b >> 32);
    g_root_cert_id = ((g_root_cert_id >> 24) & 0x000000FF) | ((g_root_cert_id >> 8) & 0x0000FF00) |
        ((g_root_cert_id << 8) & 0x00FF0000) | ((g_root_cert_id << 24) & 0xFF000000);

    ChipLogDetail(Crypto, "SE05x: Root certificate id: 0x%08" PRIX32, g_root_cert_id);
    ReturnErrorOnFailure(reader.ExitContainer(outertype));

    // Extract fabric index
    ReturnErrorOnFailure(reader.Next(kTLVType_UnsignedInteger, ContextTag(0x05)));
    ReturnErrorOnFailure(reader.Get(g_fabric_index));

    VerifyOrReturnError(g_fabric_index == 1, CHIP_ERROR_INVALID_FABRIC_INDEX);
    ChipLogDetail(Crypto, "SE05x: Fabric index: %" PRIu8, g_fabric_index);

    // Read NOC chain (contains NOC and ICA)
    buf_len = original_buf_len;
    status  = se05x_get_certificate(g_noc_chain_id, noc_buf, &buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);

    // NOC Chain format for applet version <= 8.18:
    //   [0-1]: Total length (2 bytes)
    //   [2-3]: Reserved (2 bytes)
    //   [4-5]: NOC length (2 bytes, little-endian)
    //   [6..]: NOC data
    //
    // NOC Chain format for applet version > 8.18:
    //   [0-1]: Total length (2 bytes)
    //   [2-4]: Reserved (3 bytes)
    //   [5-6]: NOC length (2 bytes, little-endian)
    //   [7..]: NOC data
    const size_t kNocChainHeaderSize = (applet_version <= VERSION_8_18) ? 6 : 7;
    const size_t kNocLengthOffset = (applet_version <= VERSION_8_18) ? 4 : 5;

    VerifyOrReturnError(buf_len >= kNocChainHeaderSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    const size_t noc_len = static_cast<size_t>((noc_buf[kNocLengthOffset + 1] << 8) | noc_buf[kNocLengthOffset]);

    VerifyOrReturnError(noc_len > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(buf_len >= (kNocChainHeaderSize + noc_len), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(original_buf_len >= noc_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Move NOC data to beginning of buffer
    memmove(noc_buf, &noc_buf[kNocChainHeaderSize], noc_len);
    *noc_buf_len = noc_len;

    // Extract Node ID and Fabric ID for IPK calculation
    MutableByteSpan nocSpan{ noc_buf, noc_len };
    NodeId g_node_id = kUndefinedNodeId;
    ReturnErrorOnFailure(chip::Credentials::ExtractNodeIdFabricIdFromOpCert(nocSpan, &g_node_id, &g_fabric_id));

    ChipLogDetail(Crypto, "SE05x: Extracted NodeId: 0x%016" PRIX64 ", FabricId: 0x%016" PRIX64, g_node_id, g_fabric_id);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_root_cert(uint8_t * root_cert_buf, size_t * root_cert_buf_len)
{
    // Validate input parameters
    VerifyOrReturnError(root_cert_buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(root_cert_buf_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*root_cert_buf_len > 0, CHIP_ERROR_INVALID_ARGUMENT);

    const size_t original_buf_len = *root_cert_buf_len;
    size_t buf_len                = original_buf_len;

    // Read root certificate from SE05x
    CHIP_ERROR status = se05x_get_certificate(g_root_cert_id, root_cert_buf, &buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);

    /* Root certificate format in SE05x:
     *   [0-1]: Total length (2 bytes, big-endian)
     *   [2-4]: Reserved/padding (3 bytes)
     *   [5..]: Root certificate data
     */
    constexpr size_t kRootCertHeaderSize = 5;
    constexpr size_t kLengthFieldSize    = 2;

    VerifyOrReturnError(buf_len >= kLengthFieldSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Extract root certificate length from header (big-endian)
    const size_t root_cert_len = static_cast<size_t>(((root_cert_buf[0] << 8) | root_cert_buf[1]) - 3);

    // Validate certificate length
    VerifyOrReturnError(root_cert_len > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(buf_len >= (kRootCertHeaderSize + root_cert_len), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(original_buf_len >= root_cert_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Move root certificate data to beginning of buffer
    memmove(root_cert_buf, &root_cert_buf[kRootCertHeaderSize], root_cert_len);
    *root_cert_buf_len = root_cert_len;

    // Extract root public key for IPK calculation
    MutableByteSpan rcacSpan{ root_cert_buf, root_cert_len };
    chip::Credentials::P256PublicKeySpan root_pub_key_span;
    ReturnErrorOnFailure(chip::Credentials::ExtractPublicKeyFromChipCert(rcacSpan, root_pub_key_span));

    // Validate public key size
    VerifyOrReturnError(root_pub_key_span.size() == chip::Crypto::kP256_PublicKey_Length, CHIP_ERROR_INTERNAL);

    // Store root public key for later IPK calculation
    memcpy(g_p256_root_public_key, root_pub_key_span.data(), root_pub_key_span.size());

    ChipLogDetail(Crypto, "SE05x: Successfully read root certificate (length: %lu bytes)", (unsigned long) root_cert_len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_ICA(uint8_t * ica_buf, size_t * ica_buf_len)
{
    // Validate input parameters
    VerifyOrReturnError(ica_buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(ica_buf_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*ica_buf_len > 0, CHIP_ERROR_INVALID_ARGUMENT);

    // Get SE05x applet version for version-specific handling
    sss_se05x_session_t * se05x_session = reinterpret_cast<sss_se05x_session_t *>(&gex_sss_chip_ctx.session);
    VerifyOrReturnError(se05x_session != nullptr, CHIP_ERROR_INTERNAL);

    const uint32_t applet_version = se05x_session->s_ctx.applet_version;

    // Validate that NOC chain ID has been initialized
    VerifyOrReturnError(g_noc_chain_id != 0, CHIP_ERROR_INCORRECT_STATE);

    const size_t original_buf_len = *ica_buf_len;
    size_t buf_len                = original_buf_len;

    // Read Node Operational Certificate chain from SE05x (contains both NOC and ICA)
    CHIP_ERROR status = se05x_get_certificate(g_noc_chain_id, ica_buf, &buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);

    /* NOC Chain format stored in SE05x:
     *   [0-1]:   Total chain length (2 bytes, big-endian)
     *   [2-3]:   Reserved/padding (2 bytes)
     *   [4-5]:   NOC length (2 bytes, little-endian)
     *   [6..]:   NOC certificate data
     *   [...]:   ICA length (2 bytes, little-endian)
     *   [...+2]: ICA certificate data
     */
    const size_t kChainHeaderSize = (applet_version <= VERSION_8_18) ? 6 : 7;
    const size_t kNocLengthOffset = (applet_version <= VERSION_8_18) ? 4 : 5;
    constexpr size_t kLengthFieldSize = 2;

    // Validate minimum buffer size for header
    VerifyOrReturnError(buf_len >= kChainHeaderSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Extract NOC length (little-endian format)
    const size_t noc_len = static_cast<size_t>((ica_buf[kNocLengthOffset + 1] << 8) | ica_buf[kNocLengthOffset]);

    // Validate NOC length
    VerifyOrReturnError(noc_len > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(noc_len <= (buf_len - kChainHeaderSize), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Calculate ICA offset: header + NOC data + ICA length field
    const size_t ica_length_offset = kChainHeaderSize + noc_len + kLengthFieldSize;

    // Validate buffer has space for ICA length field
    VerifyOrReturnError(buf_len >= (ica_length_offset + kLengthFieldSize), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Extract ICA length (little-endian format)
    const size_t ica_len = static_cast<size_t>((ica_buf[ica_length_offset + 1] << 8) | ica_buf[ica_length_offset]);

    // Validate ICA length
    VerifyOrReturnError(ica_len > 0, CHIP_ERROR_INTERNAL);

    // Calculate ICA data offset
    const size_t ica_data_offset = ica_length_offset + kLengthFieldSize;

    // Validate buffer has complete ICA certificate
    VerifyOrReturnError((SIZE_MAX - ica_len) >= ica_data_offset, CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(buf_len >= (ica_data_offset + ica_len), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Validate output buffer can hold ICA certificate
    VerifyOrReturnError(original_buf_len >= ica_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Move ICA certificate data to beginning of buffer
    memmove(ica_buf, &ica_buf[ica_data_offset], ica_len);
    *ica_buf_len = ica_len;

    ChipLogDetail(Crypto, "SE05x: Successfully extracted ICA certificate (NOC length: %lu, ICA length: %lu bytes)", (unsigned long) noc_len,
                  (unsigned long) ica_len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_ipk(uint8_t * ipk_buf, size_t * ipk_buf_len)
{
    // Validate input parameters
    VerifyOrReturnError(ipk_buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(ipk_buf_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*ipk_buf_len > 0, CHIP_ERROR_INVALID_ARGUMENT);

    // Validate that we have the required fabric information
    VerifyOrReturnError(g_fabric_id != kUndefinedFabricId, CHIP_ERROR_INCORRECT_STATE);

    constexpr size_t kEpochKeySize            = Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;
    constexpr size_t kIPKValueSize            = 16;
    constexpr size_t kCertBufferSize          = 128;
    constexpr size_t kIPKDataBufferSize       = 128;
    constexpr uint8_t kGroupKeySetId          = 0x00;
    constexpr uint8_t kGroupKeySecurityPolicy = 0x01;
    constexpr uint32_t kEpochKey0Id           = 0xEAD0;
    constexpr uint32_t kGroupKeyMapId         = 0x01A3;

    uint8_t cert_buf[kCertBufferSize]    = { 0 };
    size_t cert_buf_len                  = sizeof(cert_buf);
    uint8_t ipk_data[kIPKDataBufferSize] = { 0 };

    // Read epoch key from SE05x
    CHIP_ERROR status = se05x_get_certificate(SE051H_IPK_ID, cert_buf, &cert_buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);
    VerifyOrReturnError(cert_buf_len >= kEpochKeySize, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Derive group operational key from epoch key
    uint8_t encryption_key_buf[kEpochKeySize]          = { 0 };
    uint8_t compressed_fabric_id_buf[sizeof(uint64_t)] = { 0 };

    MutableByteSpan compressed_fabric_id_span(compressed_fabric_id_buf);
    MutableByteSpan encryption_key_span(encryption_key_buf);
    ByteSpan epoch_key_span(cert_buf, kEpochKeySize);
    chip::Credentials::P256PublicKeySpan root_pub_key_span(g_p256_root_public_key);

    // Generate compressed fabric ID
    ReturnErrorOnFailure(chip::Crypto::GenerateCompressedFabricId(root_pub_key_span, g_fabric_id, compressed_fabric_id_span));

    // Derive the group operational key
    ReturnErrorOnFailure(Crypto::DeriveGroupOperationalKey(epoch_key_span, compressed_fabric_id_span, encryption_key_span));

    // Build IPK TLV structure
    TLVWriter writer;
    TLVType outer_container = kTLVType_NotSpecified;
    TLVType inner_container = kTLVType_NotSpecified;

    writer.Init(ipk_data, sizeof(ipk_data));
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outer_container));

    // GroupKeySetID (context tag 1)
    ReturnErrorOnFailure(writer.Put(ContextTag(1), kGroupKeySetId));

    // GroupKeySecurityPolicy (context tag 2)
    ReturnErrorOnFailure(writer.Put(ContextTag(2), kGroupKeySecurityPolicy));

    // EpochKeys array (context tag 3)
    ReturnErrorOnFailure(writer.StartContainer(ContextTag(0x03), kTLVType_Array, inner_container));

    // Helper lambda to write epoch key entries
    uint8_t ipk_value[kIPKValueSize] = { 0 };
    auto write_epoch_key_entry       = [&](uint8_t epoch_index, uint32_t epoch_start_time) -> CHIP_ERROR {
        TLVType epoch_container = kTLVType_NotSpecified;
        ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, epoch_container));
        ReturnErrorOnFailure(writer.Put(ContextTag(4), epoch_index));
        ReturnErrorOnFailure(writer.Put(ContextTag(5), epoch_start_time));
        ReturnErrorOnFailure(writer.PutBytes(ContextTag(6), ipk_value, sizeof(ipk_value)));
        ReturnErrorOnFailure(writer.EndContainer(epoch_container));
        return CHIP_NO_ERROR;
    };

    // Write three epoch key entries
    ReturnErrorOnFailure(write_epoch_key_entry(0x00, kEpochKey0Id));
    ReturnErrorOnFailure(write_epoch_key_entry(0x00, 0x00));
    ReturnErrorOnFailure(write_epoch_key_entry(0x00, 0x00));

    ReturnErrorOnFailure(writer.EndContainer(inner_container));

    // GroupKeyMapID (context tag 7)
    ReturnErrorOnFailure(writer.Put(ContextTag(7), kGroupKeyMapId));

    ReturnErrorOnFailure(writer.EndContainer(outer_container));
    ReturnErrorOnFailure(writer.Finalize());

    const size_t tlv_written = writer.GetLengthWritten();
    VerifyOrReturnError(tlv_written >= 36, CHIP_ERROR_INTERNAL);

    // Construct final IPK buffer: [TLV prefix] + [encryption key] + [TLV suffix]
    constexpr size_t kTLVPrefixSize   = 20;
    constexpr size_t kTLVSuffixOffset = 36;

    const size_t required_size = kTLVPrefixSize + kEpochKeySize + (tlv_written - kTLVSuffixOffset);
    VerifyOrReturnError(*ipk_buf_len >= required_size, CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t offset = 0;

    // Copy TLV prefix (first 20 bytes)
    memcpy(ipk_buf + offset, ipk_data, kTLVPrefixSize);
    offset += kTLVPrefixSize;

    // Insert derived encryption key
    memcpy(ipk_buf + offset, encryption_key_buf, kEpochKeySize);
    offset += kEpochKeySize;

    // Copy TLV suffix (remaining bytes after offset 36)
    const size_t suffix_size = tlv_written - kTLVSuffixOffset;
    memcpy(ipk_buf + offset, ipk_data + kTLVSuffixOffset, suffix_size);
    offset += suffix_size;

    *ipk_buf_len = offset;

    ChipLogDetail(Crypto, "SE05x: Successfully constructed IPK (length: %lu bytes)", (unsigned long) offset);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_acl_data(uint8_t * acl, size_t * acl_len)
{
    // Validate input parameters
    VerifyOrReturnError(acl != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(acl_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*acl_len > 0, CHIP_ERROR_INVALID_ARGUMENT);

    const size_t original_buf_len = *acl_len;

    // Read ACL data from SE05x
    CHIP_ERROR status = se05x_get_certificate(SE051H_ACL_ID, acl, acl_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);
    VerifyOrReturnError(*acl_len > 0, CHIP_ERROR_INTERNAL);

    // Get SE05x applet version for version-specific handling
    sss_se05x_session_t * se05x_session = reinterpret_cast<sss_se05x_session_t *>(&gex_sss_chip_ctx.session);
    VerifyOrReturnError(se05x_session != nullptr, CHIP_ERROR_INTERNAL);

    const uint32_t applet_version = se05x_session->s_ctx.applet_version;

    // Handle version-specific ACL format differences
    if (applet_version <= VERSION_8_4)
    {
        // For version 8.4 and earlier, use fixed ACL length
        constexpr size_t kVersion84AclLength = 18;
        VerifyOrReturnError(original_buf_len >= kVersion84AclLength, CHIP_ERROR_BUFFER_TOO_SMALL);
        *acl_len = kVersion84AclLength;
        ChipLogDetail(Crypto, "SE05x: ACL data for version 8.4 (length: %lu bytes)", (unsigned long) *acl_len);
    }
    else
    {
        // For newer versions, extract length from header and remove it
        constexpr size_t kLengthHeaderSize = 2;
        VerifyOrReturnError(*acl_len >= kLengthHeaderSize, CHIP_ERROR_BUFFER_TOO_SMALL);

        // Extract ACL length from header (big-endian)
        const uint16_t acl_data_len = static_cast<uint16_t>((acl[0] << 8) | acl[1]);

        VerifyOrReturnError(acl_data_len > 0, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(*acl_len >= (kLengthHeaderSize + acl_data_len), CHIP_ERROR_BUFFER_TOO_SMALL);
        VerifyOrReturnError(original_buf_len >= acl_data_len, CHIP_ERROR_BUFFER_TOO_SMALL);

        // Remove length header by moving data to beginning of buffer
        memmove(acl, acl + kLengthHeaderSize, acl_data_len);
        *acl_len = acl_data_len;

        ChipLogDetail(Crypto, "SE05x: ACL data extracted (length: %lu bytes)", (unsigned long) *acl_len);
    }

    // Apply version-specific ACL format corrections
    constexpr size_t kByte7Offset = 7;
    constexpr size_t kByte9Offset = 9;
    constexpr uint8_t kByte7Value = 0x36;
    constexpr uint8_t kByte9Value = 0x06;

    // Validate buffer has sufficient size for modifications
    VerifyOrReturnError(*acl_len > kByte7Offset, CHIP_ERROR_INTERNAL);

    // Fix byte at offset 7 for all versions
    acl[kByte7Offset] = kByte7Value;

    // Fix byte at offset 9 for version 8.12 and earlier
    if (applet_version <= VERSION_8_12)
    {
        VerifyOrReturnError(*acl_len > kByte9Offset, CHIP_ERROR_INTERNAL);
        acl[kByte9Offset] = kByte9Value;
    }

    // Parse TLV to find and fix target index position
    TLVReader reader;
    TLVType outerContainerType = kTLVType_NotSpecified;
    TLVType innerContainerType = kTLVType_NotSpecified;

    reader.Init(acl, *acl_len);

    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.EnterContainer(outerContainerType));

    // Skip to the inner container
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(reader.EnterContainer(innerContainerType));
    ReturnErrorOnFailure(reader.ExitContainer(innerContainerType));

    // Get position after inner container
    const uint32_t target_index = reader.GetLengthRead();

    ReturnErrorOnFailure(reader.ExitContainer(outerContainerType));

    // Apply correction at target index position
    VerifyOrReturnError(target_index < *acl_len, CHIP_ERROR_INTERNAL);
    acl[target_index] = kByte7Value;

    ChipLogDetail(Crypto, "SE05x: Successfully read and processed ACL data (final length: %lu bytes)", (unsigned long) *acl_len);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_fabric_groups(uint8_t * fabgrp_data, size_t * fabgrp_data_len)
{
    // Validate input parameters
    VerifyOrReturnError(fabgrp_data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(fabgrp_data_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*fabgrp_data_len > 0, CHIP_ERROR_INVALID_ARGUMENT);

    // Validate fabric index is within valid range
    VerifyOrReturnError(g_fabric_index > 0, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(g_fabric_index <= UINT8_MAX, CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Define fabric group structure field values
    constexpr uint8_t kVendorId      = 0x00;
    constexpr uint8_t kVendorGroupId = 0x00;
    constexpr uint8_t kGroupKeySetId = 0x00;
    constexpr uint8_t kReserved1     = 0x00;
    constexpr uint8_t kReserved2     = 0x00;
    constexpr uint8_t kReserved3     = 0x00;

    // Estimate minimum required buffer size for TLV structure
    constexpr size_t kMinimumBufferSize = 32;
    VerifyOrReturnError(*fabgrp_data_len >= kMinimumBufferSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    TLVWriter writer;
    TLVType outerContainer = kTLVType_NotSpecified;

    // Initialize TLV writer
    writer.Init(fabgrp_data, *fabgrp_data_len);

    // Build fabric groups TLV structure
    // Structure format:
    //   Context Tag 1: VendorId (uint8)
    //   Context Tag 2: VendorGroupId (uint8)
    //   Context Tag 3: GroupKeySetId (uint8)
    //   Context Tag 4: Reserved field (uint8)
    //   Context Tag 5: Reserved field (uint8)
    //   Context Tag 6: FabricIndex (uint8)
    //   Context Tag 7: Reserved field (uint8)
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainer));

    // Write VendorId field (context tag 1)
    ReturnErrorOnFailure(writer.Put(ContextTag(1), kVendorId));

    // Write VendorGroupId field (context tag 2)
    ReturnErrorOnFailure(writer.Put(ContextTag(2), kVendorGroupId));

    // Write GroupKeySetId field (context tag 3)
    ReturnErrorOnFailure(writer.Put(ContextTag(3), kGroupKeySetId));

    // Write reserved field (context tag 4)
    ReturnErrorOnFailure(writer.Put(ContextTag(4), kReserved1));

    // Write reserved field (context tag 5)
    ReturnErrorOnFailure(writer.Put(ContextTag(5), kReserved2));

    // Write FabricIndex field (context tag 6) - use actual fabric index
    ReturnErrorOnFailure(writer.Put(ContextTag(6), g_fabric_index));

    // Write reserved field (context tag 7)
    ReturnErrorOnFailure(writer.Put(ContextTag(7), kReserved3));

    // Finalize the TLV structure
    ReturnErrorOnFailure(writer.EndContainer(outerContainer));
    ReturnErrorOnFailure(writer.Finalize());

    // Update output length with actual written data
    const size_t written_length = writer.GetLengthWritten();
    VerifyOrReturnError(written_length > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(written_length <= *fabgrp_data_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    *fabgrp_data_len = written_length;

    ChipLogDetail(Crypto, "SE05x: Successfully created fabric groups data (fabric index: %" PRIu8 ", length: %lu bytes)",
                  g_fabric_index, (unsigned long) written_length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_meta_data(uint8_t * meta_data, size_t * meta_data_len)
{
    // Validate input parameters
    VerifyOrReturnError(meta_data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(meta_data_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*meta_data_len > 0, CHIP_ERROR_INVALID_ARGUMENT);

    const size_t original_buf_len = *meta_data_len;

    // Read basic info cluster data from SE05x (reuse input buffer)
    CHIP_ERROR status = se05x_get_certificate(SE051H_BASIC_INFO_CLUSTER_ID, meta_data, meta_data_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);
    VerifyOrReturnError(*meta_data_len > 0, CHIP_ERROR_INTERNAL);

    // Parse the basic info cluster TLV structure to extract vendor ID
    TLVReader reader;
    uint32_t vendor_id = 0;

    reader.Init(meta_data, *meta_data_len);

    // Navigate through the cluster data structure
    // Expected structure hierarchy:
    //   Context Tag 0xFF: Cluster metadata (UnsignedInteger)
    //   Context Tag 0xFD: Cluster metadata (UnsignedInteger)
    //   Context Tag 0xFC: Cluster metadata (UnsignedInteger)
    //   Context Tag 0xFB: Cluster data array (Array)
    //   Context Tag 0xF9: Cluster attributes array (Array)
    //   Context Tag 0xF8: Attribute entries array (Array)
    //   Context Tag 0x00: DataModelRevision (UnsignedInteger)
    //   Context Tag 0x01: VendorName (UTF8String)
    //   Context Tag 0x1F: Unknown field (SignedInteger)
    //   Context Tag 0x02: VendorID (UnsignedInteger) <- Target field

    TEMPORARY_RETURN_IGNORED reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFF));
    TEMPORARY_RETURN_IGNORED reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFD));
    TEMPORARY_RETURN_IGNORED reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFC));
    TEMPORARY_RETURN_IGNORED reader.Next(kTLVType_Array, ContextTag(0xFB));
    TEMPORARY_RETURN_IGNORED reader.Next(kTLVType_Array, ContextTag(0xF9));
    TEMPORARY_RETURN_IGNORED reader.Next(kTLVType_Array, ContextTag(0xF8));

    // Navigate to basic info attributes
    TEMPORARY_RETURN_IGNORED reader.Next(kTLVType_UnsignedInteger, ContextTag(0x00)); // DataModelRevision
    TEMPORARY_RETURN_IGNORED reader.Next(kTLVType_UTF8String, ContextTag(0x01));      // VendorName
    TEMPORARY_RETURN_IGNORED reader.Next(kTLVType_SignedInteger, ContextTag(0x1F));   // Unknown field

    // Extract VendorID
    TEMPORARY_RETURN_IGNORED reader.Next(kTLVType_UnsignedInteger, ContextTag(0x02));

    TEMPORARY_RETURN_IGNORED reader.Get(vendor_id);

    ChipLogDetail(Crypto, "SE05x: Extracted VendorID: 0x%04" PRIX32, vendor_id);

    // Build simplified metadata TLV structure with VendorID and empty VendorName
    TLVWriter writer;
    TLVType outerContainer = kTLVType_NotSpecified;

    // Estimate minimum required buffer size
    constexpr size_t kMinimumMetadataBufferSize = 32;
    VerifyOrReturnError(original_buf_len >= kMinimumMetadataBufferSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    writer.Init(meta_data, original_buf_len);

    // Create metadata structure:
    //   Context Tag 0: VendorID (uint32)
    //   Context Tag 1: VendorName (empty string)
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainer));

    // Write VendorID field (context tag 0)
    ReturnErrorOnFailure(writer.Put(ContextTag(0), vendor_id));

    // Write empty VendorName field (context tag 1)
    ReturnErrorOnFailure(writer.PutString(ContextTag(1), ""));

    ReturnErrorOnFailure(writer.EndContainer(outerContainer));
    ReturnErrorOnFailure(writer.Finalize());

    // Update output length with actual written data
    const size_t written_length = writer.GetLengthWritten();
    VerifyOrReturnError(written_length > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(written_length <= original_buf_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    *meta_data_len = written_length;

    ChipLogDetail(Crypto, "SE05x: Successfully created metadata (VendorID: 0x%04" PRIX32 ", length: %lu bytes)", vendor_id,
                  (unsigned long) written_length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_fabric_index_info_data(uint8_t * fab_info_data, size_t * fab_info_data_len)
{
    // Validate input parameters
    VerifyOrReturnError(fab_info_data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(fab_info_data_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*fab_info_data_len > 0, CHIP_ERROR_INVALID_ARGUMENT);

    // Validate fabric index is in valid state
    VerifyOrReturnError(g_fabric_index > 0, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(g_fabric_index <= UINT8_MAX, CHIP_ERROR_INVALID_FABRIC_INDEX);

    const size_t original_buf_len = *fab_info_data_len;

    // Define fabric index info structure constants
    constexpr uint8_t kCurrentFabricIndex = 0x02; // Current fabric index identifier
    constexpr size_t kMinimumBufferSize   = 32;   // Minimum buffer size for TLV structure

    // Validate buffer has sufficient capacity
    VerifyOrReturnError(original_buf_len >= kMinimumBufferSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    TLVWriter writer;
    TLVType outerContainer = kTLVType_NotSpecified;
    TLVType innerContainer = kTLVType_NotSpecified;

    // Initialize TLV writer
    writer.Init(fab_info_data, original_buf_len);

    // Build fabric index info TLV structure
    // Structure format:
    //   Anonymous Tag: Structure container
    //     Context Tag 0: CurrentFabricIndex (uint8) - Identifier for current fabric
    //     Context Tag 1: Array container
    //       Anonymous Tag: FabricIndex value (uint8) - Actual fabric index from SE05x
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outerContainer));

    // Write CurrentFabricIndex field (context tag 0)
    // This identifies which fabric index is currently active
    ReturnErrorOnFailure(writer.Put(ContextTag(0), kCurrentFabricIndex));

    // Start fabric indices array (context tag 1)
    ReturnErrorOnFailure(writer.StartContainer(ContextTag(1), kTLVType_Array, innerContainer));

    // Write the actual fabric index value retrieved from SE05x
    const uint8_t fabric_index_value = g_fabric_index;
    ReturnErrorOnFailure(writer.Put(AnonymousTag(), fabric_index_value));

    // Close the array container
    ReturnErrorOnFailure(writer.EndContainer(innerContainer));

    // Close the structure container
    ReturnErrorOnFailure(writer.EndContainer(outerContainer));

    // Finalize the TLV encoding
    ReturnErrorOnFailure(writer.Finalize());

    // Validate and update output length
    const size_t written_length = writer.GetLengthWritten();
    VerifyOrReturnError(written_length > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(written_length <= original_buf_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    *fab_info_data_len = written_length;

    ChipLogDetail(Crypto, "SE05x: Successfully created fabric index info (fabric index: %" PRIu8 ", length: %lu bytes)",
                  g_fabric_index, (unsigned long) written_length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_wifi_and_thread_credentials(uint8_t * buf, size_t buflen, char * ssid, size_t * ssid_len, char * password,
                                                  size_t * password_len, char * opdata, size_t * opdata_len, uint32_t * keyId)
{
    // Validate input parameters
    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(ssid != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(ssid_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(password != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(password_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(opdata != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(opdata_len != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(keyId != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(buflen > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*ssid_len > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*password_len > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(*opdata_len > 0, CHIP_ERROR_INVALID_ARGUMENT);

    // Read credential data from SE05x using the provided key ID
    size_t read_len   = buflen;
    CHIP_ERROR status = se05x_get_certificate(*keyId, buf, &read_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);
    VerifyOrReturnError(read_len > 0, CHIP_ERROR_INTERNAL);

    // Credential data format in SE05x:
    //   [0-1]: Total length (2 bytes, big-endian)
    //   [2..]: Credential payload
    constexpr size_t kLengthHeaderSize = 2;
    constexpr uint8_t kWiFiTLVMarker   = 0x30; // TLV marker indicating WiFi credentials

    VerifyOrReturnError(read_len >= kLengthHeaderSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Extract credential payload length from header (big-endian)
    const uint16_t credential_len = static_cast<uint16_t>((buf[0] << 8) | buf[1]);

    VerifyOrReturnError(credential_len > 0, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(read_len >= (kLengthHeaderSize + credential_len), CHIP_ERROR_BUFFER_TOO_SMALL);

    // Get SE05x applet version for version-specific handling
    sss_se05x_session_t * se05x_session = reinterpret_cast<sss_se05x_session_t *>(&gex_sss_chip_ctx.session);
    VerifyOrReturnError(se05x_session != nullptr, CHIP_ERROR_INTERNAL);

    const uint32_t applet_version = se05x_session->s_ctx.applet_version;

    bool is_wifi_credential = false;

    if (applet_version < VERSION_8_12) {
        // Determine credential type based on TLV marker
        // WiFi credentials are stored in TLV format (starts with 0x30)
        // Thread operational data is stored in raw binary format (does not start with 0x30)
        is_wifi_credential = (read_len > kLengthHeaderSize) && (buf[2] == kWiFiTLVMarker);
    }
    else {
        is_wifi_credential = true;
    }

    if (is_wifi_credential)
    {
        // Parse WiFi credentials from TLV structure
        // Expected structure:
        //   Context Tag 0x00: SSID (UTF8String)
        //   Context Tag 0x01: Password (UTF8String) - optional
        TLVReader reader;
        reader.Init(&buf[kLengthHeaderSize], credential_len);

        // Read SSID field (context tag 0x00)
        TEMPORARY_RETURN_IGNORED reader.Next();

        const size_t ssid_data_len = reader.GetLength();
        VerifyOrReturnError(ssid_data_len > 0, CHIP_ERROR_INVALID_TLV_ELEMENT);
        VerifyOrReturnError(ssid_data_len <= *ssid_len, CHIP_ERROR_BUFFER_TOO_SMALL);

        status = reader.GetString(ssid, (*ssid_len) + 1);
        VerifyOrReturnError(status == CHIP_NO_ERROR, status);
        *ssid_len = ssid_data_len;

        // Read next field to check if it's a password or operational data
        TEMPORARY_RETURN_IGNORED reader.Next();
        const Tag current_tag = reader.GetTag();
        if (current_tag == ContextTag(0x01))
        {
            // Password field found (context tag 0x01)
            const size_t password_data_len = reader.GetLength();
            VerifyOrReturnError(password_data_len > 0, CHIP_ERROR_INVALID_TLV_ELEMENT);
            VerifyOrReturnError(password_data_len <= *password_len, CHIP_ERROR_BUFFER_TOO_SMALL);

            status = reader.GetString(password, (*password_len) + 1);
            VerifyOrReturnError(status == CHIP_NO_ERROR, status);
            *password_len = password_data_len;

            ChipLogDetail(Crypto, "SE05x: WiFi SSID extracted (length: %lu bytes)", (unsigned long) ssid_data_len);
            ChipLogDetail(Crypto, "SE05x: WiFi password extracted (length: %lu bytes)", (unsigned long) password_data_len);
        }
        else
        {
            // Not a password field - treat SSID data as operational data
            ChipLogDetail(Crypto, "SE05x: No password field found, treating as operational data");
            ChipLogDetail(Crypto, "SE05x: Thread operational data detected (length: %lu bytes)", (unsigned long) ssid_data_len);

            VerifyOrReturnError(*ssid_len <= *opdata_len, CHIP_ERROR_BUFFER_TOO_SMALL);
            memcpy(opdata, ssid, *ssid_len);
            *opdata_len   = *ssid_len;
            *ssid_len     = 0;
            *password_len = 0;
        }
    }
    else
    {
        // Thread operational data (raw binary format)
        ChipLogDetail(Crypto, "SE05x: Thread operational data detected (length: %" PRIu16 " bytes)", credential_len);

        VerifyOrReturnError(credential_len <= *opdata_len, CHIP_ERROR_BUFFER_TOO_SMALL);

        // Copy raw operational data (skip length header)
        memcpy(opdata, &buf[kLengthHeaderSize], credential_len);
        *opdata_len   = credential_len;
        *ssid_len     = 0;
        *password_len = 0;
    }

    ChipLogDetail(Crypto, "SE05x: Successfully read credentials (keyID: 0x%08" PRIX32 ")", *keyId);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_reset_breadcrumb()
{
    // Check if general commissioning cluster exists in SE05x
    bool cluster_exists = false;
    CHIP_ERROR status   = se05x_check_object_exists(SE051H_GENERAL_COMM_CLUSTER_ID, &cluster_exists);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);

    if (!cluster_exists)
    {
        ChipLogDetail(Crypto, "SE05x: General commissioning cluster does not exist, nothing to reset");
        return CHIP_NO_ERROR;
    }

    // Read existing general commissioning cluster data from SE05x
    constexpr size_t kMaxClusterDataSize      = 256;
    uint8_t cluster_data[kMaxClusterDataSize] = { 0 };
    size_t cluster_data_len                   = sizeof(cluster_data);

    status = se05x_get_certificate(SE051H_GENERAL_COMM_CLUSTER_ID, cluster_data, &cluster_data_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);
    VerifyOrReturnError(cluster_data_len > 0, CHIP_ERROR_INTERNAL);

    // Parse TLV structure to locate breadcrumb attribute
    // Expected structure hierarchy:
    //   Multiple metadata fields (context tags 0xFF, 0xFD, 0xFC, 0xFB, 0xF9, 0xF8)
    //   Context Tag 0x00: Breadcrumb attribute (UnsignedInteger, 8 bytes)
    TLVReader reader;
    TLVType outer_container = kTLVType_NotSpecified;

    reader.Init(cluster_data, cluster_data_len);

    // Navigate through cluster metadata fields
    TEMPORARY_RETURN_IGNORED reader.Next();
    TEMPORARY_RETURN_IGNORED reader.Next();
    TEMPORARY_RETURN_IGNORED reader.Next();
    TEMPORARY_RETURN_IGNORED reader.Next();
    TEMPORARY_RETURN_IGNORED reader.Next();
    TEMPORARY_RETURN_IGNORED reader.Next();

    // Enter and exit container to position reader correctly
    ReturnErrorOnFailure(reader.EnterContainer(outer_container));
    ReturnErrorOnFailure(reader.ExitContainer(outer_container));

    // Get current read position (points to breadcrumb attribute)
    const uint8_t * breadcrumb_ptr = reader.GetReadPoint();
    VerifyOrReturnError(breadcrumb_ptr != nullptr, CHIP_ERROR_INTERNAL);

    // Validate pointer is within buffer bounds
    VerifyOrReturnError(breadcrumb_ptr >= cluster_data, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(breadcrumb_ptr < (cluster_data + cluster_data_len), CHIP_ERROR_INTERNAL);

    // Move to next element (breadcrumb attribute)
    TEMPORARY_RETURN_IGNORED reader.Next();

    const Tag current_tag      = reader.GetTag();
    const TLVType current_type = reader.GetType();

    // Verify this is the breadcrumb attribute
    VerifyOrReturnError(current_tag == ContextTag(0x00), CHIP_ERROR_NOT_FOUND);
    VerifyOrReturnError(current_type == kTLVType_UnsignedInteger, CHIP_ERROR_INVALID_TLV_ELEMENT);

    // Calculate breadcrumb offset in buffer
    const size_t breadcrumb_offset = static_cast<size_t>(breadcrumb_ptr - cluster_data);

    // Breadcrumb TLV format:
    //   [offset + 0]: TLV control byte (0x27 for 8-byte unsigned integer)
    //   [offset + 1]: Context tag (0x00)
    //   [offset + 2 to offset + 9]: 8-byte breadcrumb value
    constexpr size_t kBreadcrumbTLVHeaderSize = 2; // Control byte + tag
    constexpr size_t kBreadcrumbValueSize     = 8; // 8-byte unsigned integer
    constexpr size_t kBreadcrumbTotalSize     = kBreadcrumbTLVHeaderSize + kBreadcrumbValueSize;
    constexpr uint8_t kBreadcrumbControlByte  = 0x27; // TLV control for 8-byte uint
    constexpr uint8_t kBreadcrumbContextTag   = 0x00;

    // Validate buffer has enough space for breadcrumb data
    VerifyOrReturnError((breadcrumb_offset + kBreadcrumbTotalSize) <= cluster_data_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Verify breadcrumb TLV structure
    if (cluster_data[breadcrumb_offset] == kBreadcrumbControlByte && cluster_data[breadcrumb_offset + 1] == kBreadcrumbContextTag)
    {
        // Read current breadcrumb value for logging
        uint64_t current_breadcrumb = 0;
        memcpy(&current_breadcrumb, &cluster_data[breadcrumb_offset + kBreadcrumbTLVHeaderSize], kBreadcrumbValueSize);

        ChipLogDetail(Crypto, "SE05x: Resetting breadcrumb from 0x%016" PRIX64 " to 0x0000000000000000", current_breadcrumb);

        // Reset breadcrumb value to zero (keep TLV header intact)
        memset(&cluster_data[breadcrumb_offset + kBreadcrumbTLVHeaderSize], 0x00, kBreadcrumbValueSize);
    }
    else
    {
        ChipLogError(Crypto, "SE05x: Invalid breadcrumb TLV structure (control: 0x%02X, tag: 0x%02X)",
                     cluster_data[breadcrumb_offset], cluster_data[breadcrumb_offset + 1]);
        return CHIP_ERROR_INVALID_TLV_ELEMENT;
    }

    // Write updated cluster data back to SE05x
    status = se05x_set_binary_data(SE051H_GENERAL_COMM_CLUSTER_ID, cluster_data, cluster_data_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);

    ChipLogDetail(Crypto, "SE05x: Successfully reset breadcrumb attribute");

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_reset_iscomm_without_power(bool is_comm_without_power)
{
    // Check if general commissioning cluster exists in SE05x
    bool cluster_exists = false;
    CHIP_ERROR status   = se05x_check_object_exists(SE051H_GENERAL_COMM_CLUSTER_ID, &cluster_exists);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);

    if (!cluster_exists)
    {
        ChipLogDetail(Crypto, "SE05x: General commissioning cluster does not exist, nothing to update");
        return CHIP_NO_ERROR;
    }

    // Read existing general commissioning cluster data from SE05x
    constexpr size_t kMaxClusterDataSize      = 256;
    uint8_t cluster_data[kMaxClusterDataSize] = { 0 };
    size_t cluster_data_len                   = sizeof(cluster_data);

    status = se05x_get_certificate(SE051H_GENERAL_COMM_CLUSTER_ID, cluster_data, &cluster_data_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);
    VerifyOrReturnError(cluster_data_len > 0, CHIP_ERROR_INTERNAL);

    // Parse TLV structure to locate IsCommWithoutPower attribute
    // Expected structure hierarchy:
    //   Multiple metadata and attribute fields
    //   Context Tag 0x0C: IsCommWithoutPower attribute (Boolean)
    //
    // Navigation path (16 Next() calls to reach the attribute):
    //   - Skip cluster metadata fields
    //   - Skip other commissioning attributes
    //   - Reach IsCommWithoutPower at context tag 0x0C
    TLVReader reader;
    reader.Init(cluster_data, cluster_data_len);

    // Navigate through TLV structure to reach IsCommWithoutPower attribute
    // This requires navigating past 15 previous elements
    constexpr size_t kNavigationSteps = 16;
    for (size_t i = 0; i < kNavigationSteps; ++i)
    {
        TEMPORARY_RETURN_IGNORED reader.Next();
    }

    // Get current read position (points to IsCommWithoutPower attribute)
    const uint8_t * attribute_ptr = reader.GetReadPoint();
    VerifyOrReturnError(attribute_ptr != nullptr, CHIP_ERROR_INTERNAL);

    // Validate pointer is within buffer bounds
    VerifyOrReturnError(attribute_ptr >= cluster_data, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(attribute_ptr < (cluster_data + cluster_data_len), CHIP_ERROR_INTERNAL);

    // Move to the attribute element
    TEMPORARY_RETURN_IGNORED reader.Next();

    const Tag current_tag      = reader.GetTag();
    const TLVType current_type = reader.GetType();

    // Verify this is the IsCommWithoutPower attribute
    constexpr uint8_t kIsCommWithoutPowerContextTag = 0x0C;
    VerifyOrReturnError(current_tag == ContextTag(kIsCommWithoutPowerContextTag), CHIP_ERROR_NOT_FOUND);
    VerifyOrReturnError(current_type == kTLVType_Boolean, CHIP_ERROR_INVALID_TLV_ELEMENT);

    // Calculate attribute offset in buffer
    const size_t attribute_offset = static_cast<size_t>(attribute_ptr - cluster_data);

    // IsCommWithoutPower TLV format:
    //   [offset]: TLV control byte
    //     0x28 = false (Boolean false)
    //     0x29 = true (Boolean true)
    constexpr uint8_t kBooleanFalseControlByte = 0x28;
    constexpr uint8_t kBooleanTrueControlByte  = 0x29;
    constexpr size_t kBooleanTLVSize           = 1; // Single byte for boolean value

    // Validate buffer has enough space for boolean data
    VerifyOrReturnError((attribute_offset + kBooleanTLVSize) <= cluster_data_len, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Read current value for logging
    const uint8_t current_value = cluster_data[attribute_offset];
    const bool current_state    = (current_value == kBooleanTrueControlByte);

    // Update IsCommWithoutPower attribute value
    const uint8_t new_value = is_comm_without_power ? kBooleanTrueControlByte : kBooleanFalseControlByte;

    if (current_value != new_value)
    {
        ChipLogDetail(Crypto, "SE05x: Updating IsCommWithoutPower from %s to %s", current_state ? "true" : "false",
                      is_comm_without_power ? "true" : "false");

        cluster_data[attribute_offset] = new_value;
    }
    else
    {
        ChipLogDetail(Crypto, "SE05x: IsCommWithoutPower already set to %s, no update needed",
                      is_comm_without_power ? "true" : "false");
        return CHIP_NO_ERROR;
    }

    // Write updated cluster data back to SE05x
    status = se05x_set_binary_data(SE051H_GENERAL_COMM_CLUSTER_ID, cluster_data, cluster_data_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);

    ChipLogDetail(Crypto, "SE05x: Successfully updated IsCommWithoutPower attribute");

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_net_id_from_net_comm_cluster(uint32_t * networkId)
{
    CHIP_ERROR status = CHIP_NO_ERROR;
    TLVReader reader;
    uint8_t readBuff[280]         = { 0 };
    size_t readBuffLen            = sizeof(readBuff);
    uint8_t maxNetworks           = 0;
    TLVType outerContainer        = kTLVType_NotSpecified;
    TLVType networksContainer     = kTLVType_NotSpecified;
    TLVType networkEntryContainer = kTLVType_NotSpecified;

    VerifyOrReturnError(networkId != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    readBuff[0] = 0x15;
    readBuffLen = readBuffLen - 1;

    // Read the network commissioning cluster data from SE05x
    status = se05x_get_certificate(SE051H_NCC_ID, readBuff + 1, &readBuffLen);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(readBuffLen < (sizeof(readBuff) - 3), CHIP_ERROR_INTERNAL);
    readBuff[readBuffLen + 1] = 0x18;
    readBuffLen               = readBuffLen + 2;

    reader.Init(readBuff, readBuffLen);

    // Navigate to the cluster data structure
    ReturnErrorOnFailure(reader.Next(kTLVType_Structure, AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outerContainer));

    // Skip initial fields to reach MaxNetworks attribute (context tag 0x00)
    while (reader.Next() == CHIP_NO_ERROR)
    {
        if (reader.GetTag() == ContextTag(0x00) && reader.GetType() == kTLVType_UnsignedInteger)
        {
            uint64_t maxNetworksValue = 0;
            ReturnErrorOnFailure(reader.Get(maxNetworksValue));
            VerifyOrReturnError(maxNetworksValue <= UINT8_MAX, CHIP_ERROR_INVALID_TLV_ELEMENT);
            maxNetworks = static_cast<uint8_t>(maxNetworksValue);
            ChipLogDetail(Crypto, "SE05x: MaxNetworks value: %u", maxNetworks);
            break;
        }
    }

    VerifyOrReturnError(maxNetworks > 0, CHIP_ERROR_INTERNAL);

    // For version 8.4, use hardcoded network ID
    if ((((sss_se05x_session_t *) (&gex_sss_chip_ctx.session))->s_ctx.applet_version) <= VERSION_8_4)
    {
        *networkId = SE051H_WIFI_CRED_ID_APP_8_4;
        ChipLogDetail(Crypto, "SE05x: Found network credential keyID = 0x%" PRIx32, *networkId);
        return CHIP_NO_ERROR;
    }

    // Find Networks attribute (context tag 0x01)
    while (reader.Next() == CHIP_NO_ERROR)
    {
        if (reader.GetTag() == ContextTag(0x01) && reader.GetType() == kTLVType_List)
        {
            ReturnErrorOnFailure(reader.EnterContainer(networksContainer));

            // Iterate through network entries
            uint8_t networkIndex = 0;
            while (reader.Next() == CHIP_NO_ERROR && networkIndex < maxNetworks)
            {
                VerifyOrReturnError(reader.GetType() == kTLVType_Structure, CHIP_ERROR_INVALID_TLV_ELEMENT);
                ReturnErrorOnFailure(reader.EnterContainer(networkEntryContainer));

                bool isConnected = false;

                // Parse network entry fields
                while (reader.Next() == CHIP_NO_ERROR)
                {
                    Tag currentTag = reader.GetTag();

                    // Check for Connected attribute (context tag 0x01)
                    if (currentTag == ContextTag(0x01) && reader.GetType() == kTLVType_Boolean)
                    {
                        ReturnErrorOnFailure(reader.Get(isConnected));

                        if (isConnected)
                        {
                            *networkId = MAKE_SE05X_NETWORK_ID(networkIndex);
                            ChipLogDetail(Crypto, "SE05x: Found connected network at index %u, keyID = 0x%" PRIx32, networkIndex,
                                          *networkId);
                            return CHIP_NO_ERROR;
                        }
                        break;
                    }
                }

                ReturnErrorOnFailure(reader.ExitContainer(networkEntryContainer));
                networkIndex++;
            }

            ReturnErrorOnFailure(reader.ExitContainer(networksContainer));
            break;
        }
    }

    ChipLogError(Crypto, "SE05x: No connected network credential found");
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR se05x_get_remain_fail_safe_time(uint16_t * fail_safe_time)
{
    // Validate input parameters
    VerifyOrReturnError(fail_safe_time != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Check if commissioning parameters object exists in SE05x
    bool params_exist = false;
    CHIP_ERROR status = se05x_check_object_exists(SE051H_COMM_PARAMETERS, &params_exist);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);

    if (!params_exist)
    {
        ChipLogDetail(Crypto, "SE05x: Commissioning parameters do not exist, returning zero fail-safe time");
        *fail_safe_time = 0;
        return CHIP_NO_ERROR;
    }

    // Read commissioning parameters data from SE05x
    constexpr size_t kMaxParamsDataSize     = 256;
    uint8_t params_data[kMaxParamsDataSize] = { 0 };
    size_t params_data_len                  = sizeof(params_data);

    status = se05x_get_certificate(SE051H_COMM_PARAMETERS, params_data, &params_data_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);
    VerifyOrReturnError(params_data_len > 0, CHIP_ERROR_INTERNAL);

    // Parse TLV structure to extract remaining fail-safe time
    // Expected structure:
    //   Element 0: Unknown/metadata field
    //   Element 1: Unknown/metadata field
    //   Element 2: Remaining fail-safe time (UnsignedInteger, uint16)
    TLVReader reader;
    reader.Init(params_data, params_data_len);

    // Navigate to the fail-safe time field (third element)
    TEMPORARY_RETURN_IGNORED reader.Next();
    TEMPORARY_RETURN_IGNORED reader.Next();
    TEMPORARY_RETURN_IGNORED reader.Next();

    // Verify the element type is unsigned integer
    const TLVType current_type = reader.GetType();
    VerifyOrReturnError(current_type == kTLVType_UnsignedInteger, CHIP_ERROR_INVALID_TLV_ELEMENT);

    // Extract the fail-safe time value
    uint64_t fail_safe_value = 0;
    ReturnErrorOnFailure(reader.Get(fail_safe_value));

    // Validate the value fits in uint16_t
    VerifyOrReturnError(fail_safe_value <= UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);

    *fail_safe_time = static_cast<uint16_t>(fail_safe_value);

    ChipLogDetail(Crypto, "SE05x: Remaining fail-safe time: %" PRIu16 " seconds (0x%04" PRIX16 ")", *fail_safe_time,
                  *fail_safe_time);

    return CHIP_NO_ERROR;
}
