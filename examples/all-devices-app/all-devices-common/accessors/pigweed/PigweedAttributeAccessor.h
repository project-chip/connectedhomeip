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

class PigweedAttributeAccessor : public chip::rpc::PigweedDebugAccessInterceptor
{
public:
    PigweedAttributeAccessor()           = default;
    ~PigweedAttributeAccessor() override = default;

    std::optional<::pw::Status> Write(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder) override
    {
        CHIP_ERROR err = AccessorRegistry::Instance().SetAttribute(path, decoder);
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
        if (err == CHIP_ERROR_KEY_NOT_FOUND || err == CHIP_ERROR_NOT_IMPLEMENTED)
        {
            // If a null is seen the Attributes service handler moves ahead to the next accessor or datamodel provider.
            return std::nullopt;
        }
        return ::pw::Status::Internal();
    }
};

} // namespace chip::app
