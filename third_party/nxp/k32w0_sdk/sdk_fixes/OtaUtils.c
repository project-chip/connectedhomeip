/*! *********************************************************************************
* Copyright 2020 NXP
* All rights reserved.
*
* \file
*
* This is the header file for the OTA Programming Support.
*
** SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

#include "OtaUtils.h"

/* Driver includes */
#include "fsl_flash.h"
#include "fsl_sha.h"

#include "flash_header.h"
#include "rom_psector.h"
#include "rom_secure.h"
#include "rom_api.h"
#include "rom_aes.h"

/************************************************************************************
*************************************************************************************
* Private Macros
*************************************************************************************
************************************************************************************/

#define THUMB_ENTRY(x)                 (void*)((x) | 1)
#define CRC_FINALIZE(x)                ((x) ^ ~0UL)

#ifdef PDM_EXT_FLASH
#define BOOT_BLOCK_OFFSET_MAX_VALUE     0x9de00
#else
#define BOOT_BLOCK_OFFSET_MAX_VALUE     0x96000
#endif

#define SIGNATURE_WRD_LEN               (SIGNATURE_LEN / 4)

#define ROM_API_efuse_LoadUniqueKey      THUMB_ENTRY(0x030016f4)
#define ROM_API_aesLoadKeyFromOTP        THUMB_ENTRY(0x0300146c)
#define ROM_API_crc_update               THUMB_ENTRY(0x0300229c)
#define ROM_API_boot_CheckVectorSum      THUMB_ENTRY(0x03000648)
#define ROM_API_flash_GetDmaccStatus     THUMB_ENTRY(0x03001f64)

#define BUFFER_SHA_LENGTH                16
#define OTA_UTILS_DEBUG(...)

/************************************************************************************
*************************************************************************************
* Private definitions
*************************************************************************************
************************************************************************************/

typedef struct {
    IMAGE_CERT_T certificate;
    uint8_t signature[SIGNATURE_LEN];
} ImageCertificate_t;

typedef struct {
    uint8_t signature[SIGNATURE_LEN];
} ImageSignature_t;

typedef struct {
    uint16_t blob_id;
    uint32_t blob_version;
} ImageCompatibilityListElem_t;

typedef union
{
    IMG_HEADER_T       imgHeader;
    BOOT_BLOCK_T       imgBootBlock;
    ImageCertificate_t imgCertificate; /* will contains only the img signature if no certificate is given */
} ImageParserUnion;

typedef int (*efuse_LoadUniqueKey_t)(void);
typedef uint32_t (*aesLoadKeyFromOTP_t)(AES_KEY_SIZE_T keySize);
typedef uint32_t (*crc_update_t)(uint32_t crc, const void* data, size_t data_len);
typedef uint32_t (*boot_CheckVectorSum_t)(const IMG_HEADER_T *image);
typedef uint32_t (*flash_GetDmaccStatus_t)(uint8_t *address);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

static const efuse_LoadUniqueKey_t efuse_LoadUniqueKey   = (efuse_LoadUniqueKey_t) ROM_API_efuse_LoadUniqueKey;
static const aesLoadKeyFromOTP_t aesLoadKeyFromOTP       = (aesLoadKeyFromOTP_t) ROM_API_aesLoadKeyFromOTP;
static const crc_update_t crc_update                       = (crc_update_t)ROM_API_crc_update;
static const boot_CheckVectorSum_t boot_CheckVectorSum     = (boot_CheckVectorSum_t)ROM_API_boot_CheckVectorSum;
static const flash_GetDmaccStatus_t flash_GetDmaccStatus   = (flash_GetDmaccStatus_t) ROM_API_flash_GetDmaccStatus;

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

static bool_t OtaUtils_IsInternalFlashAddr(uint32_t image_addr)
{
    uint32_t internalFlashAddrStart = 0;
    uint32_t internalFlashSize = 0;
    ROM_GetFlash(&internalFlashAddrStart, &internalFlashSize);
    return ((image_addr >= internalFlashAddrStart)
            && image_addr < (internalFlashAddrStart+internalFlashSize));
}

/* In case of wrong ImgType, IMG_TYPE_NB is returned  */
static uint8_t OtaUtils_CheckImageTypeFromImgHeader(const IMG_HEADER_T *pImageHeader)
{
    uint8_t imgType = IMG_DIRECTORY_MAX_SIZE;
    if (pImageHeader && pImageHeader->imageSignature >= IMAGE_SIGNATURE
            && pImageHeader->imageSignature < IMAGE_SIGNATURE + IMG_DIRECTORY_MAX_SIZE)
    {
        imgType = (pImageHeader->imageSignature - IMAGE_SIGNATURE);
    }
    return imgType;
}

static otaUtilsResult_t OtaUtils_ReadFromEncryptedExtFlash(uint16_t nbBytesToRead,
                                                           uint32_t address,
                                                           uint8_t *pOutbuf,
                                                           OtaUtils_EEPROM_ReadData pFunctionEepromRead,
                                                           eEncryptionKeyType eType,
                                                           void *pParam)
{
    otaUtilsResult_t result = gOtaUtilsError_c;
    otaUtilsResult_t readResult = gOtaUtilsSuccess_c;
    uint8_t alignedBufferStart[16];
    uint8_t alignedBufferEnd[16];
    uint16_t nbByteToRead = nbBytesToRead;
    uint8_t *pBuf = pOutbuf;

    uint32_t lastAddrToRead = address+nbBytesToRead-1;

    /* Encrypted reads require to have an addr aligned on 16 bytes */
    uint16_t nbByteToAlignStart = address % 16;
    uint16_t nbByteToAlignEnd = (16*(lastAddrToRead/16) + 15) - lastAddrToRead;
    uint16_t nbByteToMoveInAlignedBufferStart = 0;
    uint16_t nbByteToMoveInAlignedBufferEnd = 0;
    uint16_t nbByteToReadBeforeEndAlignBuffer = 0;

    address -= nbByteToAlignStart;

    do {
#ifdef DEBUG
        if ((nbByteToRead + nbByteToAlignStart + nbByteToAlignEnd)%16 != 0)
            break;
#endif
        /* Get the number of block that we will need to read */
        int nb_blocks =  (nbByteToRead + nbByteToAlignStart + nbByteToAlignEnd)/16;

        if (nbByteToAlignStart)
        {
            if ((readResult=pFunctionEepromRead(sizeof(alignedBufferStart),  address, &alignedBufferStart[0])) != gOtaUtilsSuccess_c)
            {
                result = readResult;
                break;
            }
            else
            {
                address+= sizeof(alignedBufferStart);
            }
        }

        /* Check if we need to read more bytes */
        if (address < lastAddrToRead)
        {
            if (nbByteToAlignStart)
            {
                nbByteToMoveInAlignedBufferStart = sizeof(alignedBufferStart) - nbByteToAlignStart;
                pBuf += nbByteToMoveInAlignedBufferStart;
            }

            if (nbByteToAlignEnd)
            {
                nbByteToMoveInAlignedBufferEnd = sizeof(alignedBufferEnd) - nbByteToAlignEnd;
            }
            nbByteToReadBeforeEndAlignBuffer = nbByteToRead - nbByteToMoveInAlignedBufferStart - nbByteToMoveInAlignedBufferEnd;
            if (nbByteToReadBeforeEndAlignBuffer%16 != 0)
                break;
            if ((readResult=pFunctionEepromRead(nbByteToReadBeforeEndAlignBuffer,  address, pBuf)) != gOtaUtilsSuccess_c)
            {
                result = readResult;
                break;
            }
            address += nbByteToReadBeforeEndAlignBuffer;
            if (nbByteToAlignEnd && (readResult=pFunctionEepromRead(sizeof(alignedBufferEnd),  address, alignedBufferEnd)) != gOtaUtilsSuccess_c)
            {
                result = readResult;
                break;
            }
        }
        else
        {
            /* The asked buffer is too small and can fit in alignedBufferStart */
            nbByteToAlignEnd = 0;
        }

        if (eType == eEfuseKey)
        {
            //aesInit();
            efuse_LoadUniqueKey();
            aesLoadKeyFromOTP(AES_KEY_128BITS);
        }
        else if (eType == eSoftwareKey && pParam != NULL)
        {
            sOtaUtilsSoftwareKey * pSoftKey = (sOtaUtilsSoftwareKey *) pParam;
            aesLoadKeyFromSW(AES_KEY_128BITS, (uint32_t*)pSoftKey->pSoftKeyAes);
            break;
        }

        aesMode(AES_MODE_ECB_DECRYPT, AES_INT_BSWAP | AES_OUTT_BSWAP);
        if (nbByteToAlignStart)
        {
            aesProcess((void*)alignedBufferStart, (void*)alignedBufferStart,  1);
            nb_blocks -=1;
        }
        if (nbByteToAlignEnd)
        {
            aesProcess((void*)pBuf, (void*)pBuf,  nb_blocks-1);
            aesProcess((void*)alignedBufferEnd, (void*)alignedBufferEnd,  1);
        }
        else
        {
            aesProcess((void*)pBuf, (void*)pBuf,  nb_blocks);
        }

        /* Fill missing pBuf bytes */
        pBuf-=nbByteToMoveInAlignedBufferStart;

        if (nbByteToAlignStart)
        {
            uint16_t i;
            uint16_t t=0;
            for (i=nbByteToAlignStart; i<sizeof(alignedBufferStart); i++)
            {
                pBuf[t++] = alignedBufferStart[i];
            }
        }

        if (nbByteToAlignEnd)
        {
            uint16_t i;
            for (i=0; i<nbByteToMoveInAlignedBufferEnd; i++)
            {
                *(pBuf+nbByteToMoveInAlignedBufferStart+nbByteToReadBeforeEndAlignBuffer+i) = alignedBufferEnd[i];
            }
        }
        result = gOtaUtilsSuccess_c;
    } while (0);

    return result;
}

static bool_t OtaUtils_VerifySignature(uint32_t address,
                                       uint32_t nbBytesToRead,
                                       const uint32_t *pPublicKey,
                                       const uint8_t *pSignatureToVerify,
                                       OtaUtils_ReadBytes pFunctionRead,
                                       void * pReadFunctionParam,
                                       OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    bool_t result = FALSE;
    uint32_t nbPageToRead = nbBytesToRead/BUFFER_SHA_LENGTH;
    uint32_t lastPageNbByteNumber = nbBytesToRead - (nbPageToRead*BUFFER_SHA_LENGTH);
    uint32_t i = 0;
    do {
        uint8_t pageContent[BUFFER_SHA_LENGTH];
        uint8_t digest[32];
        sha_ctx_t hash_ctx;
        size_t sha_sz = 32;
        /* Initialise SHA clock do not call SHA_ClkInit(SHA0) because the HAL pulls in too much code  */
        SYSCON->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_HASH_MASK;
        if (SHA_Init(SHA0, &hash_ctx, kSHA_Sha256) != kStatus_Success)
            break;
        for (i=0; i<nbPageToRead; i++)
        {
            if (pFunctionRead(sizeof(pageContent),  address+(i*BUFFER_SHA_LENGTH), &pageContent[0], pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
                break;
            if (SHA_Update(SHA0, &hash_ctx, (const uint8_t*)pageContent, BUFFER_SHA_LENGTH) != kStatus_Success)
                break;
        }
        /* Read bytes located on the last page */
        if (pFunctionRead(lastPageNbByteNumber,  address+(i*BUFFER_SHA_LENGTH), &pageContent[0], pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
            break;
        if (SHA_Update(SHA0, &hash_ctx, (const uint8_t*)pageContent, lastPageNbByteNumber) != kStatus_Success)
                break;
        if (SHA_Finish(SHA0,  &hash_ctx, &digest[0], &sha_sz) != kStatus_Success)
            break;
        if (!secure_VerifySignature(digest, pSignatureToVerify, pPublicKey))
            break;
        result = TRUE;
    } while (0);

    SYSCON->AHBCLKCTRLCLR[1] =  SYSCON_AHBCLKCTRL1_HASH_MASK; /* equivalent to SHA_ClkDeinit(SHA0) */
    return result;
}

static bool_t OtaUtils_FindBlankPage(uint32_t startAddr, uint16_t size)
{
	bool_t result = FALSE;
	uint32_t addrIterator = startAddr;


	while (addrIterator < startAddr+size)
	{
		if (flash_GetDmaccStatus((uint8_t *)addrIterator) == 0)
		{
			result = TRUE;
			break;
		}
		addrIterator += FLASH_PAGE_SIZE;
	}

	/* Check the endAddr */
	if (!result && flash_GetDmaccStatus((uint8_t *)startAddr+size) == 0)
		result = TRUE;

	return result;
}

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/

otaUtilsResult_t OtaUtils_ReadFromInternalFlash(uint16_t nbBytesToRead,
                                                uint32_t address,
                                                uint8_t *pOutbuf,
                                                void *pParam,
                                                OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    otaUtilsResult_t result = gOtaUtilsError_c;

    do {
        if (!OtaUtils_IsInternalFlashAddr(address))
            break;
        /* If one blank page is found return error */
        if (OtaUtils_FindBlankPage(address, nbBytesToRead))
            break;
        if (pFunctionEepromRead == NULL || pOutbuf == NULL)
            break;
        pFunctionEepromRead(nbBytesToRead, address, pOutbuf);
        result = gOtaUtilsSuccess_c;
    } while (0);

    return result;
}


otaUtilsResult_t OtaUtils_ReadFromUnencryptedExtFlash(uint16_t nbBytesToRead,
                                                            uint32_t address,
                                                            uint8_t *pOutbuf,
                                                            void *pParam,
                                                            OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    otaUtilsResult_t result = gOtaUtilsError_c;
    if (pFunctionEepromRead != NULL)
    {
        result = pFunctionEepromRead(nbBytesToRead, address, pOutbuf);
    }
    return result;
}

otaUtilsResult_t OtaUtils_ReadFromEncryptedExtFlashEfuseKey(uint16_t nbBytesToRead,
                                                            uint32_t address,
                                                            uint8_t *pOutbuf,
                                                            void *pParam,
                                                            OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    return OtaUtils_ReadFromEncryptedExtFlash(nbBytesToRead, address, pOutbuf, pFunctionEepromRead, eEfuseKey, NULL);
}

otaUtilsResult_t OtaUtils_ReadFromEncryptedExtFlashSoftwareKey(uint16_t nbBytesToRead,
                                                            uint32_t address,
                                                            uint8_t *pOutbuf,
                                                            void *pParam,
                                                            OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    return OtaUtils_ReadFromEncryptedExtFlash(nbBytesToRead, address, pOutbuf, pFunctionEepromRead, eSoftwareKey, pParam);
}


uint32_t OtaUtils_ValidateImage(OtaUtils_ReadBytes pFunctionRead,
                                void *pReadFunctionParam,
                                OtaUtils_EEPROM_ReadData pFunctionEepromRead,
                                uint32_t imgAddr,
                                uint32_t minValidAddr,
                                const IMAGE_CERT_T * pRootCert,
                                bool_t inOtaCheck, bool_t isRemappable)
{
    uint32_t result_addr = OTA_UTILS_IMAGE_INVALID_ADDR;
    ImageParserUnion uImgParser;
    uint32_t headerBootBlockMarker = 0;
    uint32_t runAddr = 0;
    uint8_t imgType = IMG_DIRECTORY_MAX_SIZE;
    uint32_t bootBlockOffsetFound = 0;
    uint32_t imgSizeFound = 0;

    do {
        /* Try to extract the imageHeader */
        if (pFunctionRead(sizeof(IMG_HEADER_T),  imgAddr, (uint8_t *)&uImgParser.imgHeader, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
            break;

        imgType = OtaUtils_CheckImageTypeFromImgHeader(&uImgParser.imgHeader);
        if (imgType == IMG_DIRECTORY_MAX_SIZE)
        {
            break;
        }

        if (isRemappable)
        {
            /* Check that entry point is within tested archive */
            runAddr = (uImgParser.imgHeader.vectors[1] & ~0xfffUL);
        }
        else
        {
            runAddr = imgAddr;
        }

        if (!inOtaCheck)
        {
            if (runAddr != imgAddr) break;
        }

        if (uImgParser.imgHeader.bootBlockOffset % sizeof(uint32_t) ) break;

        if (uImgParser.imgHeader.bootBlockOffset + sizeof(BOOT_BLOCK_T) >= BOOT_BLOCK_OFFSET_MAX_VALUE) break;

        /* compute CRC of the header */
        uint32_t crc = ~0UL;
        crc = crc_update(crc, &uImgParser.imgHeader, sizeof(IMG_HEADER_T)-sizeof(uImgParser.imgHeader.header_crc));
        crc = CRC_FINALIZE(crc);

        if (uImgParser.imgHeader.header_crc != crc) break;

        if (boot_CheckVectorSum(&uImgParser.imgHeader) != 0) break;

        /* Save data before parsing the bootblock */
        bootBlockOffsetFound = uImgParser.imgHeader.bootBlockOffset;

        /* Try to extract the bootblock */
        if (pFunctionRead(sizeof(BOOT_BLOCK_T),  bootBlockOffsetFound + imgAddr, (uint8_t *)&uImgParser.imgBootBlock, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
            break;

        headerBootBlockMarker = uImgParser.imgBootBlock.header_marker;

        if (!(  (headerBootBlockMarker >= BOOT_BLOCK_HDR_MARKER) &&
                (headerBootBlockMarker <= BOOT_BLOCK_HDR_MARKER+2))) break;
        if (!inOtaCheck)
        {
            if (uImgParser.imgBootBlock.target_addr != runAddr) break;
        }
        else
        {
            runAddr = uImgParser.imgBootBlock.target_addr;
        }
        if (runAddr < minValidAddr) break;
        if (uImgParser.imgBootBlock.stated_size < (bootBlockOffsetFound + sizeof(BOOT_BLOCK_T))) break;
        if (uImgParser.imgBootBlock.img_len > uImgParser.imgBootBlock.stated_size) break;

        if (uImgParser.imgBootBlock.compatibility_offset != 0)
        {
            uint32_t compatibility_list_sz = 0;
            OTA_UTILS_DEBUG("Compatibility list found\n");
            if (uImgParser.imgBootBlock.compatibility_offset < (bootBlockOffsetFound - sizeof(uint32_t)))
            {
                /* Try to read the compatibility list size */
                if (pFunctionRead(sizeof(uint32_t),  imgAddr + uImgParser.imgBootBlock.compatibility_offset, (uint8_t *)&compatibility_list_sz, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
                    break;
                if (uImgParser.imgBootBlock.compatibility_offset != bootBlockOffsetFound -
                        (sizeof(uint32_t) + compatibility_list_sz *sizeof(ImageCompatibilityListElem_t)) )
                    break;
            }
            else
                break;
        }

        /* Save bootblock data */
        imgSizeFound = uImgParser.imgBootBlock.img_len;

        /* Security check */
        if (pRootCert != NULL)
        {
            uint32_t imgSignatureOffset = bootBlockOffsetFound + sizeof(BOOT_BLOCK_T);
            const uint32_t *pKey =  (const uint32_t *)&pRootCert->public_key[0];
            OTA_UTILS_DEBUG("==> Img authentication is enabled\n");
            if (uImgParser.imgBootBlock.certificate_offset != 0)
            {
                OTA_UTILS_DEBUG("Certificate found\n");
                /* Check that the certificate is inside the img */
                if ((uImgParser.imgBootBlock.certificate_offset + sizeof(ImageCertificate_t)) != imgSizeFound)
                    break;
                /* If there is a certificate is must comply with the expectations */
                /* There must be a trailing ImageAuthTrailer_t appended to boot block */
                if (pFunctionRead(sizeof(ImageCertificate_t),  imgAddr + uImgParser.imgBootBlock.certificate_offset, (uint8_t *)&uImgParser.imgCertificate, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
                    break;
                if (uImgParser.imgCertificate.certificate.certificate_marker != CERTIFICATE_MARKER)
                    break;
                /* Accesses to certificate header, certificate signature and image signature fields
                 * indirectly allow their correct presence via the Bus Fault TRY-CATCH.
                 */
                if (uImgParser.imgCertificate.certificate.public_key[0] == uImgParser.imgCertificate.certificate.public_key[SIGNATURE_WRD_LEN-1])
                    break;
                const uint32_t * cert_sign = (uint32_t*)&uImgParser.imgCertificate.signature[0];
                if (cert_sign[0] == cert_sign[SIGNATURE_WRD_LEN-1])
                {
                    break;
                }
                /* Check the signature of the certificate */
                if (!secure_VerifyCertificate(&uImgParser.imgCertificate.certificate, pKey, &uImgParser.imgCertificate.signature[0]))
                    break;
                pKey =  (const uint32_t *)&uImgParser.imgCertificate.certificate.public_key[0];
                imgSignatureOffset += sizeof(ImageCertificate_t);
            }
            else if (imgSignatureOffset != imgSizeFound)
            {
                break;
            }
            OTA_UTILS_DEBUG("Img signature found\n");
            /* Read the img signature */
            if (pFunctionRead(sizeof(ImageSignature_t), imgAddr + imgSignatureOffset, (uint8_t *)&uImgParser.imgCertificate.signature, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
                    break;

            const uint8_t * img_sign = (uint8_t*)&uImgParser.imgCertificate.signature[0];
            if (img_sign[0] == img_sign[SIGNATURE_WRD_LEN-1])
                break;

            if (!OtaUtils_VerifySignature(imgAddr, imgSignatureOffset, pKey, img_sign, pFunctionRead,
                                            pReadFunctionParam, pFunctionEepromRead))
                break;
        }
        result_addr = runAddr;

    } while (0);
    OTA_UTILS_DEBUG("OTA_Utils => OtaUtils_ValidateImage result addr = 0x%x\n", result_addr);
    return result_addr;
}

otaUtilsResult_t OtaUtils_ReconstructRootCert(IMAGE_CERT_T *pCert, const psector_page_data_t* pPage0, const psector_page_data_t* pFlashPage)
{
    otaUtilsResult_t result = gOtaUtilsError_c;
    uint32_t keyValid;
    do
    {
        if (pCert == NULL || pPage0 == NULL)
            break;

        keyValid = pPage0->page0_v3.img_pk_valid;
        if (keyValid < 2)
        {
            result = gOtaUtilsInvalidKey_c;
            break;
        }
        /* Decrypt the public key using the efuse key */
        efuse_LoadUniqueKey();
        aesLoadKeyFromOTP(AES_KEY_128BITS);
        aesMode(AES_MODE_ECB_DECRYPT, AES_INT_BSWAP | AES_OUTT_BSWAP);
        aesProcess((uint32_t*)&pPage0->page0_v3.image_pubkey[0], &pCert->public_key[0],  SIGNATURE_LEN/16);

        if (pFlashPage != NULL)
        {
            pCert->customer_id   = pFlashPage->pFlash.customer_id;
            pCert->min_device_id = pFlashPage->pFlash.min_device_id;
            pCert->max_device_id = pFlashPage->pFlash.max_device_id;
        }
        else
        {
            pCert->customer_id = psector_Read_CustomerId();
            pCert->min_device_id = psector_Read_MinDeviceId();
            pCert->max_device_id = psector_Read_MaxDeviceId();
        }

        pCert->certificate_marker = CERTIFICATE_MARKER;
        pCert->certificate_id = 0UL;
        pCert->usage_flags = 0UL;
        result = gOtaUtilsSuccess_c;
    } while (0);
    return result;
}
