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

#pragma once

#include <lib/core/Optional.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <platform/nxp/k32w/common/OTATlvProcessor.h>
#include <platform/nxp/k32w/k32w0/K32W0FactoryDataProvider.h>

namespace chip {

using FactoryProvider = DeviceLayer::K32W0FactoryDataProvider;
using Tags            = FactoryProvider::FactoryDataId;

/**
 * OTA custom payload that uses Matter TLVs.
 * The custom payload is used when factory data needs updating.
 * Factory data will be encoded using Matter TLV format to make
 * use of the ChipTlv reader. A payload contains metadata (size of
 * TLVs) and the TLVs themselves contained in a structure.
 * If no factory data need to be updated, the metadata will be 0
 */
struct OTAFactoryPayload
{
    Optional<ByteSpan> mCertDacKey;
    Optional<ByteSpan> mCertDac;
    Optional<ByteSpan> mCertPai;
    Optional<ByteSpan> mCertDeclaration;

    void Clear()
    {
        mCertDacKey.ClearValue();
        mCertDac.ClearValue();
        mCertPai.ClearValue();
        mCertDeclaration.ClearValue();
    }
};

class OTAFactoryDataProcessor : public OTATlvProcessor
{
public:
    CHIP_ERROR Init() override;
    CHIP_ERROR Clear() override;
    CHIP_ERROR ApplyAction() override;
    CHIP_ERROR AbortAction() override;

private:
    CHIP_ERROR ProcessInternal(ByteSpan & block) override;
    CHIP_ERROR DecodeTlv();
    CHIP_ERROR Update(uint8_t tag, Optional<ByteSpan> & optional);

    CHIP_ERROR Read();
    CHIP_ERROR Backup();
    CHIP_ERROR Restore();
    void ClearBuffer();
    CHIP_ERROR UpdateValue(uint8_t tag, ByteSpan & newValue);

    OTAFactoryPayload mPayload;
    OTADataAccumulator mAccumulator;
    uint8_t * mFactoryData = nullptr;
};

} // namespace chip
