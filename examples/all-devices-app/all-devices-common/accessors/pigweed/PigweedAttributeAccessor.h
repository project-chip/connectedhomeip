/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <accessors/common/AccessorRegistry.h>
#include <lib/support/logging/CHIPLogging.h>
#include <pigweed/rpc_services/AccessInterceptor.h>

namespace chip::app {

// TODO: currently added to be able to get the TLV reader from the decoder. Options to void using TestOnly:
// A. Update chip::rpc::PigweedDebugAccessInterceptor::Write interface to take in TLV reader instead of decoder.
// B. Add a new chip::rpc::PigweedDebugAccessInterceptor::Write interface that takes in a TLV.
class TestOnlyAttributeValueDecoderAccessor
{
public:
    static TLV::TLVReader & GetReader(AttributeValueDecoder & decoder) { return decoder.mReader; }
};

class PigweedAttributeAccessor : public chip::rpc::PigweedDebugAccessInterceptor
{
public:
    PigweedAttributeAccessor()           = default;
    ~PigweedAttributeAccessor() override = default;

    std::optional<::pw::Status> Write(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder) override
    {
        TLV::TLVReader & reader = TestOnlyAttributeValueDecoderAccessor::GetReader(decoder);

        auto result = AccessorRegistry::Instance().HandleAction(OOBAccessor::kActionSetAttribute, reader, &path);
        if (result.has_value())
        {
            CHIP_ERROR err = *result;
            if (err == CHIP_NO_ERROR)
            {
                if (!decoder.TriedDecode())
                {
                    ChipLogError(Support,
                                 "OOB Accessor returned success but did not decode the value. Path: (%d, " ChipLogFormatMEI
                                 ", " ChipLogFormatMEI ")",
                                 path.mEndpointId, ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mAttributeId));
                    return ::pw::Status::FailedPrecondition();
                }
                return ::pw::OkStatus();
            }
            ChipLogError(Support, "OOB Accessor failed to write attribute: %" CHIP_ERROR_FORMAT, err.Format());
            return ::pw::Status::Internal();
        }

        return std::nullopt;
    }
};

} // namespace chip::app
