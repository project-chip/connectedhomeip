/*
 * Copyright (c) 2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <osi_kernel.h>

#include "ti/common/nv/nvintf.h"
#include "ti/common/nv/nvocmp.h"
#include "ti_flash_map_config.h"

/*!
    \brief the below functions are needed for file management of the upper mac.
           currently only supported file is the WiFi part firmware bin file.
           in the sitara AM243x launchpad, the firmware bin is saved in a specific flash offset.

           those functions are redirected from fxxx to osi_fxxx in the linker command (--symbol_map=fread=osi_fread).
           if already have system that use fopen fclose fread etc. this file should not be compiled in.
*/
#ifndef ATTRIBUTE
#define ATTRIBUTE __attribute__((used))
#endif
#define NVID_BLE_OUR_SEC_OFFSET_IN_FLASH (0x1)
#define NVID_BLE_PEER_SEC_OFFSET_IN_FLASH (0x2)
#define NVID_BLE_CCCD_OFFSET_IN_FLASH (0x3)

#define NVID_WLAN_CONNECTION_POLICY_OFFSET_IN_FLASH (0x4)
#define NVID_WLAN_PROFILES_OFFSET_IN_FLASH (0x5)
#define NVID_WLAN_FAST_CONNECT_OFFSET_IN_FLASH (0x6)

#define GPE_MANIFEST_IMAGE_START_OFFSET (0xFFC)
#define GPE_MANIFEST_INTEGRITY_LEN (0x10)
#define GPE_HEADER_MAGIC_NUM (0x690c47c2)
#define GPE_PROTECTED_TLV_MAGIC_NUM (0x6908)
#define CONF_BIN_TLV_TYPE (0x00B2)
#define GPE_MANIFEST_TLV_TYPE (0x00A0)

typedef enum
{
    OSI_FILE_CONNECTIVITY_FW_SLOT_1,
    OSI_FILE_CONNECTIVITY_FW_SLOT_2,
    OSI_FILE_CONF,
    OSI_FILE_RAMBTLR,
    OSI_FILE_BLE_OUR_SEC,
    OSI_FILE_BLE_PEER_SEC,
    OSI_FILE_BLE_CCCD,
    OSI_FILE_WLAN_CONNECTION_POLICY,
    OSI_FILE_WLAN_PROFILE,
    OSI_FILE_WLAN_FAST_CONNECT,
} osiInternalType_e;

typedef struct
{
    osiInternalType_e ftype;
    void * ptr;
} osiFileP_t;

typedef struct
{
    uint32_t ihMagic;
    uint32_t ihLoadAddr;
    uint16_t ihHdrSize;
    uint16_t ihProtectTlvSize;
    uint32_t ihImgSize;
    uint32_t ihFlags;
} gpeManifestHeader_t;

typedef struct
{
    uint16_t itMagic;
    uint16_t itTlvTot;
} gpeTlvInfo_t;

typedef struct
{
    uint16_t itType;
    uint16_t itLen;
} gpeTlvManifestHeader_t;

typedef struct
{
    uint16_t itType;
    uint16_t reserved;
    uint32_t tlvTablePointerShift;
} gpeTlvManifestTableLine_t;

typedef struct
{
    uint16_t itType;
    uint16_t itLen;
} gpeTlvHeader_t;

#define IS_OSI_FILE(osiFileType)                                                                                                   \
    ((osiFileType) == OSI_FILE_BLE_OUR_SEC || (osiFileType) == OSI_FILE_BLE_PEER_SEC || (osiFileType) == OSI_FILE_BLE_CCCD ||      \
     (osiFileType) == OSI_FILE_WLAN_CONNECTION_POLICY || (osiFileType) == OSI_FILE_WLAN_PROFILE ||                                 \
     (osiFileType) == OSI_FILE_WLAN_FAST_CONNECT)

XMEM_Handle fwHandle;
XMEM_Handle confFileHandle;
NVINTF_nvFuncts_t * nvFptrs = NULL;
uint32_t Fwslot             = OSI_FLASH_CONNECTIVITY_FW_SLOT_1;
uint32_t FwGpeDataOffset    = 0x101c;
uint32_t confGpeDataOffset  = 0;

/******************************************************************************

    NAME:        getProtectedTlvManifestHdr

    DESCRIPTION: This function gets the offset to the protected TLV manifest
                 zone within a flash GPE.

                 Following is a detailed description of a GPE (General
                 Purpose Entity) slot layout and the offset this function
                 returns.

                        ┌────────────────────┬────►Image
                        │       Blank        │     Start
                        ├────────────────────┤
                        │    GPE Manifest    │
                        │     Integrity      │
                        ├────────────────────┤
                        │    GPE Manifest    │
                        │                    │
                        │                    │
                        ├────────────────────┤
                        │                    │
                        │    GPE Container   │
                        │                    │
                 GPE    │                    │
                 Slot   ├────────────────────┼──────────────────┐
                        │    GPE Protected   │   GPE TLV info   │
                        │      TLV Zone      │     tlv_info     │
                        │                    ├──────────────────┤◄─────┐
                        │                    │ GPE Manifest TLV │      │
                        │          .         │   manifest_tlv   │     Get
                        │          .         ├──────────────────┤     Here
                        │          .         │         .        │
                        │                    │         .        │
                        │                    │         .        │
                        ├────────────────────┼──────────────────┘
                        │          .         │
                        │          .         │
                        │          .         │
                        └────────────────────┘
                 Note: We assume that the GPE header magic number is correct.

    PARAMETERS:  flashHandle     [Input] - Handle to GPE flash slot.
                 tlvManifestHdr [Output] - Pre-allocated buffer to hold the
                                           result TLV manifest header.
                 hdrOffset      [Output] - Manifest TLV offset in GPE layout.

    RETURNS:     0 upon success, negative value otherwise.
******************************************************************************/
int32_t getProtectedTlvManifestHdr(XMEM_Handle flashHandle, gpeTlvManifestHeader_t * tlvManifestHdr, uint32_t * hdrOffset)
{
    int32_t ret                        = 0;
    uint32_t offset                    = 0;
    gpeTlvInfo_t tlvInfo               = { 0 };
    gpeManifestHeader_t gpeManifestHdr = { 0 };

    /* The GPE Manifest starts at this address */
    offset = GPE_MANIFEST_IMAGE_START_OFFSET;

    ret = XMEMWFF3_read(flashHandle, offset, &gpeManifestHdr, sizeof(gpeManifestHeader_t), 0);
    if (ret < 0)
    {
        Report("\n\rERROR: Failed reading GPE manifest header");
        return -1;
    }

    /* Goto protected TLV info struct */
    offset += gpeManifestHdr.ihHdrSize + gpeManifestHdr.ihImgSize;

    ret = XMEMWFF3_read(flashHandle, offset, &tlvInfo, sizeof(gpeTlvInfo_t), 0);
    if (ret < 0)
    {
        Report("\n\rERROR: Failed reading GPE protected tlv info");
        return -1;
    }

    if (tlvInfo.itMagic != GPE_PROTECTED_TLV_MAGIC_NUM)
    {
        Report("\n\rERROR: TLV info magic number is wrong");
        return -1;
    }

    /* Goto protected TLV manifest Header */
    offset += sizeof(gpeTlvInfo_t);

    ret = XMEMWFF3_read(flashHandle, offset, tlvManifestHdr, sizeof(gpeTlvManifestHeader_t), 0);
    if (ret < 0)
    {
        Report("\n\rERROR: Failed reading GPE tlv manifest header");
        return -1;
    }

    if (tlvManifestHdr->itType != GPE_MANIFEST_TLV_TYPE)
    {
        Report("\n\rERROR: Wrong GPE manifest header tlv type");
        return -1;
    }

    *hdrOffset = offset;

    return 0;
}

/******************************************************************************

    NAME:        findTlvInTlvManifestTlv

    DESCRIPTION: This function iterates a GPE TLV zone (either protected or
                 not) and extracts a requested TLV.

                 It utilizes the TLV manifest`s records table in order to
                 find out if the requested TLV exists and to calculate its
                 offset relative to the start of the image (GPE slot).

                 Following is a detailed description of a GPE TLV zone
                 (protected or not).

                             ┌──────────────────┬──────────────┐
                             │   GPE TLV info   │   it_magic   │
                             │     tlv_info     ├──────────────┤
                             │                  │  it_tlv_tot  │
                             ├──────────────────┼──────────────┤◄──Input
                             │                  │    it_type   │   offset
                             │                  ├──────────────┤
                             │                  │    it_len    │
                             │ GPE Manifest TLV ├──────────────┤
                             │   manifest_tlv   │              │
                             │                  │ Record Table │
                GPE TLV      │                  │              │
                 zone        ├──────────────────┼──────────────┤
                             │                  │    it_type   │
                             │                  ├──────────────┤
                             │     GPE TLV      │    it_len    │
                             │                  ├──────────────┤
                             │                  │    it_value  │
                             ├──────────────────┼──────────────┘
                             │                  │
                             │     GPE TLV      │
                             │                  │
                             └──────────────────┘
                                      .
                                      .
                                      .
                             ┌──────────────────┐
                             │                  │
                             │     GPE TLV      │
                             │                  │
                             └──────────────────┘

                 Following is a TLV Manifest record table entry description:
                             ┌───────────────┐
                             │    it_type    │
                             ├───────────────┤
                             │    reserved   │
                             ├───────────────┤
                             │   TLV Table   │
                             │ Pointer shift │
                             └───────────────┘

    PARAMETERS:  flashHandle             [Input] - Handle to GPE flash slot.
                 tlvManifestHdrOffset    [Input] - Offset to TLV manifest
                                                   header.
                 tlvManifestHdr         [Output] - TLV manifest header buffer.
                 requestedTlvType        [Input] - Requested TLV type.
                 requestedTlvOffset     [Output] - Offset to requested TLV.

    RETURNS:     0 upon success, negative value otherwise.
******************************************************************************/
int32_t findTlvInTlvManifestTlv(XMEM_Handle flashHandle, uint32_t tlvManifestHdrOffset, gpeTlvManifestHeader_t * tlvManifestHdr,
                                uint16_t requestedTlvType, uint32_t * requestedTlvOffset)
{
    int32_t ret                = 0;
    uint16_t i                 = 0;
    uint16_t protectedTlvCount = 0;
    uint32_t tlvStartOffset = 0, tableOffset = 0;
    gpeTlvManifestTableLine_t manifestTableLine = { 0 };

    /* Goto TLV records table lines */
    tableOffset = tlvManifestHdrOffset + sizeof(gpeTlvManifestHeader_t);

    /* Calculate count of protected TLV in protected TLV zone */
    protectedTlvCount = tlvManifestHdr->itLen / sizeof(gpeTlvManifestTableLine_t);

    /* Calculate end of manifest TLV records table offset */
    tlvStartOffset = tableOffset + tlvManifestHdr->itLen;

    for (i = 0; i < protectedTlvCount; i++)
    {
        ret = XMEMWFF3_read(flashHandle, tableOffset, &manifestTableLine, sizeof(gpeTlvManifestTableLine_t), 0);
        if (ret < 0)
        {
            Report("\n\rERROR: Failed reading GPE tlv manifest table line");
            return -1;
        }

        if (manifestTableLine.itType == requestedTlvType)
        {
            /* The requested TLV start at the end of the TLV record table,
             * in addition to the matching TLV record table line shift.
             */
            *requestedTlvOffset = tlvStartOffset + manifestTableLine.tlvTablePointerShift;
            return 0;
        }

        tableOffset += sizeof(gpeTlvManifestTableLine_t);
    }

    Report("\n\rConf binary wasn't find in vendor image");

    return -1;
}

/******************************************************************************

    NAME:        getProtectedTlvOffset

    DESCRIPTION: This function returns the offset of a protected TLV in a GPE
                 slot, given its type.

    PARAMETERS:  gpeFlashHandle      [Input] - Handle to GPE flash slot.
                 requestedTlvOffset [Output] - Offset to requested TLV.
                 requestedTlvType    [Input] - Requested TLV type.

    RETURNS:     0 upon success, negative value otherwise.
******************************************************************************/
int32_t getProtectedTlvOffset(XMEM_Handle gpeFlashHandle, uint32_t * requestedTlvOffset, uint16_t requestedTlvType)
{
    int32_t ret                           = 0;
    uint32_t tlvManifestHdrOffset         = 0;
    gpeTlvManifestHeader_t tlvManifestHdr = { 0 };

    ret = getProtectedTlvManifestHdr(gpeFlashHandle, &tlvManifestHdr, &tlvManifestHdrOffset);
    if (ret < 0)
    {
        return ret;
    }

    ret = findTlvInTlvManifestTlv(gpeFlashHandle, tlvManifestHdrOffset, &tlvManifestHdr, requestedTlvType, requestedTlvOffset);
    if (ret < 0)
    {
        return ret;
    }

    return 0;
}

/******************************************************************************

    NAME:        getConfContent

    DESCRIPTION: This function gets the cc35xx-conf.bin file offset within the
                 vendor image GPE slot.

    PARAMETERS:  flashHandle    [Input] - Handle to GPE flash slot.

    RETURNS:     0 upon success, negative value otherwise.
******************************************************************************/
int32_t getConfContent(XMEM_Handle flashHandle)
{
    int32_t ret            = 0;
    uint32_t confTlvOffset = 0;

    ret = getProtectedTlvOffset(flashHandle, &confTlvOffset, CONF_BIN_TLV_TYPE);
    if (ret < 0)
    {
        return ret;
    }

    /* Skip TLV header (type, length) */
    confTlvOffset += sizeof(gpeTlvHeader_t);

    /* Set global variable to keep offset to the beginning of the conf file */
    confGpeDataOffset = confTlvOffset;

    return ret;
}

int ATTRIBUTE osi_fset(osiFileSetType containerType, void * params)
{
    if (containerType == OSI_FILESYSTEM_SET_CONNECTIVITY_FW_CONTAINER)
    {
        osiFlashFwSlot_e osiFlashFwSlot = *(osiFlashFwSlot_e *) params;
        if (osiFlashFwSlot == OSI_FLASH_CONNECTIVITY_FW_SLOT_1)
        {
            Fwslot = OSI_FLASH_CONNECTIVITY_FW_SLOT_1;
        }
        else if (osiFlashFwSlot == OSI_FLASH_CONNECTIVITY_FW_SLOT_2)
        {
            Fwslot = OSI_FLASH_CONNECTIVITY_FW_SLOT_2;
        }
    }

    return 0;
}

int ATTRIBUTE osi_fget(osiFileGetType containerType, void * params)
{
    if (containerType == OSI_FILESYSTEM_GET_CONNECTIVITY_FW_CONTAINER)
    {
        osiFlashFwSlot_e * osiFlashFwSlot = (osiFlashFwSlot_e *) params;
        *osiFlashFwSlot                   = Fwslot;
    }

    return 0;
}

int ATTRIBUTE osi_fclose(FILE * _fp)
{
    osiFileP_t * osiFile;
    osiFile = (osiFileP_t *) _fp;

    if (NULL == osiFile)
    {
        return 0;
    }

    if ((osiFile->ftype == OSI_FILE_CONNECTIVITY_FW_SLOT_1) || (osiFile->ftype == OSI_FILE_CONNECTIVITY_FW_SLOT_2) ||
        (osiFile->ftype == OSI_FILE_CONF))
    {
        XMEMWFF3_close((XMEM_Handle) osiFile->ptr);
    }
    else if ((osiFile->ftype == OSI_FILE_BLE_OUR_SEC) || (osiFile->ftype == OSI_FILE_BLE_PEER_SEC) ||
             (osiFile->ftype == OSI_FILE_BLE_CCCD))
    {
        if (osiFile->ptr != NULL)
        {
            nvFptrs->deleteItem((*(NVINTF_itemID_t *) osiFile->ptr));
            os_free(osiFile->ptr);
        }
    }
    else if ((osiFile->ftype == OSI_FILE_WLAN_FAST_CONNECT) || (osiFile->ftype == OSI_FILE_WLAN_CONNECTION_POLICY) ||
             (osiFile->ftype == OSI_FILE_WLAN_PROFILE))
    {
        if (osiFile->ptr != NULL)
        {
            os_free(osiFile->ptr);
        }
    }
    os_free(_fp);
    return 0;
}

FILE * ATTRIBUTE osi_fopen(const char * _fname, const char * _mode)
{
    osiFileP_t * osiFile     = NULL;
    NVINTF_itemID_t * nvItem = NULL;
    XMEM_Params params;

    if (NULL == nvFptrs)
    {
        nvFptrs = os_malloc(sizeof(NVINTF_nvFuncts_t));
        NVOCMP_loadApiPtrs(nvFptrs);
        if (nvFptrs->initNV(NULL) != 0)
        {
            Report("\n\rInit NV failed\n\r");
            ASSERT_GENERAL(0);
        }
    }

    if (strcmp("rambtlr", _fname) == 0)
    {
        osiFile        = os_malloc(sizeof(osiFileP_t));
        osiFile->ftype = OSI_FILE_RAMBTLR;
        osiFile->ptr   = NULL;
        return (FILE *) (osiFile);
    }
    else if (strcmp("fw", _fname) == 0)
    {
        osiFile = os_malloc(sizeof(osiFileP_t));
        if (Fwslot == OSI_FLASH_CONNECTIVITY_FW_SLOT_1)
        {
            osiFile->ftype         = OSI_FILE_CONNECTIVITY_FW_SLOT_1;
            params.regionBase      = wifi_connectivity_physical_slot_1_address;
            params.regionStartAddr = wifi_connectivity_logical_slot_1_address;
            params.regionSize      = wifi_connectivity_slot_1_region_size;
            params.deviceNum       = 0;
            fwHandle               = XMEMWFF3_open(&params);
        }
        else if (Fwslot == OSI_FLASH_CONNECTIVITY_FW_SLOT_2)
        {
            osiFile->ftype         = OSI_FILE_CONNECTIVITY_FW_SLOT_2;
            params.regionBase      = wifi_connectivity_physical_slot_2_address;
            params.regionStartAddr = wifi_connectivity_logical_slot_2_address;
            params.regionSize      = wifi_connectivity_slot_2_region_size;
            params.deviceNum       = 0;
            fwHandle               = XMEMWFF3_open(&params);
        }
        osiFile->ptr = (void *) fwHandle;
        return (FILE *) (osiFile);
    }
    else if (strcmp("cc35xx-conf", _fname) == 0)
    {
        osiFile = os_malloc(sizeof(osiFileP_t));
        if (!osiFile)
        {
            return (FILE *) (NULL);
        }
        osiFile->ftype = OSI_FILE_CONF;

        params.regionBase      = vendor_image_physical_slot_1_address;
        params.regionStartAddr = vendor_image_logical_slot_1_address;
        params.regionSize      = vendor_image_slot_1_region_size;
        params.deviceNum       = 0;

        confFileHandle = XMEMWFF3_open(&params);
        if (confFileHandle == NULL)
        {
            os_free(osiFile);
            return (FILE *) NULL;
        }

        if (getConfContent(confFileHandle) < 0)
        {
            XMEMWFF3_close(confFileHandle);
            os_free(osiFile);
            return (FILE *) NULL;
        }

        osiFile->ptr = (void *) confFileHandle;
        return (FILE *) (osiFile);
    }
    else if (strcmp("our_sec", _fname) == 0)
    {
        osiFile          = os_malloc(sizeof(osiFileP_t));
        nvItem           = os_malloc(sizeof(NVINTF_itemID_t));
        osiFile->ftype   = OSI_FILE_BLE_OUR_SEC;
        nvItem->itemID   = NVID_BLE_OUR_SEC_OFFSET_IN_FLASH;
        nvItem->systemID = NVINTF_SYSID_BLE;
        nvItem->subID    = 0;
        osiFile->ptr     = nvItem;
        return (FILE *) (osiFile);
    }
    else if (strcmp("peer_sec", _fname) == 0)
    {
        osiFile          = os_malloc(sizeof(osiFileP_t));
        nvItem           = os_malloc(sizeof(NVINTF_itemID_t));
        osiFile->ftype   = OSI_FILE_BLE_PEER_SEC;
        nvItem->itemID   = NVID_BLE_PEER_SEC_OFFSET_IN_FLASH;
        nvItem->systemID = NVINTF_SYSID_BLE;
        nvItem->subID    = 0;
        osiFile->ptr     = nvItem;
        return (FILE *) (osiFile);
    }
    else if (strcmp("cccd", _fname) == 0)
    {
        osiFile          = os_malloc(sizeof(osiFileP_t));
        nvItem           = os_malloc(sizeof(NVINTF_itemID_t));
        osiFile->ftype   = OSI_FILE_BLE_CCCD;
        nvItem->itemID   = NVID_BLE_CCCD_OFFSET_IN_FLASH;
        nvItem->systemID = NVINTF_SYSID_BLE;
        nvItem->subID    = 0;
        osiFile->ptr     = nvItem;
        return (FILE *) (osiFile);
    }
    else if (strcmp("conn_p", _fname) == 0)
    {
        osiFile          = os_malloc(sizeof(osiFileP_t));
        nvItem           = os_malloc(sizeof(NVINTF_itemID_t));
        osiFile->ftype   = OSI_FILE_WLAN_CONNECTION_POLICY;
        nvItem->itemID   = NVID_WLAN_CONNECTION_POLICY_OFFSET_IN_FLASH;
        nvItem->systemID = NVINTF_SYSID_WIFI;
        nvItem->subID    = 0;
        osiFile->ptr     = nvItem;
        return (FILE *) (osiFile);
    }
    else if (strcmp("profiles", _fname) == 0)
    {
        osiFile          = os_malloc(sizeof(osiFileP_t));
        nvItem           = os_malloc(sizeof(NVINTF_itemID_t));
        osiFile->ftype   = OSI_FILE_WLAN_PROFILE;
        nvItem->itemID   = NVID_WLAN_PROFILES_OFFSET_IN_FLASH;
        nvItem->systemID = NVINTF_SYSID_WIFI;
        nvItem->subID    = 0;
        osiFile->ptr     = nvItem;
        return (FILE *) (osiFile);
    }
    else if (strcmp(".fast", _fname) == 0)
    {
        osiFile          = os_malloc(sizeof(osiFileP_t));
        nvItem           = os_malloc(sizeof(NVINTF_itemID_t));
        osiFile->ftype   = OSI_FILE_WLAN_FAST_CONNECT;
        nvItem->itemID   = NVID_WLAN_FAST_CONNECT_OFFSET_IN_FLASH;
        nvItem->systemID = NVINTF_SYSID_WIFI;
        nvItem->subID    = 0;
        osiFile->ptr     = nvItem;
        return (FILE *) (osiFile);
    }

    return (FILE *) (NULL);
}

size_t ATTRIBUTE osi_fread(void * _ptr, size_t len, size_t offset, FILE * _fp)
{
    osiFileP_t * osiFile;
    osiFile     = (osiFileP_t *) _fp;
    int16_t ret = 0;

    if (NULL == osiFile)
    {
        return 0;
    }

    if (osiFile->ftype == OSI_FILE_RAMBTLR)
    {
        memcpy(_ptr, (void *) ((uint32_t) (osiFile->ptr) + offset), len);
        return len;
    }
    else if (osiFile->ftype == OSI_FILE_CONF)
    {
        ret = XMEMWFF3_read((XMEM_Handle) osiFile->ptr, confGpeDataOffset + offset, (void *) _ptr, len, 0);
        if (ret < 0)
        {
            return 0;
        }
        return len;
    }
    else if ((osiFile->ftype == OSI_FILE_CONNECTIVITY_FW_SLOT_1) || (osiFile->ftype == OSI_FILE_CONNECTIVITY_FW_SLOT_2))
    {

        ret = XMEMWFF3_read((XMEM_Handle) osiFile->ptr, FwGpeDataOffset + offset, (void *) _ptr, len, 0);
        if (ret < 0)
        {
            return 0;
        }
        return len;
    }
    else if (IS_OSI_FILE(osiFile->ftype))
    {
        if (nvFptrs->readItem((*(NVINTF_itemID_t *) osiFile->ptr), 0, len, _ptr) == 0)
        {
            return (len);
        }
        else
        {
            return 0;
        }
    }

    return 0;
}

size_t osi_fwrite(const void * _ptr, size_t _size, size_t _count, FILE * _fp)
{
    osiFileP_t * osiFile;
    osiFile = (osiFileP_t *) _fp;

    if (NULL == osiFile)
    {
        return 0;
    }

    if (IS_OSI_FILE(osiFile->ftype))
    {
        nvFptrs->deleteItem((*(NVINTF_itemID_t *) osiFile->ptr));
        if (nvFptrs->writeItem((*(NVINTF_itemID_t *) osiFile->ptr), _size, (void *) _ptr) == 0)
        {
            return (_size);
        }
        else
        {
            return (0);
        }
    }

    return 0;
}

int osi_fremove(FILE * _fp)
{
    osiFileP_t * osiFile;
    osiFile = (osiFileP_t *) _fp;

    if (NULL == osiFile)
    {
        return -1;
    }

    if (osiFile->ftype == OSI_FILE_WLAN_FAST_CONNECT)
    {
        if (nvFptrs->deleteItem((*(NVINTF_itemID_t *) osiFile->ptr)) == 0)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

/*!
    \brief initialize the SPI module
    \param data - input buffer
    \param length - length in bytes to read
    \return 0 - success, -1 - failed
    \note
    1. if initializing all the MCU drivers at the beginning of the world, this function can stay empty.
       this function is called in the wlan_start and will stay initialized even if used wlan_stop
    2. the SPI must have the following parameters
       - master
       - 4 pin mode SPI
       - chip select active low
       - polarity 0 phase 0
       - max frequency 40000000Hz
       - data block 32bits
    \warning
*/
size_t ATTRIBUTE osi_filelength(const char * FileName)
{
    return 0;
}
//
