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

#include <app/AttributeValueDecoder.h>
#include <app/ConcreteAttributePath.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/TLV.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/Span.h>

namespace chip::app {

class SingleEndpointDeviceAccessor : public chip::IntrusiveListNodeBase<chip::IntrusiveMode::AutoUnlink>
{
public:
    SingleEndpointDeviceAccessor() = delete;
    SingleEndpointDeviceAccessor(SingleEndpointDevice * device) : mDevice(device) { VerifyOrDie(device != nullptr); }
    virtual ~SingleEndpointDeviceAccessor() = default;

    EndpointId GetEndpointId() const { return mDevice->GetEndpointId(); }

    /**
     * @brief Writes a value to a cluster attribute on the device.
     *
     * This method is called by the OOB framework to simulate an attribute write.
     * Implementations should decode the value using the provided decoder and
     * update the corresponding cluster attribute on the device.
     *
     * Note: Accessors should assume the endpoint ID in the path is correct (verified
     * by the registry) and do not need to validate it.
     *
     * @param path The concrete path of the attribute being written.
     * @param decoder The decoder containing the value to be written.
     * @return CHIP_NO_ERROR on success. The implementation MUST have successfully decoded
     *         the value using `decoder.Decode()`.
     *         CHIP_ERROR_NOT_IMPLEMENTED if the attribute is not supported or not handled
     *         by this accessor. This allows the framework to fall back to the default write path.
     *         Other CHIP_ERROR codes on hard failure (e.g., invalid arguments). This blocks
     *         fallback and returns an error to the client.
     */
    virtual CHIP_ERROR SetAttribute(const ConcreteDataAttributePath & path, AttributeValueDecoder & decoder)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * @brief Invokes a custom action or simulation on the device.
     *
     * This method is called by the OOB framework to trigger actions that do not
     * map directly to simple attribute writes (e.g., simulating physical hazards,
     * triggering complex state transitions).
     *
     * @param actionName The name of the action to invoke.
     * @param arguments A TLVReader positioned at the arguments for the action.
     * @return CHIP_NO_ERROR on success.
     *         CHIP_ERROR_NOT_IMPLEMENTED if the action is not supported.
     *         Other CHIP_ERROR codes on failure.
     */
    virtual CHIP_ERROR InvokeAction(CharSpan actionName, chip::TLV::TLVReader & arguments) { return CHIP_ERROR_NOT_IMPLEMENTED; }

protected:
    SingleEndpointDevice * mDevice;
};

} // namespace chip::app
