/*
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

#include <app/data-model/EncodableToTLV.h>
#include <clusters/NetworkCommissioning/Enums.h>
#include <clusters/NetworkCommissioning/Commands.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace app {
namespace Clusters {
namespace NetworkCommissioning {

/// Handles common encode tasks for scan response encoding
class ScanResponseEncoder : public chip::app::DataModel::EncodableToTLV
{
public:
    ScanResponseEncoder(NetworkCommissioningStatusEnum status, CharSpan debugText) :
        mStatus(status),
        mDebugText(debugText)
    {}

    CHIP_ERROR EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex aAccessingFabricIndex) const override
    {
        return EncodeTo(writer, tag);
    }

    CHIP_ERROR EncodeTo(TLV::TLVWriter & writer, TLV::Tag tag) const override;

protected:
    virtual CHIP_ERROR EncodeList(TLV::TLVWriter &writer) const = 0;
    virtual Commands::ScanNetworksResponse::Fields GetResultField() const = 0;

    NetworkCommissioningStatusEnum GetStatus() const { return mStatus; }

private:
    NetworkCommissioningStatusEnum mStatus;
    CharSpan mDebugText;
};

} // namespace NetworkCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
