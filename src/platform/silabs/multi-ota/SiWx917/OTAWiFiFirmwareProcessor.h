/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include <lib/support/Span.h>
#include <platform/silabs/multi-ota/OTATlvProcessor.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "sl_si91x_protocol_types.h"
#ifdef __cplusplus
}
#endif

namespace chip {

class OTAWiFiFirmwareProcessor : public OTATlvProcessor
{

public:
    CHIP_ERROR ApplyAction() override;
    CHIP_ERROR FinalizeAction() override;

private:
    uint8_t mFWchunkType                    = SL_FWUP_RPS_HEADER;
    bool mReset                             = false;
    bool mDescriptorProcessed               = false;
    static constexpr size_t kAlignmentBytes = 64;
    static constexpr size_t kBlockSize      = 1024;

    CHIP_ERROR ProcessInternal(ByteSpan & block) override;

    /**
     * This function accumulates the descriptor data from the provided block,
     * invokes the registered descriptor processing callback with the accumulated data
     *
     *  @param[in/out] block The ByteSpan containing the descriptor data to process. ByteSpan is used as output of the block
     * processing as well.
     */
    CHIP_ERROR ProcessDescriptor(ByteSpan & block);
};

} // namespace chip
