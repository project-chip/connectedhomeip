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
#include <platform/KeyValueStoreManager.h>

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

uint32_t gfabricIndex;

using namespace chip;
using namespace chip::TLV;

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

CHIP_ERROR se05x_is_nfc_commissioning_done()
{
    CHIP_ERROR status = CHIP_NO_ERROR;
    std::vector<uint8_t> op_cred_buff(1024);
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

CHIP_ERROR se05x_read_operational_credentials_cluster(char * noc_key, uint8_t * noc_buf, size_t * noc_buf_len, char * root_cert_key,
                                                      uint8_t * root_cert_buf, size_t * root_cert_buf_len, char * icac_key,
                                                      uint8_t * icac_buf, size_t * icac_buf_len, char * ipk_key, uint8_t * ipk_buf,
                                                      size_t * ipk_buf_len)
{
    CHIP_ERROR status = CHIP_NO_ERROR;
    std::vector<uint8_t> op_cred_buff(1024);
    size_t op_cred_buff_len = op_cred_buff.size();
    uint32_t noc_chain_id   = 0;
    uint32_t root_cert_id   = 0;
    uint64_t id_64b         = 0;
    uint8_t cert_buf[2048];
    size_t cert_buf_len = sizeof(cert_buf);
    uint16_t nocLen     = 0;
    uint16_t icacLen    = 0;
    TLVType outertype   = kTLVType_NotSpecified;
    TLVReader reader;

    VerifyOrReturnError(noc_key != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(noc_buf != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(noc_buf_len != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(root_cert_key != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(root_cert_buf != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(root_cert_buf_len != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(icac_key != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(icac_buf != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(icac_buf_len != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(ipk_key != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(ipk_buf != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(ipk_buf_len != NULL, CHIP_ERROR_INTERNAL);

    /* TODO: Remove this and use TLV apis to create the ipk buffer */
    uint8_t ipk_start_data[] = { 0x15, 0x24, 0x01, 0x00, 0x24, 0x02, 0x01, 0x36, 0x03, 0x15,
                                 0x24, 0x04, 0x00, 0x25, 0x05, 0xd0, 0xea, 0x30, 0x06, 0x10 };
    uint8_t ipk_end_data[]   = { 0x18, 0x15, 0x24, 0x04, 0x00, 0x24, 0x05, 0x00, 0x30, 0x06, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x15, 0x24, 0x04, 0x00,
                                 0x24, 0x05, 0x00, 0x30, 0x06, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x25, 0x07, 0xa3, 0x01, 0x18 };

    status = se05x_get_certificate(SE05X_OPERATIONAL_CRED_CLUSTER_ID, op_cred_buff.data(), &op_cred_buff_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    reader.Init(op_cred_buff.data(), op_cred_buff_len);
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

    noc_chain_id = static_cast<uint32_t>(id_64b >> 32);
    noc_chain_id = ((noc_chain_id >> 24) & 0x000000FF) | ((noc_chain_id >> 8) & 0x0000FF00) | ((noc_chain_id << 8) & 0x00FF0000) |
        ((noc_chain_id << 24) & 0xFF000000);

    ChipLogDetail(Crypto, "SE05x: Node Operational certificate chain id is : 0x%08X", noc_chain_id);
    reader.ExitContainer(outertype);

    reader.Next();
    reader.Next();
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0x02));
    reader.Next(kTLVType_UnsignedInteger, ContextTag(0x03));
    reader.Next(kTLVType_List, ContextTag(0x04));
    reader.EnterContainer(outertype);
    reader.Next();
    reader.Get(id_64b);

    root_cert_id = static_cast<uint32_t>(id_64b >> 32);
    root_cert_id = ((root_cert_id >> 24) & 0x000000FF) | ((root_cert_id >> 8) & 0x0000FF00) | ((root_cert_id << 8) & 0x00FF0000) |
        ((root_cert_id << 24) & 0xFF000000);

    ChipLogDetail(Crypto, "SE05x: Root Cert is is : 0x%08X", root_cert_id);
    reader.ExitContainer(outertype);

    reader.Next(kTLVType_UnsignedInteger, ContextTag(0x05));
    reader.Get(gfabricIndex);

    VerifyOrReturnError(gfabricIndex == 1, CHIP_ERROR_INTERNAL);

    /* Read Node Operational certificate chain id. Contains NOC and ICA.*/

    status = se05x_get_certificate(noc_chain_id, cert_buf, &cert_buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    /* NOC Chain format ==>
        2 Bytes totoal length,
            NOC Length , NOC
            ICA length, ICA
    */

    VerifyOrReturnError(cert_buf_len > 2, CHIP_ERROR_INTERNAL);
    cert_buf_len = (cert_buf[0] << 8) | cert_buf[1];

    nocLen = (cert_buf[5] << 8) | cert_buf[4];
    VerifyOrReturnError(nocLen <= *noc_buf_len, CHIP_ERROR_INTERNAL);
    memcpy(noc_buf, &cert_buf[6], nocLen);
    *noc_buf_len = nocLen;

    size_t offset = 6 + nocLen + 2;
    icacLen       = (cert_buf[offset + 1] << 8) | cert_buf[offset];
    VerifyOrReturnError(icacLen <= *icac_buf_len, CHIP_ERROR_INTERNAL);
    memcpy(icac_buf, &cert_buf[offset + 2], icacLen);
    *icac_buf_len = icacLen;

    /* read root certificate */

    memset(cert_buf, 0, cert_buf_len);
    cert_buf_len = sizeof(cert_buf);
    status       = se05x_get_certificate(root_cert_id, cert_buf, &cert_buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(cert_buf_len > 2, CHIP_ERROR_INTERNAL);
    cert_buf_len = (cert_buf[0] << 8) | cert_buf[1];
    memcpy(root_cert_buf, &cert_buf[5], cert_buf_len);
    *root_cert_buf_len = cert_buf_len;

    /* read epok ipk key */

    memset(cert_buf, 0, cert_buf_len);
    cert_buf_len = sizeof(cert_buf);
    status       = se05x_get_certificate(SE05X_IPK_ID, cert_buf, &cert_buf_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(cert_buf_len == 16, CHIP_ERROR_INTERNAL);

    /* Derive the IPK key using IPK epock key */

    uint8_t compressedFabricIdBuf[sizeof(uint64_t)];
    chip::Credentials::P256PublicKeySpan rootPubKeySpan;
    uint8_t encryption_key_buf[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = { 0 };
    FabricId mFabricId                                                         = kUndefinedFabricId;
    NodeId mNodeId                                                             = kUndefinedNodeId;
    MutableByteSpan compressedFabricIdSpan(compressedFabricIdBuf);
    MutableByteSpan encryption_key(encryption_key_buf);
    MutableByteSpan nocSpan{ noc_buf, *noc_buf_len };
    MutableByteSpan rcacSpan{ root_cert_buf, *root_cert_buf_len };
    ByteSpan epoch_key(cert_buf, Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);

    ReturnErrorOnFailure(chip::Credentials::ExtractNodeIdFabricIdFromOpCert(nocSpan, &mNodeId, &mFabricId));
    ReturnErrorOnFailure(chip::Credentials::ExtractPublicKeyFromChipCert(rcacSpan, rootPubKeySpan));
    chip::Crypto::GenerateCompressedFabricId(rootPubKeySpan, mFabricId, compressedFabricIdSpan);
    ReturnErrorOnFailure(Crypto::DeriveGroupOperationalKey(epoch_key, compressedFabricIdSpan, encryption_key));

    offset = 0;
    memcpy(ipk_buf + offset, ipk_start_data, sizeof(ipk_start_data));
    offset += sizeof(ipk_start_data);

    memcpy(ipk_buf + offset, encryption_key_buf, Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
    offset += Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;

    memcpy(ipk_buf + offset, ipk_end_data, sizeof(ipk_end_data));
    offset += sizeof(ipk_end_data);

    *ipk_buf_len = offset;

    if (snprintf(noc_key, SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS, "f/%x/n", gfabricIndex) < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }
    if (snprintf(root_cert_key, SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS, "f/%x/r", gfabricIndex) < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }
    if (snprintf(icac_key, SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS, "f/%x/i", gfabricIndex) < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }
    if (snprintf(ipk_key, SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS, "f/%x/k/0", gfabricIndex) < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_node_operational_keypair(char * op_key_name, uint8_t * op_key_ref_key, size_t * op_key_ref_key_len)
{
    CHIP_ERROR status = CHIP_NO_ERROR;

    VerifyOrReturnError(op_key_name != NULL, CHIP_ERROR_INTERNAL);
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

    if (snprintf(op_key_name, SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS, "f/%x/o", gfabricIndex) < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_acl_data(char * acl_key, uint8_t * acl, size_t * acl_len)
{

    CHIP_ERROR status = CHIP_NO_ERROR;

    VerifyOrReturnError(acl_key != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(acl != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(acl_len != NULL, CHIP_ERROR_INTERNAL);

    status = se05x_get_certificate(SE05X_ACCESS_CONTROL_CLUSTER, acl, acl_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    *acl_len = 18;

    /*
     * Mismatch in the way acl is stored in se05x and the way chip tool expects.
     * Need to check this.
     */
    acl[7]  = 0x36;
    acl[9]  = 0x06;
    acl[15] = 0x34;

    if (snprintf(acl_key, SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS, "f/%x/ac/0/0", gfabricIndex) < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_fabric_groups(char * fgrp_key, uint8_t * fabgrp_data, size_t * fabgrp_data_len)
{
    TLVType container = kTLVType_NotSpecified;
    TLVWriter writer;

    VerifyOrReturnError(fgrp_key != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(fabgrp_data != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(fabgrp_data_len != NULL, CHIP_ERROR_INTERNAL);

    writer.Init(fabgrp_data, *fabgrp_data_len);
    writer.StartContainer(AnonymousTag(), kTLVType_Structure, container);
    writer.Put(ContextTag(1), static_cast<uint8_t>(0x00));
    writer.Put(ContextTag(2), static_cast<uint8_t>(0x00));
    writer.Put(ContextTag(3), static_cast<uint8_t>(0x00));
    writer.Put(ContextTag(4), static_cast<uint8_t>(0x00));
    writer.Put(ContextTag(5), static_cast<uint8_t>(0x00));
    writer.Put(ContextTag(6), static_cast<uint8_t>(gfabricIndex));
    writer.Put(ContextTag(7), static_cast<uint8_t>(0x00));
    writer.EndContainer(container);

    *fabgrp_data_len = writer.GetLengthWritten();

    if (snprintf(fgrp_key, SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS, "f/%x/g", gfabricIndex) < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_meta_data(char * meta_data_key, uint8_t * meta_data, size_t * meta_data_len)
{
    VerifyOrReturnError(meta_data_key != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(meta_data != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(meta_data_len != NULL, CHIP_ERROR_INTERNAL);

    CHIP_ERROR status = CHIP_NO_ERROR;
    std::vector<uint8_t> bi_info_buff(1024);
    size_t bi_info_buff_len = bi_info_buff.size();
    uint32_t vendor_id;
    TLVReader reader;
    TLVWriter writer;
    TLVType container = kTLVType_NotSpecified;

    status = se05x_get_certificate(SE05X_BASIC_INFO_CONTROL_ID, bi_info_buff.data(), &bi_info_buff_len);
    VerifyOrReturnError(status == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    reader.Init(bi_info_buff.data(), bi_info_buff_len);
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

    if (snprintf(meta_data_key, SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS, "f/%x/m", gfabricIndex) < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05x_read_fabric_index_info_data(char * fab_index_info_key, uint8_t * fab_info_data, size_t * fab_info_data_len)
{
    VerifyOrReturnError(fab_index_info_key != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(fab_info_data != NULL, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(fab_info_data_len != NULL, CHIP_ERROR_INTERNAL);

    static const uint8_t tempData[] = { 0x15, 0x24, 0x00, 0x02, 0x36, 0x01, 0x04, 0x01, 0x18, 0x18 };

    memcpy(fab_info_data, tempData, sizeof(tempData));
    *fab_info_data_len = sizeof(tempData);

    if (snprintf(fab_index_info_key, SE05X_KEY_BUFFER_LEN_KVS_FUNCTIONS, "g/fidx") < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}
