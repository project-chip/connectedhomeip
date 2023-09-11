/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <lib/support/Span.h>
#include <platform/nxp/k32w/common/OTATlvProcessor.h>

namespace chip {

class OTAFirmwareProcessor : public OTATlvProcessor
{
public:
    struct Descriptor
    {
        uint32_t version;
        char versionString[kVersionStringSize];
        char buildDate[kBuildDateSize];
    };

    CHIP_ERROR Init() override;
    CHIP_ERROR Clear() override;
    CHIP_ERROR ApplyAction() override;
    CHIP_ERROR AbortAction() override;
    CHIP_ERROR ExitAction() override;

private:
    CHIP_ERROR ProcessInternal(ByteSpan & block) override;
    CHIP_ERROR ProcessDescriptor(ByteSpan & block);

    OTADataAccumulator mAccumulator;
    bool mDescriptorProcessed = false;
};

} // namespace chip
