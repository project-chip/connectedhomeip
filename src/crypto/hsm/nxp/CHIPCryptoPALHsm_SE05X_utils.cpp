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

#include "CHIPCryptoPALHsm_SE05X_utils.h"

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

#endif //#if ENABLE_REENTRANCY

/* Open session to se05x */
void se05x_sessionOpen(void)
{
    static int is_session_open = 0;
    if (is_session_open)
    {
        return;
    }

    memset(&gex_sss_chip_ctx, 0, sizeof(gex_sss_chip_ctx));

    const char * portName = nullptr;
    sss_status_t status   = ex_sss_boot_connectstring(0, NULL, &portName);
    if (kStatus_SSS_Success != status)
    {
        ChipLogError(Crypto, "se05x error: %s\n", "ex_sss_boot_connectstring failed");
        return;
    }

    status = ex_sss_boot_open(&gex_sss_chip_ctx, portName);
    if (kStatus_SSS_Success != status)
    {
        ChipLogError(Crypto, "se05x error: %s\n", "ex_sss_boot_open failed");
        return;
    }

    status = ex_sss_key_store_and_object_init(&gex_sss_chip_ctx);
    if (kStatus_SSS_Success != status)
    {
        ChipLogError(Crypto, "se05x error: %s\n", "ex_sss_key_store_and_object_init failed");
        return;
    }

    is_session_open = 1;
}

/* Delete key in se05x */
void se05x_delete_key(uint32_t keyid)
{
    smStatus_t smstatus   = SM_NOT_OK;
    SE05x_Result_t exists = kSE05x_Result_NA;

    se05x_sessionOpen();

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
                    ChipLogError(Crypto, "se05x error: %s\n", "Error in deleting key");
                }
            }
            else
            {
                ChipLogError(Crypto, "se05x warn: %s\n", "Key doesnot exists");
            }
        }
        else
        {
            ChipLogError(Crypto, "se05x error: %s\n", "Error in Se05x_API_CheckObjectExists");
        }
    }
}

/* Set key in se05x */
CHIP_ERROR se05x_set_key(uint32_t keyid, const uint8_t * key, size_t keylen, sss_key_part_t keyPart, sss_cipher_type_t cipherType)
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

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &keyObject, keyBuf, keyBufLen, bitlen, NULL, 0);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

#if ENABLE_REENTRANCY

/* Init crypto object mutext */
void init_cryptoObj_mutex(void)
{
#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    Mutex::Init(sSEObjMutex);
#endif
    return;
}

/* Delete all crypto objects in se05x */
void delete_crypto_objects(void)
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
    se05x_sessionOpen();
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
SE05x_CryptoObjectID_t getObjID(void)
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
void setObjID(SE05x_CryptoObjectID_t objId, uint8_t status)
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

#endif //#if ENABLE_REENTRANCY
