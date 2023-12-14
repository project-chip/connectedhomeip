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
 *      HSM based implementation of CHIP crypto primitives
 *      Based on configurations in CHIPCryptoPALHsm_config.h file,
 *      chip crypto apis use either HSM or rollback to software implementation.
 */

#include "CHIPCryptoPALHsm_se05x_utils.h"
#include "fsl_sss_policy.h"

ex_sss_boot_ctx_t gex_sss_chip_ctx;

#if ENABLE_REENTRANCY

uint8_t objIDtable[MAX_SPAKE_CRYPTO_OBJECT][2] = {
    { kSE05x_CryptoObject_End + 0, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 1, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 2, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 3, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 4, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 5, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 6, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 7, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 8, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 9, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 10, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 11, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 12, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 13, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 14, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 15, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 16, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 17, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 18, OBJ_ID_TABLE_OBJID_STATUS_FREE },
    { kSE05x_CryptoObject_End + 19, OBJ_ID_TABLE_OBJID_STATUS_FREE },
};

int spake_objects_created = 0;

#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
using namespace chip::System;
static Mutex sSEObjMutex;
#define LOCK_SECURE_ELEMENT()                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        sSEObjMutex.Lock();                                                                                                        \
    } while (0)
#define UNLOCK_SECURE_ELEMENT()                                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        sSEObjMutex.Unlock();                                                                                                      \
    } while (0)

#else
#define LOCK_SECURE_ELEMENT()
#define UNLOCK_SECURE_ELEMENT()
#endif // !CHIP_SYSTEM_CONFIG_NO_LOCKING

#endif // #if ENABLE_REENTRANCY

/* Open session to se05x */
CHIP_ERROR se05x_sessionOpen(void)
{
    static int is_session_open = 0;
    if (is_session_open)
    {
        return CHIP_NO_ERROR;
    }

    memset(&gex_sss_chip_ctx, 0, sizeof(gex_sss_chip_ctx));

    char * portName     = nullptr;
    sss_status_t status = ex_sss_boot_connectstring(0, NULL, &portName);
    if (kStatus_SSS_Success != status)
    {
        ChipLogError(Crypto, "se05x error: ex_sss_boot_connectstring failed");
        return CHIP_ERROR_INTERNAL;
    }

    status = ex_sss_boot_open(&gex_sss_chip_ctx, portName);
    if (kStatus_SSS_Success != status)
    {
        ChipLogError(Crypto, "se05x error: ex_sss_boot_open failed");
        return CHIP_ERROR_INTERNAL;
    }

    status = ex_sss_key_store_and_object_init(&gex_sss_chip_ctx);
    if (kStatus_SSS_Success != status)
    {
        ChipLogError(Crypto, "se05x error: ex_sss_key_store_and_object_init failed");
        return CHIP_ERROR_INTERNAL;
    }

    is_session_open = 1;
    return CHIP_NO_ERROR;
}

/* Delete key in se05x */
void se05x_delete_key(uint32_t keyid)
{
    smStatus_t smstatus   = SM_NOT_OK;
    SE05x_Result_t exists = kSE05x_Result_NA;

    if (se05x_sessionOpen() != CHIP_NO_ERROR)
    {
        ChipLogError(Crypto, "se05x error: Error in session open");
        return;
    }

    if (gex_sss_chip_ctx.ks.session != NULL)
    {

        smstatus = Se05x_API_CheckObjectExists(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, keyid, &exists);
        if (smstatus == SM_OK)
        {
            if (exists == kSE05x_Result_SUCCESS)
            {
                smstatus = Se05x_API_DeleteSecureObject(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, keyid);
                if (smstatus != SM_OK)
                {
                    ChipLogError(Crypto, "se05x error: Error in deleting key");
                }
            }
            else
            {
                ChipLogError(Crypto, "se05x warn: Key doesnot exists");
            }
        }
        else
        {
            ChipLogError(Crypto, "se05x error: Error in Se05x_API_CheckObjectExists");
        }
    }
    return;
}

/* Set key in se05x */
CHIP_ERROR se05x_set_key_for_spake(uint32_t keyid, const uint8_t * key, size_t keylen, sss_key_part_t keyPart,
                                   sss_cipher_type_t cipherType)
{
    sss_status_t status       = kStatus_SSS_Success;
    sss_object_t keyObject    = { 0 };
    const uint8_t keyBuf[128] = {
        0,
    };
    size_t keyBufLen  = 0;
    uint8_t header1[] = { 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01,
                          0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00 };
    size_t bitlen     = 0;

    static sss_policy_u commonPol;
    commonPol.type                     = KPolicy_Common;
    commonPol.auth_obj_id              = 0;
    commonPol.policy.common.req_Sm     = 0;
    commonPol.policy.common.can_Delete = 1;
    commonPol.policy.common.can_Read   = 0;
    commonPol.policy.common.can_Write  = 1;

    static sss_policy_u hmac_withPol;
    hmac_withPol.type                     = KPolicy_Sym_Key;
    hmac_withPol.auth_obj_id              = 0;
    hmac_withPol.policy.symmkey.can_Write = 1;
    hmac_withPol.policy.symmkey.can_KA    = 1;

    sss_policy_t policy_for_hmac_key;
    policy_for_hmac_key.nPolicies   = 2;
    policy_for_hmac_key.policies[0] = &hmac_withPol;
    policy_for_hmac_key.policies[1] = &commonPol;

    if (cipherType == kSSS_CipherType_EC_NIST_P)
    {
        VerifyOrReturnError(keylen < (sizeof(keyBuf) - sizeof(header1)), CHIP_ERROR_INTERNAL);

        memcpy((void *) keyBuf, (const uint8_t *) header1, sizeof(header1));
        keyBufLen = keyBufLen + sizeof(header1);

        memcpy((void *) (keyBuf + keyBufLen), key, keylen);
        keyBufLen = keyBufLen + keylen;

        bitlen = 256;
    }
    else
    {
        VerifyOrReturnError(keylen < sizeof(keyBuf), CHIP_ERROR_INTERNAL);

        memcpy((void *) keyBuf, (const uint8_t *) key, keylen);
        keyBufLen = keylen;
        bitlen    = (size_t) keylen * 8;
    }

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(&keyObject, keyid, keyPart, cipherType, keyBufLen, kKeyObject_Mode_Persistent);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &keyObject, keyBuf, keyBufLen, bitlen, &policy_for_hmac_key,
                                   sizeof(policy_for_hmac_key));
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05xGetCertificate(uint32_t keyId, uint8_t * buf, size_t * buflen)
{
    sss_object_t keyObject = { 0 };
    sss_status_t status    = kStatus_SSS_Fail;
    size_t certBitLen      = 0;

    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(buflen != nullptr, CHIP_ERROR_INTERNAL);

    certBitLen = (*buflen) * 8;

    VerifyOrReturnError(se05x_sessionOpen() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_object_get_handle(&keyObject, keyId);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_store_get_key(&gex_sss_chip_ctx.ks, &keyObject, buf, buflen, &certBitLen);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05xSetCertificate(uint32_t keyId, const uint8_t * buf, size_t buflen)
{
    sss_object_t keyObject = { 0 };
    sss_status_t status    = kStatus_SSS_Fail;

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(&keyObject, keyId, kSSS_KeyPart_Default, kSSS_CipherType_Certificate, buflen,
                                            kKeyObject_Mode_Transient);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &keyObject, buf, buflen, buflen * 8, NULL, 0);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR se05xPerformInternalSign(uint32_t keyId, uint8_t * sigBuf, size_t * sigBufLen)
{
#if SSS_HAVE_APPLET_SE051_H
    smStatus_t status                                   = SM_NOT_OK;
    sss_se05x_session_t * pSe05xCtx                     = (sss_se05x_session_t *) &gex_sss_chip_ctx.session;
    uint8_t hashData[chip::Crypto::kSHA256_Hash_Length] = { 0 };
    size_t hashDataLen                                  = sizeof(hashData);

    status = Se05x_API_ECDSA_Internal_Sign(&(pSe05xCtx->s_ctx), keyId, kSE05x_ECSignatureAlgo_SHA_256, sigBuf, sigBufLen, hashData,
                                           &hashDataLen);
    VerifyOrReturnError(status == SM_OK, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
#else
    /* Enable Se051H to use internal sign */
    return CHIP_ERROR_INTERNAL;
#endif
}

#if ENABLE_REENTRANCY

/* Init crypto object mutext */
void se05x_init_cryptoObj_mutex(void)
{
#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    Mutex::Init(sSEObjMutex);
#endif
    return;
}

/* Delete all crypto objects in se05x */
void se05x_delete_crypto_objects(void)
{
    static int obj_deleted = 0;
    smStatus_t smstatus    = SM_NOT_OK;
    uint8_t list[1024]     = {
        0,
    };
    size_t listlen = sizeof(list);
    size_t i;

    if (obj_deleted == 1)
    {
        return;
    }

    if (se05x_sessionOpen() != CHIP_NO_ERROR)
    {
        return;
    }

    if (gex_sss_chip_ctx.ks.session != NULL)
    {
        smstatus = Se05x_API_ReadCryptoObjectList(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, list, &listlen);
        if (smstatus != SM_OK)
        {
            return;
        }

        for (i = 0; i < listlen; i += 4)
        {
            uint32_t cryptoObjectId = list[i + 1] | (list[i + 0] << 8);
            Se05x_API_DeleteCryptoObject(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx,
                                         (SE05x_CryptoObjectID_t) cryptoObjectId);
        }
    }

    obj_deleted           = 1;
    spake_objects_created = 0;
    return;
}

/* Get unused object id */
SE05x_CryptoObjectID_t se05x_getCryptoObjID(void)
{
    SE05x_CryptoObjectID_t objId = (SE05x_CryptoObjectID_t) 0;
    SE05x_Result_t exists        = kSE05x_Result_NA;

    LOCK_SECURE_ELEMENT();

    for (int i = 0; i < MAX_SPAKE_CRYPTO_OBJECT; i++)
    {
        if (objIDtable[i][OBJ_ID_TABLE_IDX_STATUS] == OBJ_ID_TABLE_OBJID_STATUS_FREE)
        {
            Se05x_API_CheckObjectExists(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx,
                                        objIDtable[i][OBJ_ID_TABLE_IDX_OBJID], &exists);
            if (exists == kSE05x_Result_SUCCESS)
            {
                // Object in use. Check for other id.
                objIDtable[i][OBJ_ID_TABLE_IDX_STATUS] = OBJ_ID_TABLE_OBJID_STATUS_USED;
                continue;
            }
            objId                                  = (SE05x_CryptoObjectID_t) objIDtable[i][OBJ_ID_TABLE_IDX_OBJID];
            objIDtable[i][OBJ_ID_TABLE_IDX_STATUS] = OBJ_ID_TABLE_OBJID_STATUS_USED;
            goto exit;
        }
    }

exit:
    UNLOCK_SECURE_ELEMENT();
    return objId;
}

/* Set object id status */
void se05x_setCryptoObjID(SE05x_CryptoObjectID_t objId, uint8_t status)
{
    LOCK_SECURE_ELEMENT();
    for (int i = 0; i < MAX_SPAKE_CRYPTO_OBJECT; i++)
    {
        if (objIDtable[i][OBJ_ID_TABLE_IDX_OBJID] == objId)
        {
            objIDtable[i][OBJ_ID_TABLE_IDX_STATUS] = status;
            break;
        }
    }
    UNLOCK_SECURE_ELEMENT();
}

#endif // #if ENABLE_REENTRANCY
