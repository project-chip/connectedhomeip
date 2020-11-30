/******************************************************************************

 @file  nvintf.h

 @brief Function pointer interface to the NV API

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

#ifndef NVINTF_H
#define NVINTF_H

#include <stdbool.h>
#include <stdint.h>
#ifdef NV_LINUX
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
// Usage Overview
//*****************************************************************************
/*
This file implements a general NV API which can be used with NV drivers
designed to conform to this API. The API requires NV items to be described by
a three number ID contained in the NVINTF_itemID_t struct. System IDs are also
predefined but the user may use their own so long as they do not conflict.
Drivers using this interface are also required to return the defined status
codes.

The particular driver is selected by loading a NVINTF_itemID_t struct with the
drivers function ptrs using the drivers public load function. Note that not all
of the function pointers are required to be populated so it is important to
check for null pointers before making a call. From this point on, NV calls are
made through the struct. The first call must be the .initNv() call.
If this call is successful, more NV calls can be made. A sample code block is
shown below:

NVINTF_nvFuncts_t nvFps;
NVDRIVER_loadApiPtrs(&nvFps);

nvFps.initNV(NULL);
// Do some Nv operations
nvFps.compactNv(NULL);
status = nvFps.readItem(id, 0, len, buf);

*/

//*****************************************************************************
// Constants and definitions
//*****************************************************************************

// NV system ID codes
#define NVINTF_SYSID_NVDRVR 0      // Refrain from use
#define NVINTF_SYSID_ZSTACK 1
#define NVINTF_SYSID_TIMAC  2
#define NVINTF_SYSID_REMOTI 3
#define NVINTF_SYSID_BLE    4
#define NVINTF_SYSID_6MESH  5
#define NVINTF_SYSID_TIOP   6
#define NVINTF_SYSID_APP    7
#define NVINTF_SYSID_WBMS   8
#define NVINTF_SYSID_BMESH  9

// NV driver status codes
#define NVINTF_SUCCESS      0
#define NVINTF_FAILURE      1
#define NVINTF_CORRUPT      2
#define NVINTF_NOTREADY     3
#define NVINTF_BADPARAM     4
#define NVINTF_BADLENGTH    5
#define NVINTF_BADOFFSET    6
#define NVINTF_BADITEMID    7
#define NVINTF_BADSUBID     8
#define NVINTF_BADSYSID     9
#define NVINTF_NOTFOUND     10
#define NVINTF_LOWPOWER     11
#define NVINTF_BADVERSION   12
#define NVINTF_EXIST        13

// doNext flag options
#define NVINTF_DOSTART      0x1     // starts new search
#define NVINTF_DOSYSID      0x2     // filters by sysID
#define NVINTF_DOITMID      0x4     // filters by itemID and sysID
#define NVINTF_DOANYID      0x8     // filters by validity
#define NVINTF_DOFIND       0x10    // no additional op
#define NVINTF_DOREAD       0x20    // reads item contents into buffer
#define NVINTF_DODELETE     0x40    // deletes found items

//*****************************************************************************
// Typedefs
//*****************************************************************************

/**
 * NV Item Identification structure
 */
typedef struct nvintf_itemid_t
{
    //! NV System ID - identifies system (ZStack, BLE, App, OAD...)
    uint8_t  systemID;
    //! NV Item ID
    uint16_t itemID;
    //! NV Item sub ID
    uint16_t subID;
} NVINTF_itemID_t;

// Proxy NV item used by doNext()
typedef struct nvintf_nvproxy_t
{
    uint8_t  sysid;   // User inputs searchable sysID, API returns item sysID
    uint16_t itemid;  // User inputs searchable itemID, API returns item itemID
    uint16_t subid;   // API returns item subID here
    void *   buffer;  // Item contents written here if requested
    uint16_t len;     // User inputs size of buffer, API returns item size
    uint8_t  flag;    // User specifies requested operation by settings flags
} NVINTF_nvProxy_t;

//! Function pointer definition for the NVINTF_initNV() function
typedef uint8_t (*NVINTF_initNV)(void *param);

//! Function pointer definition for the NVINTF_compactNV() function
typedef uint8_t (*NVINTF_compactNV)(uint16_t minBytes);

//! Function pointer definition for the NVINTF_createItem() function
typedef uint8_t (*NVINTF_createItem)(NVINTF_itemID_t id,
                                     uint32_t length,
                                     void *buffer );

//! Function pointer definition for the NVINTF_updateItem() function
typedef uint8_t (*NVINTF_updateItem)(NVINTF_itemID_t id,
                                     uint32_t length,
                                     void *buffer );

//! Function pointer definition for the NVINTF_deleteItem() function
typedef uint8_t (*NVINTF_deleteItem)(NVINTF_itemID_t id);

//! Function pointer definition for the NVINTF_readItem() function
typedef uint8_t (*NVINTF_readItem)(NVINTF_itemID_t id,
                                   uint16_t offset,
                                   uint16_t length,
                                   void *buffer );

//! Function pointer definition for the NVINTF_readContItem() function
typedef uint8_t (*NVINTF_readContItem)(NVINTF_itemID_t id,
                                    uint16_t offset,
                                    uint16_t rlength,
                                    void *rbuffer,
                                    uint16_t clength,
                                    uint16_t coffset,
                                    void *cbuffer,
                                    uint16_t *pSubId);

//! Function pointer definition for the NVINTF_writeItem() function
typedef uint8_t (*NVINTF_writeItem)(NVINTF_itemID_t id,
                                    uint16_t length,
                                    const void *buffer );

//! Function pointer definition for the NVINTF_getItemLen() function
typedef uint32_t (*NVINTF_getItemLen)(NVINTF_itemID_t id);

//! Function pointer definition for the NVINTF_lockItem() function
typedef int32_t (*NVINTF_lockNV)(void);

//! Function pointer definition for the NVINTF_unlockItem() function
typedef void (*NVINTF_unlockNV)(int32_t);

//! Function pointer definition for the NVINTF_doNext() function
typedef uint8_t (*NVINTF_doNext)(NVINTF_nvProxy_t *nvProxy);

//! Function pointer definition for the NVINTF_expectComp() function
typedef bool (*NVINTF_expectComp)(uint16_t length);

//! Function pointer definition for the NVINTF_eraseNV() function
typedef uint8_t (*NVINTF_eraseNV)(void);

//! Function pointer definition for the NVINTF_getFreeNV() function
typedef uint32_t (*NVINTF_getFreeNV)(void);

//! Structure of NV API function pointers
typedef struct nvintf_nvfuncts_t
{
    //! Initialization function
    NVINTF_initNV initNV;
    //! Compact NV function
    NVINTF_compactNV compactNV;
    //! Create item function
    NVINTF_createItem createItem;
    //! Update item function
    NVINTF_updateItem updateItem;
    //! Delete NV item function
    NVINTF_deleteItem deleteItem;
    //! Read item function based on ID
    NVINTF_readItem readItem;
    //! Read item function based on Content
    NVINTF_readContItem readContItem;
    //! Write item function
    NVINTF_writeItem writeItem;
    //! Get item length function
    NVINTF_getItemLen getItemLen;
    //! Iterator Like doNext function
    NVINTF_doNext doNext;
    //! Lock item function
    NVINTF_lockNV lockNV;
    //! Unlock item function
    NVINTF_unlockNV unlockNV;
    //! Expect compact function
    NVINTF_expectComp expectComp;
    //! Erase NV function
    NVINTF_eraseNV eraseNV;
    //! Get Free NV function
    NVINTF_getFreeNV getFreeNV;
} NVINTF_nvFuncts_t;

//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* NVINTF_H */

