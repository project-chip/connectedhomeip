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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterfaceRegistry.h>

namespace bridge {

/**
 * A AttributeAccessInterface that explicitly overrides an existing AttributeAccessInterface
 * that is set up as "AllEndpoints" and restricts it to "Endpoint 0"
 *
 * This can be used as a shim to replace a "handle all endpoints" to "handle endpoint 0 only"
 */
class RootEndpointOnlyAccessInterface : public chip::app::AttributeAccessInterface
{
public:
    RootEndpointOnlyAccessInterface(chip::ClusterId clusterId) :
        chip::app::AttributeAccessInterface(chip::MakeOptional(chip::kRootEndpointId), clusterId), mClusterId(clusterId)
    {}

    /// Overrides any existing interface on the root endpoint and replaces it with `self`
    CHIP_ERROR Init();

    /// Removes self from the registered interfaces and re-registers the old interface
    CHIP_ERROR Shutdown();

    // AttributeAccessInterface Methods: everything is forwarded to the original interface
    CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override
    {
        VerifyOrReturnError(mOriginalAttributeInterface != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return mOriginalAttributeInterface->Read(aPath, aEncoder);
    }
    CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
    {
        VerifyOrReturnError(mOriginalAttributeInterface != nullptr, CHIP_ERROR_INCORRECT_STATE);
        return mOriginalAttributeInterface->Write(aPath, aDecoder);
    }

    void OnListWriteBegin(const chip::app::ConcreteAttributePath & aPath) override
    {
        VerifyOrReturn(mOriginalAttributeInterface != nullptr);
        return mOriginalAttributeInterface->OnListWriteBegin(aPath);
    }

    void OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override
    {
        VerifyOrReturn(mOriginalAttributeInterface != nullptr);
        return mOriginalAttributeInterface->OnListWriteEnd(aPath, aWriteWasSuccessful);
    }

private:
    const chip::ClusterId mClusterId;
    chip::app::AttributeAccessInterface * mOriginalAttributeInterface = nullptr;
};

} // namespace bridge
