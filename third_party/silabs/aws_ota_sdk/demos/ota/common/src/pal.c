/*
 * OTA PAL V2.0.1 for POSIX
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */



/* OTA PAL implementation for POSIX platform. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <libgen.h>
#include <unistd.h>
#include "silabs_utils.h"

#ifdef EFR32MG24 //For efr32 NCP combos
#include "btl_interface.h"
#include "em_bus.h" // For CORE_CRITICAL_SECTION
#if (defined(EFR32MG24) && defined(WF200_WIFI))
#include "sl_wfx_host_api.h"
#include "spi_multiplex.h"
#endif // EFR32MG24 && WF200_WIFI
#else
#ifdef __cplusplus
extern "C" {
#endif
#include "sl_si91x_driver.h"
#include "sl_si91x_hal_soc_soft_reset.h"
#ifdef __cplusplus
}
#endif // __cplusplus
#endif // EFR32MG24

#include "ota.h"
#include "pal.h"

/**
 * @brief Size of buffer used in file operations on this platform (POSIX).
 */
#define OTA_PAL_POSIX_BUF_SIZE           ( ( size_t ) 4096U )

/**
 * @brief Name of the file used for storing platform image state.
 */
#define OTA_PLATFORM_IMAGE_STATE_FILE    "PlatformImageState.txt"

#ifdef SIWX_917
#define SL_STATUS_FW_UPDATE_DONE ((sl_status_t)0x10003)
#define SL_FWUP_RPS_HEADER  1
#define SL_FWUP_RPS_CONTENT 2

static uint8_t flag = SL_FWUP_RPS_HEADER;
bool reset_flag = false;
#endif // Siwx917

/**
 * @brief Specify the OTA signature algorithm we support on this platform.
 */
const char OTA_JsonFileSignatureKey[ OTA_FILE_SIG_KEY_STR_MAX_LENGTH ] = "sig-sha256-ecdsa";

OtaPalStatus_t otaPal_Abort( OtaFileContext_t * const C )
{
    /* Set default return status to uninitialized. */
    OtaPalMainStatus_t mainErr = OtaPalSuccess;
    int32_t subErr = 0;
    return OTA_PAL_COMBINE_ERR( mainErr, subErr );
}

OtaPalStatus_t otaPal_CreateFileForRx( OtaFileContext_t * const C )
{
    OtaPalStatus_t result = OTA_PAL_COMBINE_ERR( OtaPalSuccess, 0 );
    // /* Exiting function without calling fclose. Context file handle state is managed by this API. */
    return result;
}

OtaPalStatus_t otaPal_CloseFile( OtaFileContext_t * const C )
{
    OtaPalMainStatus_t mainErr = OtaPalSuccess;
    OtaPalSubStatus_t subErr = 0;
    return OTA_PAL_COMBINE_ERR( mainErr, subErr );
}

static bool bl_init_done = false;

#ifdef EFR32MG24
int16_t otaPal_WriteBlock_efr32( OtaFileContext_t * const C,
                           uint32_t ulOffset,
                           uint8_t * const pcData,
                           uint32_t ulBlockSize )
{
    int32_t filerc = 0;
    int32_t err = 0;
    uint32_t const kAlignmentBytes = 64;
    uint8_t mSlotId;
    static uint32_t mWriteOffset;
    uint16_t writeBufOffset = 0;

    static uint64_t downloadedBytes;
    
    uint32_t blockReadOffset = 0;
    uint8_t writeBuffer[64] = { 0 };
    if (!bl_init_done)
    {
     SILABS_LOG("otaPal_WriteBlock bootloader Init");
     CORE_CRITICAL_SECTION(bootloader_init();)
     bl_init_done = true;
    }
    mSlotId = 0; // Single slot until we support multiple images
    downloadedBytes = 0;

    if(err != 0){
        filerc = -1;
        SILABS_LOG("otaPal_WriteBlock bootloader Init Failed %d",err);
        return -1;
    }

    while (blockReadOffset < ulBlockSize)
    {
        writeBuffer[writeBufOffset] = *(pcData + blockReadOffset);
        writeBufOffset++;
        blockReadOffset++;
        if (writeBufOffset == kAlignmentBytes)
        {
            SILABS_LOG("packets mWriteOffset %d, blockReadOffset %d writeBufOffset %d", mWriteOffset, blockReadOffset, writeBufOffset);
            writeBufOffset = 0;

#if (defined(EFR32MG24) && defined(WF200_WIFI))
           sl_wfx_host_pre_bootloader_spi_transfer();
#endif
            CORE_CRITICAL_SECTION(err = bootloader_eraseWriteStorage(mSlotId, mWriteOffset, writeBuffer, kAlignmentBytes);)
#if (defined(EFR32MG24) && defined(WF200_WIFI))
            sl_wfx_host_post_bootloader_spi_transfer();
#endif
            if (err)
            {
                SILABS_LOG("otaPal_WriteBlock bootloader_eraseWriteStorage FAILED: %ld", err);
                filerc = -1;
            }
            mWriteOffset += kAlignmentBytes;
            downloadedBytes += kAlignmentBytes;
            filerc = ( int32_t ) ulBlockSize;
        }
        else if ((blockReadOffset == ulBlockSize) && ulBlockSize!=1024)
        {
            SILABS_LOG("last packet mWriteOffset %d, blockReadOffset %d writeBufOffset %d", mWriteOffset, blockReadOffset, writeBufOffset);
            if (writeBufOffset != 0)
            {
                // Account for last bytes of the image not yet written to storage
                downloadedBytes += writeBufOffset;
                while (writeBufOffset != kAlignmentBytes)
                {
                    writeBuffer[writeBufOffset] = 0;
                    writeBufOffset++;
                }
                SILABS_LOG("while loop final reminder (blockReadOffset == ulBlockSize)writeBufOffset %d",writeBufOffset);
        #if (defined(EFR32MG24) && defined(WF200_WIFI))
                sl_wfx_host_pre_bootloader_spi_transfer();
        #endif
                CORE_CRITICAL_SECTION(err = bootloader_eraseWriteStorage(mSlotId, mWriteOffset, writeBuffer, kAlignmentBytes);)
        #if (defined(EFR32MG24) && defined(WF200_WIFI))
                sl_wfx_host_post_bootloader_spi_transfer();
        #endif
                if (err)
                {
                    SILABS_LOG("ERROR: In HandleFinalize bootloader_eraseWriteStorage() error %ld", err);
                    return -1;
                }
                mWriteOffset += writeBufOffset;
                filerc = ( int32_t ) ulBlockSize;
            }
        }
    }

    SILABS_LOG("otaPal_WriteBlock bootloader Init END:");
    filerc = ( int32_t ) ulBlockSize;
    return ( int16_t ) filerc;
}

#else
int16_t otaPal_WriteBlock_siwx917( OtaFileContext_t * const C,
                           uint32_t ulOffset,
                           uint8_t * const pcData,
                           uint32_t ulBlockSize )
{

    int32_t status = 0;
    int32_t filerc = 0;
    uint32_t const kAlignmentBytes = 64;
    uint8_t mSlotId;
    static uint32_t mWriteOffset;
    uint16_t writeBufOffset = 0;

    static uint64_t downloadedBytes;
    
    uint32_t blockReadOffset = 0;
    uint8_t writeBuffer[64] = { 0 };
 
    mSlotId = 0; // Single slot until we support multiple images
    downloadedBytes = 0;

    while (blockReadOffset < ulBlockSize)
    {
        writeBuffer[writeBufOffset] = *(pcData + blockReadOffset);
        writeBufOffset++;
        blockReadOffset++;
        if (writeBufOffset == kAlignmentBytes)
        {
            SILABS_LOG("packets mWriteOffset %d, blockReadOffset %d writeBufOffset %d", mWriteOffset, blockReadOffset, writeBufOffset);
            writeBufOffset = 0;
            if(flag == SL_FWUP_RPS_HEADER)
            {
                // Send RPS header which is received as first chunk
                status = sl_si91x_fwup_start(writeBuffer);

                // Send RPS content
                status = sl_si91x_fwup_load(writeBuffer, kAlignmentBytes);

                flag = SL_FWUP_RPS_CONTENT;
            }
            else if(flag == SL_FWUP_RPS_CONTENT)
            {
                // Send RPS content
                status = sl_si91x_fwup_load(writeBuffer, kAlignmentBytes);
	            if (status != SL_STATUS_OK) {
                    if (status == SL_STATUS_FW_UPDATE_DONE) {
                        reset_flag = true;
                    }
                }
                else
                {
                    SILABS_LOG("ERROR: In HandleProcessBlock for middle chunk sl_si91x_fwup_load error %ld", status);
                    return -1;
                }
            } 
            mWriteOffset += kAlignmentBytes;
            downloadedBytes += kAlignmentBytes;
            filerc = ( int32_t ) ulBlockSize;
        }
        else if ((blockReadOffset == ulBlockSize) && ulBlockSize!=1024)
        {
            SILABS_LOG("last packet mWriteOffset %d, blockReadOffset %d writeBufOffset %d", mWriteOffset, blockReadOffset, writeBufOffset);
            if (writeBufOffset != 0)
            {
                // Account for last bytes of the image not yet written to storage
                downloadedBytes += writeBufOffset;
          
                if(flag == SL_FWUP_RPS_CONTENT)
                {
                    // Send RPS content
                    status = sl_si91x_fwup_load(writeBuffer, writeBufOffset);
                    SILABS_LOG("status: 0x%lX", status);
	                if (status != SL_STATUS_OK) {
                        if (status == SL_STATUS_FW_UPDATE_DONE) {
                            reset_flag = true;
                        }
                    }
                    else
                    {
                        SILABS_LOG("ERROR: In HandleProcessBlock for last chunk sl_si91x_fwup_load error %ld", status);
                        return -1;
                    }
                }
            }
            mWriteOffset += writeBufOffset;
            filerc = ( int32_t ) ulBlockSize;
        }
    }
    filerc = ( int32_t ) ulBlockSize;
    return ( int16_t ) filerc;
}
#endif

int16_t otaPal_WriteBlock( OtaFileContext_t * const C,
                           uint32_t ulOffset,
                           uint8_t * const pcData,
                           uint32_t ulBlockSize )
{
#ifdef EFR32MG24
    return otaPal_WriteBlock_efr32( C,ulOffset,pcData,ulBlockSize );
#else
    return otaPal_WriteBlock_siwx917( C,ulOffset,pcData,ulBlockSize );
#endif
}


/* Return no error. POSIX implementation simply does nothing on activate. */

#ifdef EFR32MG24
OtaPalStatus_t otaPal_ActivateNewImage_efr32( OtaFileContext_t * const C )
{
    ( void ) C;

    uint8_t mSlotId = 0;
    int32_t err = 0;
    SILABS_LOG("otaPal_ActivateNewImage OTAImageProcessorImpl::HandleApply()");

#if (defined(EFR32MG24) && defined(WF200_WIFI))
    sl_wfx_host_pre_bootloader_spi_transfer();
#endif
    CORE_CRITICAL_SECTION(err = bootloader_verifyImage(mSlotId, NULL);)
    if (err != 0)
    {
        SILABS_LOG("otaPal_ActivateNewImage ERROR: bootloader_verifyImage() error %ld", err);
        return OTA_PAL_COMBINE_ERR( OtaPalFileAbort, 0 );
    }

    CORE_CRITICAL_SECTION(err = bootloader_setImageToBootload(mSlotId);)
    if (err != 0)
    {
        SILABS_LOG("otaPal_ActivateNewImage ERROR: bootloader_setImageToBootload() error %ld", err);
        return OTA_PAL_COMBINE_ERR( OtaPalFileAbort, 0 );
    }

    // This reboots the device
    CORE_CRITICAL_SECTION(bootloader_rebootAndInstall();)

    return OTA_PAL_COMBINE_ERR( OtaPalSuccess, 0 );
}

#else
OtaPalStatus_t otaPal_ActivateNewImage_siwx917( OtaFileContext_t * const C )
{
    ( void ) C;
    if(reset_flag){
        SILABS_LOG("M4/TA/combined image update completed - reset started");
        sl_si91x_soc_soft_reset();
    }
    return OTA_PAL_COMBINE_ERR( OtaPalSuccess, 0 );
}
#endif

OtaPalStatus_t otaPal_ActivateNewImage( OtaFileContext_t * const C )
{
#ifdef EFR32MG24
    return otaPal_ActivateNewImage_efr32( C );
#else
    return otaPal_ActivateNewImage_siwx917( C );
#endif
}

/* Set the final state of the last transferred (final) OTA file (or bundle).
 * On POSIX, the state of the OTA image is stored in PlatformImageState.txt. */
OtaPalStatus_t otaPal_SetPlatformImageState( OtaFileContext_t * const C,
                                             OtaImageState_t eState )
{
    OtaPalMainStatus_t mainErr = OtaPalSuccess;
    int32_t subErr = 0;
     // /* Allow calls to fopen and fclose in this context. */
    return OTA_PAL_COMBINE_ERR( mainErr, subErr );
}

OtaPalStatus_t otaPal_ResetDevice( OtaFileContext_t * const C )
{
    ( void ) C;
    // /* Return no error.  POSIX implementation does not reset device. */
    return OTA_PAL_COMBINE_ERR( OtaPalSuccess, 0 );
}

/* Get the state of the currently running image.
 *
 * On POSIX, this is simulated by looking for and reading the state from
 * the PlatformImageState.txt file in the current working directory.
 *
 * We read this at OTA_Init time so we can tell if the MCU image is in self
 * test mode. If it is, we expect a successful connection to the OTA services
 * within a reasonable amount of time. If we don't satisfy that requirement,
 * we assume there is something wrong with the firmware and reset the device,
 * causing it to rollback to the previous code. On POSIX, this is not
 * fully simulated as there is no easy way to reset the simulated device.
 */
OtaPalImageState_t otaPal_GetPlatformImageState( OtaFileContext_t * const C )
{
    OtaPalImageState_t ePalState = OtaPalImageStateValid;
    return ePalState;
}

/*-----------------------------------------------------------*/
