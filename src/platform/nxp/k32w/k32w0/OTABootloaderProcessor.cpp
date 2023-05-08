/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/TLV.h>
#include <lib/support/BufferReader.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/nxp/k32w/k32w0/CHIPDevicePlatformConfig.h>
#include <platform/nxp/k32w/k32w0/OTABootloaderProcessor.h>

#include "OtaSupport.h"
#include "OtaUtils.h"

namespace chip {

CHIP_ERROR OTABootloaderProcessor::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTABootloaderProcessor::Clear()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTABootloaderProcessor::ProcessInternal(ByteSpan & block)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTABootloaderProcessor::ApplyAction()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTABootloaderProcessor::AbortAction()
{
    return CHIP_NO_ERROR;
}

} // namespace chip
