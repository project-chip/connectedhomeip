/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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

/* definition needed to ensure the OAD image header has room for the ecdsa sig */
#define SECURITY
#include <ti/common/cc26xx/oad/oad_image_header.h>

// clang-format off

#define MY_APP_SOFTWARE_VER   { '0', '0', '0', '1' }

#if defined(__GNUC__)

/* provided by the linkerfile */
extern char _flash_end_address;
extern char _intvecs_base_address;

#define FLASH_END_ADDRESS        ((uint32_t)(&_flash_end_address))
#define RESET_VECT_ADDRESS       ((uint32_t)(&_intvecs_base_address))

const imgHdr_t __attribute__((section( ".oad_image_header"))) __attribute__((used)) oad_image_header =
#else

#error "compiler currently note supported"

#endif

{
    .fixedHdr = {
        .imgID      = OAD_IMG_ID_VAL,           // User-defined Image Identification bytes
        .crc32      = DEFAULT_CRC,              // Image's 32-bit CRC value
        .bimVer     = BIM_VER,                  // BIM version
        .metaVer    = META_VER,                 // Metadata version
        .techType   = OAD_WIRELESS_TECH_THREAD, // Wireless protocol type BLE/TI-MAC/ZIGBEE etc.
        .imgCpStat  = DEFAULT_STATE,            // Image copy status
        .crcStat    = DEFAULT_STATE,            // CRC status
        .imgType    = OAD_IMG_TYPE_APP,         // Image Type
        .imgNo      = 0x0,                      // Image number of 'image type'
        .imgVld     = DEFAULT_STATE,            // Image validation bytes, used by BIM.
        .len        = INVALID_LEN,              // Image length in bytes.
        .prgEntry   = RESET_VECT_ADDRESS,       // Program entry address
        .softVer    = MY_APP_SOFTWARE_VER,      // Software version of the image
        .imgEndAddr = FLASH_END_ADDRESS,        // Address of the last byte of a contiguous image
        .hdrLen     = OAD_IMG_FULL_HDR_LEN,     // Total length of the image header
        .rfu        = 0xFFFF,                   // Reserved bytes
    },
#if (defined(SECURITY))
    .secInfoSeg = {
        .segTypeSecure  = IMG_SECURITY_SEG_ID,      // Segment type - for Secuirty info payload
        .wirelessTech   = OAD_WIRELESS_TECH_THREAD, // Wireless technology type
        .verifStat      = DEFAULT_STATE,            // Verification status
        .secSegLen      = SECURITY_SEG_LEN,         // Payload segment length
        .secVer         = SECURITY_VER,             // Security version
        .secTimestamp   = 0x0,                      // Security timestamp, filled by python script
        .secSignerInfo  = {0},                      // filled by python script
        /* .eccSign,                                    filled by python script */
    },
#endif
    .imgPayload = {
        .segTypeImg     = IMG_PAYLOAD_SEG_ID,               // Segment type - for Contiguous image payload
        .wirelessTech   = OAD_WIRELESS_TECH_THREAD,         // Wireless technology type
        .rfu            = 0,                                // Reserved bytes
        .imgSegLen      = 0,                                // filled by python script
        .startAddr      = (uint32_t)(&oad_image_header),    // Start address of image on internal flash
    },
};
