/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 */

#include "app_conf.h"
#include "app_common.h"

#if (OTA_SUPPORT == 1)
/* Matter cluster */
#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
/* ST OTA implementation */
#include "OTAImageProcessorImpl.h"
#include "ota.h"

#if (OTA_EXTERNAL_FLASH_ENABLE == 1)
#include "stm_ota_spi_flash.h"
#define F_OTA_FLASH_WriteChunk      STM_OTA_SPI_FLASH_WriteChunk
#define F_OTA_FLASH_Delete_Image    STM_OTA_SPI_FLASH_Delete_Image
#else
#include "stm_ota_flash.h"
#define F_OTA_FLASH_WriteChunk      STM_OTA_FLASH_WriteChunk
#define F_OTA_FLASH_Delete_Image    STM_OTA_FLASH_Delete_Image
#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */


#define STM_HEADER_SIZE 8
static uint32_t mCPU1Size;
static uint32_t mCPU2Size; 
static bool stm_header_decoded;
static uint32_t mFlashWriteOffset;

#define TEMPBUF_SIZE 16
#define TEMPBUF_PADDING 0xFF
static uint8_t tempBUF[TEMPBUF_SIZE];
static uint32_t extra_bytes;

/* OEMiROT Magic value */
const uint32_t MagicTrailerValue[] =
{
  0xf395c277,
  0x7fefd260,
  0x0f505235,
  0x8079b62c,
};

namespace chip {

bool OTAImageProcessorImpl::WriteMagicValue(uint32_t dest)
{
    ChipLogProgress(DeviceLayer, "WriteMagicValue:  @ %p", (void*)dest);

    STM_OTA_StatusTypeDef status;
    status = F_OTA_FLASH_WriteChunk(  reinterpret_cast<uint32_t*>(dest), 
                                        reinterpret_cast<uint32_t*>(const_cast<uint32_t*>(MagicTrailerValue)), 
                                        static_cast<uint32_t>(sizeof(MagicTrailerValue)) );
    if (status != STM_OTA_FLASH_OK) 
    {
        ChipLogError(SoftwareUpdate, "Magic value write failed");
        return false;
    }

    return true;
}

bool OTAImageProcessorImpl::WriteFlashChunk(
    uint32_t dest, 
    uint8_t* pSrc, 
    uint32_t size, 
    OTAImageProcessorImpl* imageProcessor)
{
    ChipLogProgress(DeviceLayer, "WriteFlashChunk:  @ %p , size = %lu", (void*)dest, size);

    STM_OTA_StatusTypeDef status;
    status = F_OTA_FLASH_WriteChunk(  reinterpret_cast<uint32_t*>(dest), 
                                        reinterpret_cast<uint32_t*>(pSrc), 
                                        static_cast<uint32_t>(size) );
    if (status != STM_OTA_FLASH_OK) 
    {
        ChipLogError(SoftwareUpdate, "Flash write failed");
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        return false;
    }

    return true;
}

bool OTAImageProcessorImpl::DeleteImages(OTAImageProcessorImpl* imageProcessor)
{
    ChipLogProgress(DeviceLayer, "DeleteImages");
    STM_OTA_StatusTypeDef status;

    // SLOT A
    status = F_OTA_FLASH_Delete_Image(SLOT_DWL_A_START, SLOT_DWL_A_SIZE);
    if (status != STM_OTA_FLASH_OK) 
    {
        ChipLogError(SoftwareUpdate, "Delete image Slot A failed");
		imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        return false;
    }
	
    // SLOT B (optional)
    //status = STM_OTA_FLASH_Delete_Image(SLOT_DWL_B_START, SLOT_DWL_B_SIZE);
    //if (status != STM_OTA_FLASH_OK) 
    //{
    //    ChipLogError(SoftwareUpdate, "Delete image Slot B failed");
	//	imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
    //    return false;
    //}

    return true;	
}

bool OTAImageProcessorImpl::IsFirstImageRun() 
{
    OTARequestorInterface *requestor = chip::GetRequestorInstance();

    /* check pointers */
    if (requestor == nullptr) 
    {
        return false;
    }

    return (requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying);
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage() 
{
    ChipLogProgress(DeviceLayer, "OTA Confirm current image");
    OTARequestorInterface *requestor = chip::GetRequestorInstance();

    if (requestor == nullptr) 
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t currentVersion;
    uint32_t targetVersion = requestor->GetTargetVersion();
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentVersion));

    if (currentVersion != targetVersion) 
    {
        ChipLogError(SoftwareUpdate,
                "Current software version = %" PRIu32 ", expected software version = %" PRIu32,
                currentVersion, targetVersion);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload() 
{

    // Get OTA status - under what circumstances does prepared break?
    // what happens if a prepare is pending and another one is invoked
    // Should we store the state here and wait until we receive notification

    mHeaderParser.Init();

    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload,
            reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan &block) 
{
    ChipLogProgress(DeviceLayer, "OTA Process Header");
    if (mHeaderParser.IsInitialized()) 
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // Needs more data to decode the header
        VerifyOrReturnError(error != CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes = header.mPayloadSize;
        mHeaderParser.Clear();

        // Load Ota_ImageHeader_t header structure and call application callback to validate image header
        Ota_ImageHeader_t OtaImgHeader;
        this->mSwVer = header.mSoftwareVersion; // Store software version in imageProcessor as well
        OtaImgHeader.vendorId = header.mVendorId;
        OtaImgHeader.productId = header.mProductId;
        OtaImgHeader.softwareVersion = header.mSoftwareVersion;
        OtaImgHeader.minApplicableVersion = header.mMinApplicableVersion.ValueOr(0);
        OtaImgHeader.maxApplicableVersion = header.mMaxApplicableVersion.ValueOr(0);

        if (true != OtaHeaderValidation(OtaImgHeader))
        {
			ChipLogProgress(DeviceLayer, "OtaHeaderValidation error");
            return CHIP_ERROR_INCORRECT_STATE;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize() 
{
	ChipLogProgress(DeviceLayer, "OTA Finalize");
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply() 
{
    ChipLogProgress(SoftwareUpdate, "Applying - resetting device");

    //TODO call DFU module to apply the ota

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort() 
{
    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan &block) 
{
    /* check pointers */
    if ((block.data() == nullptr) || block.empty()) 
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err = ProcessHeader(block);
    ChipLogProgress(DeviceLayer, "OTA Process Block");
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Matter image header parser error: %" CHIP_ERROR_FORMAT, err.Format());
        this->mDownloader->EndDownload(CHIP_ERROR_INVALID_FILE_IDENTIFIER);
        return err;
    }

    // Store block data for HandleProcessBlock to access
    err = SetBlock(block);
    if (err != CHIP_NO_ERROR) 
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandlePrepareDownload(intptr_t context) 
{
    auto *imageProcessor = reinterpret_cast<OTAImageProcessorImpl*>(context);
    
    ChipLogProgress(DeviceLayer, "OTA Prepare DL");

    /* check pointers */    
    if (imageProcessor == nullptr) 
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    } 
    else if (imageProcessor->mDownloader == nullptr) 
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    // running this in a thread so won't block main event loop
    ChipLogProgress(SoftwareUpdate, "HandlePrepareDownload");

	/* Delete image slots */
    if (!DeleteImages(imageProcessor))
	{
        return;		
	}

    // Initialize tracking variables
    imageProcessor->mParams.downloadedBytes = 0;
	mFlashWriteOffset = 0;
	stm_header_decoded = false;
    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context) 
{
    ChipLogProgress(SoftwareUpdate, "HandleFinalize, extra_bytes=%lu", extra_bytes);
	
    auto *imageProcessor = reinterpret_cast<OTAImageProcessorImpl*>(context);
    /* check pointers */    
    if (imageProcessor == nullptr) {
        return;
    }

    // remaining extra bytes to flush ?
    if (extra_bytes != 0U)
    {
        // write last tempBUF 	
        if (!WriteFlashChunk(
                mFlashWriteOffset + SLOT_DWL_A_START,
                reinterpret_cast<std::uint8_t*>(tempBUF),
                static_cast<std::uint32_t>(TEMPBUF_SIZE),
				imageProcessor)) 
        {
            ChipLogError(SoftwareUpdate, "Flash write failed");
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        }	
    }

    imageProcessor->ReleaseBlock();
    // Start from scratch
    //imageProcessor->mParams.downloadedBytes = 0;
	mFlashWriteOffset = 0;
	extra_bytes = 0;
	stm_header_decoded = false;
	
	ChipLogProgress(SoftwareUpdate, "HandleFinalize done");
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context) 
{
    auto *imageProcessor = reinterpret_cast<OTAImageProcessorImpl*>(context);
    /* check pointers */
    if (imageProcessor == nullptr) {
        return;
    }

    ChipLogProgress(SoftwareUpdate, "HandleAbort");

	/* Delete image slots */
    if (!DeleteImages(imageProcessor))
	{
        return;		
	}

    imageProcessor->ReleaseBlock();

    // Start from scratch
    imageProcessor->mParams.downloadedBytes = 0;
	mFlashWriteOffset = 0;
    extra_bytes = 0;	
	stm_header_decoded = false;
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context) 
{
    auto *imageProcessor = reinterpret_cast<OTAImageProcessorImpl*>(context);

    /* check pointers */
    if (imageProcessor == nullptr) {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    } else if (imageProcessor->mDownloader == nullptr) {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    /* Handle block */
	// if first block received, analyze STM_HEADER
	if (!stm_header_decoded)
    { 
		// first block received, get STM_Header
        uint8_t STMHeader[STM_HEADER_SIZE];

        memcpy( STMHeader, 
                reinterpret_cast<std::uint8_t*>(imageProcessor->mBlock.data()),
                sizeof(STMHeader));

        memset(tempBUF, TEMPBUF_PADDING, TEMPBUF_SIZE);
        extra_bytes = 0U;
		
        //retrieve the cpu1/cpu2 size with STM header
        mCPU1Size = STMHeader[0] + ((STMHeader[1]) << 8) + ((STMHeader[2]) << 16)
                + ((STMHeader[3]) << 24);
        mCPU2Size = STMHeader[4] + ((STMHeader[5]) << 8) + ((STMHeader[6]) << 16)
                + ((STMHeader[7]) << 24);

		stm_header_decoded = true;
		
		//ChipLogError(SoftwareUpdate, "mCPU1Size=%lu - mCPU2Size=%lu", mCPU1Size, mCPU2Size);
		
        // check the header
        if ((mCPU1Size > SLOT_DWL_A_SIZE) || 
			(mCPU2Size != 0))
        {
            ChipLogError(SoftwareUpdate, "Flash decode failed");
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_DECODE_FAILED);
            return;
        }

        if (mCPU1Size == 0) 
        { 
			// empty image received for CPU1
			ChipLogError(SoftwareUpdate, "Empty image received");
            imageProcessor->mDownloader->EndDownload(CHIP_ERROR_DECODE_FAILED);				
			return;
        }
        else 
        {
			// internal flash requirement : destination address is 128 bits aligned
            // calculate maximum number of 128-bit blocks and remaining bytes from received data buffer
            uint32_t nb_blocks = (static_cast<std::uint32_t>(imageProcessor->mBlock.size()) - STM_HEADER_SIZE) / 16;
            extra_bytes = (static_cast<std::uint32_t>(imageProcessor->mBlock.size()) - STM_HEADER_SIZE) % 16;

			//ChipLogProgress(SoftwareUpdate, "nb_blocks=%lu extra_bytes=%lu", nb_blocks, extra_bytes);				
            // write data buffer to flash (truncated for alignment to 128 bits)
            if (nb_blocks != 0U)
            {
                // write in DWL_SLOT_A data without STM header
                if (!WriteFlashChunk(
                    mFlashWriteOffset + SLOT_DWL_A_START,
                    reinterpret_cast<std::uint8_t*>(imageProcessor->mBlock.data()) + STM_HEADER_SIZE,                    
                    static_cast<std::uint32_t>(nb_blocks * 16U),
					imageProcessor)) 
                {
                    ChipLogError(SoftwareUpdate, "Flash write failed");
                    imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);	
                    return;
                }

                mFlashWriteOffset += (nb_blocks * 16U);
				//ChipLogProgress(SoftwareUpdate, "downloadedBytes=%lu", imageProcessor->mParams.downloadedBytes);
            }
			
			imageProcessor->mParams.downloadedBytes += (static_cast<std::uint32_t>(imageProcessor->mBlock.size()) - STM_HEADER_SIZE);
			
            // copy extra bytes to temporary buffer
            if (extra_bytes != 0U)
            {
                memcpy(	tempBUF,
						reinterpret_cast<std::uint8_t*>(imageProcessor->mBlock.data()) + STM_HEADER_SIZE + (nb_blocks * 16U),
						extra_bytes );
            }
			
			//ChipLogProgress(SoftwareUpdate, "extra_bytes=%lu", extra_bytes);
        }
    } 		
    else 
    {   // new block received
		uint32_t tempBuf_complement = 0U;
		uint32_t blocksize = static_cast<std::uint32_t>(imageProcessor->mBlock.size());			
		
		// check if we have remaning bytes from previous block
	    if (extra_bytes != 0U)
        {
            // check if size of received block is enough to complete temporary buffer
            if ((blocksize + extra_bytes) < TEMPBUF_SIZE)
            {
				// received block is too small to complete temporary buffer 
				//ChipLogProgress(SoftwareUpdate, "block too small case");					
                memcpy(	tempBUF + extra_bytes,
                        reinterpret_cast<std::uint8_t*>(imageProcessor->mBlock.data()),
                        tempBuf_complement );    

				extra_bytes += blocksize;
				
				imageProcessor->mParams.downloadedBytes += static_cast<std::uint32_t>(imageProcessor->mBlock.size());					                                 
                return;
            }
			else
			{
				// complete tempBUF with the beginning of the new block
				tempBuf_complement = TEMPBUF_SIZE - extra_bytes;
				
				//ChipLogProgress(SoftwareUpdate, "tempBuf_complement=%lu", tempBuf_complement);
				memcpy(	tempBUF + extra_bytes,
						reinterpret_cast<std::uint8_t*>(imageProcessor->mBlock.data()),
						tempBuf_complement );

				//  write temporary buffer in DWL_SLOT_A
				if (!WriteFlashChunk(
						mFlashWriteOffset + SLOT_DWL_A_START,
						reinterpret_cast<std::uint8_t*>(tempBUF),
						static_cast<std::uint32_t>(TEMPBUF_SIZE),
						imageProcessor)) 
				{
					ChipLogError(SoftwareUpdate, "Flash write failed");
					imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);	              
					return;
				}

				mFlashWriteOffset += TEMPBUF_SIZE;
				memset(tempBUF, TEMPBUF_PADDING, TEMPBUF_SIZE);
			}
		}
         

        // calculate remaining bytes from new block
		// then calculate maximum number of 128-bit blocks and remaining bytes from received data buffer
        uint32_t remain_bytes = blocksize - tempBuf_complement;
        uint32_t nb_blocks = remain_bytes / 16;
        extra_bytes = remain_bytes % 16;
		
		//ChipLogProgress(SoftwareUpdate, "nb_blocks=%lu extra_bytes=%lu remain_bytes=%lu ", nb_blocks, extra_bytes, remain_bytes);
		//ChipLogProgress(SoftwareUpdate, "tempBuf_complement=%lu imageProcessor->mParams.downloadedBytes=%lu ", tempBuf_complement, imageProcessor->mParams.downloadedBytes);
		
        // write data buffer to flash (truncated for alignment to 128 bits)
        if (nb_blocks != 0U)
        {
            //write in DWL_SLOT_A 
            if (!WriteFlashChunk(
                mFlashWriteOffset + SLOT_DWL_A_START,
                reinterpret_cast<std::uint8_t*>(imageProcessor->mBlock.data()) + tempBuf_complement,                     
                static_cast<std::uint32_t>(nb_blocks * 16U),
				imageProcessor)) 
            {
				ChipLogError(SoftwareUpdate, "Flash write failed");
				imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);	
                return;
            }

            mFlashWriteOffset += (nb_blocks * 16U);
        }

		imageProcessor->mParams.downloadedBytes += static_cast<std::uint32_t>(imageProcessor->mBlock.size());
		
        // copy extra bytes to temporary buffer
        if (extra_bytes != 0U)
        {
            memcpy( tempBUF,
					reinterpret_cast<std::uint8_t*>(imageProcessor->mBlock.data()) + tempBuf_complement + (nb_blocks * 16U),
					extra_bytes );
        } 
		else			
		{
			memset(tempBUF, TEMPBUF_PADDING, TEMPBUF_SIZE);
		}
    }

    imageProcessor->mDownloader->FetchNextData();
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan &block) 
{
    if(block.empty())
    {
		ReleaseBlock();
        return CHIP_NO_ERROR;
    }

    if (mBlock.size() < block.size()) 
    {
        if (!mBlock.empty()) 
        {
            ReleaseBlock();
        }
        uint8_t *mBlock_ptr = static_cast<uint8_t*>(chip::Platform::MemoryAlloc(block.size()));
        if (mBlock_ptr == nullptr) 
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mBlock = MutableByteSpan(mBlock_ptr, block.size());
    }
    CHIP_ERROR err = CopySpanToMutableSpan(block, mBlock);
    if (err != CHIP_NO_ERROR) 
    {
        ChipLogError(SoftwareUpdate, "Cannot copy block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ReleaseBlock() 
{
    if (mBlock.data() != nullptr) 
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}

} // namespace chip
#endif
