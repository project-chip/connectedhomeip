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

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>

#include "OTAImageProcessorImpl.h"

#include <sys/stat.h>

namespace chip {
namespace {

size_t Sha256DigestLength(OTAImageDigestType type)
{
    switch (type)
    {
    case OTAImageDigestType::kSha256:
        return 32;
    case OTAImageDigestType::kSha256_128:
        return 16;
    case OTAImageDigestType::kSha256_120:
        return 15;
    case OTAImageDigestType::kSha256_96:
        return 12;
    case OTAImageDigestType::kSha256_64:
        return 8;
    case OTAImageDigestType::kSha256_32:
        return 4;
    default:
        return 0;
    }
}

} // namespace

CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    if (mImageFile == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Invalid output image file supplied");
        return CHIP_ERROR_INTERNAL;
    }

    return DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    return DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    if (!mPayloadVerified)
    {
        ChipLogError(SoftwareUpdate, "Image does not match the payload size or digest in its header");
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    }

    return DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    if (mImageFile == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Invalid output image file supplied");
        return CHIP_ERROR_INTERNAL;
    }

    return DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR OTAImageProcessorImpl::SuspendDownload()
{
    return DeviceLayer::PlatformMgr().ScheduleWork(HandleSuspend, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    if (!mOfs.is_open() || !mOfs.good())
    {
        return CHIP_ERROR_INTERNAL;
    }

    // Store block data for HandleProcessBlock to access
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
    }

    return DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
}

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return false;
    }

    return requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying;
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t currentVersion;
    uint32_t targetVersion = requestor->GetTargetVersion();
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentVersion));
    if (currentVersion != targetVersion)
    {
        ChipLogError(SoftwareUpdate, "Current software version = %" PRIu32 ", expected software version = %" PRIu32, currentVersion,
                     targetVersion);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandlePrepareDownload(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    if (!imageProcessor->mSuspended)
    {
        unlink(imageProcessor->mImageFile);
        imageProcessor->mParams.downloadedBytes = 0;
        imageProcessor->mParams.totalFileBytes  = 0;
        imageProcessor->mImageBytesReceived     = 0;
        imageProcessor->mPayloadDigestLength    = 0;
        imageProcessor->mHeaderParser.Init();
    }
    imageProcessor->mSuspended       = false;
    imageProcessor->mPayloadVerified = false;
    imageProcessor->mOfs.open(imageProcessor->mImageFile, std::ofstream::out | std::ofstream::ate | std::ofstream::app);
    if (!imageProcessor->mOfs.good())
    {
        TEMPORARY_RETURN_IGNORED imageProcessor->mDownloader->OnPreparedForDownload(CHIP_ERROR_OPEN_FAILED);
        return;
    }

    TEMPORARY_RETURN_IGNORED imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    imageProcessor->mOfs.close();
    TEMPORARY_RETURN_IGNORED imageProcessor->ReleaseBlock();
    imageProcessor->mPayloadVerified = imageProcessor->VerifyPayload();

    ChipLogProgress(SoftwareUpdate, "OTA image downloaded to %s", imageProcessor->mImageFile);
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    VerifyOrReturn(imageProcessor != nullptr);

    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    VerifyOrReturn(requestor != nullptr);

    // Move the downloaded image to the location where the new image is to be executed from
    unlink(kImageExecPath);
    rename(imageProcessor->mImageFile, kImageExecPath);
    chmod(kImageExecPath, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    // Shutdown the stack and expect to boot into the new image once the event loop is stopped
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t) { DeviceLayer::PlatformMgr().HandleServerShuttingDown(); });
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t) { TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().StopEventLoopTask(); });
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    imageProcessor->mOfs.close();
    unlink(imageProcessor->mImageFile);
    imageProcessor->mSuspended          = false;
    imageProcessor->mPayloadVerified    = false;
    imageProcessor->mImageBytesReceived = 0;
    TEMPORARY_RETURN_IGNORED imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleSuspend(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    imageProcessor->mOfs.close();
    imageProcessor->mSuspended = imageProcessor->mPayloadDigestLength > 0;
    LogErrorOnFailure(imageProcessor->ReleaseBlock());
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    ByteSpan block       = imageProcessor->mBlock;
    size_t receivedBytes = block.size();
    CHIP_ERROR error     = imageProcessor->ProcessHeader(block);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Image does not contain a valid header");
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INVALID_FILE_IDENTIFIER);
        return;
    }

    if (block.size() > imageProcessor->mParams.totalFileBytes - imageProcessor->mParams.downloadedBytes)
    {
        ChipLogError(SoftwareUpdate, "Image is longer than the payload size in its header");
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INVALID_FILE_IDENTIFIER);
        return;
    }

    if (!imageProcessor->mOfs.write(reinterpret_cast<const char *>(block.data()), static_cast<std::streamsize>(block.size())))
    {
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_WRITE_FAILED);
        return;
    }

    if (imageProcessor->mPayloadDigestLength > 0 && imageProcessor->mPayloadHash.AddData(block) != CHIP_NO_ERROR)
    {
        imageProcessor->mDownloader->EndDownload(CHIP_ERROR_INTERNAL);
        return;
    }

    imageProcessor->mParams.downloadedBytes += block.size();
    imageProcessor->mImageBytesReceived += receivedBytes;
    TEMPORARY_RETURN_IGNORED imageProcessor->mDownloader->FetchNextData();
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // Needs more data to decode the header
        VerifyOrReturnError(error != CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes    = header.mPayloadSize;
        const size_t digestLength = Sha256DigestLength(header.mImageDigestType);
        mPayloadHash.Clear();
        if (digestLength > 0)
        {
            VerifyOrReturnError(header.mImageDigest.size() == digestLength, CHIP_ERROR_INVALID_FILE_IDENTIFIER);
            memcpy(mPayloadDigest, header.mImageDigest.data(), digestLength);
            ReturnErrorOnFailure(mPayloadHash.Begin());
        }
        mPayloadDigestLength = digestLength;
        mHeaderParser.Clear();
    }

    return CHIP_NO_ERROR;
}

bool OTAImageProcessorImpl::VerifyPayload()
{
    if (mHeaderParser.IsInitialized() || mParams.downloadedBytes != mParams.totalFileBytes)
    {
        return false;
    }
    if (mPayloadDigestLength == 0)
    {
        return true;
    }

    uint8_t digest[Crypto::kSHA256_Hash_Length];
    MutableByteSpan digestSpan(digest);
    return mPayloadHash.Finish(digestSpan) == CHIP_NO_ERROR &&
        digestSpan.SubSpan(0, mPayloadDigestLength).data_equal(ByteSpan(mPayloadDigest, mPayloadDigestLength));
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    if (block.empty())
    {
        TEMPORARY_RETURN_IGNORED ReleaseBlock();
        return CHIP_NO_ERROR;
    }
    if (mBlock.size() < block.size())
    {
        if (!mBlock.empty())
        {
            TEMPORARY_RETURN_IGNORED ReleaseBlock();
        }
        uint8_t * mBlock_ptr = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(block.size()));
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
