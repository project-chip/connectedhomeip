/******************************************************************************

 @file settings.c

 @brief Implements otPlatSettings NV storage API in settings.h

 Group: CMCU, LPC
 Target Device: cc13x2_26x2

 ******************************************************************************

 Copyright (c) 2017-2020, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

//*****************************************************************************
// Basic Operation Overview
//*****************************************************************************
/*
 * This implementation of otPlatSettings uses NVOCMP, or Non Volatile On Chip
 * Multi-Page driver. Please see the nvocmp.c for driver details. NVOCMP stores
 * items that are identified by their system/item/sub ID's. Each time an
 * operation is requested on an NV item, the driver will traverse the active
 * NV pages until it finds the requested item. This means that if the deleteItem
 * function is called on ten items, NVOCMP must traverse the page ten times.
 * In order to make some operations faster, NVOCMP provides an API call
 * doNext() which allows the user to perform operations on items matching a
 * search criteria, while only traversing a page once. This implementation
 * uses doNext heavily.
 *
 * OT keys can have multiple settings which act like a linked list. If a middle
 * setting is deleted, it is expected that there is no "empty" setting and the
 * settings adjacent to the deleted setting are now themselves adjacent. So
 * while an NVOCMP item ID corresponds to an OT key, an NVOCMP sub ID does not
 * correspond to an OT index. Instead the index corresponds to the Nth setting
 * with the correct system ID and item ID (key). Since all items use the same
 * system ID (NVINTF_SYSID_TIOP), the 3rd setting of key 4 corresponds to the
 * 3rd NV item in the page with item ID of 4.
 *
 * Note that the order of NV items on the page may change on any write
 * operation as allowed by the settings.h API. This means that setting order is
 * unreliable after a write operation.
 */

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include <openthread/config.h>

#include <openthread/platform/settings.h>

#include "nv/nvintf.h"
#include "nv/nvocmp.h"

/* CONSTANTS AND MACROS */
#define SUBIDMAX    ((2 << 10) - 1)

/* Static local variables */
static NVINTF_nvFuncts_t sNvoctpFps = { 0 };

/* settings API */
void otPlatSettingsInit(otInstance *aInstance)
{
    /* Load NVOCMP function pointers, extended API */
    NVOCMP_loadApiPtrsExt(&sNvoctpFps);

    /* Initialize NVOCMP */
    sNvoctpFps.initNV(NULL);
}

otError otPlatSettingsBeginChange(otInstance *aInstance)
{
    return(OT_ERROR_NONE);
}

otError otPlatSettingsCommitChange(otInstance *aInstance)
{
    return(OT_ERROR_NONE);
}

otError otPlatSettingsAbandonChange(otInstance *aInstance)
{
    return(OT_ERROR_NONE);
}

otError otPlatSettingsGet(otInstance *aInstance, uint16_t aKey, int aIndex,
                          uint8_t *aValue, uint16_t *aValueLength)
{
    NVINTF_itemID_t nvID;
    uint8_t status = NVINTF_SUCCESS;
    uint16_t count = 0;
    otError error  = OT_ERROR_NOT_FOUND;
    uint32_t itemLen;

    /* doNext search for nth item */
    NVINTF_nvProxy_t nvProxy = {0};
    nvProxy.sysid  = NVINTF_SYSID_TIOP;
    nvProxy.itemid = aKey;
    nvProxy.flag   = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DOFIND;

    /* Lock and call doNext to find nth item of "aKey" */
    intptr_t key = sNvoctpFps.lockNV();
    while(!status && (count <= aIndex))
    {
        status = sNvoctpFps.doNext(&nvProxy);
        count++;
    }
    sNvoctpFps.unlockNV(key);

    /* If we didn't find the nth item, return */
    if (NVINTF_NOTFOUND == status)
    {
        return(OT_ERROR_NOT_FOUND);
    }

    /* Make item ID */
    nvID.systemID = NVINTF_SYSID_TIOP;
    nvID.itemID   = aKey;
    nvID.subID    = nvProxy.subid;

    /* Get length */
    itemLen = sNvoctpFps.getItemLen(nvID);

    /* Determine requested operation */
    if (NULL == aValue)
    {
        if (NULL == aValueLength)
        {
            /* Check if item exists */
            if (itemLen > 0)
            {
                error = OT_ERROR_NONE;
            }
            else
            {
                error = OT_ERROR_NOT_FOUND;
            }
        }
        else
        {
            /* Item length requested */
            *aValueLength = itemLen;
            error = OT_ERROR_NONE;
        }
    }
    else if (NULL != aValueLength)
    {
        /* Read operation requested */
        // fix typing here
        //status = sNvoctpFps.readItem(nvID, NULL, (uint16_t)itemLen, aValue);
        status = sNvoctpFps.readItem(nvID, 0U, (uint16_t)itemLen, aValue);
        /* Replace parameter with actual size of item */
        *aValueLength = itemLen;
        if (status)
        {
            error = OT_ERROR_NOT_FOUND;
        }
        else
        {
            error = OT_ERROR_NONE;
        }
    }
    else
    {
        error = OT_ERROR_NOT_FOUND;
    }

    return(error);
}

otError otPlatSettingsSet(otInstance *aInstance, uint16_t aKey,
                          const uint8_t *aValue, uint16_t aValueLength)
{
    /* Function deletes all items with specific itemID and writes one
     *  item with that item ID */
    NVINTF_itemID_t nvID;
    NVINTF_nvProxy_t nvProxy = {0};
    uint8_t status = NVINTF_SUCCESS;
    otError error  = OT_ERROR_NONE;

    /* Setup doNext call */
    nvProxy.sysid  = NVINTF_SYSID_TIOP;
    nvProxy.itemid = aKey;
    nvProxy.flag   = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DODELETE;

    /* Lock and call doNext to delete all items with itemID of "aKey" */
    intptr_t key = sNvoctpFps.lockNV();
    while(!status)
    {
        status = sNvoctpFps.doNext(&nvProxy);
    }
    sNvoctpFps.unlockNV(key);

    /* Make item ID of new item */
    nvID.systemID = NVINTF_SYSID_TIOP;
    nvID.itemID   = aKey;
    nvID.subID    = 0;

    /* Write item */
    status = sNvoctpFps.writeItem(nvID, aValueLength, (void *)aValue);

    return(error);
}

otError otPlatSettingsAdd(otInstance *aInstance, uint16_t aKey,
                          const uint8_t *aValue, uint16_t aValueLength)
{
    NVINTF_itemID_t nvID;
    NVINTF_nvProxy_t nvProxy = {0};
    uint8_t status           = NVINTF_SUCCESS;
    otError error            = OT_ERROR_FAILED;
    uint32_t itemLen         = 1;
    uint16_t maxSubId        = 0;
    uint16_t minSubId        = 0;

    /* Setup doNext call */
    nvProxy.sysid  = NVINTF_SYSID_TIOP;
    nvProxy.itemid = aKey;
    nvProxy.subid  = 0;
    nvProxy.flag   = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DOFIND;

    /* Lock and call doNext to iterate through all items of itemID "aKey" */
    /* Store min/max of sub id's found */
    intptr_t key = sNvoctpFps.lockNV();
    while(!status)
    {
        status    = sNvoctpFps.doNext(&nvProxy);
        maxSubId  = (nvProxy.subid > maxSubId ? nvProxy.subid : maxSubId);
        minSubId  = (nvProxy.subid < minSubId ? nvProxy.subid : minSubId);
    }
    sNvoctpFps.unlockNV(key);

    /* Populate item ID */
    nvID.systemID = NVINTF_SYSID_TIOP;
    nvID.itemID   = aKey;

    /* Look for an unused subid */
    uint16_t count = 0;
    bool looking   = true;
    while(looking && (count < SUBIDMAX))
    {
        if (maxSubId < SUBIDMAX)
        {
            nvID.subID = ++maxSubId;
            itemLen    = sNvoctpFps.getItemLen(nvID);
            if (!itemLen)
            {
                looking = false;
            }
        }
        else
        {
            maxSubId = 0;
        }
        if (minSubId > 0 && looking)
        {
            nvID.subID = --minSubId;
            itemLen    = sNvoctpFps.getItemLen(nvID);
            if (!itemLen)
            {
                looking = false;
            }
        }
        else
        {
            minSubId = SUBIDMAX;
        }
        count++;
    }

    /* Write item */
    if (!itemLen)
    {
        status = sNvoctpFps.writeItem(nvID, aValueLength, (void *)aValue);
        error = (status == NVINTF_SUCCESS ? OT_ERROR_NONE : OT_ERROR_FAILED);
    }

    return(error);
}

otError otPlatSettingsDelete(otInstance *aInstance, uint16_t aKey, int aIndex)
{
    NVINTF_itemID_t nvID;
    NVINTF_nvProxy_t nvProxy = {0};
    uint8_t status           = NVINTF_SUCCESS;
    otError error            = OT_ERROR_NONE;

    if (aIndex < 0)
    {
        /* Setup doNext call */
        nvProxy.sysid  = NVINTF_SYSID_TIOP;
        nvProxy.itemid = aKey;
        nvProxy.flag   = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DODELETE;

        /* Call doNext to delete all items of itemID "aKey". */
        intptr_t key = sNvoctpFps.lockNV();
        while(!status)
        {
           status = sNvoctpFps.doNext(&nvProxy);
        }
        sNvoctpFps.unlockNV(key);
    }
    else
    {
        /* Setup doNext call */
        nvProxy.sysid  = NVINTF_SYSID_TIOP;
        nvProxy.itemid = aKey;
        nvProxy.flag   = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DOFIND;

        /* Call doNext to find nth matching item, where n = (aIndex - 1) */
        int itemCount = 0;
        intptr_t key  = sNvoctpFps.lockNV();
        while(!status && (itemCount <= aIndex))
        {
           status = sNvoctpFps.doNext(&nvProxy);
           itemCount++;
        }
        sNvoctpFps.unlockNV(key);

        /* If we found our nth item, delete it */
        if (!status)
        {
            nvID.systemID = NVINTF_SYSID_TIOP;
            nvID.itemID   = aKey;
            nvID.subID    = nvProxy.subid;
            status = sNvoctpFps.deleteItem(nvID);
        }

        error = (status == NVINTF_SUCCESS ? error : OT_ERROR_NOT_FOUND);
    }

    return(error);
}

void otPlatSettingsWipe(otInstance *aInstance)
{
    NVINTF_nvProxy_t nvProxy = {0};
    uint8_t status           = NVINTF_SUCCESS;

    /* Setup doNext call */
    nvProxy.sysid = NVINTF_SYSID_TIOP;
    nvProxy.flag  = NVINTF_DOSTART | NVINTF_DOSYSID | NVINTF_DODELETE;

    /* Lock and wipe all items with sysid TIOP */
    intptr_t key = sNvoctpFps.lockNV();
    while(!status)
    {
        status = sNvoctpFps.doNext(&nvProxy);
    }
    sNvoctpFps.unlockNV(key);

    sNvoctpFps.compactNV(0);
}
