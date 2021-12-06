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

/* This file contains the declarations for OTAImageProcessor, a platform-agnostic
 * interface for processing downloaded chunks of OTA image data.
 * Each platform should provide an implementation of this interface.
 */

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>

class OTAImageProcessorImpl
{
public:
    // Open file, find block of space in persistent memory, or allocate a buffer, etc.
    CHIP_ERROR PrepareDownload();

    // Must not be a blocking call to support cases that require IO to elements such as // external peripherals/radios
    CHIP_ERROR ProcessBlock(chip::ByteSpan & data);

    // Close file, close persistent storage, etc
    CHIP_ERROR Finalize();

    // Clean up the download which could mean erasing everything that was written,
    // releasing buffers, etc.
    CHIP_ERROR Abort();
};
