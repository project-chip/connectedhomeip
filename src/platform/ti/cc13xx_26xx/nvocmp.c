/******************************************************************************

 @file  nvocmp.c

 @brief NV driver for CC26x2 devices - On-Chip Multi-Page Flash Memory

 Group: CMCU, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2023-2024, Texas Instruments Incorporated
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
// Design Overview
//*****************************************************************************
/*
This driver implements a non-volatile (NV) memory system that utilizes multi pages
(consecutive) of on-chip Flash memory. After initialization, all pages except one
are ACTIVE and the remaining one page is available for "compaction" when the ACTIVE
pages do not have enough empty space for data write operation. Compaction can occur
'just in time' during a data write operation or 'on demand' by application request.
The compaction process is designed to survive a power cycle before it completes. It
will resume where it was interrupted and complete the process.

This driver makes the following assumptions and uses them to optimize the code
and data storage design: (1) Flash memory is addressable at individual, 1-byte
resolution so no padding or word-alignment is necessary (2) Flash has limited
number of writes per flash 'sector' between erases. To prevent going over this
limit, "small" items are written in one operation.

Each Flash page has a "page header" which indicates its current state,
located at the first byte of the Flash page and "compact header" which indicates
its compaction state, located following "page header". The remainder of
the Flash page contains NV data items which are packed together following the
page header and compact header. Each NV data item has two parts, (1) a data block
which is stored first (lower memory address), (2) immediately followed by item header
(higher memory address). The item header contains information necessary to traverse the
packed data items, as well as, current status of each data item. Obsolete items
marked accordingly but a search for the newest instance of an item is sped up
by starting the search at the last entry in the page (higher memory address).

Each item is unique, addressed using three ID values (system ID, item ID, sub
ID). These three values are stored in the header along with a 'signature', a
CRC8 value, the length of the data block, and two status bits. The two status
bits indicate whether an item is still active and the health or validity of an
item. The signature byte is used by the driver to detect the presence of an
item, and is the same for all items as well as the page header and compact header.
The CRC8 value allows the driver to confirm the integrity of the items during
compaction and optionally when an item read operation is requested. The length
of the data block is used to jump from one item to the next. If this field is
corrupted, the driver is forced to search for items by signature and possibly
compute multiple CRC's to confirm it has found a valid item. Note that any
corruption event forces a compaction to recover.

To reduce further RAM consumption, the user can define NVOCMP_RAM_OPTIMIZATION to
enable this feature. Note that for cc23x0 and cc27xx, NVOCMP_RAM_OPTIMIZATION is enabled
by default. Alternatively, if this optimization is not needed in a particular
application for cc23x0 and cc27xx, the user can define NVOCMP_NO_RAM_OPTIMIZATION to
effectively disable this feature for cc23x0 and cc27xx.

When RAM optimization is enabled, the user can configure the size of the
working buffer by setting NVOCMP_RAM_BUFFER_SIZE, which defaults to 500.

*/
//*****************************************************************************
// Use / Configuration
//*****************************************************************************
/*
Since this is multi page NV driver, the number of NV pages is configurable.
NVOCMP_NVPAGES = 1 means 1 page storage and 0 compaction page.
NVOCMP_NVPAGES = 2 means 1 page storage and 1 compaction page.
NVOCMP_NVPAGES = 3 means 2 pages storage and 1 compaction page.
NVOCMP_NVPAGES = 4 means 3 pages storage and 1 compaction page.
NVOCMP_NVPAGES = 5 means 4 pages storage and 1 compaction page.
NVOCMP_NVPAGES = 6 means 5 pages storage and 1 compaction page.
NVOCMP_NVPAGES can be configured from project option. If this flag is not
configured, NVOCMP_NVPAGES = 2 will be by default.
"nvintf.h" describes the generic NV interface which is used to access NVOCMP
after initialization. Initialization is done by passing a function pointer
struct to one of NVOCMP pointer loader functions. Once this is done, the
pointer struct (which is part of the nvintf interface) should be used to call
the nvintf initialization function, which will initialize NVOCMP. At this point
NVOCMP is ready and loaded API functions can be called through the pointer
structure. Note that some pointers may be NULL depending on which NVOCMP
loader function was called. For example NVOCMP_loadApiPtrsMin() loads only
the essential functions to reduce code size at link time.
A sample code block is shown below:

NVINTF_nvFuncts_t nvFps;
NVOCMP_loadApiPtrs(&nvFps);

nvFps.initNV(NULL);
// Do some NV operations
nvFps.compactNV(NULL);
status = nvFps.readItem(id, 0, len, buf);

Note: Each item operation results in a traversal of the page starting at
the most recently written item. This makes 'finding' items by 'trying' item IDs
in order extremely inefficient. The doNext() API call allows the user to find,
read, or delete items in one page traversal. However, this call requires the
user to lock access to NV until the operation is complete so it should be used
carefully and sparingly.

Note: The compile flag NVDEBUG can be passed to enable ASSERT and ALERT
macros which provide assert and logging functionality. When this flag is used,
a printf function of the form void nvprint(char * str) MUST be
provided to link the driver. The function need not be functional, but it must
exist. NVDEBUG also exposes driver global variables for debug and testing.

Not all user-defines (such as NVDEBUG) are supported when using NVOCMP in a
Linux environment. If debugging/logging functionality is required, the
"nv-debug" or "nv-rdwr" logging flags can be enabled in the cfg INI file.

Configuration:
NVOCMP_STATS - Places a protected item with driver stats

NVOCMP_CRCONREAD (on:1 off:0) - item crc is checked on read. Disabling this may
increase driver speed but safety is reduced.

NVOCMP_NVS_INDEX - The index of the NVS_Config structure which describes the
flash sector that NVOCMP should use. Default is 0.

NVOCMP_RECOVER_FROM_COMPACT_FAILURE - This define needs to be enabled by
the customer. It is disabled be default. When enabled, it causes the
NV driver to reformat all NV pages when there is an error while
collecting valid items for compaction. When disabled and an error
occurs while collecting valid items for compaction, NV pages will
preserve the original information prior to the start of such
operation.

ENABLE_SANITY_CHECK - This define needs to be enabled if user needs the
NVOCMP_sanityCheckApi() to be available. This function is used to perform
a sanity check on the active partition to report if corruption has been
detected.

NVOCMP_RAM_OPTIMIZATION - Enables RAM optimization.
NVOCMP_NO_RAM_OPTIMIZATION - Disables RAM optimization for cc23x0 and cc27xx, as it
is enabled by default on this family of devices.
NVOCMP_RAM_BUFFER_SIZE - Sets the size for the RAM buffer used when
RAM optimization is enabled. Default value is 500.

Dependencies:
Requires NVS for NV access.
Requires TI-RTOS GateMutexPri or POSIX mutex to be enabled in configuration.
Requires API's in a crc.h to implement CRC functionality.
*/

//*****************************************************************************
// Includes
//*****************************************************************************

#include <string.h>
#ifdef NVOCMP_POSIX_MUTEX
#include <pthread.h>
#elif defined(NVOCMP_POSIX_SEM)
#include <semaphore.h>
#else
#include <ti/sysbios/gates/GateMutexPri.h>
#endif
#include <crc.h>
#include <nvocmp.h>
#ifndef NV_LINUX

#include <ti/devices/DeviceFamily.h>
#include <ti/devices/cc13x4_cc26x4/driverlib/vims.h>

#ifdef NVOCMP_MIN_VDD_FLASH_MV
#include <driverlib/aon_batmon.h>
#endif
#endif

#ifdef NV_LINUX
#include "nv_linux.h"
#endif

//*****************************************************************************
// Constants and Definitions
//*****************************************************************************

#define NVINTF_DONOWRAP 0x80 // Do not wrap around NV space
#define NVOCMP_FASTCP 1      // Fast Compaction by Skipping All Active Item Pages
#define NVOCMP_COMPR 0       // Order Change When Compaction
#define NVOCMP_HDRLE 0       // Little Endian Format Item Header
#define NVOCMP_FASTOFF 1     // Fast Search Offset
#define NVOCMP_FASTITEM 0    // Fast Find Item

#ifndef NVOCMP_NWSAMEITEM
#define NVOCMP_NWSAMEITEM 0 // Not Write Same Item
#endif

#ifndef NVOCMP_MIGRATE_ENABLED
#define NVOCMP_MIGRATE_DISABLED // Migration from old NVOCTP disabled by default
#endif

#define NVOCMP_NVONEP 1 // One Page NV
#define NVOCMP_NVTWOP 2 // Two Page NV

#define NVOCMP_NVSIZE NVOCMP_size
#define NVOCMP_ADDPAGE(p, n) (((p) + (n)) % NVOCMP_NVSIZE)
#define NVOCMP_INCPAGE(p) NVOCMP_ADDPAGE(p, 1)
#define NVOCMP_DECPAGE(p) NVOCMP_ADDPAGE(p, NVOCMP_NVSIZE - 1)
// Which NVS_config indice is used to initialize NVS.
#ifndef NVOCMP_NVS_INDEX
#define NVOCMP_NVS_INDEX 0
#endif // NVOCMP_NVS_INDEX

// Maximum ID parameters - must be coordinated with header format
#define NVOCMP_MAXSYSID 0x003F     //  6 bits
#define NVOCMP_MAXITEMID 0x03FF    // 10 bits
#define NVOCMP_MAXSUBID 0x03FF     // 10 bits
#define NVOCMP_MAXLEN 0x0FFF       // 12 bits
#define NVOCMP_INVALIDSUBID 0xFFFF // Invalid Sub Id

// Contents of an erased Flash memory locations
#define NVOCMP_ERASEDBYTE 0xFF
#define NVOCMP_ERASEDWORD 0xFFFFFFFF

// Size of byte
#define NVOCMP_ONEBYTE 1

// Compressed ID bit spacing
#define NVOCMP_CMPSPACE 12

// Invalid NV page - if 0xFF is ever used, change this definition
#define NVOCMP_NULLPAGE 0xFF

// Block size for Flash-Flash XFER (Bytes)
#define NVOCMP_XFERBLKMAX 32

// Size in bytes of biggest item size that will be concatenated
// in RAM before write, instead of header/data written separately
#define NVOCMP_SMALLITEM 12

#if defined(NVOCMP_STATS)
// NV item ID for driver diagnostics
static const NVINTF_itemID_t diagId = NVOCMP_NVID_DIAG;
#endif // NVOCMP_STATS

// CRC options
// When not NULL, reads will result in a CRC check before returning
#define NVOCMP_CRCONREAD 1

// findItem search types
// Find any item, item of spec'd sysid, item of spec'd sys and item id
// or find the exact item specified
enum
{
    NVOCMP_FINDANY = 0x00,
    NVOCMP_FINDSYSID,
    NVOCMP_FINDITMID,
    NVOCMP_FINDSTRICT,
    NVOCMP_FINDCONTENT = 0x10
};

#define NVOCMP_FINDLMASK 0x0F
#define NVOCMP_FINDHMASK 0xF0
//*****************************************************************************
// Macros
//*****************************************************************************

// Makes an NV Flash address (for 0x2000 page size)
#define NVOCMP_FLASHOFFSET(pg, ofs) ((uint32_t) (((pg) << PAGE_SIZE_LSHIFT) + (ofs)))

// Optional user provided function is called before writes/erases
// Intention is to check for sufficient voltage for operation
#ifndef NV_LINUX
#define NVOCMP_FLASHACCESS(err)                                                                                                    \
    {                                                                                                                              \
        if (NVOCMP_voltCheckFptr)                                                                                                  \
        {                                                                                                                          \
            if (!NVOCMP_voltCheckFptr())                                                                                           \
            {                                                                                                                      \
                err = NVINTF_LOWPOWER;                                                                                             \
            }                                                                                                                      \
        }                                                                                                                          \
    }
#endif

#ifdef NVOCMP_POSIX_MUTEX
// Lock driver access via TI-RTOS gatemutex
#define NVOCMP_LOCK() pthread_mutex_lock(&NVOCMP_gPosixMutex);

// Unlock driver access via TI-RTOS gatemutex and return error code
#define NVOCMP_UNLOCK(err)                                                                                                         \
    {                                                                                                                              \
        pthread_mutex_unlock(&NVOCMP_gPosixMutex);                                                                                 \
        return (err);                                                                                                              \
    }
#elif defined(NVOCMP_POSIX_SEM)
// Lock driver access via POSIX semaphore
#define NVOCMP_LOCK() sem_wait(&NVOCMP_gPosixSem);

// Unlock driver access via POSIX semaphore and return error code
#define NVOCMP_UNLOCK(err)                                                                                                         \
    {                                                                                                                              \
        sem_post(&NVOCMP_gPosixSem);                                                                                               \
        return (err);                                                                                                              \
    }
#else
// Lock driver access via TI-RTOS gatemutex
#define NVOCMP_LOCK() int32_t key = GateMutexPri_enter(NVOCMP_gMutexPri);

// Unlock driver access via TI-RTOS gatemutex and return error code
#define NVOCMP_UNLOCK(err)                                                                                                         \
    {                                                                                                                              \
        GateMutexPri_leave(NVOCMP_gMutexPri, key);                                                                                 \
        return (err);                                                                                                              \
    }
#endif

// Generate a compressed NV ID (NOTE: bit31 must be zero)
#define NVOCMP_CMPRID(s, i, b)                                                                                                     \
    ((uint32_t) ((((((s) &NVOCMP_MAXSYSID) << NVOCMP_CMPSPACE) | ((i) &NVOCMP_MAXITEMID)) << NVOCMP_CMPSPACE) |                    \
                 ((b) &NVOCMP_MAXSUBID)))

// NVOCMP Unit Test Assert Macro/Function
#ifdef NVDEBUG
extern void nvprint(char * message);
extern void Main_assertHandler(uint8_t assertReason);
static void NVOCMP_assert(bool cond, char * message, bool fatal)
{
    if (!cond)
    {
        nvprint("NVDEBUG: ");
        nvprint(message);
        if (fatal)
        {
            Main_assertHandler(0);
        }
    }
}
#define NVOCMP_ASSERT(cond, message) NVOCMP_assert((cond), (message), true);
#define NVOCMP_ALERT(cond, message) NVOCMP_assert((cond), (message), false);
#define NVOCMP_ASSERT1(cond)                                                                                                       \
    {                                                                                                                              \
        if (!cond)                                                                                                                 \
            while (1)                                                                                                              \
                ;                                                                                                                  \
    }
#else
#ifdef NV_LINUX
#define NVOCMP_ASSERT1(cond) NVOCMP_ASSERT((cond), "NVOCMP_ASSERT1")
#else
#define NVOCMP_ASSERT(cond, message)
#define NVOCMP_ALERT(cond, message)
#define NVOCMP_ASSERT1(cond)                                                                                                       \
    {                                                                                                                              \
        if (!cond)                                                                                                                 \
            while (1)                                                                                                              \
                ;                                                                                                                  \
    }
#endif
#endif // NVDEBUG

//*****************************************************************************
// Page and Header Definitions
//*****************************************************************************
#if defined(DeviceFamily_CC13X4) || defined(DeviceFamily_CC26X4) || defined(DeviceFamily_CC26X3) ||                                \
    defined(DeviceFamily_CC23X0R5) || defined(DeviceFamily_CC23X0R2) || defined(DeviceFamily_CC27XX)
// CC26x4/CC13x4/CC23x0/cc27xx devices flash page size is (1 << 11) or 0x800
#define PAGE_SIZE_LSHIFT 11
#else
// CC26x2/CC13x2 devices flash page size is (1 << 13) or 0x2000
#define PAGE_SIZE_LSHIFT 13
#endif
#if !defined(FLASH_PAGE_SIZE)
#define FLASH_PAGE_SIZE (1 << PAGE_SIZE_LSHIFT)
#endif // FLASH_PAGE_SIZE

#if !defined(NVOCMP_VERSION)
// Version of NV page format (do not use 0xFF)
#define NVOCMP_VERSION 0x03
#endif // NVOCMP_VERSION

#if !defined(NVOCMP_SIGNATURE)
// Page header validation byte (do not use 0xFF)
#define NVOCMP_SIGNATURE 0x96
#endif // NVOCMP_SIGNATURE

#ifndef NVOCMP_NO_RAM_OPTIMIZATION
#if defined(DeviceFamily_CC23X0R5) || defined(DeviceFamily_CC23X0R2) || defined(DeviceFamily_CC27XX)
#define NVOCMP_RAM_OPTIMIZATION
#endif
#endif

#ifndef NVOCMP_RAM_OPTIMIZATION
// Compact Memory
#if !defined(NV_LINUX) && !defined(DeviceFamily_CC13X4) && !defined(DeviceFamily_CC26X4) && !defined(DeviceFamily_CC26X3) &&       \
    !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC27XX)
#define NVOCMP_GPRAM
#endif

#ifdef NVOCMP_GPRAM
#define RAM_BUFFER_ADDRESS (uint8_t *) GPRAM_BASE
#else
/* When CC23X0/CC27XX is used, as GPRAM is not supported,
 * an SRAM buffer of FLASH_PAGE_SIZE length is declared,
 * as the NVOCMP algorithm relies on it.
 * Also, when CC13X4 / CC26X3 / CC26X4 is used,
 * GPRAM cannot be used as it is always mapped to secure
 * address space, and therefore cannot be used by non-secure
 * application. A buffer in SRAM is used instead.
 * */
uint32_t tBuffer[FLASH_PAGE_SIZE >> 2];
#endif
#else
#ifdef NVOCMP_FASTOFF
#undef NVOCMP_FASTOFF
#endif
#define NVOCMP_FASTOFF 0

#ifdef NVOCMP_FASTITEM
#undef NVOCMP_FASTITEM
#endif
#define NVOCMP_FASTITEM 0

#ifndef NVOCMP_RAM_BUFFER_SIZE
#define NVOCMP_RAM_BUFFER_SIZE 500
#endif
uint8_t tBuffer[NVOCMP_RAM_BUFFER_SIZE];
#endif

// Page header structure
typedef struct
{
    uint32_t state : 8;
    uint32_t cycle : 8; // Rolling page compaction count (0x00, 0xFF not used)
    uint32_t allActive : 2;
    uint32_t version : 6;   // Version of NV page format
    uint32_t signature : 8; // Signature for formatted NV page
} NVOCMP_pageHdr_t;

typedef struct
{
    uint16_t pageOffset;
    uint8_t page;
    uint8_t signature;
} NVOCMP_compactHdr_t;

// Page header size (bytes)
#define NVOCMP_PGHDRLEN (sizeof(NVOCMP_pageHdr_t))
#define NVOCMP_COMPACTHDRLEN (sizeof(NVOCMP_compactHdr_t))

// Page header offsets (from 1st byte of page)
#define NVOCMP_PGHDROFS 0
#define NVOCMP_PGHDRPST 0 // Page state
#define NVOCMP_PGHDRCYC 1 // Cycle count
#define NVOCMP_PGHDRVER 2 // Format version
#define NVOCMP_PGHDRSIG 3 // Page signature

// Compact header offsets
#define NVOCMP_COMPMODEOFS 6 // Compact Mode Offset

// Number of Compact headers
#define NVOCMP_NOCOMPHDR 3

// Page data size, offset into page
#define NVOCMP_PGDATAOFS (NVOCMP_PGHDRLEN + NVOCMP_NOCOMPHDR * NVOCMP_COMPACTHDRLEN)
#define NVOCMP_PGDATAEND (FLASH_PAGE_SIZE - 1)
#define NVOCMP_PGDATALEN (FLASH_PAGE_SIZE - NVOCMP_PGDATAOFS)

// Page mode of operation
#define NVOCMP_PGNORMAL 0xFF // normal operation
#define NVOCMP_PGCDST 0xFE   // used as compact destination
#define NVOCMP_PGCDONE 0xFC
#define NVOCMP_PGCSRC 0xF8 // used as compact source
#define NVOCMP_PGMODEBIT 0x04

// NVOCTP header defines
#define NVOCTP_PGACTIVE 0xA5 // Current active page
#define NVOCTP_PGXFER 0x24   // Active page being compacted
#define NVOCTP_PGDATAOFS NVOCMP_PGHDRLEN
#define NVOCTP_VERSION 0x02
#define NVOCTP_SIGNATURE 0x96

// Page header state values - transitions change 1 bit in each nybble
typedef enum NVOCMP_pageState
{
    NVOCMP_PGNACT = 0xFF,
    NVOCMP_PGXDST = 0xFE,
    NVOCMP_PGRDY  = 0x7E,
    NVOCMP_PGACT  = 0x7C,
    NVOCMP_PGFULL = 0x78,
    NVOCMP_PGXSRC = 0x70,
    NVOCMP_PGNDEF = 0x00,
} NVOCMP_pageState_t;

typedef enum NVOCMP_compactStatus
{
    NVOCMP_COMPACT_SUCCESS = 0x00,
    NVOCMP_COMPACT_SRCDONE = 0x01,
    NVOCMP_COMPACT_DSTDONE = 0x02,
    NVOCMP_COMPACT_BOTHDOE = 0x03,
    NVOCMP_COMPACT_FAILURE = 0x10,
} NVOCMP_compactStatus_t;

// Page compaction cycle count limits (0x00 and 0xFF not used)
#define NVOCMP_MINCYCLE 0x01 // Minimum cycle count (after rollover)
#define NVOCMP_MAXCYCLE 0xFE // Maximum cycle count (before rollover)

#define NVOCMP_ALLACTIVE 0x3    // All Items are active
#define NVOCMP_SOMEINACTIVE 0x0 // Some Items are inactive

//*****************************************************************************
// Item Header Definitions
//*****************************************************************************

// Item header structure
typedef struct
{
    uint32_t cmpid;  // Compressed ID
    uint16_t subid;  // Sub ID
    uint16_t itemid; // Item ID
    uint8_t sysid;   // System ID
    uint8_t crc8;    // crc byte
    uint8_t sig;     // signature byte
    uint8_t stats;   // Status 'marks'
    uint16_t hofs;   // Header offset
    uint16_t len;    // Data length
    uint8_t hpage;   // Header page
} NVOCMP_itemHdr_t;

// Length (bytes) of compressed header
#define NVOCMP_ITEMHDRLEN 7

// Offset from beginning (low address) of header to fields in the header
#define NVOCMP_HDRSIGOFS 6
#define NVOCMP_HDRVLDOFS 5

// Number of bytes in header to include in CRC calculation
#define NVOCMP_HDRCRCINC 5

// Compressed item header information <-- Lower Addr    Higher Addr-->
// Byte: [0]      [1]      [2]      [3]      [4]      [5]      [6]
// Item: SSSSSSII IIIIIIII SSSSSSSS SSLLLLLL LLLLLLCC CCCCCCAV SSSSSSSS
// LSB of field:         ^           ^            ^        ^          ^
// Bit:  0              15          25           37       45         55
//
// Bit(s)  Bit Field Description
// =============================
// 48-55:  Signature byte (NVOCMP_SIGNATURE)
//    47:  valid id mark (0=valid)
//    46:  active id mark (1=active)
// 38-45:  CRC8 value
// 26-37:  data length (0-4095)
// 16-25:  item sub id (0-1023)
//  6-15:  nv item id (0-1023)
//   0-5:  system id (0-63)

// Bit47 in compressed header - '1' indicates 'active' NV item
// A deleted item is 'inactive'
#define NVOCMP_ACTIVEIDBIT 0x2
// Bit46 in compressed header - '0' indicates 'valid' NV item
// A corrupted item is 'invalid'
#define NVOCMP_VALIDIDBIT 0x1
// This bit is NOT included in the NV item itself but is encoded
// the 'stats' field of the itemHdr_t struct when the item is read
#define NVOCMP_FOLLOWBIT 0x4

// Index of last item header byte
#define NVOCMP_ITEMHDREND (NVOCMP_ITEMHDRLEN - 1)

// Compressed item header byte array
typedef uint8_t cmpIH_t[NVOCMP_ITEMHDRLEN];

// Item write parameters
typedef struct
{
    NVOCMP_itemHdr_t * iHdr; // Ptr to item header
    uint16_t dOfs;           // Source data offset
    uint16_t bOfs;           // Buffer data offset
    uint16_t len;            // Buffer data length
    uint8_t * pBuf;          // Ptr to data buffer
} NVOCMP_itemWrp_t;

typedef struct
{
    void * cBuf;      // Pointer to content to search
    uint16_t clength; // Length of content to search
    uint16_t coff;    // Offset content to search
    void * rBuf;      // Pointer to content to read
    uint16_t rlength; // Length content to read
} NVOCMP_itemInfo_t;

typedef enum NVOCMP_initAction
{
    NVOCMP_NORMAL_INIT = 0,
    NVOCMP_NORMAL_RESUME,
    NVOCMP_RECOVER_COMPACT,
    NVOCMP_RECOVER_ERASE,
    NVOCMP_FORCE_CLEAN,
    NVOCMP_NORMAL_MIGRATE,
    NVOCMP_ERROR_UNKNOWN,
} NVOCMP_initAction_t;

typedef enum NVOCMP_writeMode
{
    NVOCMP_WRITE = 0,
    NVOCMP_CREATE,
    NVOCMP_UPDATE,
} NVOCMP_writeMode_t;

typedef struct
{
    uint8_t state;     // page state
    uint8_t cycle;     // page compaction cycle count. Used to select the 'newest' active page
                       // at device reset, in the very unlikely scenario that both pages are active.
    uint8_t mode;      // compact mode
    uint8_t allActive; // all items are active or not
    uint8_t sPage;
    uint8_t ePage;
    uint16_t offset; // page offset
    uint16_t sOffset;
    uint16_t eOffset;
} NVOCMP_pageInfo_t;

typedef struct
{
    uint8_t xDstPage;     // xdst page
    uint8_t xSrcSPage;    // xsrc start page
    uint8_t xSrcEPage;    // xsrc end page
    uint8_t xSrcPages;    // no of xsrc pages
    uint16_t xDstOffset;  // xdst offset
    uint16_t xSrcSOffset; // xsrc start offset
    uint16_t xSrcEOffset; // xsrc end offset
} NVOCMP_compactInfo_t;

typedef struct
{
    uint8_t nvSize;       // no of NV pages
    uint8_t headPage;     // head active page
    uint8_t tailPage;     // transfer destination page
    uint8_t actPage;      // current active page
    uint8_t xsrcPage;     // transfer source page
    uint8_t forceCompact; // force compaction to happen
    uint16_t actOffset;   // active page offset
    uint16_t xsrcOffset;  // transfer source page offset
    uint16_t xdstOffset;  // transfer destination page offset
    NVOCMP_compactInfo_t compactInfo;
    NVOCMP_pageInfo_t pageInfo[NVOCMP_NVPAGES];
} NVOCMP_nvHandle_t;
//*****************************************************************************
// Local variables
//*****************************************************************************
#define NVOCMP_NULLOFFSET 0xFFFF
#define DEFAULT_COMPACTHDR { NVOCMP_NULLOFFSET, NVOCMP_NULLPAGE, NVOCMP_SIGNATURE };
#define THISPAGEHDR 0
#define XSRCSTARTHDR 1
#define XSRCENDHDR 2
#define NVOCMP_COMPACTHDRLEN (sizeof(NVOCMP_compactHdr_t))
// NVS Objects
#ifdef NVDEBUG
// Expose these in debug mode
NVS_Handle NVOCMP_nvsHandle;
NVS_Attrs NVOCMP_nvsAttrs;
NVOCMP_nvHandle_t NVOCMP_nvHandle;
#else
static NVS_Handle NVOCMP_nvsHandle;
static NVS_Attrs NVOCMP_nvsAttrs;

/* The following variable has been made non-static, so it can be accessed
 * through other modules through "extern". However, this is a temporary
 * solution, as this variable is meant to be used only by NVOCMP.
 * Users of NVOCMP must only access these features through the available
 * APIs.*/
/*static*/ NVOCMP_nvHandle_t NVOCMP_nvHandle;
#endif // NVDEBUG

// Flag to indicate that a fatal error occurred while writing to or erasing the
// Flash memory. If flag is set, it's unsafe to attempt another write or erase.
// This flag locks writes to Flash until the next system reset.
static uint8_t NVOCMP_failF = NVINTF_NOTREADY;

// Flag to indicate that a non-fatal error occurred while writing to or erasing
// Flash memory. With flag set, it's still safe to attempt a write or erase.
// This flag is reset by any API calls that cause an erase/write to Flash.
static uint8_t NVOCMP_failW;

// TI-RTOS gateMutexPri for the NV driver API functions
#ifdef NVOCMP_POSIX_MUTEX
static pthread_mutex_t NVOCMP_gPosixMutex;
#elif defined(NVOCMP_POSIX_SEM)
static sem_t NVOCMP_gPosixSem;
#else
static GateMutexPri_Handle NVOCMP_gMutexPri;
#endif

// Small NV Item Buffer, for item construction
static uint8_t NVOCMP_itemBuffer[NVOCMP_SMALLITEM];

// Function Pointer to an optional user provided voltage check function
static bool (*NVOCMP_voltCheckFptr)(void);
// Diagnostic counter for bad CRCs
#ifdef NVOCMP_STATS
static uint16_t NVOCMP_badCRCCount = 0;
#endif // NVOCMP_STATS

NVOCMP_initAction_t gAction;
uint8_t NVOCMP_size;

//*****************************************************************************
// NV API Function Prototypes
//*****************************************************************************

static uint8_t NVOCMP_initNvApi(void * param);
static uint8_t NVOCMP_compactNvApi(uint16_t min);
static uint8_t NVOCMP_createItemApi(NVINTF_itemID_t id, uint32_t len, void * buf);
static uint8_t NVOCMP_updateItemApi(NVINTF_itemID_t id, uint32_t len, void * buf);
static uint8_t NVOCMP_deleteItemApi(NVINTF_itemID_t id);
static uint32_t NVOCMP_getItemLenApi(NVINTF_itemID_t id);
static uint8_t NVOCMP_readItemApi(NVINTF_itemID_t id, uint16_t ofs, uint16_t len, void * buf);
static uint8_t NVOCMP_readContItemApi(NVINTF_itemID_t id, uint16_t ofs, uint16_t rlen, void * rBuf, uint16_t clen, uint16_t coff,
                                      void * cBuf, uint16_t * pSubId);
static uint8_t NVOCMP_writeItemApi(NVINTF_itemID_t id, uint16_t len, void * buf);
static uint8_t NVOCMP_doNextApi(NVINTF_nvProxy_t * prx);
static int32_t NVOCMP_lockNvApi(void);
static void NVOCMP_unlockNvApi(int32_t);
static bool NVOCMP_expectCompApi(uint16_t len);
static uint8_t NVOCMP_eraseNvApi(void);
static uint32_t NVOCMP_getFreeNvApi(void);

#ifdef ENABLE_SANITY_CHECK
static uint32_t NVOCMP_sanityCheckApi(void);
#endif

//*****************************************************************************
// NV Local Function Prototypes
//*****************************************************************************

static void NVOCMP_initNv(NVOCMP_nvHandle_t * pNvHandle);
static uint8_t NVOCMP_scanPage(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, NVOCMP_pageInfo_t * pPageInfo);
static int8_t NVOCMP_findItem(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, uint16_t ofs, NVOCMP_itemHdr_t * pHdr, int8_t flag,
                              NVOCMP_itemInfo_t * pInfo);
static uint8_t NVOCMP_addItem(NVOCMP_nvHandle_t * pNvHandle, NVOCMP_itemHdr_t * iHdr, uint8_t * pBuf, NVOCMP_writeMode_t wm);
static void NVOCMP_writeItem(NVOCMP_nvHandle_t * pNvHandle, NVOCMP_itemHdr_t * pHdr, uint8_t dstPg, uint16_t dstOff,
                             uint8_t * pBuf);
static uint8_t NVOCMP_erase(NVOCMP_nvHandle_t * pNvHandle, uint8_t dstPg);
static int16_t NVOCMP_compactPage(NVOCMP_nvHandle_t * pNvHandle, uint16_t nBytes);
static NVOCMP_compactStatus_t NVOCMP_compact(NVOCMP_nvHandle_t * pNvHandle);
static uint8_t NVOCMP_getDstPage(NVOCMP_nvHandle_t * pNvHandle, uint16_t len);
static void NVOCMP_changePageState(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, NVOCMP_pageState_t state);
static void NVOCMP_setPageState(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, NVOCMP_pageState_t state);
static void NVOCMP_setItemInactive(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, uint16_t iOfs);
static uint8_t NVOCMP_readItem(NVOCMP_itemHdr_t * iHdr, uint16_t ofs, uint16_t len, void * pBuf, bool flag);
static uint8_t NVOCMP_checkItem(NVINTF_itemID_t * id, uint16_t len, NVOCMP_itemHdr_t * iHdr, uint8_t flag);
static inline void NVOCMP_read(uint8_t pg, uint16_t off, uint8_t * pBuf, uint16_t len);
static uint8_t NVOCMP_write(uint8_t dstPg, uint16_t off, uint8_t * pBuf, uint16_t len);
static void NVOCMP_readHeader(uint8_t pg, uint16_t ofs, NVOCMP_itemHdr_t * iHdr, bool flag);
static void NVOCMP_setCompactHdr(uint8_t dstPg, uint8_t pg, int16_t offset, uint16_t location);
static uint16_t NVOCMP_findOffset(uint8_t pg, uint16_t ofs);
static uint8_t NVOCMP_doNVCRC(uint8_t pg, uint16_t ofs, uint16_t len, uint8_t crc, bool flag);
static uint8_t NVOCMP_doRAMCRC(uint8_t * input, uint16_t len, uint8_t crc);
static uint8_t NVOCMP_verifyCRC(uint16_t iOfs, uint16_t len, uint8_t crc, uint8_t pg, bool flag);
static uint8_t NVOCMP_readByte(uint8_t pg, uint16_t ofs);
static void NVOCMP_writeByte(uint8_t pg, uint16_t ofs, uint8_t bwv);

#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
static uint8_t NVOCMP_findDstPage(NVOCMP_nvHandle_t * pNvHandle);
static uint8_t NVOCMP_cleanPage(NVOCMP_nvHandle_t * pNvHandle);
static uint8_t NVOCMP_findPage(NVOCMP_pageState_t state);
static void NVOCMP_getCompactHdr(uint8_t dstPg, uint16_t location, NVOCMP_compactHdr_t * pHdr);
#endif

#if (NVOCMP_NVPAGES > NVOCMP_NVONEP) && !defined(NVOCMP_MIGRATE_DISABLED)
static void NVOCMP_migratePage(NVOCMP_nvHandle_t * pNvHandle, uint8_t page);
#endif

#if ((NVOCMP_NVPAGES > NVOCMP_NVONEP) && !defined(NVOCMP_MIGRATE_DISABLED)) || defined NVOCMP_RAM_OPTIMIZATION
static void NVOCMP_copyItem(uint8_t srcPg, uint8_t dstPg, uint16_t sOfs, uint16_t dOfs, uint16_t len);
#endif

//*****************************************************************************
// Load Pointer Functions (These are declared in nvoctp.h)
//*****************************************************************************

/**
 * @fn      NVOCMP_loadApiPtrs
 *
 * @brief   Global function to return function pointers for NV driver API that
 *          are supported by this module, NULL for functions not supported.
 *
 * @param   pfn - pointer to caller's structure of NV function pointers
 *
 * @return  none
 */
void NVOCMP_loadApiPtrs(NVINTF_nvFuncts_t * pfn)
{
    // Load caller's structure with pointers to the NV API functions
    pfn->initNV       = &NVOCMP_initNvApi;
    pfn->compactNV    = &NVOCMP_compactNvApi;
    pfn->createItem   = &NVOCMP_createItemApi;
    pfn->updateItem   = &NVOCMP_updateItemApi;
    pfn->deleteItem   = &NVOCMP_deleteItemApi;
    pfn->readItem     = &NVOCMP_readItemApi;
    pfn->readContItem = &NVOCMP_readContItemApi;
    pfn->writeItem    = &NVOCMP_writeItemApi;
    pfn->getItemLen   = &NVOCMP_getItemLenApi;
    pfn->lockNV       = NULL;
    pfn->unlockNV     = NULL;
    pfn->doNext       = NULL;
    pfn->expectComp   = &NVOCMP_expectCompApi;
    pfn->eraseNV      = &NVOCMP_eraseNvApi;
    pfn->getFreeNV    = &NVOCMP_getFreeNvApi;
#ifdef ENABLE_SANITY_CHECK
    pfn->sanityCheck = &NVOCMP_sanityCheckApi;
#endif
}

/**
 * @fn      NVOCMP_loadApiPtrsMin
 *
 * @brief   Global function to return function pointers for NV driver API that
 *          are supported by this module, NULL for functions not supported.
 *          This function loads the minimum necessary API functions.
 *          This should allow smaller code size.
 *
 * @param   pfn - pointer to caller's structure of NV function pointers
 *
 * @return  none
 */
void NVOCMP_loadApiPtrsMin(NVINTF_nvFuncts_t * pfn)
{
    // Load caller's structure with pointers to the NV API functions
    pfn->initNV       = &NVOCMP_initNvApi;
    pfn->compactNV    = &NVOCMP_compactNvApi;
    pfn->createItem   = NULL;
    pfn->updateItem   = NULL;
    pfn->deleteItem   = NULL;
    pfn->readItem     = &NVOCMP_readItemApi;
    pfn->readContItem = NULL;
    pfn->writeItem    = &NVOCMP_writeItemApi;
    pfn->getItemLen   = NULL;
    pfn->lockNV       = NULL;
    pfn->unlockNV     = NULL;
    pfn->doNext       = NULL;
    pfn->expectComp   = &NVOCMP_expectCompApi;
    pfn->eraseNV      = &NVOCMP_eraseNvApi;
    pfn->getFreeNV    = &NVOCMP_getFreeNvApi;
#ifdef ENABLE_SANITY_CHECK
    pfn->sanityCheck = &NVOCMP_sanityCheckApi;
#endif
}

/**
 * @fn      NVOCMP_loadApiPtrsExt
 *
 * @brief   Global function to return function pointers for NV driver API that
 *          are supported by this module, NULL for functions not supported.
 *          This function also loads the 'extended' API function pointers.
 *
 * @param   pfn - pointer to caller's structure of NV function pointers
 *
 * @return  none
 */
void NVOCMP_loadApiPtrsExt(NVINTF_nvFuncts_t * pfn)
{
    // Load caller's structure with pointers to the NV API functions
    pfn->initNV       = &NVOCMP_initNvApi;
    pfn->compactNV    = &NVOCMP_compactNvApi;
    pfn->createItem   = &NVOCMP_createItemApi;
    pfn->updateItem   = &NVOCMP_updateItemApi;
    pfn->deleteItem   = &NVOCMP_deleteItemApi;
    pfn->readItem     = &NVOCMP_readItemApi;
    pfn->readContItem = &NVOCMP_readContItemApi;
    pfn->writeItem    = &NVOCMP_writeItemApi;
    pfn->getItemLen   = &NVOCMP_getItemLenApi;
    pfn->lockNV       = &NVOCMP_lockNvApi;
    pfn->unlockNV     = &NVOCMP_unlockNvApi;
    pfn->doNext       = &NVOCMP_doNextApi;
    pfn->expectComp   = &NVOCMP_expectCompApi;
    pfn->eraseNV      = &NVOCMP_eraseNvApi;
    pfn->getFreeNV    = &NVOCMP_getFreeNvApi;
#ifdef ENABLE_SANITY_CHECK
    pfn->sanityCheck = &NVOCMP_sanityCheckApi;
#endif
}

/**
 * @fn      NVOCMP_setCheckVoltage
 *
 * @brief   Global function to allow user to provide a voltage check function
 *          for the driver to use. If a pointer is provided, the driver will
 *          call the provided function before flash erases and writes. The
 *          provided function should return true when the battery voltage is
 *          sufficient and vice versa. The user can withdraw their function
 *          by passing a NULL pointer to this function.
 *
 * @param   funcPtr - pointer to a function which returns a bool.
 *
 * @return  none
 */
extern void NVOCMP_setCheckVoltage(void * funcPtr)
{
#ifndef NV_LINUX
    NVOCMP_voltCheckFptr = (bool (*)()) funcPtr;
#else
    // Do nothing
    (void) NVOCMP_voltCheckFptr;
#endif
}

#ifdef NVOCMP_MIN_VDD_FLASH_MV
/**
 * @fn      NVOCMP_setLowVoltageCb
 *
 * @brief   Global function to allow user to provide a low voltage callback function
 *          for the driver to use. If a pointer is provided, the driver will
 *          call the provided function when low voltage detected.
 *
 * @param   funcPtr - pointer to a function.
 *
 * @return  none
 */
static lowVoltCbFptr NVOCMP_lowVoltCbFptr = NULL;
extern void NVOCMP_setLowVoltageCb(lowVoltCbFptr funcPtr)
{
#ifndef NV_LINUX
    NVOCMP_lowVoltCbFptr = (lowVoltCbFptr) funcPtr;
#else
    // Do nothing
    (void) NVOCMP_lowVoltCbFptr;
#endif
}

/*******************************************************************************
 * @fn      NVOCMP_checkVoltage()
 *
 * @brief   Checks the caller supplied voltage threshold against the value read
 *          from the CC26xx BATMON register.
 *
 * @param   none
 *
 * @return  false if device voltage less than limit, otherwise true
 *******************************************************************************
 */
static bool NVOCMP_checkVoltage(void)
{
    uint32_t voltage = AONBatMonBatteryVoltageGet();
    voltage          = (voltage * 1000) >> AON_BATMON_BAT_FRAC_W;
    if (voltage < NVOCMP_MIN_VDD_FLASH_MV)
    {
        // Measured device voltage is below threshold
        if (NVOCMP_lowVoltCbFptr)
        {
            NVOCMP_lowVoltCbFptr(voltage);
        }
        return (false);
    }

    return (true);
}
#endif

#ifdef NVDEBUG
void NVOCMP_corruptData(uint8_t pg, uint16_t off, uint16_t len, uint8_t buf)
{
    NVS_write(NVOCMP_nvsHandle, NVOCMP_FLASHOFFSET(pg, off), (uint8_t *) &buf, len, NVS_WRITE_POST_VERIFY);
}
#endif

/******************************************************************************
 * @fn      NVOCMP_initNvApi
 *
 * @brief   API function to initialize the specified NV Flash pages
 *
 * @param   param - pointer to caller's structure of NV init parameters
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_initNvApi(void * param)
{
    NVOCMP_ALERT(false, "NVOCMP Init. Called!")
    NVOCMP_failW = NVOCMP_failF;

    if (NVOCMP_failF == NVINTF_NOTREADY)
    {
#ifdef NVOCMP_POSIX_MUTEX
        pthread_mutexattr_t attr;
#elif defined(NVOCMP_POSIX_SEM)
#else
        GateMutexPri_Params gateParams;
#endif

        // Only one init per device reset
        NVOCMP_failF = NVINTF_SUCCESS;
        NVOCMP_failW = NVINTF_SUCCESS;

        // Create a priority gate mutex for the NV driver
#ifdef NVOCMP_POSIX_MUTEX
        if (pthread_mutexattr_init(&attr) != 0)
        {
            NVOCMP_failF = NVINTF_FAILURE;
            return (NVOCMP_failF);
        }

#ifndef NV_LINUX
        attr.type = PTHREAD_MUTEX_RECURSIVE;
#else
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
        if (pthread_mutex_init(&NVOCMP_gPosixMutex, &attr) != 0)
        {
            NVOCMP_failF = NVINTF_FAILURE;
            return (NVOCMP_failF);
        }
#elif defined(NVOCMP_POSIX_SEM)
        sem_init(&NVOCMP_gPosixSem, 0 /* ignored */, 1);
#else
        GateMutexPri_Params_init(&gateParams);
        NVOCMP_gMutexPri = GateMutexPri_create(&gateParams, NULL);
#endif

        memset(&NVOCMP_nvHandle, 0, sizeof(NVOCMP_nvHandle_t));
        memset(&NVOCMP_nvHandle.compactInfo, 0xFF, sizeof(NVOCMP_compactInfo_t));

#ifndef NV_LINUX
#ifdef NVOCMP_MIN_VDD_FLASH_MV
        NVOCMP_setCheckVoltage((void *) &NVOCMP_checkVoltage);
#endif
        // Initialize NVS objects
        NVS_init();

        // Use default NVS_Params to open this flash region
        NVOCMP_nvsHandle = NVS_open(NVOCMP_NVS_INDEX, param);

        // Get NV hardware attributes
        NVS_getAttrs(NVOCMP_nvsHandle, &NVOCMP_nvsAttrs);
#else
        NV_LINUX_init();

        NVOCMP_nvsHandle           = NVS_HANDLE;
        NVOCMP_nvsAttrs.sectorSize = FLASH_PAGE_SIZE;
        NVOCMP_nvsAttrs.regionSize = FLASH_PAGE_SIZE * NVOCMP_NVPAGES;
#endif

        NVOCMP_nvHandle.nvSize = NVOCMP_nvsAttrs.regionSize / NVOCMP_nvsAttrs.sectorSize;
        NVOCMP_nvHandle.nvSize = NVOCMP_nvHandle.nvSize > NVOCMP_NVPAGES ? NVOCMP_NVPAGES : NVOCMP_nvHandle.nvSize;
        NVOCMP_size            = NVOCMP_nvHandle.nvSize;

        // Confirm NV region has expected characteristics
        if (FLASH_PAGE_SIZE != NVOCMP_nvsAttrs.sectorSize || (NVOCMP_NVSIZE * FLASH_PAGE_SIZE > NVOCMP_nvsAttrs.regionSize))
        {
            NVOCMP_failF = NVINTF_FAILURE;
            NVOCMP_EXCEPTION(pg, NVINTF_FAILURE)
            return (NVOCMP_failF);
        }

        // Confirm that the NVS region opened properly
        if (NVOCMP_nvsHandle == NULL)
        {
            NVOCMP_failF = NVINTF_FAILURE;
            NVOCMP_ASSERT(false, "NVS HANDLE IS NULL")
            NVOCMP_EXCEPTION(pg, NVINTF_NOTREADY);
            return (NVOCMP_failF);
        }

        // Initialize force compaction to false
        NVOCMP_nvHandle.forceCompact = 0;

        // Look for active page and clean up the other if necessary
        NVOCMP_nvHandle.actPage   = NVOCMP_NULLPAGE;
        NVOCMP_nvHandle.actOffset = FLASH_PAGE_SIZE;

        NVOCMP_initNv(&NVOCMP_nvHandle);

#if defined(NVOCMP_STATS)
        {
            uint8_t err;
            NVOCMP_diag_t diags;

            // Look for a copy of diagnostic info
            err = NVOCMP_readItemApi(diagId, 0, sizeof(diags), &diags);
            if (err == NVINTF_NOTFOUND)
            {
                // Assume this is the first time,
                memset(&diags, 0, sizeof(diags));
                // Space available for everything else
                diags.available = FLASH_PAGE_SIZE - (NVOCMP_nvHandle.actOffset + NVOCMP_ITEMHDRLEN + sizeof(diags));
            }
            // Remember this reset
            diags.resets += 1;
            // Create/Update the diagnostic NV item
            NVOCMP_writeItemApi(diagId, sizeof(diags), &diags);
        }
#endif // NVOCMP_STATS
    }

    return (NVOCMP_failW);
}

/******************************************************************************
 * @fn      NVOCMP_eraseNvApi
 *
 * @brief   API function to erase whole NV pages
 *
 * @param   none
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_eraseNvApi(void)
{
    uint8_t pg;
    uint8_t err = NVINTF_SUCCESS;

    // Check voltage if possible
    NVOCMP_FLASHACCESS(err)
    if (err)
    {
        return (err);
    }

    NVOCMP_LOCK();

    // Erase All pages before start
    for (pg = 0; pg < NVOCMP_NVSIZE; pg++)
    {
        NVOCMP_failW |= NVOCMP_erase(&NVOCMP_nvHandle, pg);
    }

    err = NVOCMP_failW;

    // initial state, set head page, act page and tail page
    NVOCMP_nvHandle.headPage  = 0;
    NVOCMP_nvHandle.tailPage  = NVOCMP_NVSIZE - 1;
    NVOCMP_nvHandle.actPage   = 0;
    NVOCMP_nvHandle.actOffset = NVOCMP_nvHandle.pageInfo[NVOCMP_nvHandle.actPage].offset;
    NVOCMP_changePageState(&NVOCMP_nvHandle, NVOCMP_nvHandle.headPage, NVOCMP_PGRDY);
    NVOCMP_changePageState(&NVOCMP_nvHandle, NVOCMP_nvHandle.tailPage, NVOCMP_PGXDST);

#ifdef NV_LINUX
    if (err == NVINTF_SUCCESS)
    {
        NV_LINUX_save();
    }
#endif

    NVOCMP_UNLOCK(err);
}

/******************************************************************************
 * @fn      NVOCMP_getFreeNvApi
 *
 * @brief   API function to get free space in whole NV pages
 *
 * @param   none
 *
 * @return  bytes of free space
 */
static uint32_t NVOCMP_getFreeNvApi(void)
{
    uint8_t pg = NVOCMP_nvHandle.actPage;
    NVOCMP_pageHdr_t pageHdr;
    uint32_t freespace = 0;
#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
    uint16_t nvSearched = 0;
    for (pg = NVOCMP_nvHandle.actPage; nvSearched < NVOCMP_NVSIZE; pg = NVOCMP_INCPAGE(pg))
    {
        nvSearched++;
        if (pg == NVOCMP_nvHandle.tailPage)
        {
            continue;
        }
#endif
        NVOCMP_read(pg, NVOCMP_PGHDROFS, (uint8_t *) &pageHdr, NVOCMP_PGHDRLEN);
        if ((pageHdr.state == NVOCMP_PGNACT) || (pageHdr.state == NVOCMP_PGRDY) || (pageHdr.state == NVOCMP_PGACT))
        {
            freespace += (FLASH_PAGE_SIZE - NVOCMP_nvHandle.pageInfo[pg].offset);
        }
#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
    }
#endif
    return (freespace);
}

/******************************************************************************
 * @fn      NVOCMP_compactNvApi
 *
 * @brief   API function to force NV active page compaction
 *
 * @param   minAvail - threshold size of available bytes on Flash page to do
 *                     compaction: 0 = always, >0 = minimum remaining bytes
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_compactNvApi(uint16_t minAvail)
{
    uint8_t err = NVINTF_SUCCESS;

    // Check voltage if possible
    NVOCMP_FLASHACCESS(err)
    if (err)
    {
        return (err);
    }

    // Prevent RTOS thread contention
    NVOCMP_LOCK();
    NVOCMP_ALERT(false, "API Compaction Request.")
    err = NVOCMP_failF;
    // Check for a fatal error
    if (err == NVINTF_SUCCESS)
    {
        int16_t left;

        // Number of bytes left on active page
        left = FLASH_PAGE_SIZE - NVOCMP_nvHandle.actOffset;

        // Time to do a compaction?
        if ((left < minAvail) || (minAvail == 0))
        {
            // Transfer all items to non-ACTIVE page
            (void) NVOCMP_compactPage(&NVOCMP_nvHandle, 0);
            // 'failW' indicates compaction status
            err = NVOCMP_failW;
        }
        else
        {
            // Indicate "bad" minAvail value
            err = NVINTF_BADPARAM;
        }
    }

#ifdef NV_LINUX
    if (err == NVINTF_SUCCESS)
    {
        NV_LINUX_save();
    }
#endif

    NVOCMP_UNLOCK(err);
}

//*****************************************************************************
// API Functions - NV Data Items
//*****************************************************************************

/******************************************************************************
 * @fn      NVOCMP_createItemApi
 *
 * @brief   API function to create a new NV item in Flash memory. This function
 *          will return an error if the specified item already exists.
 *
 * @param   id - NV item type identifier
 * @param   len - length of NV data
 * @param   pBuf - pointer to caller's data buffer  (NULL is illegal)
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_createItemApi(NVINTF_itemID_t id, uint32_t len, void * pBuf)
{
    uint8_t err;
    NVOCMP_itemHdr_t iHdr;

    // Parameter Sanity Check
    if (pBuf == NULL || len == 0)
    {
        return (NVINTF_BADPARAM);
    }

    err = NVOCMP_checkItem(&id, len, &iHdr, NVOCMP_FINDSTRICT);
    if (err)
    {
        return (err);
    }

    // Check voltage if possible
    NVOCMP_FLASHACCESS(err)
    if (err)
    {
        return (err);
    }

    // Prevent RTOS thread contention
    NVOCMP_LOCK();

    err = NVOCMP_findItem(&NVOCMP_nvHandle, NVOCMP_nvHandle.actPage, NVOCMP_nvHandle.actOffset, &iHdr, NVOCMP_FINDSTRICT, NULL);

    if (err == NVINTF_SUCCESS)
    {
        err = NVINTF_EXIST;
    }
    else if (err == NVINTF_NOTFOUND)
    {
        // Create the new item
        err = NVOCMP_addItem(&NVOCMP_nvHandle, &iHdr, pBuf, NVOCMP_CREATE);
        if (err != NVINTF_SUCCESS)
        {
            NVOCMP_ALERT(false, "createItem failed.")
            err = NVINTF_FAILURE;
        }
    }
    else
    {
        NVOCMP_ALERT(false, "createItem failed.")
        err = NVINTF_FAILURE;
    }

#ifdef NV_LINUX
    if (err == NVINTF_SUCCESS)
    {
        NV_LINUX_save();
    }
#endif

    NVOCMP_UNLOCK(err);
}

/******************************************************************************
 * @fn      NVOCMP_updateItemApi
 *
 * @brief   API function to update an existing NV item in Flash memory. This function
 *          will return an error if the specified item does not exist.
 *
 * @param   id - NV item type identifier
 * @param   len - length of NV data
 * @param   pBuf - pointer to caller's data buffer  (NULL is illegal)
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_updateItemApi(NVINTF_itemID_t id, uint32_t len, void * pBuf)
{
    uint8_t err;
    NVOCMP_itemHdr_t iHdr;

    // Parameter Sanity Check
    if (pBuf == NULL || len == 0)
    {
        return (NVINTF_BADPARAM);
    }

    err = NVOCMP_checkItem(&id, len, &iHdr, NVOCMP_FINDSTRICT);
    if (err)
    {
        return (err);
    }

    // Check voltage if possible
    NVOCMP_FLASHACCESS(err)
    if (err)
    {
        return (err);
    }

    // Prevent RTOS thread contention
    NVOCMP_LOCK();

    err = NVOCMP_findItem(&NVOCMP_nvHandle, NVOCMP_nvHandle.actPage, NVOCMP_nvHandle.actOffset, &iHdr, NVOCMP_FINDSTRICT, NULL);

    if (err == NVINTF_SUCCESS)
    {
        // Create the new item
        err = NVOCMP_addItem(&NVOCMP_nvHandle, &iHdr, pBuf, NVOCMP_UPDATE);
        if ((err == NVINTF_SUCCESS) && (iHdr.hofs > 0))
        {
            // Mark old item as inactive
            NVOCMP_setItemInactive(&NVOCMP_nvHandle, iHdr.hpage, iHdr.hofs);

            err = NVOCMP_failW;
        }
    }
    else if (err == NVINTF_NOTFOUND)
    {
        err = NVINTF_NOTFOUND;
    }
    else
    {
        NVOCMP_ALERT(false, "updateItem failed.")
        err = NVINTF_FAILURE;
    }

#ifdef NV_LINUX
    if (err == NVINTF_SUCCESS)
    {
        NV_LINUX_save();
    }
#endif

    NVOCMP_UNLOCK(err);
}

/******************************************************************************
 * @fn      NVOCMP_deleteItemApi
 *
 * @brief   API function to delete an existing NV item from Flash memory. Note,
 *          it is inefficient to use this function to delete a range of items.
 *          The doNext call is recommended for that use case.
 *
 * @param   id - NV item type identifier
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_deleteItemApi(NVINTF_itemID_t id)
{
    uint8_t err;
    NVOCMP_itemHdr_t iHdr;

#if defined(NVOCMP_STATS)
    if (!memcmp(&id, &diagId, sizeof(NVINTF_itemID_t)))
    {
        // Protect NV driver item(s)
        return (NVINTF_BADSYSID);
    }
#endif // NVOCMP_STATS

    err = NVOCMP_checkItem(&id, 0, &iHdr, NVOCMP_FINDSTRICT);
    if (err)
    {
        return (err);
    }

    // Check voltage if possible
    NVOCMP_FLASHACCESS(err)
    if (err)
    {
        return (err);
    }

    // Prevent RTOS thread contention
    NVOCMP_LOCK();

    err = NVOCMP_findItem(&NVOCMP_nvHandle, NVOCMP_nvHandle.actPage, NVOCMP_nvHandle.actOffset, &iHdr, NVOCMP_FINDSTRICT, NULL);

    if (!err)
    {
        // Mark this item as inactive
        NVOCMP_setItemInactive(&NVOCMP_nvHandle, iHdr.hpage, iHdr.hofs);

        // Verify that item has been removed
        err = (NVOCMP_findItem(&NVOCMP_nvHandle, NVOCMP_nvHandle.actPage, NVOCMP_nvHandle.actOffset, &iHdr, NVOCMP_FINDSTRICT,
                               NULL) == NVINTF_NOTFOUND)
            ? NVOCMP_failW
            : NVINTF_FAILURE;

        // If item did get deleted, report 'failW' status
        NVOCMP_ALERT(err == NVOCMP_failW, "Item delete failed.")
    }

#ifdef NV_LINUX
    if (err == NVINTF_SUCCESS)
    {
        NV_LINUX_save();
    }
#endif

    NVOCMP_UNLOCK(err);
}

/******************************************************************************
 * @fn      NVOCMP_getItemLenApi
 *
 * @brief   API function to return the length of an NV data item
 *
 * @param   id - NV item type identifier
 *
 * @return  NV item length or 0 if item not found
 */
static uint32_t NVOCMP_getItemLenApi(NVINTF_itemID_t id)
{
    uint8_t err;
    uint32_t len = 0;
    NVOCMP_itemHdr_t iHdr;

    err = NVOCMP_checkItem(&id, 0, &iHdr, NVOCMP_FINDSTRICT);
    if (err)
    {
        return (len);
    }

    // Prevent RTOS thread contention
    NVOCMP_LOCK();

    // If there was any error, report zero length
    len = (NVOCMP_findItem(&NVOCMP_nvHandle, NVOCMP_nvHandle.actPage, NVOCMP_nvHandle.actOffset, &iHdr, NVOCMP_FINDSTRICT, NULL) !=
           NVINTF_SUCCESS)
        ? 0
        : iHdr.len;

    NVOCMP_UNLOCK(len);
}

/******************************************************************************
 * @fn      NVOCMP_readContItemApi
 *
 * @brief   API function to read data from an NV item by comparing content
 *
 * @param   id   - NV item type identifier
 * @param   ofs - offset into NV data
 * @param   rlen - length of NV data to return (0 is illegal)
 * @param   rBuf - pointer to caller's read data buffer  (NULL is illegal)
 * @param   clen - length of NV data to return (0 is illegal)
 * @param   coff - offset of content in data
 * @param   cBuf - pointer to caller's read data buffer  (NULL is illegal)
 * @param   pSubId - pointer to store sub Id  (NULL is illegal)
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_readContItemApi(NVINTF_itemID_t id, uint16_t ofs, uint16_t rlen, void * rBuf, uint16_t clen, uint16_t coff,
                                      void * cBuf, uint16_t * pSubId)
{
    uint8_t err;
    NVOCMP_itemHdr_t iHdr;
    NVOCMP_itemInfo_t itemInfo;

    *pSubId = NVOCMP_INVALIDSUBID;
    // Parameter Sanity Check
    if (rBuf == NULL || rlen == 0 || cBuf == NULL || clen == 0 || coff > FLASH_PAGE_SIZE)
    {
        return (NVINTF_BADPARAM);
    }

    err = NVOCMP_checkItem(&id, rlen, &iHdr, NVOCMP_FINDSTRICT);
    if (err)
    {
        return (err);
    }

    // Prevent RTOS thread contention
    NVOCMP_LOCK();

    itemInfo.cBuf    = cBuf;
    itemInfo.clength = clen;
    itemInfo.coff    = coff;
    itemInfo.rBuf    = rBuf;
    itemInfo.rlength = rlen;
    err              = NVOCMP_findItem(&NVOCMP_nvHandle, NVOCMP_nvHandle.actPage, NVOCMP_nvHandle.actOffset, &iHdr,
                                       NVOCMP_FINDITMID | NVOCMP_FINDCONTENT, &itemInfo);

    if (!err)
    {
        *pSubId = iHdr.subid;
    }

    NVOCMP_UNLOCK(err);
}

/******************************************************************************
 * @fn      NVOCMP_readItemApi
 *
 * @brief   API function to read data from an NV item
 *
 * @param   id   - NV item type identifier
 * @param   ofs - offset into NV data
 * @param   len - length of NV data to return (0 is illegal)
 * @param   pBuf - pointer to caller's read data buffer  (NULL is illegal)
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_readItemApi(NVINTF_itemID_t id, uint16_t ofs, uint16_t len, void * pBuf)
{
    uint8_t err;
    NVOCMP_itemHdr_t iHdr;

    // Parameter Sanity Check
    if (pBuf == NULL || len == 0)
    {
        return (NVINTF_BADPARAM);
    }

    err = NVOCMP_checkItem(&id, len, &iHdr, NVOCMP_FINDSTRICT);
    if (err)
    {
        return (err);
    }

    // Prevent RTOS thread contention
    NVOCMP_LOCK();

    err = NVOCMP_findItem(&NVOCMP_nvHandle, NVOCMP_nvHandle.actPage, NVOCMP_nvHandle.actOffset, &iHdr, NVOCMP_FINDSTRICT, NULL);

    // Read Item
    if (!err)
    {
        err = NVOCMP_readItem(&iHdr, ofs, len, pBuf, false);
    }

    NVOCMP_UNLOCK(err);
}

/******************************************************************************
 * @fn      NVOCMP_writeItemApi
 *
 * @brief   API function to write data to item, creates item if needed.
 *          Note that when writing to an existing item, data is not
 *          checked for redundancy. Data passed to this function will be
 *          written to NV. NOTE: It is not recommended to write items with
 *          SYSID 0 as this is reserved for the driver. NVOCMP will not
 *          delete items with this SYSID.
 *
 * @param   id   - NV item type identifier
 * @param   len - data buffer length to write into NV block  (0 is illegal)
 * @param   pBuf - pointer to caller's data buffer to write  (NULL is illegal)
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_writeItemApi(NVINTF_itemID_t id, uint16_t len, void * pBuf)
{
    uint8_t err;
    NVOCMP_itemHdr_t iHdr;

    // Parameter Sanity Check
    if (pBuf == NULL || len == 0)
    {
        return (NVINTF_BADPARAM);
    }

    err = NVOCMP_checkItem(&id, len, &iHdr, NVOCMP_FINDSTRICT);
    if (err)
    {
        return (err);
    }

    // Check voltage if possible
    NVOCMP_FLASHACCESS(err)
    if (err)
    {
        return (err);
    }

    // Prevent RTOS thread contention
    NVOCMP_LOCK();

    // Create a new item
    err = NVOCMP_addItem(&NVOCMP_nvHandle, &iHdr, pBuf, NVOCMP_WRITE);
    if ((err == NVINTF_SUCCESS) && (iHdr.hofs > 0))
    {
        // Mark old item as inactive
        NVOCMP_setItemInactive(&NVOCMP_nvHandle, iHdr.hpage, iHdr.hofs);

        err = NVOCMP_failW;
    }

#ifdef NV_LINUX
    if (err == NVINTF_SUCCESS)
    {
        NV_LINUX_save();
    }
#endif

    NVOCMP_UNLOCK(err);
}

//*****************************************************************************
// Extended API Functions
//*****************************************************************************

/**
 * @fn      NVOCMP_lockNvApi
 *
 * @brief   Global function to lock the NV priority gate mutex
 *
 * @return  Key value needed to unlock the gate
 */
static int32_t NVOCMP_lockNvApi(void)
{
#ifdef NVOCMP_POSIX_MUTEX
    return (pthread_mutex_lock(&NVOCMP_gPosixMutex));
#elif defined(NVOCMP_POSIX_SEM)
    return (sem_wait(&NVOCMP_gPosixSem));
#else
    return (GateMutexPri_enter(NVOCMP_gMutexPri));
#endif
}

/**
 * @fn      NVOCMP_unlockNvApi
 *
 * @brief   Global function to unlock the NV priority gate mutex
 *
 * @return  none
 */
static void NVOCMP_unlockNvApi(int32_t key)
{
#ifdef NVOCMP_POSIX_MUTEX
    (void) key;
    pthread_mutex_unlock(&NVOCMP_gPosixMutex);
#elif defined(NVOCMP_POSIX_SEM)
    (void) key;
    sem_post(&NVOCMP_gPosixSem);
#else
    GateMutexPri_leave(NVOCMP_gMutexPri, key);
#endif
}

/******************************************************************************
 * @fn      NVOCMP_doNextApi
 *
 * @brief   API function which allows operations on batches of NV items. This
 * function provides a faster way of finding, reading, or deleting multiple
 * NV items. However, the user must first lock access to NV with lockNV() to
 * ensure consistent results. The user must take care to minimize the time NV
 * is locked if NV access is shared. User must also remember to unlock NV when
 * done with unlockNV().
 *
 * Usage Details:
 * doNext is controlled through the nvProxy item pointed to by prx
 * User will set flag bit NVINTF_DOSTART and then other flags based on the
 * desired operation. For example to find all items in system NVINTF_SYS_BLE,
 * the user would set flag bit NVINTF_DOFIND and set prx->sysid = NVINTF_SYS_BLE.
 * Then every call to doNextApi() returns with a status code and with the proxy
 * item populated with the found item if there was one. NVINTF_SUCCESS is
 * returned on a successful item operation, NVINTF_NOTFOUND is returned when a
 * matching item is not found, and other error codes can be returned.
 * Sample Code:
 *
 * // Use doNext to delete items of sysid
 * NVINTF_nvFuncts_t nvFps;
 * NVINTF_nvProxy_t nvProxy;
 * NVOCMP_loadApiPtrsExt(&nvFps);
 * nvFps.initNV(NULL);
 * nvProxy.sysid = sysid;
 * nvProxy.flag = NVINTF_DOSTART | NVINTF_DOSYSID | NVINTF_DODELETE;
 *
 * key = nvFps.lockNV();
 * while(!status)
 * {
 *     status |= nvFps.doNext(&nvProxy);
 * }
 * nvFps.unlockNV(key);
 *
 * Notes:
 * -User changes to the proxy struct will have no effect until a new search is
 * started by setting NVINTF_DOSTART
 * -On read operations, the user will supply a buffer and length into the proxy
 * -Items with system id NVINTF_SYSID_NVDRVR cannot be deleted with this API,
 * deleteItemApi must be used one an individual item basis
 *
 * @param   prx - pointer to nvProxy item which contains user inputs
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */

static uint8_t NVOCMP_doNextApi(NVINTF_nvProxy_t * prx)
{
    static enum { doFind, doRead, doDelete } op = doFind;
    NVOCMP_itemHdr_t hdr;
    static uint8_t search;
    static uint8_t sPage;
    static int16_t fOfs            = FLASH_PAGE_SIZE;
    static uint16_t bufLen         = 0;
    uint8_t status                 = NVINTF_SUCCESS;
    int16_t iOfs                   = NVOCMP_nvHandle.actOffset;
    static int16_t initialFindiofs = -1;
    static uint8_t initialsPage    = 0xFF;

    // Sanitize inputs
    if (NULL == prx)
    {
        return (NVINTF_BADPARAM);
    }
    else if (0 == prx->flag)
    {
        return (NVINTF_BADPARAM);
    }

    // Locks NV
    NVOCMP_LOCK();

    // New search if start flag set
    if (prx->flag & NVINTF_DOSTART)
    {
        // Remove start flag
        prx->flag &= ~NVINTF_DOSTART;
        // Start at latest item
        sPage = NVOCMP_nvHandle.actPage;
        fOfs  = NVOCMP_nvHandle.actOffset;

        initialFindiofs = -1;
        initialsPage    = 0xFF;

        // Read in buffer len
        bufLen = prx->len;
        // Decode flag
        if (prx->flag & NVINTF_DOSYSID)
        {
            search = NVOCMP_FINDSYSID;
        }
        else if (prx->flag & NVINTF_DOITMID)
        {
            search = NVOCMP_FINDITMID;
        }
        else if (prx->flag & NVINTF_DOANYID)
        {
            search = NVOCMP_FINDANY;
        }
        if (prx->flag & NVINTF_DOFIND)
        {
            op = doFind;
        }
        else if (prx->flag & NVINTF_DOREAD)
        {
            op = doRead;
        }
        else if (prx->flag & NVINTF_DODELETE)
        {
            op = doDelete;
        }
    }

    hdr.sysid  = prx->sysid;
    hdr.itemid = prx->itemid;
    hdr.subid  = prx->subid;
    // Look for item
    if (!NVOCMP_findItem(&NVOCMP_nvHandle, sPage, fOfs, &hdr, search, NULL))
    {
        iOfs = hdr.hofs;
        // store its attributes
        prx->sysid  = hdr.sysid;
        prx->itemid = hdr.itemid;
        prx->subid  = hdr.subid;
        prx->len    = hdr.len;

        if (prx->flag & NVINTF_DONOWRAP)
        {
            if ((initialFindiofs == hdr.hofs) && (initialsPage == hdr.hpage))
            {
                status = NVINTF_NOTFOUND;
            }

            if ((initialFindiofs == -1) && (initialsPage == 0xFF))
            {
                initialFindiofs = hdr.hofs;
                initialsPage    = hdr.hpage;
            }
        }

        // start from this item on next findItem()
        fOfs  = iOfs - hdr.len;
        sPage = hdr.hpage;

        if (status == NVINTF_SUCCESS)
        {
            // Do operation based on flag
            switch (op)
            {
            case doFind:
                // nothing, we already stored its info
                break;
            case doRead:
                // read item into user supplied buffer
                if (prx->buffer != NULL && hdr.len <= bufLen)
                {
                    status = NVOCMP_readItem(&hdr, 0, hdr.len, prx->buffer, false);
                }
                break;
            case doDelete:
                if (prx->sysid != NVINTF_SYSID_NVDRVR)
                {
                    NVOCMP_setItemInactive(&NVOCMP_nvHandle, hdr.hpage, iOfs);
                }
                break;
            default:
                NVOCMP_ALERT(false, "doNext flag is invalid.")
                status = NVINTF_BADPARAM;
            }
        }
    }
    else
    {
        // No more items match, done.
        status = NVINTF_NOTFOUND;
    }

    // Unlocks NV
    NVOCMP_UNLOCK(status);
}
/******************************************************************************
 * @fn      NVOCMP_expectCompApi
 *
 * @brief   API function to check if compaction will happen if a data with size = len is written
 *
 * @param   len - data buffer length to write into NV block
 *
 * @return  true or false
 */
static bool NVOCMP_expectCompApi(uint16_t len)
{
    uint8_t dstPg;
    uint16_t iLen;
    bool compact = false;

    if (len)
    {
        iLen  = NVOCMP_ITEMHDRLEN + len;
        dstPg = NVOCMP_getDstPage(&NVOCMP_nvHandle, iLen);

        if (dstPg == NVOCMP_NULLPAGE)
        {
            compact = true;
        }
    }
    return (compact);
}

//*****************************************************************************
// Local NV Driver Utility Functions
//*****************************************************************************

#ifdef NVOCMP_GPRAM
/******************************************************************************
 * @fn      NVOCMP_disableCache
 *
 * @brief   Local function to disable cache
 *
 * @param   vm - pointer to mode storage
 *
 * @return  none
 */
static void NVOCMP_disableCache(uint32_t * vm)
{
    // Save current cache mode
    *vm = VIMSModeGet(VIMS_BASE) & VIMS_STAT_MODE_M;
    // Disable the cache
    VIMSModeSet(VIMS_BASE, VIMS_MODE_DISABLED);
    // Wait until it is
    while (VIMSModeGet(VIMS_BASE) != VIMS_MODE_DISABLED)
        ;
}

/******************************************************************************
 * @fn      NVOCMP_restoreCache
 *
 * @brief   Local function to disable cache
 *
 * @param   vm - mode
 *
 * @return  none
 */
static void NVOCMP_restoreCache(uint32_t vm)
{
    // Restore cache to previous state
    VIMSModeSet(VIMS_BASE, vm);
}
#endif

#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
/******************************************************************************
 * @fn      NVOCMP_findPage
 *
 * @brief   Local function to find page with specified state
 *
 * @param   state - page state to find
 *
 * @return  page number or NVOCMP_NULLPAGE
 */
static uint8_t NVOCMP_findPage(NVOCMP_pageState_t state)
{
    uint8_t pg;
    NVOCMP_pageHdr_t pageHdr;

    for (pg = 0; pg < NVOCMP_NVSIZE; pg++)
    {
        // Get page header
        NVOCMP_read(pg, NVOCMP_PGHDROFS, (uint8_t *) &pageHdr, NVOCMP_PGHDRLEN);
        if (pageHdr.state == state)
        {
            return (pg);
        }
    }
    return (NVOCMP_NULLPAGE);
}
#endif

#if (NVOCMP_NVPAGES > NVOCMP_NVONEP)
#if !defined(NVOCMP_MIGRATE_DISABLED)
/******************************************************************************
 * @fn      NVOCMP_migratePage
 *
 * @brief   Local function to find page with specified state
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   page - page to convert
 *
 * @return  none
 */
static void NVOCMP_migratePage(NVOCMP_nvHandle_t * pNvHandle, uint8_t page)
{
    uint8_t dstPg;
    uint8_t tmp;
    uint16_t offset1;
    uint16_t offset2;
    NVOCMP_itemHdr_t iHdr;

    offset1 = pNvHandle->pageInfo[page].offset;
    offset2 = NVOCTP_PGDATAOFS;
    if (offset1 - NVOCTP_PGDATAOFS > FLASH_PAGE_SIZE - NVOCMP_PGDATAOFS)
    {
        offset2 = offset1;
        while (offset2 - NVOCTP_PGDATAOFS > FLASH_PAGE_SIZE - NVOCMP_PGDATAOFS)
        {
            NVOCMP_findItem(pNvHandle, page, offset2, &iHdr, NVOCMP_FINDANY, NULL);
            offset2 = iHdr.hofs - iHdr.len;
        }
    }
    if ((pNvHandle->nvSize <= NVOCMP_NVTWOP) && (offset2 != NVOCTP_PGDATAOFS))
    {
        offset1 = offset2;
        offset2 = NVOCTP_PGDATAOFS;
    }

    dstPg = page;
    // copy from offset2 to top
    if (offset2 - NVOCTP_PGDATAOFS > 0)
    {
        dstPg = NVOCMP_INCPAGE(dstPg);
        NVOCMP_copyItem(page, dstPg, NVOCTP_PGDATAOFS, NVOCMP_PGDATAOFS, offset2 - NVOCTP_PGDATAOFS);
        NVOCMP_changePageState(pNvHandle, dstPg, NVOCMP_PGFULL);
        pNvHandle->pageInfo[dstPg].allActive = NVOCMP_SOMEINACTIVE;
        pNvHandle->pageInfo[dstPg].offset    = NVOCMP_PGDATAOFS + offset2 - NVOCTP_PGDATAOFS;
        pNvHandle->actPage                   = dstPg;
        pNvHandle->actOffset                 = pNvHandle->pageInfo[dstPg].offset;
        tmp                                  = NVOCMP_readByte(dstPg, NVOCMP_PGHDRVER);
        tmp &= ~NVOCMP_ALLACTIVE;
        NVOCMP_writeByte(dstPg, NVOCMP_PGHDRVER, tmp);
    }
    // copy from offset 1 to offset2
    if (offset1 - offset2 > 0)
    {
        dstPg = NVOCMP_INCPAGE(dstPg);
        NVOCMP_copyItem(page, dstPg, offset2, NVOCMP_PGDATAOFS, offset1 - offset2);
        NVOCMP_changePageState(pNvHandle, dstPg, NVOCMP_PGFULL);
        pNvHandle->pageInfo[dstPg].allActive = NVOCMP_SOMEINACTIVE;
        pNvHandle->pageInfo[dstPg].offset    = NVOCMP_PGDATAOFS + offset1 - offset2;
        pNvHandle->actPage                   = dstPg;
        pNvHandle->actOffset                 = pNvHandle->pageInfo[dstPg].offset;
        tmp                                  = NVOCMP_readByte(dstPg, NVOCMP_PGHDRVER);
        tmp &= ~NVOCMP_ALLACTIVE;
        NVOCMP_writeByte(dstPg, NVOCMP_PGHDRVER, tmp);
    }

    NVOCMP_failW |= NVOCMP_erase(pNvHandle, page);
    pNvHandle->tailPage = page;
    pNvHandle->headPage = NVOCMP_INCPAGE(page);
    NVOCMP_changePageState(pNvHandle, page, NVOCMP_PGXDST);
}
#endif
#endif

#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
/******************************************************************************
 * @fn      NVOCMP_scanPage
 *
 * @brief   Local function to scan page to get page information
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   pg - page to scan
 * @param   pPageInfo - page info pointer
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_scanPage(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, NVOCMP_pageInfo_t * pPageInfo)
{
    uint32_t pageHdr;
    NVOCMP_compactHdr_t thisHdr;
    NVOCMP_compactHdr_t startHdr;
    NVOCMP_compactHdr_t endHdr;
    NVOCMP_pageHdr_t * pHdr = (NVOCMP_pageHdr_t *) &pageHdr;

    // Get page header
    NVOCMP_read(pg, NVOCMP_PGHDROFS, (uint8_t *) pHdr, NVOCMP_PGHDRLEN);
#if ((NVOCMP_NVPAGES != NVOCMP_NVONEP) && !defined(NVOCMP_MIGRATE_DISABLED))
    uint8_t version = (pHdr->version << 2) | (pHdr->allActive);
    if ((pHdr->signature == NVOCTP_SIGNATURE) && (version == NVOCTP_VERSION))
    {
        pPageInfo->state     = pHdr->state;
        pPageInfo->cycle     = pHdr->cycle;
        pPageInfo->allActive = NVOCMP_SOMEINACTIVE;
        pPageInfo->mode      = NVOCMP_PGNORMAL;
        pPageInfo->offset    = NVOCMP_findOffset(pg, FLASH_PAGE_SIZE);
        return (NVINTF_SUCCESS);
    }
#endif

    NVOCMP_getCompactHdr(pg, THISPAGEHDR, &thisHdr);
    NVOCMP_getCompactHdr(pg, XSRCSTARTHDR, &startHdr);
    NVOCMP_getCompactHdr(pg, XSRCENDHDR, &endHdr);
    uint8_t corruptFlag = ((pHdr->state != NVOCMP_PGNACT) && (pHdr->state != NVOCMP_PGXDST) && (pHdr->state != NVOCMP_PGRDY) &&
                           (pHdr->state != NVOCMP_PGACT) && (pHdr->state != NVOCMP_PGFULL) && (pHdr->state != NVOCMP_PGXSRC));

    if (corruptFlag || (pHdr->version != NVOCMP_VERSION) || (pHdr->signature != NVOCMP_SIGNATURE))
    {
        // NV page and NV driver versions are different
        NVOCMP_ALERT(false, "Corrupted or Version/Signature mismatch.")
        NVOCMP_EXCEPTION(pg, NVINTF_BADVERSION);
        NVOCMP_failW = NVOCMP_erase(pNvHandle, pg);
        if (NVOCMP_failW == NVINTF_SUCCESS)
        {
            // Get page header
            NVOCMP_read(pg, NVOCMP_PGHDROFS, (uint8_t *) pHdr, NVOCMP_PGHDRLEN);
            NVOCMP_getCompactHdr(pg, THISPAGEHDR, &thisHdr);
            NVOCMP_getCompactHdr(pg, XSRCSTARTHDR, &startHdr);
            NVOCMP_getCompactHdr(pg, XSRCENDHDR, &endHdr);
        }
        else if (NVOCMP_failW == NVINTF_LOWPOWER)
        {
            return (NVINTF_LOWPOWER);
        }
        else
        {
            return (NVINTF_FAILURE);
        }
    }

    pPageInfo->state     = pHdr->state;
    pPageInfo->cycle     = pHdr->cycle;
    pPageInfo->allActive = pHdr->allActive;
    pPageInfo->mode      = thisHdr.page;
    if ((pPageInfo->state == NVOCMP_PGNACT) || (pPageInfo->state == NVOCMP_PGXDST))
    {
        pPageInfo->offset = NVOCMP_PGDATAOFS;
    }
    else if (thisHdr.pageOffset != NVOCMP_NULLOFFSET)
    {
        pPageInfo->offset = thisHdr.pageOffset;
    }
    else
    {
        pPageInfo->offset = NVOCMP_findOffset(pg, FLASH_PAGE_SIZE);
    }
    pPageInfo->sPage   = startHdr.page;
    pPageInfo->sOffset = startHdr.pageOffset;
    pPageInfo->ePage   = endHdr.page;
    pPageInfo->eOffset = endHdr.pageOffset;
    return (NVINTF_SUCCESS);
}
#else
/******************************************************************************
 * @fn      NVOCMP_scanPage
 *
 * @brief   Local function to scan page to get page information
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   pg - page to scan
 * @param   pPageInfo - page info pointer
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_scanPage(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, NVOCMP_pageInfo_t * pPageInfo)
{
    uint32_t pageHdr;
    NVOCMP_pageHdr_t * pHdr = (NVOCMP_pageHdr_t *) &pageHdr;

    // Get page header
    NVOCMP_read(pg, NVOCMP_PGHDROFS, (uint8_t *) pHdr, NVOCMP_PGHDRLEN);
    pPageInfo->state = pHdr->state;
    pPageInfo->cycle = pHdr->cycle;
#if ((NVOCMP_NVPAGES > NVOCMP_NVONEP) && !defined(NVOCMP_MIGRATE_DISABLED))
    uint8_t version  = (pHdr->version << 2) | (pHdr->allActive);
    if ((pHdr->signature == NVOCTP_SIGNATURE) && (version == NVOCTP_VERSION))
    {
        pPageInfo->allActive = NVOCMP_SOMEINACTIVE;
        pPageInfo->mode      = NVOCMP_PGNORMAL;
        pPageInfo->offset    = NVOCMP_findOffset(pg, FLASH_PAGE_SIZE);
    }
#endif
    if ((pHdr->version == NVOCMP_VERSION) || (pHdr->signature == NVOCMP_SIGNATURE))
    {
        pPageInfo->allActive = pHdr->allActive;
        if ((pPageInfo->state == NVOCMP_PGNACT) || (pPageInfo->state == NVOCMP_PGXDST) || (pPageInfo->state == NVOCMP_PGRDY))
        {
            pPageInfo->offset = NVOCMP_PGDATAOFS;
        }
        else
        {
            pPageInfo->offset = NVOCMP_findOffset(pg, FLASH_PAGE_SIZE);
        }
    }
    return (NVINTF_SUCCESS);
}
#endif

#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
/******************************************************************************
 * @fn      NVOCMP_findDstPage
 *
 * @brief   Local function to find dst page after power loss
 *
 * @param   pNvHandle - pointer to NV handle
 *
 * @return  page
 */
static uint8_t NVOCMP_findDstPage(NVOCMP_nvHandle_t * pNvHandle)
{
    uint8_t pg;
    for (pg = 0; pg < NVOCMP_NVSIZE; pg++)
    {
        if (pNvHandle->pageInfo[pg].mode == NVOCMP_PGCDST)
        {
            return (pg);
        }
    }
    return (NVOCMP_NVSIZE);
}
#endif

#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
/******************************************************************************
 * @fn      NVOCMP_initNv
 *
 * @brief   Local function to init whole NV area
 *
 * @param   pNvHandle - pointer to NV handle
 *
 * @return  none
 */
static void NVOCMP_initNv(NVOCMP_nvHandle_t * pNvHandle)
{
    uint8_t status;
    uint8_t pg;
    uint8_t tmpPg;
    NVOCMP_initAction_t action = NVOCMP_NORMAL_INIT;
    NVOCMP_pageInfo_t * pPageInfo;
    uint8_t noPgNact = 0;
    uint8_t noPgXdst = 0;
    uint8_t noPgRdy  = 0;
    uint8_t noPgAct  = 0;
    uint8_t noPgFull = 0;
    uint8_t noPgXsrc = 0;
    uint8_t noPgNdef = 0;
    uint16_t pgXdst  = NVOCMP_NULLPAGE;
    uint16_t pgRdy   = NVOCMP_NULLPAGE;
    uint16_t pgAct   = NVOCMP_NULLPAGE;
    uint16_t pgNact  = NVOCMP_NULLPAGE;
#if !defined(NVOCMP_MIGRATE_DISABLED)
    uint8_t noPgLeg    = 0;
    uint16_t pgLegAct  = NVOCMP_NULLPAGE;
    uint16_t pgLegXsrc = NVOCMP_NULLPAGE;
#endif
    uint16_t cleanPages = 0;

    // Scan Pages
    for (pg = 0; pg < NVOCMP_NVSIZE; pg++)
    {
        pPageInfo = &pNvHandle->pageInfo[pg];
        status    = NVOCMP_scanPage(pNvHandle, pg, pPageInfo);
        if (status != NVINTF_SUCCESS)
        {
            return;
        }
        if (pPageInfo->state == NVOCMP_PGNACT)
        {
            noPgNact++;
            pgNact = pg;
        }
        else if (pPageInfo->state == NVOCMP_PGXDST)
        {
            pgXdst = pg;
            noPgXdst++;
        }
        else if (pPageInfo->state == NVOCMP_PGRDY)
        {
            pgRdy = pg;
            noPgRdy++;
        }
        else if (pPageInfo->state == NVOCMP_PGACT)
        {
            pgAct = pg;
            noPgAct++;
        }
        else if (pPageInfo->state == NVOCMP_PGFULL)
        {
            noPgFull++;
        }
        else if (pPageInfo->state == NVOCMP_PGXSRC)
        {
            noPgXsrc++;
        }
#if ((NVOCMP_NVPAGES != NVOCMP_NVONEP) && !defined(NVOCMP_MIGRATE_DISABLED))
        else if (pPageInfo->state == NVOCTP_PGACTIVE)
        {
            pgLegAct = pg;
            noPgLeg++;
        }
        else if (pPageInfo->state == NVOCTP_PGXFER)
        {
            pgLegXsrc = pg;
            noPgLeg++;
        }
#endif
        else
        {
            noPgNdef++;
        }
    }

    // Decide Action based on Page Informations
#if ((NVOCMP_NVPAGES != NVOCMP_NVONEP) && !defined(NVOCMP_MIGRATE_DISABLED))
    if (noPgLeg > 0)
    {
        action = NVOCMP_NORMAL_MIGRATE;
    }
    else
#endif
        if ((noPgNdef > 0) || (noPgXdst > 1) || (noPgXsrc > 1) || (noPgRdy > 1))
    {
        // This should not happen
        NVOCMP_ASSERT(false, "Something wrong serious");
        action = NVOCMP_FORCE_CLEAN;
    }
    else
    {
        if (noPgNact == NVOCMP_NVSIZE)
        {
            action = NVOCMP_NORMAL_INIT;
        }
        else if (noPgXsrc)
        {
            // Power lost during compaction in progress
            if (noPgXdst)
            {
                action = NVOCMP_RECOVER_COMPACT;
            }
            // Corrupted due to power lost while writing onto XDST page and erased the XDST page
            else if (noPgNact)
            {
                pgXdst = pgNact;
                action = NVOCMP_RECOVER_COMPACT;
            }
            // Power lost after compaction done, but before erasing PGXSRC page
            else
            {
                if (NVOCMP_findDstPage(pNvHandle) < NVOCMP_NVSIZE)
                {
                    action = NVOCMP_RECOVER_ERASE;
                }
                else
                {
                    action = NVOCMP_ERROR_UNKNOWN;
                }
            }
        }
        else if (noPgXdst)
        {
            action = NVOCMP_NORMAL_RESUME;
        }
        else if (noPgAct || noPgFull)
        {
            if (NVOCMP_findDstPage(pNvHandle) < NVOCMP_NVSIZE)
            {
                action = NVOCMP_RECOVER_ERASE;
            }
            else if (noPgNact)
            {
                pgXdst = pgNact;
                NVOCMP_changePageState(pNvHandle, pNvHandle->tailPage, NVOCMP_PGXDST);
                action = NVOCMP_NORMAL_RESUME;
            }
            else
            {
                action = NVOCMP_ERROR_UNKNOWN;
            }
        }
        else
        {
            // This case should be considered more
            NVOCMP_ASSERT(false, "Something wrong serious");
            action = NVOCMP_FORCE_CLEAN;
        }
    }

    gAction = action;

    switch (action)
    {
    case NVOCMP_FORCE_CLEAN:
        // Erase All pages before start
        for (pg = 0; pg < NVOCMP_NVSIZE; pg++)
        {
            NVOCMP_failW |= NVOCMP_erase(pNvHandle, pg);
        }
        // init should be followed by force clean
    case NVOCMP_NORMAL_INIT:
        // initial state, set head page, act page and tail page
        pNvHandle->headPage  = 0;
        pNvHandle->tailPage  = NVOCMP_NVSIZE - 1;
        pNvHandle->actPage   = 0;
        pNvHandle->actOffset = pNvHandle->pageInfo[pNvHandle->actPage].offset;
        NVOCMP_changePageState(pNvHandle, pNvHandle->headPage, NVOCMP_PGRDY);
        NVOCMP_changePageState(pNvHandle, pNvHandle->tailPage, NVOCMP_PGXDST);
        break;
    case NVOCMP_NORMAL_RESUME:
        // resume state, set head page, act page and tail page
        pNvHandle->tailPage = pgXdst;
        pNvHandle->headPage = NVOCMP_INCPAGE(pgXdst);
        if (pgAct != NVOCMP_NULLPAGE)
        {
            pNvHandle->actPage   = pgAct;
            pNvHandle->actOffset = pNvHandle->pageInfo[pgAct].offset;

            NVOCMP_itemHdr_t iHdr;
            int8_t status;
            if (pNvHandle->actOffset > NVOCMP_PGDATAOFS + NVOCMP_ITEMHDRLEN)
            {
                NVOCMP_readHeader(pNvHandle->actPage, pNvHandle->actOffset - NVOCMP_ITEMHDRLEN, &iHdr, false);
                if (iHdr.stats & NVOCMP_FOLLOWBIT)
                {
                    status = NVOCMP_findItem(pNvHandle, pNvHandle->actPage, pNvHandle->actOffset - NVOCMP_ITEMHDRLEN - iHdr.len,
                                             &iHdr, NVOCMP_FINDSTRICT, NULL);
                    if ((status == NVINTF_SUCCESS) && (iHdr.hofs > 0))
                    {
                        NVOCMP_setItemInactive(pNvHandle, iHdr.hpage, iHdr.hofs);
                    }
                }
                else
                {
                    NVOCMP_compactPage(pNvHandle, 0);
                }
            }
        }
        else if (pgRdy != NVOCMP_NULLPAGE)
        {
            pNvHandle->actPage   = pgRdy;
            pNvHandle->actOffset = pNvHandle->pageInfo[pgRdy].offset;
        }
        else
        {
            pNvHandle->actPage   = pNvHandle->headPage;
            pNvHandle->actOffset = pNvHandle->pageInfo[pNvHandle->actPage].offset;
        }
#ifdef NVOCMP_COMPACT_WHEN_RESUME
        NVOCMP_compactPage(pNvHandle, 0);
#endif
        break;
    case NVOCMP_RECOVER_COMPACT:
        pNvHandle->tailPage = pgXdst;
        pNvHandle->headPage = NVOCMP_INCPAGE(pgXdst);
        NVOCMP_failW        = NVOCMP_erase(pNvHandle, pgXdst);
        NVOCMP_changePageState(pNvHandle, pgXdst, NVOCMP_PGXDST);
        pNvHandle->forceCompact = 1;
        NVOCMP_compactPage(pNvHandle, 0);
        break;
    case NVOCMP_RECOVER_ERASE:
        pg                               = NVOCMP_findDstPage(pNvHandle);
        pNvHandle->compactInfo.xDstPage  = pg;
        pNvHandle->compactInfo.xSrcSPage = pNvHandle->pageInfo[pg].sPage;
        pNvHandle->compactInfo.xSrcEPage = pNvHandle->pageInfo[pg].ePage;
        cleanPages                       = NVOCMP_cleanPage(pNvHandle);
        pNvHandle->tailPage              = NVOCMP_ADDPAGE(pg, cleanPages);
        pNvHandle->headPage              = NVOCMP_INCPAGE(pNvHandle->tailPage);

        tmpPg = NVOCMP_findPage(NVOCMP_PGACT);
        if (tmpPg == NVOCMP_NULLPAGE)
        {
            tmpPg = NVOCMP_findPage(NVOCMP_PGRDY);
            if (tmpPg == NVOCMP_NULLPAGE)
            {
                tmpPg = NVOCMP_findPage(NVOCMP_PGNACT);
                if (tmpPg == NVOCMP_NULLPAGE)
                {
                    tmpPg = NVOCMP_findPage(NVOCMP_PGFULL);
                    if (tmpPg == NVOCMP_NULLPAGE)
                    {
                        tmpPg = pNvHandle->headPage;
                    }
                }
            }
        }

        pNvHandle->actPage   = tmpPg;
        pNvHandle->actOffset = pNvHandle->pageInfo[pNvHandle->actPage].offset;
        NVOCMP_changePageState(pNvHandle, pNvHandle->tailPage, NVOCMP_PGXDST);
        break;
#if !defined(NVOCMP_MIGRATE_DISABLED)
    case NVOCMP_NORMAL_MIGRATE:
        if (pgLegAct != NVOCMP_NULLPAGE)
        {
            pNvHandle->actPage = pgLegAct;
        }
        else
        {
            pNvHandle->actPage = pgLegXsrc;
        }
        pNvHandle->headPage  = pNvHandle->actPage;
        pNvHandle->tailPage  = NVOCMP_DECPAGE(pNvHandle->headPage);
        pNvHandle->actOffset = pNvHandle->pageInfo[pNvHandle->actPage].offset;
        NVOCMP_migratePage(pNvHandle, pNvHandle->actPage);
        NVOCMP_compactPage(pNvHandle, 0);
        break;
#endif
    case NVOCMP_ERROR_UNKNOWN:
        /* When this error happens, NV area should be erased to restart.
         * This while loop is for only debug purpose */
        NVOCMP_ASSERT1(0);
    default:
        break;
    }
}
#endif
#if (NVOCMP_NVPAGES == NVOCMP_NVTWOP)
/******************************************************************************
 * @fn      NVOCMP_initNv
 *
 * @brief   Local function to init whole NV area
 *
 * @param   pNvHandle - pointer to NV handle
 *
 * @return  none
 */
static void NVOCMP_initNv(NVOCMP_nvHandle_t * pNvHandle)
{
    uint8_t status, prevactPage;
    NVOCMP_itemHdr_t iHdr;
    uint8_t pg;
    NVOCMP_initAction_t action;
    NVOCMP_pageInfo_t * pPageInfo;
    uint16_t pgAct;
#if !defined(NVOCMP_MIGRATE_DISABLED)
    uint8_t noPgLeg = 0;
#endif
    bool compact = false, compaction_occurred = false;

    // Scan Pages
    pNvHandle->xsrcPage = NVOCMP_NULLPAGE;
    pNvHandle->tailPage = NVOCMP_NULLPAGE;

    for (pg = 0; pg < NVOCMP_NVSIZE; pg++)
    {
        pPageInfo = &pNvHandle->pageInfo[pg];
        NVOCMP_scanPage(pNvHandle, pg, pPageInfo);
#if ((NVOCMP_NVPAGES != NVOCMP_NVONEP) && !defined(NVOCMP_MIGRATE_DISABLED))
        if (pPageInfo->state == NVOCTP_PGACTIVE)
        {
            pNvHandle->actPage = pg;
            noPgLeg++;
        }
        else if (pPageInfo->state == NVOCTP_PGXFER)
        {
            pNvHandle->xsrcPage = pg;
            noPgLeg++;
        }
        else
#endif
            if ((pPageInfo->state == NVOCMP_PGACT) || (pPageInfo->state == NVOCMP_PGFULL))
        {
            pNvHandle->actPage = pg;
        }
        else if (pPageInfo->state == NVOCMP_PGXSRC)
        {
            pNvHandle->xsrcPage = pg;
        }
        else if (pPageInfo->state == NVOCMP_PGXDST)
        {
            pNvHandle->tailPage = pg;
        }
    }

    action = NVOCMP_NORMAL_INIT;
    // Decide Action based on Page Informations
#if ((NVOCMP_NVPAGES != NVOCMP_NVONEP) && !defined(NVOCMP_MIGRATE_DISABLED))
    if (noPgLeg > 0)
    {
        action = NVOCMP_NORMAL_MIGRATE;
    }
    else
    {
#endif
        if (pNvHandle->actPage != NVOCMP_NULLPAGE)
        {
            action = NVOCMP_NORMAL_RESUME;
        }
        else if (pNvHandle->xsrcPage != NVOCMP_NULLPAGE)
        {
            pNvHandle->actPage = pNvHandle->xsrcPage;
            action             = NVOCMP_RECOVER_COMPACT;
        }
        else if (pNvHandle->tailPage != NVOCMP_NULLPAGE)
        {
            pgAct = NVOCMP_INCPAGE(pNvHandle->tailPage);
            if (pNvHandle->pageInfo[pgAct].offset)
            {
                pNvHandle->actPage = pgAct;
                action             = NVOCMP_RECOVER_COMPACT;
            }
            else
            {
                pNvHandle->tailPage = NVOCMP_NULLPAGE;
            }
        }

        if (pNvHandle->actPage == NVOCMP_NULLPAGE)
        {
            pNvHandle->actPage = 0;
            NVOCMP_failW |= NVOCMP_erase(pNvHandle, 0);
            NVOCMP_changePageState(pNvHandle, 0, NVOCMP_PGRDY);
        }

        if (pNvHandle->tailPage == NVOCMP_NULLPAGE)
        {
            pNvHandle->tailPage = NVOCMP_INCPAGE(pNvHandle->actPage);
            ;
            NVOCMP_failW |= NVOCMP_erase(pNvHandle, pNvHandle->tailPage);
            NVOCMP_changePageState(pNvHandle, pNvHandle->tailPage, NVOCMP_PGXDST);
        }

        pNvHandle->headPage  = pNvHandle->actPage;
        pNvHandle->actOffset = pNvHandle->pageInfo[pNvHandle->actPage].offset;
#if ((NVOCMP_NVPAGES != NVOCMP_NVONEP) && !defined(NVOCMP_MIGRATE_DISABLED))
    }
#endif

    gAction = action;

    switch (action)
    {
    case NVOCMP_NORMAL_RESUME:
        // resume state, set head page, act page and tail page
        do
        {
            compaction_occurred = false;
            if (pNvHandle->actOffset > NVOCMP_PGDATAOFS + NVOCMP_ITEMHDRLEN)
            {
                NVOCMP_readHeader(pNvHandle->actPage, pNvHandle->actOffset - NVOCMP_ITEMHDRLEN, &iHdr, false);
                if (iHdr.stats & NVOCMP_FOLLOWBIT)
                {
                    /* Cache current active page value before search starts */
                    prevactPage = pNvHandle->actPage;
                    status = NVOCMP_findItem(pNvHandle, pNvHandle->actPage, pNvHandle->actOffset - NVOCMP_ITEMHDRLEN - iHdr.len,
                                             &iHdr, NVOCMP_FINDSTRICT, NULL);

                    /* If the current active page is different than previous, then
                     * compaction occurred and the search for duplicate must occur
                     * again.
                     * If item is found and compaction did not occur, it is a true
                     * duplicate, so the item can be deleted. */
                    compaction_occurred = (prevactPage != pNvHandle->actPage);
                    if ((status == NVINTF_SUCCESS) && (iHdr.hofs > 0) && !compaction_occurred)
                    {
                        NVOCMP_setItemInactive(pNvHandle, iHdr.hpage, iHdr.hofs);
                    }
                }
                else
                {
                    pNvHandle->forceCompact = 1;
                    compact                 = true;
                }
            }
        } while (compaction_occurred); /* Repeat until compaction does not occur while searching */
#ifdef NVOCMP_COMPACT_WHEN_RESUME
        compact = true;
#endif
        break;
#if !defined(NVOCMP_MIGRATE_DISABLED)
    case NVOCMP_NORMAL_MIGRATE:
        if (pNvHandle->actPage == NVOCMP_NULLPAGE)
        {
            pNvHandle->actPage = pNvHandle->xsrcPage;
        }
        pNvHandle->headPage  = pNvHandle->actPage;
        pNvHandle->tailPage  = NVOCMP_DECPAGE(pNvHandle->headPage);
        pNvHandle->actOffset = pNvHandle->pageInfo[pNvHandle->actPage].offset;
        NVOCMP_failW         = NVOCMP_erase(pNvHandle, pNvHandle->tailPage);
        NVOCMP_migratePage(pNvHandle, pNvHandle->actPage);
        compact = true;
        break;
#endif
    case NVOCMP_RECOVER_COMPACT:
        pNvHandle->forceCompact = 1;
        compact                 = true;
        break;
    default:
        break;
    }

    if (compact)
    {
        NVOCMP_compactPage(pNvHandle, 0);
    }
}
#endif

#if (NVOCMP_NVPAGES == NVOCMP_NVONEP)
/******************************************************************************
 * @fn      NVOCMP_initNv
 *
 * @brief   Local function to init whole NV area
 *
 * @param   pNvHandle - pointer to NV handle
 *
 * @return  none
 */
static void NVOCMP_initNv(NVOCMP_nvHandle_t * pNvHandle)
{
    uint8_t status;
    NVOCMP_itemHdr_t iHdr;
    NVOCMP_initAction_t action;
    NVOCMP_pageInfo_t * pPageInfo;
    bool compact = false;

    // Scan Pages
    pNvHandle->xsrcPage = NVOCMP_NULLPAGE;
    pNvHandle->tailPage = NVOCMP_NULLPAGE;

    pPageInfo = &pNvHandle->pageInfo[0];
    NVOCMP_scanPage(pNvHandle, 0, pPageInfo);
    if ((pPageInfo->state == NVOCMP_PGACT) || (pPageInfo->state == NVOCMP_PGFULL))
    {
        pNvHandle->actPage = 0;
    }
    else if (pPageInfo->state == NVOCMP_PGXSRC)
    {
        pNvHandle->xsrcPage = 0;
    }

    action = NVOCMP_NORMAL_INIT;
    // Decide Action based on Page Informations
    if (pNvHandle->actPage != NVOCMP_NULLPAGE)
    {
        action = NVOCMP_NORMAL_RESUME;
    }
    else if (pNvHandle->xsrcPage != NVOCMP_NULLPAGE)
    {
        pNvHandle->actPage = 0;
        action             = NVOCMP_RECOVER_COMPACT;
    }

    if (pNvHandle->actPage == NVOCMP_NULLPAGE)
    {
        pNvHandle->actPage = 0;
        NVOCMP_failW |= NVOCMP_erase(pNvHandle, 0);
        NVOCMP_changePageState(pNvHandle, 0, NVOCMP_PGRDY);
    }

    pNvHandle->tailPage  = 0;
    pNvHandle->headPage  = 0;
    pNvHandle->actOffset = pNvHandle->pageInfo[0].offset;

    gAction = action;

    switch (action)
    {
    case NVOCMP_NORMAL_RESUME:
        // resume state, set head page, act page and tail page
        if (pNvHandle->actOffset > NVOCMP_PGDATAOFS + NVOCMP_ITEMHDRLEN)
        {
            NVOCMP_readHeader(pNvHandle->actPage, pNvHandle->actOffset - NVOCMP_ITEMHDRLEN, &iHdr, false);
            if (iHdr.stats & NVOCMP_FOLLOWBIT)
            {
                status = NVOCMP_findItem(pNvHandle, pNvHandle->actPage, pNvHandle->actOffset - NVOCMP_ITEMHDRLEN - iHdr.len, &iHdr,
                                         NVOCMP_FINDSTRICT, NULL);
                if ((status == NVINTF_SUCCESS) && (iHdr.hofs > 0))
                {
                    NVOCMP_setItemInactive(pNvHandle, iHdr.hpage, iHdr.hofs);
                }
            }
            else
            {
                pNvHandle->forceCompact = 1;
                compact                 = true;
            }
        }
#ifdef NVOCMP_COMPACT_WHEN_RESUME
        compact = true;
#endif
        break;
    case NVOCMP_RECOVER_COMPACT:
        pNvHandle->forceCompact = 1;
        compact                 = true;
        break;
    default:
        break;
    }

    if (compact)
    {
        NVOCMP_compactPage(pNvHandle, 0);
    }
}
#endif

/******************************************************************************
 * @fn      NVOCMP_checkItem
 *
 * @brief   Local function to check parameters and locate existing item
 *
 * @param   id   - NV item type identifier
 * @param   len - NV item data length
 * @param   pHdr - pointer to header buffer
 * @param   flag - flag for item search
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_checkItem(NVINTF_itemID_t * id, uint16_t len, NVOCMP_itemHdr_t * pHdr, uint8_t flag)
{
    if (len > NVOCMP_MAXLEN)
    {
        // Item data is too long
        NVOCMP_ALERT(false, "Item data too large.")
        return (NVINTF_BADLENGTH);
    }
    if (id->systemID > NVOCMP_MAXSYSID)
    {
        // Too large for compressed header
        NVOCMP_ALERT(false, "Item sysid too large.")
        return (NVINTF_BADSYSID);
    }
    if (id->itemID > NVOCMP_MAXITEMID)
    {
        // Too large for compressed header
        NVOCMP_ALERT(false, "Item itemid too large.")
        return (NVINTF_BADITEMID);
    }
    if (id->subID > NVOCMP_MAXSUBID)
    {
        // Too large for compressed header
        NVOCMP_ALERT(false, "Item subid too large.")
        return (NVINTF_BADSUBID);
    }

    if (NVOCMP_failF == NVINTF_NOTREADY)
    {
        // NV driver has not been initialized
        NVOCMP_ASSERT(false, "Driver uninitialized.")
        return (NVINTF_NOTREADY);
    }

    pHdr->len    = len;
    pHdr->hofs   = 0;
    pHdr->cmpid  = NVOCMP_CMPRID(id->systemID, id->itemID, id->subID);
    pHdr->subid  = id->subID;
    pHdr->itemid = id->itemID;
    pHdr->sysid  = id->systemID;
    pHdr->sig    = NVOCMP_SIGNATURE;

    return (NVINTF_SUCCESS);
}

/******************************************************************************
 * @fn      NVOCMP_getDstPage
 *
 * @brief   Local function to find the page where an item will be written on
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   len - item size to write
 *
 * @return  page number or NVOCMP_NULLPAGE
 */
static uint8_t NVOCMP_getDstPage(NVOCMP_nvHandle_t * pNvHandle, uint16_t len)
{
    uint8_t dstPg = NVOCMP_NULLPAGE;
    uint8_t pg    = pNvHandle->actPage;
    NVOCMP_pageHdr_t pageHdr;
#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
    uint16_t nvSearched = 0;
    for (pg = pNvHandle->actPage; nvSearched < NVOCMP_NVSIZE; pg = NVOCMP_INCPAGE(pg))
    {
        nvSearched++;
        if (pg == pNvHandle->tailPage)
        {
            continue;
        }
#endif
        NVOCMP_read(pg, NVOCMP_PGHDROFS, (uint8_t *) &pageHdr, NVOCMP_PGHDRLEN);
        if (pageHdr.state == NVOCMP_PGFULL)
        {
#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
            continue;
#else
        return (dstPg);
#endif
        }
        if ((pageHdr.state == NVOCMP_PGNACT) || (pageHdr.state == NVOCMP_PGRDY))
        {
            NVOCMP_changePageState(pNvHandle, pg, NVOCMP_PGACT);
        }

        pNvHandle->actPage   = pg;
        pNvHandle->actOffset = pNvHandle->pageInfo[pg].offset;
        if ((pNvHandle->pageInfo[pg].offset + len) <= FLASH_PAGE_SIZE)
        {
            dstPg = pNvHandle->actPage;
#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
            break;
#endif
        }
        else
        {
            NVOCMP_changePageState(pNvHandle, pg, NVOCMP_PGFULL);
        }
#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
    }
#endif
    return (dstPg);
}

/******************************************************************************
 * @fn      NVOCMP_addItem
 *
 * @brief   Local function to check for adequate space and create a new item
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   iHdr - pointer to header buffer
 * @param   pBuf - pointer to item
 * @param   wm - write mode
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_addItem(NVOCMP_nvHandle_t * pNvHandle, NVOCMP_itemHdr_t * iHdr, uint8_t * pBuf, NVOCMP_writeMode_t wm)
{
    uint8_t err;
    uint8_t dstPg;
    uint16_t iLen;
    bool compact = false;
    NVOCMP_itemHdr_t hdr;

    iLen  = NVOCMP_ITEMHDRLEN + iHdr->len;
    dstPg = NVOCMP_getDstPage(pNvHandle, iLen);

    if (dstPg == NVOCMP_NULLPAGE)
    {
        compact = true;
        // Won't fit on the active page, compact and check again
        if (NVOCMP_compactPage(pNvHandle, iLen) < iLen)
        {
            // Failure means there's no place to put this item
            NVOCMP_ALERT(false, "Out of NV.")
            err = (NVOCMP_failW != NVINTF_SUCCESS) ? NVOCMP_failW : NVINTF_BADLENGTH;
            return (err);
        }
    }

    if ((NVOCMP_failW == NVINTF_SUCCESS) && ((wm == NVOCMP_WRITE) || ((wm == NVOCMP_UPDATE) && (compact == true))))
    {
        /* get item offset after compaction */
        hdr.sysid  = iHdr->sysid;
        hdr.itemid = iHdr->itemid;
        hdr.subid  = iHdr->subid;
        (void) NVOCMP_findItem(pNvHandle, pNvHandle->actPage, pNvHandle->actOffset, &hdr, NVOCMP_FINDSTRICT, NULL);
        iHdr->hpage = hdr.hpage;
        iHdr->hofs  = hdr.hofs;
    }

#if NVOCMP_NWSAMEITEM
    bool changed = false;
    if ((iHdr->hofs) && (iHdr->len))
    {
#define NVOCMP_COMPARE_SIZE 32
        uint8_t readBuf[NVOCMP_COMPARE_SIZE];
        uint16_t iOfs    = (iHdr->hofs - iHdr->len);
        uint16_t dOfs    = 0;
        uint16_t len2cmp = 0;

        // Failure to parse the command
        NVOCMP_ALERT(iHdr->hofs >= iHdr->len, "Something wrong in parsing.")

        do
        {
            len2cmp = (iHdr->len - dOfs) > NVOCMP_COMPARE_SIZE ? NVOCMP_COMPARE_SIZE : (iHdr->len - dOfs);
            NVOCMP_read(iHdr->hpage, iOfs + dOfs, readBuf, len2cmp);
            if (memcmp(readBuf, pBuf + dOfs, len2cmp))
            {
                changed = true;
                break;
            }
            dOfs += len2cmp;
        } while (dOfs < iHdr->len);
    }
    else
    {
        changed = true;
    }

    if (changed)
    {
        // Create the new NV item
        NVOCMP_writeItem(pNvHandle, iHdr, pNvHandle->actPage, pNvHandle->actOffset, pBuf);
    }
    else
    {
        iHdr->hofs = 0;
    }
#else
    // Create the new NV item
    NVOCMP_writeItem(pNvHandle, iHdr, pNvHandle->actPage, pNvHandle->actOffset, pBuf);
#endif

    // Status of writing/erasing Flash
    return (NVOCMP_failW);
}

/******************************************************************************
 * @fn      NVOCMP_read
 *
 * @brief   Writes to a flash buffer from RAM
 *
 * @param   pg  - Flash page to write from
 * @param   off - Offset in destination page to read from
 * @param   pBuf  - Pointer to write the results into
 * @param   len - Number of bytes to write into
 *
 * @return  NVS_STATUS_SUCCESS or other NVS status code
 */
static inline void NVOCMP_read(uint8_t pg, uint16_t off, uint8_t * pBuf, uint16_t len)
{
#ifndef NV_LINUX
    NVS_read(NVOCMP_nvsHandle, NVOCMP_FLASHOFFSET(pg, off), (uint8_t *) pBuf, len);
#else
    NV_LINUX_read(pg, off, pBuf, len);
#endif
}

/******************************************************************************
 * @fn      NVOCMP_write
 *
 * @brief   Writes to a flash buffer from RAM
 *
 * @param   dstPg  - Flash page to write to
 * @param   off - offset in destination page to write to
 * @param   pBuf  - Pointer to caller's buffer to write & verify
 * @param   len - number of bytes to write from pBuf
 *
 * @return  NVS_STATUS_SUCCESS or other NVS status code
 */
static uint8_t NVOCMP_write(uint8_t dstPg, uint16_t off, uint8_t * pBuf, uint16_t len)
{
    uint8_t err         = NVINTF_SUCCESS;
    int_fast16_t nvsRes = 0;

    // check voltage if possible
    NVOCMP_FLASHACCESS(err)

    if (NVINTF_SUCCESS == err)
    {
#ifndef NV_LINUX
        nvsRes = NVS_write(NVOCMP_nvsHandle, NVOCMP_FLASHOFFSET(dstPg, off), pBuf, len, NVS_WRITE_POST_VERIFY);
#else
        nvsRes  = NV_LINUX_write(dstPg, off, pBuf, len);
#endif
    }
    else
    {
        err = NVINTF_LOWPOWER;
    }

    if (nvsRes < 0)
    {
        err = NVINTF_FAILURE;
    }

    NVOCMP_ALERT(NVINTF_LOWPOWER != err, "Voltage check failed.")
    if (NVINTF_FAILURE == err)
    {
        NVOCMP_ALERT(NVINTF_FAILURE != err, "NVS write failure.")
    }

    return (err);
}

/******************************************************************************
 * @fn      NVOCMP_erase
 *
 * @brief   Erases a flash page
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   dstPg - Flash page to write to
 *
 * @return  NVINT_SUCCESS or other NVINTF status code
 */
static uint8_t NVOCMP_erase(NVOCMP_nvHandle_t * pNvHandle, uint8_t dstPg)
{
    uint8_t err         = NVINTF_SUCCESS;
    int_fast16_t nvsRes = 0;

    // Check voltage if possible
    NVOCMP_FLASHACCESS(err)

    if (NVINTF_SUCCESS == err)
    {
#ifndef NV_LINUX
        nvsRes = NVS_erase(NVOCMP_nvsHandle, NVOCMP_FLASHOFFSET(dstPg, 0), NVOCMP_nvsAttrs.sectorSize);
#else
        nvsRes  = NV_LINUX_erase(dstPg);
#endif
        if (nvsRes < 0)
        {
            err = NVINTF_FAILURE;
        }
        else
        {
            // Bump the compaction cycle counter, wrap-around if at maximum
            pNvHandle->pageInfo[dstPg].cycle =
                (pNvHandle->pageInfo[dstPg].cycle < NVOCMP_MAXCYCLE) ? (pNvHandle->pageInfo[dstPg].cycle + 1) : NVOCMP_MINCYCLE;
            NVOCMP_setPageState(pNvHandle, dstPg, NVOCMP_PGNACT);
            NVOCMP_setCompactHdr(dstPg, NVOCMP_NULLPAGE, NVOCMP_NULLOFFSET, THISPAGEHDR);
            NVOCMP_setCompactHdr(dstPg, NVOCMP_NULLPAGE, NVOCMP_NULLOFFSET, XSRCSTARTHDR);
            NVOCMP_setCompactHdr(dstPg, NVOCMP_NULLPAGE, NVOCMP_NULLOFFSET, XSRCENDHDR);
            pNvHandle->pageInfo[dstPg].offset = NVOCMP_PGDATAOFS;
            pNvHandle->pageInfo[dstPg].mode   = NVOCMP_PGNORMAL;
        }
    }
    else
    {
        err = NVINTF_LOWPOWER;
    }

    NVOCMP_ALERT(NVINTF_LOWPOWER != err, "Voltage check failed.")
    NVOCMP_ALERT(NVINTF_FAILURE != err, "NVS erase failure.")

    return (err);
}

/******************************************************************************
 * @fn      NVOCMP_writeItem
 *
 * @brief   Write entire NV item to new location on active Flash page.
 *          Each call to NVS_write() does a read-back to verify. If an
 *          error is detected, the 'failW' flag is set to inhibit further
 *          flash write attempts until the next NV transaction.
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   pHdr  - Pointer to caller's item header buffer
 * @param   dstPg - Destination NV Flash page
 * @param   dstOff - Destination offset
 * @param   pBuf  - Points to buffer which will be written to item
 *
 * @return  none
 */
static void NVOCMP_writeItem(NVOCMP_nvHandle_t * pNvHandle, NVOCMP_itemHdr_t * pHdr, uint8_t dstPg, uint16_t dstOff, uint8_t * pBuf)
{
    uint16_t iLen;
    NVOCMP_pageHdr_t pageHdr;

    NVOCMP_read(dstPg, NVOCMP_PGHDROFS, (uint8_t *) &pageHdr, NVOCMP_PGHDRLEN);
    if (pageHdr.state == NVOCMP_PGRDY)
    {
        NVOCMP_changePageState(pNvHandle, dstPg, NVOCMP_PGACT);
    }

    if (pageHdr.state != NVOCMP_PGACT)
    {
        NVOCMP_failW = NVINTF_FAILURE;
        return;
    }

    NVOCMP_ALERT(pageHdr.state == NVOCMP_PGACT, "Something wrong.")

    // Total length of this item
    iLen = NVOCMP_ITEMHDRLEN + pHdr->len;

    if ((dstOff + iLen) <= FLASH_PAGE_SIZE)
    {
        cmpIH_t cHdr;
        uint16_t hOfs, dLen;
        uint8_t newCRC;

        // Compressed item header information <-- Lower Addr    Higher Addr-->
        // Byte: [0]      [1]      [2]      [3]      [4]      [5]      [6]
        // Item: SSSSSSII IIIIIIII SSSSSSSS SSLLLLLL LLLLLLCC CCCCCCAV SSSSSSSS
        // LSB of field:         ^           ^            ^        ^          ^
#if NVOCMP_HDRLE
        cHdr[0] = (pHdr->sysid & 0x3F) | ((pHdr->itemid & 0x3) << 6);
        cHdr[1] = (pHdr->itemid >> 2) & 0xFF;
        cHdr[2] = pHdr->subid & 0xFF;
        cHdr[3] = ((pHdr->subid >> 8) & 0x3) | ((pHdr->len & 0x3F) << 2);
        cHdr[4] = (pHdr->len >> 6) & 0x3F;
#else
        cHdr[0] = ((pHdr->sysid << 2) | ((pHdr->itemid >> 8) & 0x3));
        cHdr[1] = (pHdr->itemid & 0xFF);
        cHdr[2] = ((pHdr->subid >> 2) & 0xFF);
        cHdr[3] = ((pHdr->subid & 0x3) << 6) | ((pHdr->len >> 6) & 0x3F);
        cHdr[4] = ((pHdr->len & 0x3F) << 2);
#endif
        // Header is located after the item data
        dLen = pHdr->len;
        hOfs = dstOff + dLen;

        if (iLen <= NVOCMP_SMALLITEM)
        {
            // Construct item in one buffer
            // Put data into buffer
            memcpy(NVOCMP_itemBuffer, (const void *) pBuf, dLen);
            // Put most of header into buffer
            memcpy(NVOCMP_itemBuffer + dLen, (const void *) cHdr, NVOCMP_HDRCRCINC);
            // Calculate CRC
            newCRC = NVOCMP_doRAMCRC(NVOCMP_itemBuffer, dLen + NVOCMP_HDRCRCINC, 0);
#if NVOCMP_HDRLE
            // Insert CRC and last bytes
            cHdr[4] |= ((newCRC & 0x3) << 6);
            // Note NVOCMP_VALIDIDBIT set implicitly zero
            cHdr[5] = ((newCRC >> 2) & 0x3F) | (NVOCMP_ACTIVEIDBIT << 6);
#else
            // Insert CRC and last bytes
            cHdr[4] |= ((newCRC >> 6) & 0x3);
            // Note NVOCMP_VALIDIDBIT set implicitly zero
            cHdr[5] = ((newCRC & 0x3F) << 2) | NVOCMP_ACTIVEIDBIT;
#endif
            cHdr[6] = NVOCMP_SIGNATURE;
            memcpy(NVOCMP_itemBuffer + dLen, (const void *) cHdr, NVOCMP_ITEMHDRLEN);
            // NVS_write
            NVOCMP_failW = NVOCMP_write(dstPg, dstOff, NVOCMP_itemBuffer, iLen);
            // Advance to next location
            dstOff += iLen;
            pNvHandle->actOffset += iLen;
            pNvHandle->pageInfo[dstPg].offset = dstOff;
        }
        else
        {
            // Write header/item separately
            // Calculate CRC on data portion
            newCRC = NVOCMP_doRAMCRC(pBuf, dLen, 0);
            // Finish CRC using header portion
            newCRC = NVOCMP_doRAMCRC(cHdr, NVOCMP_HDRCRCINC, newCRC);
            // Complete Header with CRC, bits, and sig
#if NVOCMP_HDRLE
            // Insert CRC and last bytes
            cHdr[4] |= ((newCRC & 0x3) << 6);
            // Note NVOCMP_VALIDIDBIT set implicitly zero
            cHdr[5] = ((newCRC >> 2) & 0x3F) | (NVOCMP_ACTIVEIDBIT << 6);
#else
            // Insert CRC and last bytes
            cHdr[4] |= ((newCRC >> 6) & 0x3);
            // Note NVOCMP_VALIDIDBIT set implicitly zero
            cHdr[5] = ((newCRC & 0x3F) << 2) | NVOCMP_ACTIVEIDBIT;
#endif
            cHdr[6] = NVOCMP_SIGNATURE;
            // Write data
            NVOCMP_failW = NVOCMP_write(dstPg, dstOff, pBuf, dLen);
            // Write header
            NVOCMP_failW |= NVOCMP_write(dstPg, hOfs, cHdr, NVOCMP_ITEMHDRLEN);
            // Advance to next location
            NVOCMP_ASSERT(dstOff < (dstOff + iLen), "Page offset overflow!")
            if (!NVOCMP_failW)
            {
                dstOff += iLen;
                pNvHandle->actOffset += iLen;
                pNvHandle->pageInfo[dstPg].offset = dstOff;
            }
            else
            {
                return;
            }
        }
        // If there was a write failure, delete item
        NVOCMP_ALERT(!NVOCMP_failW, "Driver write failure. Item deleted.")
        if (NVOCMP_failW)
        {
            NVOCMP_setItemInactive(pNvHandle, dstPg, hOfs);
        }
    }
    else
    {
        // Not enough room on page
        NVOCMP_failW = NVINTF_BADLENGTH;
    }
}

/******************************************************************************
 * @fn      NVOCMP_readHeader
 *
 * @brief   Read header block from NV and expand into caller's buffer
 *
 * @param   pg   - A valid NV Flash page
 * @param   ofs  - A valid offset into the page
 * @param   pHdr - Pointer to caller's item header buffer
 * @param   flag - fast flag (not used if NVOCMP_RAM_OPTIMIZATION is defined)
 *
 * @return  none
 */
static void NVOCMP_readHeader(uint8_t pg, uint16_t ofs, NVOCMP_itemHdr_t * pHdr, bool flag)
{
#ifndef NVOCMP_RAM_OPTIMIZATION
#ifdef NVOCMP_GPRAM
    uint8_t * pTBuffer = RAM_BUFFER_ADDRESS;
#else
    uint8_t * pTBuffer = (uint8_t *) tBuffer;
#endif
#endif

    cmpIH_t cHdr;
#ifndef NVOCMP_RAM_OPTIMIZATION
    if (flag)
    {
        memcpy((uint8_t *) cHdr, (uint8_t *) (pTBuffer + ofs), NVOCMP_ITEMHDRLEN);
    }
    else
    {
#endif
        // Get item header from Flash
        NVOCMP_read(pg, ofs, (uint8_t *) cHdr, NVOCMP_ITEMHDRLEN);
#ifndef NVOCMP_RAM_OPTIMIZATION
    }
#endif
    // Offset to compressed header
    pHdr->hofs  = ofs;
    pHdr->hpage = pg;

    // Compressed item header information <-- Lower Addr    Higher Addr-->
    // Byte: [0]      [1]      [2]      [3]      [4]      [5]      [6]
    // Item: SSSSSSII IIIIIIII SSSSSSSS SSLLLLLL LLLLLLCC CCCCCCAV SSSSSSSS
    // LSB of field:         ^           ^            ^        ^          ^
#if NVOCMP_HDRLE
    pHdr->sysid  = cHdr[0] & 0x3F;
    pHdr->itemid = ((cHdr[0] >> 6) & 0x3) | (cHdr[1] << 2);
    pHdr->subid  = cHdr[2] | ((cHdr[3] & 0x3) << 8);
    pHdr->len    = ((cHdr[3] >> 2) & 0x3F) | ((cHdr[4] & 0x3F) << 6);
    pHdr->crc8   = ((cHdr[4] >> 6) & 0x3) | ((cHdr[5] & 0x3F) << 2);
    pHdr->stats  = (cHdr[5] >> 6) & (NVOCMP_VALIDIDBIT | NVOCMP_ACTIVEIDBIT);
    pHdr->sig    = cHdr[6];
#else
    pHdr->sysid  = (cHdr[0] >> 2) & 0x3F;
    pHdr->itemid = ((cHdr[0] & 0x3) << 8) | cHdr[1];
    pHdr->subid  = (cHdr[2] << 2) | ((cHdr[3] >> 6) & 0x3);
    pHdr->len    = ((cHdr[3] & 0x3F) << 6) | ((cHdr[4] >> 2) & 0x3F);
    pHdr->crc8   = ((cHdr[4] & 0x3) << 6) | ((cHdr[5] >> 2) & 0x3F);
    pHdr->stats  = cHdr[5] & (NVOCMP_VALIDIDBIT | NVOCMP_ACTIVEIDBIT);
    pHdr->sig    = cHdr[6];
#endif
    pHdr->cmpid = NVOCMP_CMPRID(pHdr->sysid, pHdr->itemid, pHdr->subid);
    // Our item has correct signature?
    if (pHdr->sig != NVOCMP_SIGNATURE)
    {
        // Indicate item is invalid
        NVOCMP_ALERT(false, "Invalid signature detected! Item corrupted.")
        pHdr->stats |= NVOCMP_VALIDIDBIT;
    }
    else
    {
        (pHdr->stats) |= NVOCMP_FOLLOWBIT;
        NVOCMP_ALERT(pHdr->stats & NVOCMP_FOLLOWBIT, "Item gap detected. Item not followed.")
    }
}

/******************************************************************************
 * @fn      NVOCMP_readItem
 *
 * @brief   Function to read an item described by iHdr into pBuf
 *
 * @param   iHdr - pointer to an item header struct
 * @param   bOfs - offset into NV data block
 * @param   len - length of NV data to return (0 is illegal)
 * @param   pBuf - pointer to caller's read data buffer  (NULL is illegal)
 * @param   flag - fast flag (not used if NVOCMP_RAM_OPTIMIZATION is defined)
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static uint8_t NVOCMP_readItem(NVOCMP_itemHdr_t * iHdr, uint16_t ofs, uint16_t len, void * pBuf, bool flag)
{
    uint8_t err = NVINTF_SUCCESS;
    uint16_t dOfs, iOfs;
#ifndef NVOCMP_RAM_OPTIMIZATION
#ifdef NVOCMP_GPRAM
    uint8_t * pTBuffer = RAM_BUFFER_ADDRESS;
#else
    uint8_t * pTBuffer = (uint8_t *) tBuffer;
#endif
#endif
    iOfs = (iHdr->hofs - iHdr->len);

    // Optional CRC integrity check
#if NVOCMP_CRCONREAD
    err = NVOCMP_verifyCRC(iOfs, iHdr->len, iHdr->crc8, iHdr->hpage, flag);
#endif

    if (err == NVINTF_SUCCESS)
    {
        // Offset to start of item data
        dOfs = iOfs + ofs;
        if ((dOfs + len) <= iHdr->hofs)
        {
#ifndef NVOCMP_RAM_OPTIMIZATION
            if (flag)
            {
                // Copy from RAM
                memcpy((uint8_t *) pBuf, (uint8_t *) (pTBuffer + dOfs), len);
            }
            else
            {
#endif
                // Copy NV data block to caller's buffer
                NVOCMP_read(iHdr->hpage, dOfs, (uint8_t *) pBuf, len);
#ifndef NVOCMP_RAM_OPTIMIZATION
            }
#endif
        }
        else
        {
            // Bad length or offset
            err = (len > iHdr->len) ? NVINTF_BADLENGTH : NVINTF_BADOFFSET;
        }
    }

    return (err);
}

/******************************************************************************
 * @fn      NVOCMP_setItemInactive
 *
 * @brief   Mark an item as inactive
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   pg - page where the item is located
 * @param   iOfs - Offset to item header (lowest address) in active page
 *
 * @return  none
 */
static void NVOCMP_setItemInactive(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, uint16_t iOfs)
{
    uint8_t tmp;

    // Get byte with validity bit
    tmp = NVOCMP_readByte(pg, iOfs + NVOCMP_HDRVLDOFS);

    // Remove ACTIVE_IDS_MARK
#if NVOCMP_HDRLE
    tmp &= ~(NVOCMP_ACTIVEIDBIT << 6);
#else
    tmp &= ~NVOCMP_ACTIVEIDBIT;
#endif
    // Mark the item as inactive
    NVOCMP_writeByte(pg, iOfs + NVOCMP_HDRVLDOFS, tmp);

    if (pNvHandle->pageInfo[pg].allActive)
    {
        tmp = NVOCMP_readByte(pg, NVOCMP_PGHDRVER);
        tmp &= ~NVOCMP_ALLACTIVE;
        NVOCMP_writeByte(pg, NVOCMP_PGHDRVER, tmp);
        pNvHandle->pageInfo[pg].allActive = NVOCMP_SOMEINACTIVE;
    }
}

/******************************************************************************
 * @fn      NVOCMP_setCompactHdr
 *
 * @brief   Set compact header
 *
 * @param   dstPg - destination page to write the header
 * @param   pg - page of compaction
 * @param   offset - offset of compaction
 * @param   location - location of the header
 *
 * @return  none
 */
static void NVOCMP_setCompactHdr(uint8_t dstPg, uint8_t pg, int16_t offset, uint16_t location)
{
    NVOCMP_compactHdr_t hdr = DEFAULT_COMPACTHDR;
    hdr.page                = pg;
    hdr.pageOffset          = offset;

    NVOCMP_failW = NVOCMP_write(dstPg, (location + 1) * NVOCMP_COMPACTHDRLEN, (uint8_t *) &hdr, NVOCMP_COMPACTHDRLEN);
}

#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
/******************************************************************************
 * @fn      NVOCMP_getCompactHdr
 *
 * @brief   Get compact header
 *
 * @param   dstPg - destination page to read the header
 * @param   location - location of the header
 * @param   pHdr - pointer to the header
 *
 * @return  none
 */
static void NVOCMP_getCompactHdr(uint8_t dstPg, uint16_t location, NVOCMP_compactHdr_t * pHdr)
{
    NVOCMP_read(dstPg, (location + 1) * NVOCMP_COMPACTHDRLEN, (uint8_t *) pHdr, NVOCMP_COMPACTHDRLEN);
}
#endif

/******************************************************************************
 * @fn      NVOCMP_setPageState
 *
 * @brief   Set specified NV page state
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   pg - target NV page
 * @param   state - state of the page
 *
 * @return  none
 */
static void NVOCMP_setPageState(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, NVOCMP_pageState_t state)
{
    NVOCMP_pageHdr_t pHdr;

    // Load header
    pHdr.state     = (uint8_t) state;
    pHdr.cycle     = (uint8_t) pNvHandle->pageInfo[pg].cycle;
    pHdr.allActive = NVOCMP_ALLACTIVE;
    pHdr.version   = (uint8_t) NVOCMP_VERSION;
    pHdr.signature = (uint8_t) NVOCMP_SIGNATURE;

    // Write to page
    NVOCMP_failW = NVOCMP_write(pg, 0, (uint8_t *) &pHdr, NVOCMP_PGHDRLEN);

    if (NVOCMP_failW == NVINTF_SUCCESS)
    {
        if (state == NVOCMP_PGACT)
        {
            // No errors, switch active page
            pNvHandle->actPage = pg;
        }
        pNvHandle->pageInfo[pg].state     = state;
        pNvHandle->pageInfo[pg].allActive = NVOCMP_ALLACTIVE;
    }
}

/******************************************************************************
 * @fn      NVOCMP_changePageState
 *
 * @brief   Change NV page state
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   pg - target NV page
 * @param   state - state of the page
 *
 * @return  none
 */
static void NVOCMP_changePageState(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, NVOCMP_pageState_t state)
{
    NVOCMP_writeByte(pg, NVOCMP_PGHDROFS, (uint8_t) state);

    if (NVOCMP_failW == NVINTF_SUCCESS)
    {
        if (state == NVOCMP_PGACT)
        {
            // No errors, switch active page
            pNvHandle->actPage = pg;
        }
        pNvHandle->pageInfo[pg].state = state;
    }
}

/******************************************************************************
 * @fn      NVOCMP_findOffset
 *
 * @brief   Find the offset to next available empty space in specified page
 *
 * @param   pg   - Valid NV page on which to find offset to next available data
 * @param   ofs - Beginning offset to start search
 *
 * @return  Number of bytes from start of page to next available item location
 */
#if NVOCMP_FASTOFF
static uint16_t NVOCMP_findOffset(uint8_t pg, uint16_t ofs)
{
    uint8_t i, j;
    uint32_t * tmp;
#ifdef NVOCMP_GPRAM
    uint32_t vm;
    uint8_t * pTBuffer = RAM_BUFFER_ADDRESS;
#else
    uint8_t * pTBuffer = (uint8_t *) tBuffer;
#endif

#ifdef NVOCMP_GPRAM
    NVOCMP_disableCache(&vm);
#endif

    NVOCMP_read(pg, 0, (uint8_t *) pTBuffer, FLASH_PAGE_SIZE);

    // Find first non-erased 4-byte location
    tmp = (uint32_t *) pTBuffer;
    while (ofs >= sizeof(uint32_t))
    {
        ofs -= sizeof(uint32_t);
        tmp = (uint32_t *) (pTBuffer + ofs);
        if ((*tmp) != NVOCMP_ERASEDWORD)
        {
            break;
        }
    }

    // Starting with LSB, look for non-erased byte
    for (i = j = 1; i <= 4; i++)
    {
        if (((*tmp) & NVOCMP_ERASEDBYTE) != NVOCMP_ERASEDBYTE)
        {
            // Last non-erased byte so far
            j = i;
        }
        (*tmp) >>= 8;
    }

#ifdef NVOCMP_GPRAM
    NVOCMP_restoreCache(vm);
#endif

    return (ofs + j);
}
#else
static uint16_t NVOCMP_findOffset(uint8_t pg, uint16_t ofs)
{
    uint8_t i, j;
    uint32_t tmp = 0;

    // Find first non-erased 4-byte location
    while (ofs >= sizeof(tmp))
    {
        ofs -= sizeof(tmp);
        NVOCMP_read(pg, ofs, (uint8_t *) &tmp, sizeof(tmp));
        if (tmp != NVOCMP_ERASEDWORD)
        {
            break;
        }
    }

    // Starting with LSB, look for non-erased byte
    for (i = j = 1; i <= 4; i++)
    {
        if ((tmp & NVOCMP_ERASEDBYTE) != NVOCMP_ERASEDBYTE)
        {
            // Last non-erased byte so far
            j = i;
        }
        tmp >>= 8;
    }

    return (ofs + j);
}
#endif

/******************************************************************************
 * @fn      NVOCMP_findItem
 *
 * @brief   Find a valid item from designated page and offset
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   pg  - Valid NV page
 * @param   ofs - Offset in NV page from where to start search
 * @param   pHdr - pointer to item header
 * @param   flag - specifies type of search
 * @param   pInfo - pointer to item info
 *
 * @return  When >0, offset to the item header for found item
 *          When <=0, -number of items searched when item not found
 *
 */
#if NVOCMP_FASTITEM
static int8_t NVOCMP_findItem(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, uint16_t ofs, NVOCMP_itemHdr_t * pHdr, int8_t flag,
                              NVOCMP_itemInfo_t * pInfo)
{
    bool found = false;
    uint8_t p;
    uint16_t items      = 0;
    uint16_t nvSearched = 0;
#ifdef NVOCMP_GPRAM
    uint32_t vm;
    uint8_t * pTBuffer = RAM_BUFFER_ADDRESS;
#else
    uint8_t * pTBuffer = (uint8_t *) tBuffer;
#endif
    uint32_t cid = NVOCMP_CMPRID(pHdr->sysid, pHdr->itemid, pHdr->subid);

#ifdef NVOCMP_GPRAM
    NVOCMP_disableCache(&vm);
#endif

    for (p = pg; nvSearched < NVOCMP_NVSIZE; p = NVOCMP_DECPAGE(p), ofs = pNvHandle->pageInfo[p].offset)
    {
        nvSearched++;
#if (NVOCMP_NVPAGES != NVOCMP_NVONEP)
        if (p == pNvHandle->tailPage)
        {
            continue;
        }
#endif
        NVOCMP_read(pg, 0, (uint8_t *) pTBuffer, FLASH_PAGE_SIZE);

        while (ofs >= (NVOCMP_PGDATAOFS + NVOCMP_ITEMHDRLEN))
        {
            NVOCMP_itemHdr_t iHdr;

            // Align to start of item header
            ofs -= NVOCMP_ITEMHDRLEN;

            // Read and decompress item header
            NVOCMP_readHeader(p, ofs, &iHdr, true);

            if ((iHdr.stats & NVOCMP_ACTIVEIDBIT) && !(iHdr.stats & NVOCMP_VALIDIDBIT))
            {
                uint32_t sysid  = pHdr->sysid;
                uint32_t itemid = pHdr->itemid;

                switch (flag & NVOCMP_FINDLMASK)
                {
                case NVOCMP_FINDANY:
                    found = true;
                    break;
                case NVOCMP_FINDSTRICT:
                    // Return first cid match
                    if (cid == iHdr.cmpid)
                    {
                        found = true;
                    }
                    break;
                case NVOCMP_FINDSYSID:
                    // return first sysid match
                    if (sysid == iHdr.sysid)
                    {
                        found = true;
                    }
                    break;
                case NVOCMP_FINDITMID:
                    // return first sysid AND itemid match
                    if (sysid == iHdr.sysid && itemid == iHdr.itemid)
                    {
                        found = true;
                    }
                    break;
                default:
                    // Should not get here
                    NVOCMP_EXCEPTION(p, NVINTF_BADPARAM);
                    NVOCMP_ASSERT(false, "Unhandled case in findItem().")
#ifdef NVOCMP_GPRAM
                    NVOCMP_restoreCache(vm);
#endif
                    return (NVINTF_BADPARAM);
                }
                // Item found - return offset of item header
                if (found)
                {
                    if ((pInfo) && ((flag & NVOCMP_FINDHMASK) == NVOCMP_FINDCONTENT))
                    {
                        if (!NVOCMP_readItem(&iHdr, 0, pInfo->rlength, pInfo->rBuf, false))
                        {
                            if (!memcmp((uint8_t *) pInfo->rBuf + pInfo->coff, pInfo->cBuf, pInfo->clength))
                            {
                                memcpy(pHdr, &iHdr, sizeof(NVOCMP_itemHdr_t));
#ifdef NVOCMP_GPRAM
                                NVOCMP_restoreCache(vm);
#endif
                                return (NVINTF_SUCCESS);
                            }
                        }
                        found = false;
                    }
                    else
                    {
                        memcpy(pHdr, &iHdr, sizeof(NVOCMP_itemHdr_t));
#ifdef NVOCMP_GPRAM
                        NVOCMP_restoreCache(vm);
#endif
                        return (NVINTF_SUCCESS);
                    }
                }
            }
            // Try to jump to next item
            if (iHdr.stats & NVOCMP_FOLLOWBIT)
            {
                // Appears to be an item there, check bounds
                if (iHdr.len < ofs)
                {
                    // Adjust offset for next try
                    ofs -= iHdr.len;
                }
                else
                {
                    // Length is corrupt, mark item invalid and compact
                    NVOCMP_ALERT(false, "Item length corrupted. Deleting item.")
                    NVOCMP_setItemInactive(pNvHandle, p, ofs);
#ifdef NVOCMP_GPRAM
                    NVOCMP_restoreCache(vm);
#endif
                    NVOCMP_compactPage(pNvHandle, 0);
                    p          = NVOCMP_INCPAGE(pNvHandle->actPage);
                    ofs        = 0;
                    nvSearched = 0;
                }
            }
            else
            {
                // Something is corrupted, compact to fix
                NVOCMP_ALERT(false,
                             "No item following current item, "
                             "compaction needed.")
#ifdef NVOCMP_GPRAM
                NVOCMP_restoreCache(vm);
#endif
                NVOCMP_compactPage(pNvHandle, 0);
                p          = NVOCMP_INCPAGE(pNvHandle->actPage);
                ofs        = 0;
                nvSearched = 0;
            }
            // Running count of items searched
            items += 1;
        }
    }
#ifdef NVOCMP_GPRAM
    NVOCMP_restoreCache(vm);
#endif
    // Item not found (negate number of items searched)
    // or nth not found, return last found
    pHdr->hofs = 0;
    return (NVINTF_NOTFOUND);
}
#else
static int8_t NVOCMP_findItem(NVOCMP_nvHandle_t * pNvHandle, uint8_t pg, uint16_t ofs, NVOCMP_itemHdr_t * pHdr, int8_t flag,
                              NVOCMP_itemInfo_t * pInfo)
{
    bool found          = false;
    uint8_t p           = pg;
    uint16_t items      = 0;
    uint32_t cid        = NVOCMP_CMPRID(pHdr->sysid, pHdr->itemid, pHdr->subid);

#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
    uint16_t nvSearched = 0;
    for (p = pg; nvSearched < NVOCMP_NVSIZE; p = NVOCMP_DECPAGE(p), ofs = pNvHandle->pageInfo[p].offset)
    {
        nvSearched++;
        if (p == pNvHandle->tailPage)
        {
            continue;
        }
#endif
        while (ofs >= (NVOCMP_PGDATAOFS + NVOCMP_ITEMHDRLEN))
        {
            NVOCMP_itemHdr_t iHdr;

            // Align to start of item header
            ofs -= NVOCMP_ITEMHDRLEN;

            // Read and decompress item header
            NVOCMP_readHeader(p, ofs, &iHdr, false);

            if ((iHdr.stats & NVOCMP_ACTIVEIDBIT) && !(iHdr.stats & NVOCMP_VALIDIDBIT))
            {
                uint32_t sysid  = pHdr->sysid;
                uint32_t itemid = pHdr->itemid;

                switch (flag & NVOCMP_FINDLMASK)
                {
                case NVOCMP_FINDANY:
                    found = true;
                    break;
                case NVOCMP_FINDSTRICT:
                    // Return first cid match
                    if (cid == iHdr.cmpid)
                    {
                        found = true;
                    }
                    break;
                case NVOCMP_FINDSYSID:
                    // return first sysid match
                    if (sysid == iHdr.sysid)
                    {
                        found = true;
                    }
                    break;
                case NVOCMP_FINDITMID:
                    // return first sysid AND itemid match
                    if (sysid == iHdr.sysid && itemid == iHdr.itemid)
                    {
                        found = true;
                    }
                    break;
                default:
                    // Should not get here
                    NVOCMP_EXCEPTION(p, NVINTF_BADPARAM);
                    NVOCMP_ASSERT(false, "Unhandled case in findItem().")
                    return (NVINTF_BADPARAM);
                }
                // Item found - return offset of item header
                if (found)
                {
                    if ((pInfo) && ((flag & NVOCMP_FINDHMASK) == NVOCMP_FINDCONTENT))
                    {
                        if (!NVOCMP_readItem(&iHdr, 0, pInfo->rlength, pInfo->rBuf, false))
                        {
                            if (!memcmp((uint8_t *) pInfo->rBuf + pInfo->coff, pInfo->cBuf, pInfo->clength))
                            {
                                memcpy(pHdr, &iHdr, sizeof(NVOCMP_itemHdr_t));
                                return (NVINTF_SUCCESS);
                            }
                        }
                        found = false;
                    }
                    else
                    {
                        memcpy(pHdr, &iHdr, sizeof(NVOCMP_itemHdr_t));
                        return (NVINTF_SUCCESS);
                    }
                }
            }
            // Try to jump to next item
            if (iHdr.stats & NVOCMP_FOLLOWBIT)
            {
                // Appears to be an item there, check bounds
                if (iHdr.len < ofs)
                {
                    // Adjust offset for next try
                    ofs -= iHdr.len;
                }
                else
                {
                    // Length is corrupt, mark item invalid and compact
                    NVOCMP_ALERT(false, "Item length corrupted. Deleting item.")
                    NVOCMP_setItemInactive(pNvHandle, p, ofs);
                    NVOCMP_compactPage(pNvHandle, 0);
#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
                    p          = NVOCMP_INCPAGE(pNvHandle->actPage);
                    ofs        = 0;
                    nvSearched = 0;
#else
                p   = pNvHandle->actPage;
                ofs = pNvHandle->actOffset;
#endif
                }
            }
            else
            {
                // Something is corrupted, compact to fix
                NVOCMP_ALERT(false,
                             "No item following current item, "
                             "compaction needed.")
                pNvHandle->forceCompact = 1;
                NVOCMP_compactPage(pNvHandle, 0);
#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
                p          = NVOCMP_INCPAGE(pNvHandle->actPage);
                ofs        = 0;
                nvSearched = 0;
#else
            p   = pNvHandle->actPage;
            ofs = pNvHandle->actOffset;
#endif
            }
            // Running count of items searched
            items += 1;
        }
#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
    }
#endif
    // Item not found (negate number of items searched)
    // or nth not found, return last found
    pHdr->hofs = 0;
    return (NVINTF_NOTFOUND);
}
#endif

#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
/******************************************************************************
 * @fn      NVOCMP_cleanPage
 *
 * @brief   Clean the page that compaction is done with
 *
 * @param   pNvHandle - pointer to NV handle
 *
 * @return  Number of pages cleaned
 */
static uint8_t NVOCMP_cleanPage(NVOCMP_nvHandle_t * pNvHandle)
{
    uint8_t pg;
    uint8_t pages = 0;
    NVOCMP_pageHdr_t pageHdr;
    NVOCMP_compactHdr_t startHdr;
    NVOCMP_compactHdr_t endHdr;

    /* correct ofset */
    pg = pNvHandle->compactInfo.xDstPage;
    NVOCMP_getCompactHdr(pg, XSRCSTARTHDR, &startHdr);
    NVOCMP_getCompactHdr(pg, XSRCENDHDR, &endHdr);
    for (pg = pNvHandle->compactInfo.xSrcSPage; pg != NVOCMP_INCPAGE(pNvHandle->compactInfo.xSrcEPage); pg = NVOCMP_INCPAGE(pg))
    {
        if (pg == endHdr.page)
        {
            pNvHandle->pageInfo[pg].offset = endHdr.pageOffset;
        }
        else
        {
            pNvHandle->pageInfo[pg].offset = NVOCMP_PGDATAOFS;
        }
    }
    /* end of correction */

    for (pg = pNvHandle->compactInfo.xSrcSPage; pg != NVOCMP_INCPAGE(pNvHandle->compactInfo.xSrcEPage); pg = NVOCMP_INCPAGE(pg))
    {
        if (pNvHandle->pageInfo[pg].offset == NVOCMP_PGDATAOFS)
        {
            pages++;
            NVOCMP_failW = NVOCMP_erase(pNvHandle, pg);
        }
        else
        {
            NVOCMP_failW = NVOCMP_write(pg, (THISPAGEHDR + 1) * NVOCMP_COMPACTHDRLEN, (uint8_t *) &pNvHandle->pageInfo[pg].offset,
                                        sizeof(pNvHandle->pageInfo[pg].offset));
            NVOCMP_read(pg, NVOCMP_PGHDROFS, (uint8_t *) &pageHdr, NVOCMP_PGHDRLEN);
            if (pageHdr.state == NVOCMP_PGACT)
            {
                NVOCMP_changePageState(pNvHandle, pg, NVOCMP_PGFULL);
            }
            else
            {
                // this should not hit
                if (pageHdr.state == NVOCMP_PGXDST)
                {
                    NVOCMP_ASSERT1(0);
                }
            }
        }
    }
    return (pages);
}
#endif

#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
/******************************************************************************
 * @fn      NVOCMP_compactPage
 *
 * @brief   Compact specified page by copying active items to other page
 *
 *          Compaction occurs under three circumstances: (1) 'maintenance'
 *          activity which is triggered by a user call to compactNvApi(),
 *          (2) 'update' activity where an NV page is packed to make room
 *          for an item being written. The 'update' mode is performed by
 *          writing the item after the rest of the page has been compacted,
 *          and (3) when corruption is detected in the NV page. The compaction
 *          operation will move all active&valid items to the other page.
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   nBytes - size of item to write if any
 *
 * @return  Number of available bytes on compacted page, -1 if error
 */
static int16_t NVOCMP_compactPage(NVOCMP_nvHandle_t * pNvHandle, uint16_t nBytes)
{
    uint8_t pg;
    uint8_t mode;
    uint8_t srcPg;
    uint8_t dstPg;
    uint16_t needBytes;
    uint16_t compactPages;
    uint16_t cleanPages;
    uint16_t skipPages = 0;
    bool foundRoom     = false;
    NVOCMP_compactStatus_t status;
    NVOCMP_pageHdr_t pageHdr;
    uint8_t allActivePages = 0;
    uint8_t err            = NVINTF_SUCCESS;

    // Check voltage if possible
    NVOCMP_FLASHACCESS(err)
    if (err)
    {
        return (0);
    }

    srcPg        = pNvHandle->headPage;
    dstPg        = pNvHandle->tailPage;
    compactPages = NVOCMP_NVSIZE - 1;
    if (nBytes)
    {
        pNvHandle->compactInfo.xSrcPages = 1;
    }
    else
    {
        pNvHandle->compactInfo.xSrcPages = NVOCMP_NVSIZE - 1;
    }

    // mark page mode
    for (pg = 0; pg < NVOCMP_NVSIZE; pg++)
    {
        if (pg != dstPg)
        {
            NVOCMP_read(pg, NVOCMP_PGHDROFS, (uint8_t *) &pageHdr, NVOCMP_PGHDRLEN);
            if (pageHdr.allActive)
            {
                allActivePages++;
            }
            if (pageHdr.state == NVOCMP_PGACT || pageHdr.state == NVOCMP_PGFULL)
            {
                mode = NVOCMP_PGCSRC;
                NVOCMP_writeByte(pg, NVOCMP_COMPMODEOFS, mode);
                pNvHandle->pageInfo[pg].mode = mode;
            }
        }
    }

    if ((allActivePages == NVOCMP_NVSIZE - 1) && !pNvHandle->forceCompact)
    {
        return (0);
    }

    while (compactPages)
    {
        if (pNvHandle->compactInfo.xSrcPages == 0)
        {
            pNvHandle->compactInfo.xSrcPages = 1;
        }
        // Get page header
        NVOCMP_read(srcPg, NVOCMP_PGHDROFS, (uint8_t *) &pageHdr, NVOCMP_PGHDRLEN);
#if NVOCMP_FASTCP
        if (nBytes && pageHdr.allActive)
        {
            srcPg                             = NVOCMP_INCPAGE(srcPg);
            pNvHandle->compactInfo.xDstOffset = FLASH_PAGE_SIZE;
            pNvHandle->compactInfo.xSrcSPage  = srcPg;
            pNvHandle->compactInfo.xSrcPages--;
            compactPages--;
            skipPages++;
            continue;
        }
#endif
        // Mark source page to be in PGXSRC state
        if (pageHdr.state != NVOCMP_PGXSRC)
        {
            NVOCMP_changePageState(pNvHandle, srcPg, NVOCMP_PGXSRC);
        }
        // Get page header
        NVOCMP_read(dstPg, NVOCMP_PGHDROFS, (uint8_t *) &pageHdr, NVOCMP_PGHDRLEN);
        // Mark destination page to be in PGXDST state
        if (pageHdr.state != NVOCMP_PGXDST)
        {
            NVOCMP_changePageState(pNvHandle, dstPg, NVOCMP_PGXDST);
        }

        mode = NVOCMP_PGCDST;
        NVOCMP_writeByte(dstPg, NVOCMP_COMPMODEOFS, mode);
        pNvHandle->pageInfo[dstPg].mode = mode;

        pNvHandle->compactInfo.xDstPage    = dstPg;
        pNvHandle->compactInfo.xDstOffset  = FLASH_PAGE_SIZE;
        pNvHandle->compactInfo.xSrcSPage   = srcPg;
        pNvHandle->compactInfo.xSrcSOffset = pNvHandle->pageInfo[srcPg].offset;
        pNvHandle->compactInfo.xSrcEPage   = NVOCMP_NULLPAGE;
        pNvHandle->compactInfo.xSrcEOffset = 0;
        status                             = NVOCMP_compact(pNvHandle);

        if (status == NVOCMP_COMPACT_FAILURE)
        {
#ifdef NVOCMP_RECOVER_FROM_COMPACT_FAILURE
            uint8_t p;
            for (p = 0; p < NVOCMP_NVSIZE; p++)
            {
                NVOCMP_failW |= NVOCMP_erase(pNvHandle, p);
                if ((p != 0) && (p != NVOCMP_NVSIZE - 1))
                {
                    NVOCMP_changePageState(pNvHandle, p, NVOCMP_PGRDY);
                }
            }
            pNvHandle->actPage   = 0;
            pNvHandle->actOffset = pNvHandle->pageInfo[0].offset;
            pNvHandle->headPage  = 0;
            pNvHandle->tailPage  = NVOCMP_NVSIZE - 1;
            NVOCMP_changePageState(pNvHandle, NVOCMP_NVSIZE - 1, NVOCMP_PGXDST);
            NVOCMP_changePageState(pNvHandle, 0, NVOCMP_PGACT);

            pNvHandle->forceCompact = 0;
#endif
            return (0);
        }

        needBytes = nBytes ? nBytes : 16;

        if (nBytes)
        {
            if (FLASH_PAGE_SIZE - pNvHandle->compactInfo.xDstOffset >= needBytes)
            {
                foundRoom = true;
            }
        }
        // change XDST page state
        if (FLASH_PAGE_SIZE - pNvHandle->compactInfo.xDstOffset >= needBytes)
        {
            NVOCMP_changePageState(pNvHandle, dstPg, NVOCMP_PGACT);
        }
        else
        {
            NVOCMP_changePageState(pNvHandle, dstPg, NVOCMP_PGFULL);
        }
        // clean XSRC pages
        cleanPages = NVOCMP_cleanPage(pNvHandle);
        compactPages -= cleanPages;
        pNvHandle->compactInfo.xSrcPages -= cleanPages;

        // mark XDST page as done
        mode = NVOCMP_PGCDONE;
        NVOCMP_writeByte(dstPg, NVOCMP_COMPMODEOFS, mode);
        pNvHandle->pageInfo[dstPg].mode = mode;

        // move tail page and head page
        pNvHandle->tailPage = NVOCMP_ADDPAGE(pNvHandle->tailPage, cleanPages + skipPages);
        pNvHandle->headPage = NVOCMP_INCPAGE(pNvHandle->tailPage);
        skipPages           = 0;
        // set next source page
        if (pNvHandle->pageInfo[pNvHandle->compactInfo.xSrcEPage].offset == NVOCMP_PGDATAOFS)
        {
            srcPg = NVOCMP_INCPAGE(pNvHandle->compactInfo.xSrcEPage);
        }
        else
        {
            srcPg = pNvHandle->compactInfo.xSrcEPage;
        }

        // set next destination page
        dstPg = pNvHandle->tailPage;
        if (nBytes && foundRoom)
        {
            break;
        }
    }

    pg = NVOCMP_findPage(NVOCMP_PGACT);
    if (pg == NVOCMP_NULLPAGE)
    {
        pg = NVOCMP_findPage(NVOCMP_PGRDY);
        if (pg == NVOCMP_NULLPAGE)
        {
            if (pNvHandle->pageInfo[pNvHandle->headPage].state == NVOCMP_PGNACT)
            {
                pg = pNvHandle->headPage;
                NVOCMP_changePageState(pNvHandle, pg, NVOCMP_PGRDY);
            }
            else
            {
                pg = NVOCMP_DECPAGE(pNvHandle->tailPage);
            }
        }
    }

    pNvHandle->actPage   = pg;
    pNvHandle->actOffset = pNvHandle->pageInfo[pNvHandle->actPage].offset;
    NVOCMP_changePageState(pNvHandle, pNvHandle->tailPage, NVOCMP_PGXDST);

    pNvHandle->forceCompact = 0;
    return (FLASH_PAGE_SIZE - pNvHandle->compactInfo.xDstOffset);
}
#else
/******************************************************************************
 * @fn      NVOCMP_compactPage
 *
 * @brief   Compact specified page by copying active items to other page
 *
 *          Compaction occurs under three circumstances: (1) 'maintenance'
 *          activity which is triggered by a user call to compactNvApi(),
 *          (2) 'update' activity where an NV page is packed to make room
 *          for an item being written. The 'update' mode is performed by
 *          writing the item after the rest of the page has been compacted,
 *          and (3) when corruption is detected in the NV page. The compaction
 *          operation will move all active&valid items to the other page.
 *
 * @param   pNvHandle - pointer to NV handle
 * @param   nBytes - size of item to write if any
 *
 * @return  Number of available bytes on compacted page, -1 if error
 */
static int16_t NVOCMP_compactPage(NVOCMP_nvHandle_t * pNvHandle, uint16_t nBytes)
{
    uint8_t srcPg;
    uint8_t dstPg;
    uint16_t needBytes;
    NVOCMP_compactStatus_t status;
    NVOCMP_pageHdr_t pageHdr;
    uint8_t err = NVINTF_SUCCESS;

    // Check voltage if possible
    NVOCMP_FLASHACCESS(err)
    if (err)
    {
        return (0);
    }

#if (NVOCMP_NVPAGES == NVOCMP_NVONEP)
    srcPg                            = 0;
    dstPg                            = 0;
#else
    srcPg  = pNvHandle->headPage;
    dstPg  = pNvHandle->tailPage;
#endif
    pNvHandle->compactInfo.xSrcPages = 1;

    // mark page mode
    NVOCMP_read(srcPg, NVOCMP_PGHDROFS, (uint8_t *) &pageHdr, NVOCMP_PGHDRLEN);
    if ((NVOCMP_ALLACTIVE == pageHdr.allActive) && !pNvHandle->forceCompact)
    {
        return (0);
    }

    NVOCMP_writeByte(srcPg, NVOCMP_COMPMODEOFS, NVOCMP_PGCSRC);
    pNvHandle->pageInfo[srcPg].mode = NVOCMP_PGCSRC;

    // Mark source page to be in PGXSRC state
    if (pageHdr.state != NVOCMP_PGXSRC)
    {
        NVOCMP_changePageState(pNvHandle, srcPg, NVOCMP_PGXSRC);
    }
#if (NVOCMP_NVPAGES > NVOCMP_NVONEP)
    // Get page header
    NVOCMP_read(dstPg, NVOCMP_PGHDROFS, (uint8_t *) &pageHdr, NVOCMP_PGHDRLEN);
    // Mark destination page to be in PGXDST state
    if (pageHdr.state != NVOCMP_PGXDST)
    {
        NVOCMP_changePageState(pNvHandle, dstPg, NVOCMP_PGXDST);
    }

    NVOCMP_writeByte(dstPg, NVOCMP_COMPMODEOFS, NVOCMP_PGCDST);
    pNvHandle->pageInfo[dstPg].mode = NVOCMP_PGCDST;
#endif

    pNvHandle->compactInfo.xSrcSOffset = pNvHandle->pageInfo[srcPg].offset;
    status                             = NVOCMP_compact(pNvHandle);

    if (status == NVOCMP_COMPACT_FAILURE)
    {
#ifdef NVOCMP_RECOVER_FROM_COMPACT_FAILURE
        uint8_t p;
        for (p = 0; p < NVOCMP_NVSIZE; p++)
        {
            NVOCMP_failW |= NVOCMP_erase(pNvHandle, p);
            if ((p != 0) && (p != NVOCMP_NVSIZE - 1))
            {
                NVOCMP_changePageState(pNvHandle, p, NVOCMP_PGRDY);
            }
        }
        pNvHandle->actPage   = 0;
        pNvHandle->actOffset = pNvHandle->pageInfo[0].offset;
        pNvHandle->headPage  = 0;
        pNvHandle->tailPage  = NVOCMP_NVSIZE - 1;
        NVOCMP_changePageState(pNvHandle, NVOCMP_NVSIZE - 1, NVOCMP_PGXDST);
        NVOCMP_changePageState(pNvHandle, 0, NVOCMP_PGACT);

        pNvHandle->forceCompact = 0;
#endif
        return (0);
    }

    needBytes = nBytes ? nBytes : 16;

    // change XDST page state
    if (FLASH_PAGE_SIZE - pNvHandle->compactInfo.xDstOffset >= needBytes)
    {
        NVOCMP_changePageState(pNvHandle, dstPg, NVOCMP_PGACT);
    }
    else
    {
        NVOCMP_changePageState(pNvHandle, dstPg, NVOCMP_PGFULL);
    }

#if (NVOCMP_NVPAGES > NVOCMP_NVONEP)
    // clean XSRC pages
    NVOCMP_failW = NVOCMP_erase(pNvHandle, srcPg);
#endif

    // mark XDST page as done
    NVOCMP_writeByte(dstPg, NVOCMP_COMPMODEOFS, NVOCMP_PGCDONE);
    pNvHandle->pageInfo[dstPg].mode = NVOCMP_PGCDONE;

    // move tail page and head page
    pNvHandle->tailPage  = srcPg;
    pNvHandle->headPage  = dstPg;
    pNvHandle->actPage   = dstPg;
    pNvHandle->actOffset = pNvHandle->pageInfo[dstPg].offset;
#if (NVOCMP_NVPAGES > NVOCMP_NVONEP)
    NVOCMP_changePageState(pNvHandle, srcPg, NVOCMP_PGXDST);
#endif

    pNvHandle->forceCompact = 0;
    return (FLASH_PAGE_SIZE - pNvHandle->actOffset);
}
#endif

/******************************************************************************
 * @fn      NVOCMP_findSignature
 *
 * @brief   Local function to scan page to get page information
 *
 * @param   pg - page to search
 * @param   pSrcOff - source off to start search from
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static bool NVOCMP_findSignature(uint8_t pg, uint16_t * pSrcOff)
{
    uint16_t i;
    uint16_t rdLen;
    uint8_t readBuffer[NVOCMP_XFERBLKMAX];
    uint16_t srcOff = *pSrcOff;
    uint16_t endOff = NVOCMP_PGDATAOFS + NVOCMP_ITEMHDRLEN - 1;

    while (srcOff > endOff)
    {
        rdLen = (srcOff - NVOCMP_XFERBLKMAX > endOff) ? NVOCMP_XFERBLKMAX : srcOff - endOff;
        srcOff -= rdLen;
        NVOCMP_read(pg, srcOff, readBuffer, rdLen);
        for (i = rdLen; i > 0; i--)
        {
            if (readBuffer[i - 1] == NVOCMP_SIGNATURE)
            {
                // Found possible header, resume normal operation
                NVOCMP_ALERT(false, "Found possible signature.")
                srcOff += i; // srcOff should point to one byte ahead
                *pSrcOff = srcOff;
                return (true);
            }
        }
    }
    return (false);
}

#if (NVOCMP_NVPAGES > NVOCMP_NVTWOP)
/******************************************************************************
 * @fn      NVOCMP_compact
 *
 * @brief   Local function to compact NV
 *
 * @param   pNvHandle - pointer to NV handler
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static NVOCMP_compactStatus_t NVOCMP_compact(NVOCMP_nvHandle_t * pNvHandle)
{
    bool needScan = false;
    bool needSkip = false;
    bool dstFull  = false;
    uint16_t dstOff;
    uint16_t endOff;
    uint16_t srcOff;
    uint16_t crcOff;
    uint8_t srcStartPg;
    uint8_t srcEndPg;
    uint8_t dstPg;
    uint8_t srcPg;
    uint32_t aItem = 0;
#ifndef NVOCMP_RAM_OPTIMIZATION
#ifdef NVOCMP_GPRAM
    uint32_t vm;
    uint8_t * pTBuffer = RAM_BUFFER_ADDRESS;
#else
    uint8_t * pTBuffer = (uint8_t *) tBuffer;
#endif
#endif

    NVOCMP_compactStatus_t status = NVOCMP_COMPACT_SUCCESS;

#ifndef NVOCMP_GPRAM
    memset(tBuffer, 0, sizeof(tBuffer));
#endif
    // Reset Flash erase/write fail indicator
    NVOCMP_failW = NVINTF_SUCCESS;
    srcStartPg   = pNvHandle->compactInfo.xSrcSPage;
    srcEndPg     = NVOCMP_ADDPAGE(srcStartPg, pNvHandle->compactInfo.xSrcPages - 1);

    // Stop looking when we get to this offset
    endOff = NVOCMP_PGDATAOFS + NVOCMP_ITEMHDRLEN - 1;

    srcPg  = srcStartPg;
    srcOff = pNvHandle->pageInfo[srcPg].offset;
    dstPg  = pNvHandle->compactInfo.xDstPage;
    dstOff = pNvHandle->pageInfo[dstPg].offset;

    NVOCMP_ALERT(false, "Compaction triggered.")

#ifdef NVOCMP_GPRAM
    NVOCMP_disableCache(&vm);
#endif
    while (srcPg != dstPg)
    {
        if (dstFull)
        {
            if (aItem)
            {
                pNvHandle->pageInfo[srcPg].offset = srcOff;
            }
            break;
        }

        if (srcOff <= endOff)
        {
            if (aItem)
            {
                pNvHandle->pageInfo[srcPg].offset = srcOff;
            }
            if (srcPg == srcEndPg)
            {
                break;
            }
            else
            {
                srcPg  = NVOCMP_INCPAGE(srcPg);
                srcOff = pNvHandle->pageInfo[srcPg].offset;
                aItem  = 0;
                continue;
            }
        }

        if (NVOCMP_failW == NVINTF_SUCCESS)
        {
            NVOCMP_itemHdr_t srcHdr;
            uint16_t dataLen;
            uint16_t itemSize;

            needScan = false;
            needSkip = false;

            // Read and decompress item header
            NVOCMP_readHeader(srcPg, srcOff - NVOCMP_ITEMHDRLEN, &srcHdr, false);
            dataLen  = srcHdr.len;
            itemSize = NVOCMP_ITEMHDRLEN + dataLen;
            crcOff   = srcOff - NVOCMP_ITEMHDRLEN - dataLen;

            // Check if length is safe
            if (srcOff < (dataLen + NVOCMP_PGDATAOFS))
            {
                NVOCMP_ALERT(false, "Item header corrupted: Data length too long")
                needScan = true;
                srcOff--;
            }
            else if (NVOCMP_SIGNATURE != srcHdr.sig)
            {
                NVOCMP_ALERT(false, "Item header corrupted: Invalid signature")
                needScan = true;
                srcOff--;
            }
            else if (NVOCMP_verifyCRC(crcOff, dataLen, srcHdr.crc8, srcPg, false))
            {
                // Invalid CRC, corruption
                NVOCMP_ALERT(false, "Item CRC incorrect!")
                needScan = true;
                srcOff--;
            }
            else if (!(srcHdr.stats & NVOCMP_VALIDIDBIT) && // Item is valid
                     (srcHdr.stats & NVOCMP_ACTIVEIDBIT))   // Item is active
            {
                // Valid CRC, item is active
                srcOff -= NVOCMP_ITEMHDRLEN;
            }
            else
            {
                // Valid CRC but item is inactive
                srcOff -= NVOCMP_ITEMHDRLEN;
                needSkip = true;
            }

            if (needScan)
            {
                // Detected a problem, find next header (scan for signature)
                NVOCMP_ALERT(false, "Attempting to find signature...")
                bool foundSig = NVOCMP_findSignature(srcPg, &srcOff);
                if (!foundSig)
                {
#ifdef NVOCMP_GPRAM
                    NVOCMP_restoreCache(vm);
#endif
                    // If we get here and foundSig is false, we never found another
                    // item in the page, break the loop so that any valid items
                    // that were collected up to this point get written to the
                    // destination page.
                    NVOCMP_ALERT(foundSig, "Attempt to find signature failed.")
                    break;
                }
            }
            else
            {
                if (!needSkip)
                {
                    if (dstOff + itemSize > FLASH_PAGE_SIZE)
                    {
                        // cannot fit one page temp buffer and revert srcOff change
                        srcOff += NVOCMP_ITEMHDRLEN;
                        dstFull = true;
                        continue;
                    }
                    else
                    {
                        // Get block of bytes from source page
#ifndef NVOCMP_RAM_OPTIMIZATION
#if NVOCMP_COMPR
                        NVOCMP_read(srcPg, crcOff, (uint8_t *) (pTBuffer + dstOff), itemSize);
#else
                        NVOCMP_read(srcPg, crcOff, (uint8_t *) (pTBuffer + FLASH_PAGE_SIZE - dstOff - itemSize), itemSize);
#endif
#else
                        NVOCMP_copyItem(srcPg, dstPg, crcOff, dstOff, itemSize);
#endif
                        dstOff += itemSize;
                        aItem++;
                        if (dstOff == FLASH_PAGE_SIZE)
                        {
                            dstFull = true;
                        }
                    }
                }
                NVOCMP_ALERT(srcOff > dataLen, "Offset overflow: srcOff")
                srcOff -= dataLen;
            }
        }
        else
        {
#ifdef NVOCMP_GPRAM
            NVOCMP_restoreCache(vm);
#endif
            // Failure during item xfer makes next findItem() unreliable
            NVOCMP_ASSERT(false, "COMPACTION FAILURE")
            return (NVOCMP_COMPACT_FAILURE);
        }
    } // end of while

    if (NVOCMP_failW != NVINTF_SUCCESS)
    {
#ifdef NVOCMP_GPRAM
        NVOCMP_restoreCache(vm);
#endif
        // Something bad happened when trying to compact the page
        NVOCMP_ASSERT(false, "COMPACTION FAILURE")
        return (NVOCMP_COMPACT_FAILURE);
    }

    // Write block to destination page
#ifndef NVOCMP_RAM_OPTIMIZATION
#if NVOCMP_COMPR
    uint16_t off = NVOCMP_PGDATAOFS;
    uint16_t len = dstOff - NVOCMP_PGDATAOFS;

    NVOCMP_failW |= NVOCMP_write(dstPg, off, (uint8_t *) (pTBuffer + off), len);
#else
    uint16_t off  = NVOCMP_PGDATAOFS;
    uint16_t doff = FLASH_PAGE_SIZE - dstOff;
    uint16_t len  = dstOff - NVOCMP_PGDATAOFS;

    NVOCMP_failW |= NVOCMP_write(dstPg, off, (uint8_t *) (pTBuffer + doff), len);
#endif
#endif

#ifdef NVOCMP_GPRAM
    NVOCMP_restoreCache(vm);
#endif

    if (srcPg == dstPg)
    {
        srcPg  = NVOCMP_DECPAGE(srcPg);
        srcOff = pNvHandle->pageInfo[srcPg].offset;
    }
    pNvHandle->compactInfo.xDstOffset  = dstOff;
    pNvHandle->compactInfo.xSrcEOffset = srcOff;

    pNvHandle->compactInfo.xSrcEPage  = srcPg;
    pNvHandle->pageInfo[dstPg].offset = dstOff;

    NVOCMP_setCompactHdr(dstPg, pNvHandle->compactInfo.xSrcSPage, pNvHandle->compactInfo.xSrcSOffset, XSRCSTARTHDR);
    NVOCMP_setCompactHdr(dstPg, pNvHandle->compactInfo.xSrcEPage, pNvHandle->compactInfo.xSrcEOffset, XSRCENDHDR);

    if (srcOff <= endOff)
    {
        status |= NVOCMP_COMPACT_SRCDONE;
    }
    if (dstOff >= FLASH_PAGE_SIZE)
    {
        status |= NVOCMP_COMPACT_DSTDONE;
    }

    return (status);
}
#else
/******************************************************************************
 * @fn      NVOCMP_compact
 *
 * @brief   Local function to compact NV
 *
 * @param   pNvHandle - pointer to NV handler
 *
 * @return  NVINTF_SUCCESS or specific failure code
 */
static NVOCMP_compactStatus_t NVOCMP_compact(NVOCMP_nvHandle_t * pNvHandle)
{
    bool needScan = false;
    bool needSkip = false;
    uint16_t dstOff;
    uint16_t endOff;
    uint16_t srcOff;
    uint16_t crcOff;
    uint8_t dstPg;
    uint8_t srcPg;
    uint32_t aItem = 0;
#ifndef NVOCMP_RAM_OPTIMIZATION
#ifdef NVOCMP_GPRAM
    uint32_t vm;
    uint8_t * pTBuffer = RAM_BUFFER_ADDRESS;
#else
    uint8_t * pTBuffer = (uint8_t *) tBuffer;
#endif
#endif

#ifndef NVOCMP_GPRAM
    memset(tBuffer, 0, sizeof(tBuffer));
#endif
    // Reset Flash erase/write fail indicator
    NVOCMP_failW = NVINTF_SUCCESS;

    // Stop looking when we get to this offset
    endOff = NVOCMP_PGDATAOFS + NVOCMP_ITEMHDRLEN - 1;

#if (NVOCMP_NVPAGES == NVOCMP_NVONEP)
    srcPg  = 0;
    srcOff = pNvHandle->pageInfo[0].offset;
    dstPg  = 0;
    dstOff = NVOCMP_PGDATAOFS;
#else
    srcPg  = pNvHandle->headPage;
    srcOff = pNvHandle->pageInfo[srcPg].offset;
    dstPg  = pNvHandle->tailPage;
    dstOff = pNvHandle->pageInfo[dstPg].offset;
#endif

    NVOCMP_ALERT(false, "Compaction triggered.")

#ifdef NVOCMP_GPRAM
    NVOCMP_disableCache(&vm);
#endif
    while (srcOff > endOff)
    {
        if (NVOCMP_failW == NVINTF_SUCCESS)
        {
            NVOCMP_itemHdr_t srcHdr;
            uint16_t dataLen;
            uint16_t itemSize;

            needScan = false;
            needSkip = false;

            // Read and decompress item header
            NVOCMP_readHeader(srcPg, srcOff - NVOCMP_ITEMHDRLEN, &srcHdr, false);
            dataLen  = srcHdr.len;
            itemSize = NVOCMP_ITEMHDRLEN + dataLen;
            crcOff   = srcOff - NVOCMP_ITEMHDRLEN - dataLen;

            // Check if length is safe
            if (srcOff < (dataLen + NVOCMP_PGDATAOFS))
            {
                NVOCMP_ALERT(false, "Item header corrupted: Data length too long")
                needScan = true;
                srcOff--;
            }
            else if (NVOCMP_SIGNATURE != srcHdr.sig)
            {
                NVOCMP_ALERT(false, "Item header corrupted: Invalid signature")
                needScan = true;
                srcOff--;
            }
            else if (NVOCMP_verifyCRC(crcOff, dataLen, srcHdr.crc8, srcPg, false))
            {
                // Invalid CRC, corruption
                NVOCMP_ALERT(false, "Item CRC incorrect!")
                needScan = true;
                srcOff--;
            }
            else if (!(srcHdr.stats & NVOCMP_VALIDIDBIT) && // Item is valid
                     (srcHdr.stats & NVOCMP_ACTIVEIDBIT))   // Item is active
            {
                // Valid CRC, item is active
                srcOff -= NVOCMP_ITEMHDRLEN;
            }
            else
            {
                // Valid CRC but item is inactive
                srcOff -= NVOCMP_ITEMHDRLEN;
                needSkip = true;
            }

            if (needScan)
            {
                // Detected a problem, find next header (scan for signature)
                NVOCMP_ALERT(false, "Attempting to find signature...")
                bool foundSig = NVOCMP_findSignature(srcPg, &srcOff);
                if (!foundSig)
                {
                    // If we get here and foundSig is false, we never found another
                    // item in the page, break the loop so that any valid items
                    // that were collected up to this point get written to the
                    // destination page.
                    NVOCMP_ALERT(foundSig, "Attempt to find signature failed.")
                    break;
                }
            }
            else
            {
                if (!needSkip)
                {
                    // Get block of bytes from source page
#ifndef NVOCMP_RAM_OPTIMIZATION
#if NVOCMP_COMPR
                    NVOCMP_read(srcPg, crcOff, (uint8_t *) (pTBuffer + dstOff), itemSize);
#else
                    NVOCMP_read(srcPg, crcOff, (uint8_t *) (pTBuffer + FLASH_PAGE_SIZE - dstOff - itemSize), itemSize);
#endif
#else
                    NVOCMP_copyItem(srcPg, dstPg, crcOff, dstOff, itemSize);
#endif
                    dstOff += itemSize;
                    aItem++;
                }
                NVOCMP_ALERT(srcOff > dataLen, "Offset overflow: srcOff")
                srcOff -= dataLen;
            }
        }
        else
        {
#ifdef NVOCMP_GPRAM
            NVOCMP_restoreCache(vm);
#endif
            // Failure during item xfer makes next findItem() unreliable
            NVOCMP_ASSERT(false, "COMPACTION FAILURE")
            return (NVOCMP_COMPACT_FAILURE);
        }
    } // end of while

    if (NVOCMP_failW != NVINTF_SUCCESS)
    {
#ifdef NVOCMP_GPRAM
        NVOCMP_restoreCache(vm);
#endif
        // Something bad happened when trying to compact the page
        NVOCMP_ASSERT(false, "COMPACTION FAILURE")
        return (NVOCMP_COMPACT_FAILURE);
    }

#if (NVOCMP_NVPAGES == NVOCMP_NVONEP)
    // Get XDST page ready
    NVOCMP_failW = NVOCMP_erase(pNvHandle, dstPg);
    NVOCMP_changePageState(pNvHandle, dstPg, NVOCMP_PGXDST);
#endif

    // Write block to destination page
#ifndef NVOCMP_RAM_OPTIMIZATION
#if NVOCMP_COMPR
    uint16_t off = NVOCMP_PGDATAOFS;
    uint16_t len = dstOff - NVOCMP_PGDATAOFS;
    NVOCMP_failW |= NVOCMP_write(dstPg, off, (uint8_t *) (pTBuffer + off), len);
#else
    uint16_t off  = NVOCMP_PGDATAOFS;
    uint16_t doff = FLASH_PAGE_SIZE - dstOff;
    uint16_t len  = dstOff - NVOCMP_PGDATAOFS;
    NVOCMP_failW |= NVOCMP_write(dstPg, off, (uint8_t *) (pTBuffer + doff), len);
#endif
#endif

#ifdef NVOCMP_GPRAM
    NVOCMP_restoreCache(vm);
#endif

    pNvHandle->pageInfo[dstPg].offset  = dstOff;
    pNvHandle->compactInfo.xSrcEOffset = srcOff;
    pNvHandle->compactInfo.xDstOffset  = dstOff;

    NVOCMP_setCompactHdr(dstPg, pNvHandle->compactInfo.xSrcSPage, pNvHandle->compactInfo.xSrcSOffset, XSRCSTARTHDR);
    NVOCMP_setCompactHdr(dstPg, pNvHandle->compactInfo.xSrcEPage, pNvHandle->compactInfo.xSrcEOffset, XSRCENDHDR);

    return (NVOCMP_COMPACT_SUCCESS);
}
#endif

#if ((NVOCMP_NVPAGES > NVOCMP_NVONEP) && !defined(NVOCMP_MIGRATE_DISABLED)) || defined NVOCMP_RAM_OPTIMIZATION
/******************************************************************************
 * @fn      NVOCMP_copyItem
 *
 * @brief   Copy an NV item from active page to specified destination page
 *
 * @param   srcPg - Source page
 * @param   dstPg - Destination page
 * @param   sOfs  - Source page offset of original data in active page
 * @param   dOfs  - Destination page offset to transferred copy of the item
 * @param   len   - Length of data to copy
 *
 * @return  none.
 */
static void NVOCMP_copyItem(uint8_t srcPg, uint8_t dstPg, uint16_t sOfs, uint16_t dOfs, uint16_t len)
{
    uint16_t num;
#ifndef NVOCMP_RAM_OPTIMIZATION
    uint8_t tmp[NVOCMP_XFERBLKMAX];
#else
    uint8_t * tmp = (uint8_t *) tBuffer;
#endif

    // Copy over the data: Flash to RAM, then RAM to Flash
    while (len > 0 && !NVOCMP_failW)
    {
        // Number of bytes to transfer in this block
#ifndef NVOCMP_RAM_OPTIMIZATION
        num = (len < NVOCMP_XFERBLKMAX) ? len : NVOCMP_XFERBLKMAX;
#else
        num = (len < NVOCMP_RAM_BUFFER_SIZE) ? len : NVOCMP_RAM_BUFFER_SIZE;
#endif

        // Get block of bytes from source page
        NVOCMP_read(srcPg, sOfs, (uint8_t *) &tmp[0], num);

        // Write block to destination page
        NVOCMP_failW = NVOCMP_write(dstPg, dOfs, (uint8_t *) &tmp[0], num);

        dOfs += num;
        sOfs += num;
        len -= num;
    }
}
#endif
/******************************************************************************
 * @fn      NVOCMP_readByte
 *
 * @brief   Read one byte from Flash memory
 *
 * @param   pg  - NV Flash page
 * @param   ofs - Offset into the page
 *
 * @return  byte read from flash memory
 */
static uint8_t NVOCMP_readByte(uint8_t pg, uint16_t ofs)
{
    uint8_t byteVal;
    NVOCMP_read(pg, ofs, &byteVal, NVOCMP_ONEBYTE);

    return (byteVal);
}

/******************************************************************************
 * @fn      NVOCMP_writeByte
 *
 * @brief   Write one byte to Flash and read back to verify
 *
 * @param   pg  - NV Flash page
 * @param   ofs - offset into the page
 * @param   bwv - byte to write & verify
 *
 * @return  none ('failF' or 'failW' will be set if write fails)
 */
static void NVOCMP_writeByte(uint8_t pg, uint16_t ofs, uint8_t bwv)
{
    NVOCMP_failW = NVOCMP_write(pg, ofs, &bwv, 1);
}

/******************************************************************************
 * @fn      NVOCMP_doNVCRC
 *
 * @brief   Computes the CRC8 on the NV buffer indicated
 *          CRC code external, API in crc.h
 *
 * @param   pg - Flash page to check
 * @param   ofs - Flash page offset to lowest address item byte
 * @param   len - Item data length
 * @param   crc - value to start with, should be NULL if new calculation
 * @param   flag - fast flag (not used if NVOCMP_RAM_OPTIMIZATION is defined)
 *
 * @return  crc byte
 */
static uint8_t NVOCMP_doNVCRC(uint8_t pg, uint16_t ofs, uint16_t len, uint8_t crc, bool flag)
{
    uint16_t rdLen = 0;
#ifndef NVOCMP_RAM_OPTIMIZATION
#ifdef NVOCMP_GPRAM
    uint8_t * pTBuffer = RAM_BUFFER_ADDRESS;
#else
    uint8_t * pTBuffer = (uint8_t *) tBuffer;
#endif
#endif
    uint8_t tmp[NVOCMP_XFERBLKMAX];
    crc_t newCRC = (crc_t) crc;

    // Read flash and compute CRC in blocks
    while (len > 0)
    {
        rdLen = (len < NVOCMP_XFERBLKMAX ? len : NVOCMP_XFERBLKMAX);
#ifndef NVOCMP_RAM_OPTIMIZATION
        if (flag)
        {
            memcpy((uint8_t *) tmp, (uint8_t *) (pTBuffer + ofs), rdLen);
        }
        else
        {
#endif
            NVOCMP_read(pg, ofs, tmp, rdLen);
#ifndef NVOCMP_RAM_OPTIMIZATION
        }
#endif
        newCRC = crc_update(newCRC, tmp, rdLen);
        len -= rdLen;
        ofs += rdLen;
    }

    return (newCRC);
}

/******************************************************************************
 * @fn      NVOCMP_doRAMCRC
 *
 * @brief   Calculates CRC8 given a buffer and length
 *          CRC code external, API in crc.h
 *
 * @param   input - pointer to data buffer
 * @param   len - length of data in buffer
 * @param   crc - value to start with, should be NULL if new calculation
 *
 * @return  CRC8 byte
 */
static uint8_t NVOCMP_doRAMCRC(uint8_t * input, uint16_t len, uint8_t crc)
{
    crc_t newCRC = crc_update((crc_t) crc, input, len);

    return ((uint8_t) newCRC);
}

/******************************************************************************
 * @fn      NVOCMP_verifyCRC
 *
 * @brief   Helper function to validate item crc from NV
 *
 * @param   iOfs - offset to item data
 * @param   len - length of item data
 * @param   crc - crc to compare against
 * @param   pg - page to work on
 * @param   flag - fast flag (not used if NVOCMP_RAM_OPTIMIZATION is defined)
 *
 * @return  status byte
 */
static uint8_t NVOCMP_verifyCRC(uint16_t iOfs, uint16_t len, uint8_t crc, uint8_t pg, bool flag)
{
    uint8_t newCRC;
    uint16_t crcLen = len + NVOCMP_HDRCRCINC - 1;
#if NVOCMP_HDRLE
    uint8_t finalByte = (len >> 6) & 0x3F;
#else
    uint8_t finalByte = (len & 0x3F) << 2;
#endif
    // CRC calculations stop at the length field of header
    // So the last byte must be done separately
    newCRC = NVOCMP_doNVCRC(pg, iOfs, crcLen, 0, flag);
    newCRC = NVOCMP_doRAMCRC(&finalByte, sizeof(finalByte), newCRC);
    NVOCMP_ALERT(newCRC == crc, "Invalid CRC detected.")
#ifdef NVOCMP_STATS
    if (newCRC != crc)
    {
        NVOCMP_badCRCCount++;
    }
#endif // NVOCMP_STATS
    return (newCRC == crc ? NVINTF_SUCCESS : NVINTF_CORRUPT);
}

#ifdef ENABLE_SANITY_CHECK
/******************************************************************************
 * @fn      NVOCMP_sanityCheckApi
 *
 * @brief   Global function to perform a sanity check on the active
 *          partition to report if corruption has been detected.
 *
 * @param   none
 *
 * @return  0: No failure found.
 *          Non-zero: failure, each bit representing a particular error
 *          as indicated in NV driver status codes defined in nvintf.h.
 */
static uint32_t NVOCMP_sanityCheckApi(void)
{
    NVOCMP_nvHandle_t * pNvHandle = &NVOCMP_nvHandle;
    bool needScan                 = false;
    bool needSkip                 = false;
    uint16_t dstOff;
    uint16_t endOff;
    uint16_t srcOff;
    uint16_t crcOff;
    uint8_t dstPg;
    uint8_t srcPg;
    uint32_t ret   = NVINTF_SUCCESS;
    uint32_t aItem = 0;

    // Reset Flash erase/write fail indicator
    NVOCMP_failW = NVINTF_SUCCESS;

    // Stop looking when we get to this offset
    endOff = NVOCMP_PGDATAOFS + NVOCMP_ITEMHDRLEN - 1;

#if (NVOCMP_NVPAGES == NVOCMP_NVONEP)
    srcPg  = 0;
    srcOff = pNvHandle->pageInfo[0].offset;
    dstPg  = 0;
    dstOff = NVOCMP_PGDATAOFS;
#else
    srcPg              = pNvHandle->headPage;
    srcOff             = pNvHandle->pageInfo[srcPg].offset;
    dstPg              = pNvHandle->tailPage;
    dstOff             = pNvHandle->pageInfo[dstPg].offset;
#endif

    NVOCMP_ALERT(false, "Sanity Check")

    while (srcOff > endOff)
    {
        if (NVOCMP_failW == NVINTF_SUCCESS)
        {
            NVOCMP_itemHdr_t srcHdr;
            uint16_t dataLen;
            uint16_t itemSize;

            needScan = false;
            needSkip = false;

            // Read and decompress item header
            NVOCMP_readHeader(srcPg, srcOff - NVOCMP_ITEMHDRLEN, &srcHdr, false);
            dataLen  = srcHdr.len;
            itemSize = NVOCMP_ITEMHDRLEN + dataLen;
            crcOff   = srcOff - NVOCMP_ITEMHDRLEN - dataLen;

            // Check if length is safe
            if (srcOff < (dataLen + NVOCMP_PGDATAOFS))
            {
                NVOCMP_ALERT(false, "Item header corrupted: Data length too long")
                ret |= (1 << NVINTF_BADLENGTH);
                needScan = true;
                srcOff--;
            }
            else if (NVOCMP_SIGNATURE != srcHdr.sig)
            {
                NVOCMP_ALERT(false, "Item header corrupted: Invalid signature")
                ret |= (1 << NVINTF_NO_SIG);
                needScan = true;
                srcOff--;
            }
            else if (NVOCMP_verifyCRC(crcOff, dataLen, srcHdr.crc8, srcPg, false))
            {
                // Invalid CRC, corruption
                NVOCMP_ALERT(false, "Item CRC incorrect!")
                ret |= (1 << NVINTF_CORRUPT);
                needScan = true;
                srcOff--;
            }
            else if (!(srcHdr.stats & NVOCMP_VALIDIDBIT) && // Item is valid
                     (srcHdr.stats & NVOCMP_ACTIVEIDBIT))   // Item is active
            {
                // Valid CRC, item is active
                srcOff -= NVOCMP_ITEMHDRLEN;
            }
            else
            {
                // Valid CRC but item is inactive
                srcOff -= NVOCMP_ITEMHDRLEN;
                needSkip = true;
            }

            if (needScan)
            {
                // Detected a problem, find next header (scan for signature)
                NVOCMP_ALERT(false, "Attempting to find signature...")
                bool foundSig = NVOCMP_findSignature(srcPg, &srcOff);
                if (!foundSig)
                {
                    // If we get here and foundSig is false, we never found another
                    // item in the page, break the loop and report that corruption
                    // has been detected
                    NVOCMP_ALERT(foundSig, "Attempt to find signature failed.")
                    ret |= (1 << NVINTF_NO_SIG);
                    break;
                }
            }
            else
            {
                if (!needSkip)
                {
                    dstOff += itemSize;
                    aItem++;
                }
                NVOCMP_ALERT(srcOff > dataLen, "Offset overflow: srcOff")
                srcOff -= dataLen;
            }
        }
        else
        {
            // Failure during item xfer makes next findItem() unreliable
            NVOCMP_ASSERT(false, "SANITY CHECK FAILURE")
            ret |= (1 << NVINTF_FAILURE);
        }
    } // end of while

    if (NVOCMP_failW != NVINTF_SUCCESS)
    {
        // Something bad happened when scanning the page
        NVOCMP_ASSERT(false, "SANITY CHECK FAILURE")
        ret |= (1 << NVINTF_FAILURE);
    }

    return (ret);
}
#endif

//*****************************************************************************
