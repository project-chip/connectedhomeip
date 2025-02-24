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

#include <app/TestEventTriggerDelegate.h>
#include <headers/ProvisionedDataProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace chip {

class SilabsTestEventTriggerDelegate : public TestEventTriggerDelegate
{
public:
    explicit SilabsTestEventTriggerDelegate() = default;

    /**
     * @brief Configures the Silabs Test Event trigger
     *
     * @param provider pointer to the silabs provisionned data provider
     *
     * @return CHIP_ERROR CHIP_NO_ERROR, if the init was succesful
     *                    CHIP_ERROR_INVALID_ARGUMENT, if the manager input is equal to nullptr
     */
    CHIP_ERROR Init(DeviceLayer::Silabs::Provision::ProvisionedDataProvider * provider);

    /**
     * @brief Checks to see if `enableKey` provided matches value chosen by the manufacturer.
     * @param enableKey Buffer of the key to verify.
     * @return True or False.
     */
    bool DoesEnableKeyMatch(const ByteSpan & enableKey) const override;

private:
    DeviceLayer::Silabs::Provision::ProvisionedDataProvider * mProvider = nullptr;
};

} // namespace chip
