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

#include "OTAImageProcessorLinux.h"

namespace chip {

CHIP_ERROR OTAImageProcessorLinux::PrepareDownload()
{
    if (mParams.imageFile == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Invalid output file supplied");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mOfs.open(mParams.imageFile, std::ofstream::out | std::ofstream::ate | std::ofstream::app);
    if (!mOfs.good())
    {
        return CHIP_ERROR_OPEN_FAILED;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorLinux::Finalize()
{
    mOfs.close();
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorLinux::Abort()
{
    mOfs.close();
    remove(mParams.imageFile);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorLinux::ProcessBlock(ByteSpan & block)
{
    if (!mOfs.is_open() || !mOfs.good())
    {
        return CHIP_ERROR_INTERNAL;
    }

    // TODO: Process block header if any

    if (!mOfs.write(reinterpret_cast<const char *>(block.data()), static_cast<std::streamsize>(block.size())))
    {
        return CHIP_ERROR_WRITE_FAILED;
    }

    mParams.downloadedBytes += block.size();

    return CHIP_NO_ERROR;
}

} // namespace chip
