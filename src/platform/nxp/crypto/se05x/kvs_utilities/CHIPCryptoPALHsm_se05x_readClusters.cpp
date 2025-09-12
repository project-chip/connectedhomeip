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

#include "CHIPCryptoPALHsm_se05x_readClusters.h"
#include "CHIPCryptoPALHsm_se05x_utils.h"
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
#define SE05X_OPERATIONAL_CRED_CLUSTER_ID 0x7FFE003E
#define SE05X_ROOT_CERTIFICATE_ID 0x7FFF3301
#define SE05X_NODE_OPERATIONAL_KEY_PAIR 0x7FFF3101
#define SE05X_IPK_ID 0x7FFF3601
#define SE05X_ACCESS_CONTROL_CLUSTER 0x7FFF3501
#define SE05X_BASIC_INFO_CONTROL_ID 0x7FFE0028
#define SE05X_WIFI_BINARY_FILE 0x7FFF3401

using namespace chip;
using namespace chip::TLV;

static uint32_t g_fabric_index = 0;
static uint32_t g_noc_chain_id = 0;
static uint32_t g_root_cert_id = 0;
/* For IPK calculation */
FabricId g_fabric_id                                                 = kUndefinedFabricId;
NodeId g_node_id                                                     = kUndefinedNodeId;
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

    VerifyOrReturnError((offset + sizeof(se05x_magic_no)) <= *outLen, CHIP_ERROR_INTERNAL);
    memcpy(&outBuf[offset], se05x_magic_no, sizeof(se05x_magic_no));
    offset += sizeof(se05x_magic_no);

    uint8_t keyIdBytes[] = { static_cast<uint8_t>((keyId >> 24) & 0xFF), static_cast<uint8_t>((keyId >> 16) & 0xFF),
                             static_cast<uint8_t>((keyId >> 8) & 0xFF), static_cast<uint8_t>(keyId & 0xFF) };

    VerifyOrReturnError((offset + sizeof(keyIdBytes)) <= *outLen, CHIP_ERROR_INTERNAL);
    memcpy(&outBuf[offset], keyIdBytes, sizeof(keyIdBytes));
    offset += sizeof(keyIdBytes);
    memset(&outBuf[offset], 0x00, 20);
    offset += 20;
    outBuf[offset++] = 0x18;
    *outLen          = offset;

    return CHIP_NO_ERROR;
}

uint32_t se05x_get_fabric_id()
{
    return g_fabric_index;
}

CHIP_ERROR se05x_is_nfc_commissioning_done()
{
    CHIP_ERROR status = CHIP_NO_ERROR;
    std::vector<uint8_t> op_cred_buff(512);
    size_t op_cred_buff_len = op_cred_buff.size();

    status = se05x_get_certificate(SE05X_OPERATIONAL_CRED_CLUSTER_ID, op_cred_buff.data(), &op_cred_buff_len);
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

#if 1
    std::vector<uint8_t> pubKeyBuf(256);
    size_t pubKeyBufLen = pubKeyBuf.size();
#else
    std::vector<uint8_t> pubKeyBuf = { 0x04, 0x70, 0x69, 0x74, 0x23, 0x2C, 0x12, 0x32, 0x09, 0x31, 0x9B, 0x18, 0xBF,
                                       0x63, 0x2A, 0x1C, 0xF5, 0xF2, 0x92, 0x08, 0x03, 0x0B, 0xCB, 0x3A, 0x76, 0x63,
                                       0xAF, 0x15, 0x55, 0x88, 0x67, 0xD7, 0xB0, 0x72, 0x3F, 0xE4, 0x68, 0x80, 0x3C,
                                       0xF8, 0x7E, 0x33, 0x6E, 0x2F, 0x94, 0xAA, 0xDF, 0xC3, 0xF3, 0xEE, 0x27, 0xA5,
                                       0x6B, 0x00, 0xAB, 0x59, 0xB3, 0x9A, 0x95, 0xE1, 0x28, 0x3A, 0x95, 0xDC, 0x39 };
    size_t pubKeyBufLen            = pubKeyBuf.size();
#endif

#if 1
    status = se05x_get_certificate(SE05X_NODE_OPERATIONAL_KEY_PAIR, pubKeyBuf.data(), &pubKeyBufLen);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
#endif

    status =
        se05x_create_refkey(pubKeyBuf.data(), pubKeyBufLen, SE05X_NODE_OPERATIONAL_KEY_PAIR, op_key_ref_key, op_key_ref_key_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_node_oper_cert(uint8_t * noc_buf, size_t * noc_buf_len)
{
    CHIP_ERROR status = CHIP_NO_ERROR;
    uint64_t id_64b   = 0;
    TLVType outertype = kTLVType_NotSpecified;
    TLVReader reader;

    VerifyOrReturnError(noc_buf != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(noc_buf_len != NULL, CHIP_ERROR_INTERNAL);

    size_t buf_len = *noc_buf_len;
    size_t noc_len = 0;

    status = se05x_get_certificate(SE05X_OPERATIONAL_CRED_CLUSTER_ID, noc_buf, &buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    reader.Init(noc_buf, *noc_buf_len);
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFF));
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFD));
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFC));
    reader.Next(kTLVType_List, ContextTag(0xFB));
    reader.Next(kTLVType_List, ContextTag(0xF9));
    reader.Next(kTLVType_List, ContextTag(0xF8));

    reader.Next(kTLVType_List, ContextTag(0x00));
    reader.EnterContainer(outertype);
    reader.Next();
    reader.Get(id_64b);

    g_noc_chain_id = static_cast<uint32_t>(id_64b >> 32);
    g_noc_chain_id = ((g_noc_chain_id >> 24) & 0x000000FF) | ((g_noc_chain_id >> 8) & 0x0000FF00) |
        ((g_noc_chain_id << 8) & 0x00FF0000) | ((g_noc_chain_id << 24) & 0xFF000000);

    ChipLogDetail(Crypto, "SE05x: Node Operational certificate chain id is : 0x%" PRIx32 "", g_noc_chain_id);
    reader.ExitContainer(outertype);

    reader.Next();
    reader.Next();
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0x02));
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0x03));
    reader.Next(kTLVType_List, ContextTag(0x04));
    reader.EnterContainer(outertype);
    reader.Next();
    reader.Get(id_64b);

    g_root_cert_id = static_cast<uint32_t>(id_64b >> 32);
    g_root_cert_id = ((g_root_cert_id >> 24) & 0x000000FF) | ((g_root_cert_id >> 8) & 0x0000FF00) |
        ((g_root_cert_id << 8) & 0x00FF0000) | ((g_root_cert_id << 24) & 0xFF000000);

    ChipLogDetail(Crypto, "SE05x: Root Cert id is : 0x%" PRIx32 "", g_root_cert_id);
    reader.ExitContainer(outertype);

    reader.Next(kTLVType_UnsignedInteger, ContextTag(0x05));
    reader.Get(g_fabric_index);

    VerifyOrReturnError(g_fabric_index == 1, CHIP_ERROR_INTERNAL);

    /* Read Node Operational certificate chain id. Contains NOC and ICA.*/

    buf_len = *noc_buf_len;
    status  = se05x_get_certificate(g_noc_chain_id, noc_buf, &buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    /* NOC Chain format ==>
        2 Bytes total length,
            NOC Length , NOC
            ICA length , ICA
    */

    VerifyOrReturnError(buf_len >= 6, CHIP_ERROR_INTERNAL);
    noc_len = (noc_buf[5] << 8) | noc_buf[4];

    VerifyOrReturnError(buf_len >= (6 + noc_len), CHIP_ERROR_INTERNAL);
    memmove(noc_buf, &noc_buf[6], noc_len);
    *noc_buf_len = noc_len;
    /* Required for IPK calculation later */
    MutableByteSpan nocSpan{ noc_buf, noc_len };
    ReturnErrorOnFailure(chip::Credentials::ExtractNodeIdFabricIdFromOpCert(nocSpan, &g_node_id, &g_fabric_id));

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_root_cert(uint8_t * root_cert_buf, size_t * root_cert_buf_len)
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    VerifyOrReturnError(root_cert_buf != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(root_cert_buf_len != NULL, CHIP_ERROR_INTERNAL);

    size_t buf_len       = *root_cert_buf_len;
    size_t root_cert_len = 0;

    /* Read Root certificate */

    status = se05x_get_certificate(g_root_cert_id, root_cert_buf, &buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(buf_len >= 2, CHIP_ERROR_INTERNAL);
    root_cert_len = (root_cert_buf[0] << 8) | root_cert_buf[1];

    VerifyOrReturnError(*root_cert_buf_len >= (5 + root_cert_len), CHIP_ERROR_INTERNAL);
    memmove(root_cert_buf, &root_cert_buf[5], root_cert_len);
    *root_cert_buf_len = root_cert_len;

    /* Required for IPK calculation later */
    MutableByteSpan rcacSpan{ root_cert_buf, *root_cert_buf_len };
    chip::Credentials::P256PublicKeySpan root_pub_key_span;
    ReturnErrorOnFailure(chip::Credentials::ExtractPublicKeyFromChipCert(rcacSpan, root_pub_key_span));

    memcpy(g_p256_root_public_key, root_pub_key_span.data(), root_pub_key_span.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_ICA(uint8_t * ica_buf, size_t * ica_buf_len)
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    VerifyOrReturnError(ica_buf != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(ica_buf_len != NULL, CHIP_ERROR_INTERNAL);

    size_t buf_len = *ica_buf_len;
    size_t ica_len = 0;
    size_t noc_len = 0;

    /* Read Node Operational certificate chain id. Contains NOC and ICA.*/

    status = se05x_get_certificate(g_noc_chain_id, ica_buf, &buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    /* NOC Chain format ==>
        2 Bytes totoal length,
            NOC Length , NOC
            ICA length, ICA
    */

    VerifyOrReturnError(buf_len >= 6, CHIP_ERROR_INTERNAL);
    noc_len = (ica_buf[5] << 8) | ica_buf[4];

    size_t offset = 6 + noc_len + 2;
    VerifyOrReturnError(buf_len >= offset + 2, CHIP_ERROR_INTERNAL);
    ica_len = (ica_buf[offset + 1] << 8) | ica_buf[offset];

    VerifyOrReturnError(*ica_buf_len > (offset + 2 + ica_len), CHIP_ERROR_INTERNAL);
    memmove(ica_buf, &ica_buf[offset + 2], ica_len);
    *ica_buf_len = ica_len;

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_ipk(uint8_t * ipk_buf, size_t * ipk_buf_len)
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    VerifyOrReturnError(ipk_buf != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(ipk_buf_len != NULL, CHIP_ERROR_INTERNAL);

    /* TODO: Remove this and use TLV apis to create the ipk buffer */
    uint8_t ipk_start_data[] = { 0x15, 0x24, 0x01, 0x00, 0x24, 0x02, 0x01, 0x36, 0x03, 0x15,
                                 0x24, 0x04, 0x00, 0x25, 0x05, 0xd0, 0xea, 0x30, 0x06, 0x10 };
    uint8_t ipk_end_data[]   = { 0x18, 0x15, 0x24, 0x04, 0x00, 0x24, 0x05, 0x00, 0x30, 0x06, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x15, 0x24, 0x04, 0x00,
                                 0x24, 0x05, 0x00, 0x30, 0x06, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x25, 0x07, 0xa3, 0x01, 0x18 };

    uint8_t cert_buf[128] = { 0 };
    size_t cert_buf_len   = sizeof(cert_buf);

    uint8_t encryption_key_buf[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0 };
    uint8_t compressedFabricIdBuf[sizeof(uint64_t)];
    MutableByteSpan compressedFabricIdSpan(compressedFabricIdBuf);
    MutableByteSpan encryption_key(encryption_key_buf);
    ByteSpan epoch_key(cert_buf, Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
    chip::Credentials::P256PublicKeySpan root_pub_key_span(g_p256_root_public_key);

    status = se05x_get_certificate(SE05X_IPK_ID, cert_buf, &cert_buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(cert_buf_len >= Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES, CHIP_ERROR_INTERNAL);

    chip::Crypto::GenerateCompressedFabricId(root_pub_key_span, g_fabric_id, compressedFabricIdSpan);
    ReturnErrorOnFailure(Crypto::DeriveGroupOperationalKey(epoch_key, compressedFabricIdSpan, encryption_key));

    VerifyOrReturnError(*ipk_buf_len >=
                            (sizeof(ipk_start_data) + Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES + sizeof(ipk_end_data)),
                        CHIP_ERROR_INTERNAL);

    size_t offset = 0;
    memcpy(ipk_buf + offset, ipk_start_data, sizeof(ipk_start_data));
    offset += sizeof(ipk_start_data);

    memcpy(ipk_buf + offset, encryption_key_buf, Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
    offset += Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;

    memcpy(ipk_buf + offset, ipk_end_data, sizeof(ipk_end_data));
    offset += sizeof(ipk_end_data);

    *ipk_buf_len = offset;

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_acl_data(uint8_t * acl, size_t * acl_len)
{

    CHIP_ERROR status = CHIP_NO_ERROR;

    VerifyOrReturnError(acl != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(acl_len != NULL, CHIP_ERROR_INTERNAL);

    status = se05x_get_certificate(SE05X_ACCESS_CONTROL_CLUSTER, acl, acl_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    *acl_len = 18;

    /*
     * TBC.
     * Mismatch in the way acl is stored in se05x and the way chip tool expects.
     * Need to check this.
     */
    acl[7]  = 0x36;
    acl[9]  = 0x06;
    acl[15] = 0x34;

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_fabric_groups(uint8_t * fabgrp_data, size_t * fabgrp_data_len)
{
    TLVType container = kTLVType_NotSpecified;
    TLVWriter writer;

    VerifyOrReturnError(fabgrp_data != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(fabgrp_data_len != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(*fabgrp_data_len >= 9, CHIP_ERROR_INTERNAL);

    writer.Init(fabgrp_data, *fabgrp_data_len);
    writer.StartContainer(AnonymousTag(), kTLVType_Structure, container);
    writer.Put(ContextTag(1), static_cast<uint8_t>(0x00));
    writer.Put(ContextTag(2), static_cast<uint8_t>(0x00));
    writer.Put(ContextTag(3), static_cast<uint8_t>(0x00));
    writer.Put(ContextTag(4), static_cast<uint8_t>(0x00));
    writer.Put(ContextTag(5), static_cast<uint8_t>(0x00));
    writer.Put(ContextTag(6), static_cast<uint8_t>(g_fabric_index));
    writer.Put(ContextTag(7), static_cast<uint8_t>(0x00));
    writer.EndContainer(container);

    *fabgrp_data_len = writer.GetLengthWritten();

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_meta_data(uint8_t * meta_data, size_t * meta_data_len)
{
    VerifyOrReturnError(meta_data != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(meta_data_len != NULL, CHIP_ERROR_INTERNAL);

    CHIP_ERROR status = CHIP_NO_ERROR;
    uint32_t vendor_id;
    TLVReader reader;
    TLVWriter writer;
    TLVType container = kTLVType_NotSpecified;
    size_t buf_len    = *meta_data_len;

    /* reuse the same input buffer */
    status = se05x_get_certificate(SE05X_BASIC_INFO_CONTROL_ID, meta_data, &buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    reader.Init(meta_data, buf_len);
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFF));
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFD));
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0xFC));
    reader.Next(kTLVType_Array, ContextTag(0xFB));
    reader.Next(kTLVType_Array, ContextTag(0xF9));
    reader.Next(kTLVType_Array, ContextTag(0xF8));
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0x00));
    reader.Next(kTLVType_UTF8String, ContextTag(0x01));
    reader.Next(kTLVType_SignedInteger, ContextTag(0x1F));
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0x02));
    reader.Get(vendor_id);

    writer.Init(meta_data, *meta_data_len);
    writer.StartContainer(AnonymousTag(), kTLVType_Structure, container);
    writer.Put(ContextTag(0), static_cast<uint32_t>(vendor_id));
    writer.PutString(ContextTag(1), "");
    writer.EndContainer(container);

    *meta_data_len = writer.GetLengthWritten();

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_fabric_index_info_data(uint8_t * fab_info_data, size_t * fab_info_data_len)
{
    VerifyOrReturnError(fab_info_data != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(fab_info_data_len != NULL, CHIP_ERROR_INTERNAL);

    static const uint8_t tempData[] = { 0x15, 0x24, 0x00, 0x02, 0x36, 0x01, 0x04, 0x01, 0x18, 0x18 };

    VerifyOrReturnError(*fab_info_data_len >= sizeof(tempData), CHIP_ERROR_INTERNAL);
    memcpy(fab_info_data, tempData, sizeof(tempData));

    *fab_info_data_len = sizeof(tempData);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_wifi_credentials(uint8_t * buf, size_t buflen, char * ssid, size_t * ssid_len, char * password,
                                       size_t * password_len)
{
    VerifyOrReturnError(buf != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(ssid != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(ssid_len != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(password != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(password_len != NULL, CHIP_ERROR_INTERNAL);

    CHIP_ERROR status = CHIP_NO_ERROR;
    TLVReader reader;

    status = se05x_get_certificate(SE05X_WIFI_BINARY_FILE, buf, &buflen);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    uint16_t len = (static_cast<uint16_t>(buf[0]) << 8) | buf[1];

    reader.Init(&buf[2], len);
    reader.Next(kTLVType_UTF8String, ContextTag(0x00));
    VerifyOrReturnError(reader.GetLength() <= (*ssid_len), CHIP_ERROR_INTERNAL);
    *ssid_len = reader.GetLength();

    status = reader.GetString(ssid, (*ssid_len) + 1);
    VerifyOrReturnError(status == CHIP_NO_ERROR, status);

    reader.Next(kTLVType_UTF8String, ContextTag(0x01));
    VerifyOrReturnError(reader.GetLength() <= (*password_len), CHIP_ERROR_INTERNAL);
    *password_len = reader.GetLength();

    status = reader.GetString(password, (*password_len) + 1);
    return status;
}
