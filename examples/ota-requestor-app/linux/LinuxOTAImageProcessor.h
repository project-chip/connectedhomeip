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

/* This file contains the decalarions for the Linux implementation of the 
 * the OTAImageProcessorDriver interface class
 */

#include "app/clusters/ota-requestor/OTAImageProcessor.h"

class LinuxOTAImageProcessor : public OTAImageProcessorDriver
{

    // Virtuial functions from OTAImageProcessorDriver -- start
    // Open file, find block of space in persistent memory, or allocate a buffer, etc.
    CHIP_ERROR PrepareDownload() { return CHIP_NO_ERROR; }

    // Must not be a blocking call to support cases that require IO to elements such as // external peripherals/radios
    CHIP_ERROR ProcessBlock(chip::ByteSpan & data) { return CHIP_NO_ERROR; }

    // Close file, close persistent storage, etc
    CHIP_ERROR Finalize() { return CHIP_NO_ERROR; }

    chip::Optional<uint8_t> PercentComplete() { return chip::Optional<uint8_t>(0); }

    // Clean up the download which could mean erasing everything that was written,
    // releasing buffers, etc.
    CHIP_ERROR Abort() { return CHIP_NO_ERROR; }

    // Virtuial functions from OTAImageProcessorDriver -- end
};
