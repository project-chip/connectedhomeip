/*
 *    Copyright (c) 2022 Project CHIP Authors
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
 *
 *    Copyright 2023 NXP
 */

#include <crypto/OperationalKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

#include "PersistentStorageOperationalKeystoreEle.h"

const uint8_t kTlvHeader = 2;

#define KEY_STORE_ID (0xAAAA)
#define AUTHEN_NONCE (0x1111)
#define KEY_GROUP (1)
#define PUBKEY_SIZE (65)
#define KEY_GROUP_SYNC (0x1U << 7)

/* Used for CSR generation */
// Organisation info.
#define SUBJECT_STR "CSR"
#define ASN1_BIT_STRING 0x03
#define ASN1_NULL 0x05
#define ASN1_OID 0x06
#define ASN1_SEQUENCE 0x10
#define ASN1_SET 0x11
#define ASN1_UTF8_STRING 0x0C
#define ASN1_CONSTRUCTED 0x20
#define ASN1_CONTEXT_SPECIFIC 0x80

namespace chip {
namespace ele {

using namespace chip::Crypto;

PersistentStorageOperationalKeystoreEle::PersistentStorageOperationalKeystoreEle()
{
    hsm_err_t err;

    // open the session
    open_session_args_t open_session_args = {0};
    open_session_args.mu_type = HSM1;
    err = hsm_open_session(&open_session_args, &hsm_session_hdl);
    if (err != HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "ELE session open failed. ret: 0x%x\n", err);
        return;
    } else {
        ChipLogDetail(Crypto, "ELE session open successfully.\n");
    }

    // open the keystore
    open_svc_key_store_args_t open_svc_key_store_args = {0};
    open_svc_key_store_args.key_store_identifier = KEY_STORE_ID;
    open_svc_key_store_args.authentication_nonce = AUTHEN_NONCE;
    // try to create a new keystore, if it already exist, open it
    open_svc_key_store_args.flags = 1;
    err = hsm_open_key_store_service(hsm_session_hdl, &open_svc_key_store_args, &key_store_hdl);
    if (err == HSM_KEY_STORE_CONFLICT) {
        ChipLogDetail(Crypto, "keystore already existed, open it...\n");
        open_svc_key_store_args.flags = 0;
        err = hsm_open_key_store_service(hsm_session_hdl, &open_svc_key_store_args, &key_store_hdl);
        if (err != HSM_NO_ERROR) {
            ChipLogDetail(Crypto, "keystore open failed. ret:0x%x\n", err);
            return;
        } else
            ChipLogDetail(Crypto, "keystore open successfully.\n");
    } else {
        ChipLogDetail(Crypto, "keystore created successfully.\n");
    }

    // open key managerment service
    open_svc_key_management_args_t key_mgmt_args = {0};
    memset(&key_mgmt_args, 0, sizeof(key_mgmt_args));
    err = hsm_open_key_management_service(key_store_hdl, &key_mgmt_args, &key_mgmt_hdl);
    if (err != HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "key management service open failed. ret:0x%x\n", err);
        return;
    } else
        ChipLogDetail(Crypto, "key management service open successfully.\n");
}

PersistentStorageOperationalKeystoreEle::~PersistentStorageOperationalKeystoreEle()
{
    hsm_err_t err;

    ResetPendingKey(true);

    ChipLogDetail(Crypto, "close all ELE services.\n");

    err = hsm_close_key_management_service(key_mgmt_hdl);
    key_mgmt_hdl = 0;
    ChipLogDetail(Crypto, "close key management service returns:0x%x\n", err);

    err = hsm_close_key_store_service(key_store_hdl);
    key_store_hdl = 0;
    ChipLogDetail(Crypto, "close key store service returns:0x%x\n", err);

    err = hsm_close_session(hsm_session_hdl);
    hsm_session_hdl = 0;
    ChipLogDetail(Crypto, "close ELE session returns:0x%x\n", err);
}

void PersistentStorageOperationalKeystoreEle::ResetPendingKey(bool delete_key)
{
    if ((mPendingFabricIndex != kUndefinedFabricIndex) && (delete_key))
    {
        EleDeleteKey(mPendingFabricIndex);
    }
    mIsExternallyOwnedKeypair = false;
    mIsPendingKeypairActive   = false;
    mPendingFabricIndex       = kUndefinedFabricIndex;
}

hsm_err_t PersistentStorageOperationalKeystoreEle::EleDeleteKey(uint32_t keyId)
{
    hsm_err_t err;
    op_delete_key_args_t del_args;

    memset(&del_args, 0, sizeof(del_args));
    del_args.key_identifier = keyId;
    del_args.flags = 0;

    err = hsm_delete_key(key_mgmt_hdl, &del_args);
    if (err != HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "Delete key %d failed. ret:0x%x\n", keyId, err);
    } else {
        ChipLogDetail(Crypto, "Delete key %d successfully.\n", keyId);
    }

    return err;
}

hsm_err_t PersistentStorageOperationalKeystoreEle::EleSignMessage(uint32_t keyId,
                                                                  const uint8_t *msg, size_t msgSize,
                                                                  uint8_t *sig, size_t sigSize) const
{
    open_svc_sign_gen_args_t open_sig_gen_args;
    op_generate_sign_args_t sig_gen_args;
    hsm_hdl_t  sig_gen_hdl;
    hsm_err_t hsmret;

    if ((msg == nullptr) || (sig == nullptr) || (sigSize == 0)) {
        ChipLogDetail(Crypto, "Invalid parameters for generating signature.\n");
        return HSM_INVALID_PARAM;
    }

    // open signature generation service
    memset(&open_sig_gen_args, 0, sizeof(open_sig_gen_args));
    hsmret = hsm_open_signature_generation_service(key_store_hdl, &open_sig_gen_args, &sig_gen_hdl);
    if (hsmret != HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "open signature generation service failed. ret:0x%x\n", hsmret);
        return hsmret;
    }

    // generate signature
    memset(&sig_gen_args, 0, sizeof(sig_gen_args));
    sig_gen_args.key_identifier = keyId;
    sig_gen_args.scheme_id = HSM_SIGNATURE_SCHEME_ECDSA_SHA256;
    sig_gen_args.message = (uint8_t *)msg;
    sig_gen_args.signature = sig;
    sig_gen_args.message_size = msgSize;
    sig_gen_args.signature_size = sigSize;
    sig_gen_args.flags = HSM_OP_GENERATE_SIGN_FLAGS_INPUT_MESSAGE;
    hsmret = hsm_generate_signature(sig_gen_hdl, &sig_gen_args);
    if (hsmret != HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "generate signature failed. ret:0x%x\n", hsmret);
        return hsmret;
    }

    // close signature generation service
    hsmret = hsm_close_signature_generation_service(sig_gen_hdl);
    if (hsmret != HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "close signature generation service failed. ret:0x%x\n", hsmret);
    }

    return hsmret;
}

static void add_tlv(uint8_t * buf, size_t buf_index, uint8_t tag, size_t len, uint8_t * val)
{
    buf[buf_index++] = (uint8_t) tag;
    buf[buf_index++] = (uint8_t) len;
    if (len > 0 && val != NULL)
    {
        memcpy(&buf[buf_index], val, len);
        buf_index = buf_index + len;
    }
}

/*
 * CSR format used in the below function,
 *
 *
 *    (ASN1_CONSTRUCTED | ASN1_SEQUENCE) LENGTH
 *
 *        (ASN1_CONSTRUCTED | ASN1_SEQUENCE) LENGTH
 *
 *            VERSION ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
 *
 *            (ASN1_CONSTRUCTED | ASN1_SEQUENCE) LENGTH
 *
 *                (ASN1_CONSTRUCTED | ASN1_SET) LENGTH
 *
 *                    (ASN1_CONSTRUCTED | ASN1_SEQUENCE) LENGTH
 *
 *                        (ASN1_OID) LENGTH VALUE(Organisation OID)
 *
 *                        (ASN1_UTF8_STRING) LENGTH VALUE(Subject Str == "CSR")
 *
 *            PUBLIC KEY {WITH HEADER. 91 Bytes}
 *
 *        (ASN1_CONSTRUCTED | ASN1_SEQUENCE) LENGTH
 *
 *            (ASN1_OID) LENGTH VALUE(ECDSA SHA256 OID)
 *
 *            (ASN1_NULL) 0x00
 *
 *        (ASN1_BIT_STRING) LENGTH VALUE(SIGNATURE)
 *
 */
CHIP_ERROR PersistentStorageOperationalKeystoreEle::EleGenerateCSR(uint32_t keyId, uint8_t * csr, size_t &csrLength)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    hsm_err_t hsmret = HSM_NO_ERROR;

    uint8_t data_to_hash[128] = {0};
    size_t data_to_hash_len   = sizeof(data_to_hash);
    uint8_t pubKey[128]       = {0};
    size_t pubKeyLen          = 0;
    uint8_t signature[128]    = {0};
    size_t signature_len      = 0x44;
    int signature_index       = 0;
    uint8_t signature_data[64];
    size_t csrIndex          = 0;
    size_t buffer_index = data_to_hash_len;

    uint8_t organisation_oid[3] = {0x55, 0x04, 0x0a};
    // Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
    uint8_t version[3]       = {0x02, 0x01, 0x00};
    uint8_t signature_oid[8] = {0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02};
    uint8_t nist256_header[] = {0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
                                0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00};
    uint8_t signature_header_t1[] = {0x02, 0x20};
    uint8_t signature_header_t2[] = {0x02, 0x21, 0x00};

    // No extensions are copied
    buffer_index -= kTlvHeader;
    add_tlv(data_to_hash, buffer_index, (ASN1_CONSTRUCTED | ASN1_CONTEXT_SPECIFIC), 0, NULL);

    // Copy public key (with header)
    {
        // copy header first
        memcpy(pubKey, nist256_header, sizeof(nist256_header));
        pubKeyLen = pubKeyLen + sizeof(nist256_header);

        // public key size is 65
        pubKey[pubKeyLen] = 0x04;
        pubKeyLen++;

        op_pub_key_recovery_args_t args = {0};
        args.key_identifier = keyId;
        args.out_key_size = 64;
        args.out_key = (pubKey + pubKeyLen);
        hsmret = hsm_pub_key_recovery(key_store_hdl, &args);
        VerifyOrExit(hsmret == HSM_NO_ERROR, error = CHIP_ERROR_HSM);
        pubKeyLen += 64;
    }

    buffer_index -= pubKeyLen;
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    memcpy((void *)&data_to_hash[buffer_index], pubKey, pubKeyLen);

    // Copy subject (in the current implementation only organisation name info is added) and organisation OID
    buffer_index -= (kTlvHeader + sizeof(SUBJECT_STR) - 1);
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, ASN1_UTF8_STRING, sizeof(SUBJECT_STR) - 1, (uint8_t *) SUBJECT_STR);

    buffer_index -= (kTlvHeader + sizeof(organisation_oid));
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, ASN1_OID, sizeof(organisation_oid), organisation_oid);

    // Add length
    buffer_index -= kTlvHeader;
    // Subject TLV ==> 1 + 1 + len(subject)
    // Org OID TLV ==> 1 + 1 + len(organisation_oid)
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, (ASN1_CONSTRUCTED | ASN1_SEQUENCE),
            ((2 * kTlvHeader) + (sizeof(SUBJECT_STR) - 1) + sizeof(organisation_oid)), NULL);

    buffer_index -= kTlvHeader;
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, (ASN1_CONSTRUCTED | ASN1_SET),
            ((3 * kTlvHeader) + (sizeof(SUBJECT_STR) - 1) + sizeof(organisation_oid)), NULL);

    buffer_index -= kTlvHeader;
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index, (ASN1_CONSTRUCTED | ASN1_SEQUENCE),
            ((4 * kTlvHeader) + (sizeof(SUBJECT_STR) - 1) + sizeof(organisation_oid)), NULL);

    buffer_index -= 3;
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    memcpy((void *) &data_to_hash[buffer_index], version, sizeof(version));

    buffer_index -= kTlvHeader;
    VerifyOrExit(buffer_index > 0, error = CHIP_ERROR_INTERNAL);
    add_tlv(data_to_hash, buffer_index,
           (ASN1_CONSTRUCTED | ASN1_SEQUENCE), (data_to_hash_len - buffer_index - kTlvHeader), NULL);

    // TLV data is created by copying from backwards. move it to start of buffer.
    data_to_hash_len = (data_to_hash_len - buffer_index);
    memmove(data_to_hash, (data_to_hash + buffer_index), data_to_hash_len);

    hsmret = EleSignMessage(keyId, (uint8_t*)data_to_hash,
                            data_to_hash_len, signature_data, sizeof(signature_data));
    VerifyOrExit(hsmret == HSM_NO_ERROR, error = CHIP_ERROR_HSM);

    if(signature_data[0] > 0x7F)
    {
        signature_len += 1;
        memcpy(signature, signature_header_t2, sizeof(signature_header_t2));
        signature_index += 3;
    } else {
        memcpy(signature, signature_header_t1, sizeof(signature_header_t1));
        signature_index += 2;
    }
    memcpy(&signature[signature_index], signature_data, 32);
    signature_index += 32;
    if(signature_data[32] > 0x7F)
    {
        signature_len +=1;
        memcpy(&signature[signature_index], signature_header_t2, 3);
        signature_index += 3;
    } else {
        memcpy(&signature[signature_index], signature_header_t1, 2);
        signature_index += 2;
    }
    memcpy(&signature[signature_index], &signature_data[32], 32);

    VerifyOrExit((csrIndex + 3) <= csrLength, error = CHIP_ERROR_INTERNAL);
    csr[csrIndex++] = (ASN1_CONSTRUCTED | ASN1_SEQUENCE);
    if ((data_to_hash_len + 14 + kTlvHeader + signature_len) >= 0x80)
        csr[csrIndex++] = 0x81;
    csr[csrIndex++] = (uint8_t)(data_to_hash_len + sizeof(signature_oid) + \
                                (kTlvHeader * 2) + 5 + signature_len);

    VerifyOrExit((csrIndex + data_to_hash_len) <= csrLength, error = CHIP_ERROR_INTERNAL);
    memcpy((csr + csrIndex), data_to_hash, data_to_hash_len);
    csrIndex = csrIndex + data_to_hash_len;

    // ECDSA SHA256 Signature OID TLV ==> 1 + 1 + len(signature_oid) (8)
    // ASN_NULL ==> 1 + 1
    VerifyOrExit((csrIndex + kTlvHeader) <= csrLength, error = CHIP_ERROR_INTERNAL);
    add_tlv(csr, csrIndex, (ASN1_CONSTRUCTED | ASN1_SEQUENCE), 0x0A, NULL);
    csrIndex = csrIndex + kTlvHeader;

    VerifyOrExit((csrIndex + sizeof(signature_oid) + kTlvHeader) <= csrLength, error = CHIP_ERROR_INTERNAL);
    add_tlv(csr, csrIndex, ASN1_OID, sizeof(signature_oid), signature_oid);
    csrIndex = csrIndex + kTlvHeader + sizeof(signature_oid);

    //VerifyOrExit((csrIndex + kTlvHeader) <= csrLength, error = CHIP_ERROR_INTERNAL);
    // add_tlv(csr, csrIndex, ASN1_NULL, 0x00, NULL);
    // csrIndex = csrIndex + kTlvHeader;

    VerifyOrExit((csrIndex + 5) <= csrLength, error = CHIP_ERROR_INTERNAL);
    csr[csrIndex++] = ASN1_BIT_STRING;
    csr[csrIndex++] = signature_len + 3;
    csr[csrIndex++] = 0x00;
    csr[csrIndex++] = (ASN1_CONSTRUCTED | ASN1_SEQUENCE);
    csr[csrIndex++] = signature_len;

    VerifyOrExit((csrIndex + signature_len) <= csrLength, error = CHIP_ERROR_INTERNAL);
    memcpy(&csr[csrIndex], signature, signature_len);
    csrLength = (csrIndex + signature_len);

    error = CHIP_NO_ERROR;

exit:
    return error;
}

bool PersistentStorageOperationalKeystoreEle::HasOpKeypairForFabric(FabricIndex fabricIndex) const
{
    op_get_key_attr_args_t keyattr_args;
    hsm_err_t err;

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), false);
    // If there was a pending keypair, then there's really a usable key
    if (mIsPendingKeypairActive && (fabricIndex == mPendingFabricIndex))
    {
        return true;
    }

    // get the key attributes, it shall fail if no such key
    memset(&keyattr_args, 0, sizeof(keyattr_args));
    keyattr_args.key_identifier = fabricIndex;
    err = hsm_get_key_attr(key_mgmt_hdl, &keyattr_args);
    if (err != HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "No keypair for fabric: %d found. ELE returns: 0x%x\n", fabricIndex, err);
        return false;
    } else {
        ChipLogDetail(Crypto, "Found keypair for fabric: %d.\n", fabricIndex);
        return true;
    }
}

CHIP_ERROR PersistentStorageOperationalKeystoreEle::NewOpKeypairForFabric(FabricIndex fabricIndex,
                                                                       MutableByteSpan & outCertificateSigningRequest)
{
    op_generate_key_args_t key_gen_args;
    op_get_key_attr_args_t keyattr_args;
    uint32_t keyId;
    hsm_err_t hsm_err;
    CHIP_ERROR err;

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    // If a key is pending, we cannot generate for a different fabric index until we commit or revert.
    if ((mPendingFabricIndex != kUndefinedFabricIndex) && (fabricIndex != mPendingFabricIndex))
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }
    VerifyOrReturnError(outCertificateSigningRequest.size() >= Crypto::kMIN_CSR_Buffer_Size, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Replace previous pending keypair
    ResetPendingKey(true);

    // Delete the key if alrady existed
    memset(&keyattr_args, 0, sizeof(keyattr_args));
    keyattr_args.key_identifier = fabricIndex;
    hsm_err = hsm_get_key_attr(key_mgmt_hdl, &keyattr_args);
    if (hsm_err == HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "Keypair for fabric: %d exists, delete it...\n", fabricIndex);
        EleDeleteKey(fabricIndex);
    }

    // Generate new key
    memset(&key_gen_args, 0, sizeof(key_gen_args));
    keyId = fabricIndex;
    key_gen_args.key_identifier = &keyId;
    key_gen_args.key_group = KEY_GROUP;
    key_gen_args.key_lifetime = HSM_SE_KEY_STORAGE_PERSISTENT;
    key_gen_args.key_usage = HSM_KEY_USAGE_SIGN_HASH | HSM_KEY_USAGE_VERIFY_HASH | \
                             HSM_KEY_USAGE_SIGN_MSG | HSM_KEY_USAGE_VERIFY_MSG;
    key_gen_args.permitted_algo = PERMITTED_ALGO_ECDSA_SHA256;
    // "sync" flag is not set so the key won't be committed to NVM immediately.
    // we will commit the key in the CommitOpKeypairForFabric().
    key_gen_args.flags = 0;
    key_gen_args.key_type = HSM_KEY_TYPE_ECC_NIST;
    key_gen_args.bit_key_sz = HSM_KEY_SIZE_ECC_NIST_256;
    key_gen_args.key_lifecycle = (hsm_key_lifecycle_t)0;
    hsm_err = hsm_generate_key(key_mgmt_hdl, &key_gen_args);
    ChipLogDetail(Crypto, "Generate new keypair returns: 0x%x\n", hsm_err);
    if (hsm_err != HSM_NO_ERROR)
        return CHIP_ERROR_HSM;

    // generate the CSR
    size_t csrLength = outCertificateSigningRequest.size();
    err = EleGenerateCSR(keyId, outCertificateSigningRequest.data(), csrLength);
    if (err != CHIP_NO_ERROR)
    {
        ResetPendingKey(true);
        return err;
    }

    outCertificateSigningRequest.reduce_size(csrLength);
    mPendingFabricIndex = fabricIndex;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreEle::ActivateOpKeypairForFabric(FabricIndex fabricIndex,
                                                                            const Crypto::P256PublicKey & nocPublicKey)
{
    uint8_t pubkey[PUBKEY_SIZE];
    hsm_err_t hsmret = HSM_NO_ERROR;

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Validate public key being activated matches last generated pending keypair
    // public key size is 65
    pubkey[0] = 0x04;

    op_pub_key_recovery_args_t args = {0};
    args.key_identifier = fabricIndex;
    args.out_key_size = 64;
    args.out_key = &pubkey[1];
    hsmret = hsm_pub_key_recovery(key_store_hdl, &args);
    if (hsmret != HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "recover public key failed. ret:0x%x\n", hsmret);
        return CHIP_ERROR_HSM;
    }

    if (memcmp(pubkey, nocPublicKey.ConstBytes(), sizeof(pubkey))) {
        ChipLogDetail(Crypto, "the public key being activated doesn't match pending key.\n");
        return CHIP_ERROR_INVALID_PUBLIC_KEY;
    }

    mIsPendingKeypairActive = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreEle::CommitOpKeypairForFabric(FabricIndex fabricIndex)
{
    hsm_err_t hsmret = HSM_NO_ERROR;
    VerifyOrReturnError(IsValidFabricIndex(fabricIndex) && \
                       (fabricIndex == mPendingFabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(mIsPendingKeypairActive == true, CHIP_ERROR_INCORRECT_STATE);

    // Try to store persistent key. On failure, leave everything pending as-is
    op_manage_key_group_args_t keygroup_args;
    memset(&keygroup_args, 0, sizeof(keygroup_args));
    keygroup_args.key_group = KEY_GROUP;
    keygroup_args.flags = KEY_GROUP_SYNC;
    hsmret = hsm_manage_key_group(key_mgmt_hdl, &keygroup_args);
    if (hsmret != HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "commit key failed. ret: 0x%x\n", hsmret);
        ReturnErrorOnFailure(CHIP_ERROR_INTERNAL);
    } else {
        ChipLogDetail(Crypto, "commit key successfully.\n");
    }

    // If we got here, we succeeded and can reset the pending key: next `SignWithOpKeypair` will use the stored key.
    ResetPendingKey(false);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PersistentStorageOperationalKeystoreEle::RemoveOpKeypairForFabric(FabricIndex fabricIndex)
{
    hsm_err_t hsmret = HSM_NO_ERROR;
    op_get_key_attr_args_t keyattr_args;

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    // Remove pending state if matching
    if ((fabricIndex == mPendingFabricIndex))
    {
        ResetPendingKey(true);
        return CHIP_NO_ERROR;
    }

    // get the key attributes, it shall fail if no such key.
    memset(&keyattr_args, 0, sizeof(keyattr_args));
    keyattr_args.key_identifier = fabricIndex;
    hsmret = hsm_get_key_attr(key_mgmt_hdl, &keyattr_args);
    if (hsmret != HSM_NO_ERROR) {
        ChipLogDetail(Crypto, "No keypair for fabric: %d found. No need to delete\n", fabricIndex);
        return CHIP_NO_ERROR;
    } else {
        hsmret = EleDeleteKey(fabricIndex);
        if (hsmret != HSM_NO_ERROR)
            return CHIP_ERROR_HSM;
    }

    return CHIP_NO_ERROR;
}

void PersistentStorageOperationalKeystoreEle::RevertPendingKeypair()
{
    // Just reset the pending key, it hasn't been stored into secure storage.
    ResetPendingKey(true);
}

CHIP_ERROR PersistentStorageOperationalKeystoreEle::SignWithOpKeypair(FabricIndex fabricIndex, const ByteSpan & message,
                                                                   Crypto::P256ECDSASignature & outSignature) const
{
    uint8_t sig[kP256_ECDSA_Signature_Length_Raw];
    hsm_err_t hsmret = HSM_NO_ERROR;

    VerifyOrReturnError(IsValidFabricIndex(fabricIndex), CHIP_ERROR_INVALID_FABRIC_INDEX);

    hsmret = EleSignMessage(fabricIndex, message.data(), message.size(), sig, kP256_ECDSA_Signature_Length_Raw);
    VerifyOrReturnError(hsmret == HSM_NO_ERROR, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(outSignature.SetLength(kP256_ECDSA_Signature_Length_Raw) == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);
    memcpy(outSignature.Bytes(), sig, kP256_ECDSA_Signature_Length_Raw);

    return CHIP_NO_ERROR;
}

Crypto::P256Keypair * PersistentStorageOperationalKeystoreEle::AllocateEphemeralKeypairForCASE()
{
    return Platform::New<Crypto::P256Keypair>();
}

void PersistentStorageOperationalKeystoreEle::ReleaseEphemeralKeypair(Crypto::P256Keypair * keypair)
{
    Platform::Delete<Crypto::P256Keypair>(keypair);
}

} // namespace ele
} // namespace chip
