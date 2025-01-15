/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/CHIPError.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {
namespace Provision {

/**
 * @brief This class provides an interface for accessing Silabs specific provisioned data
 *
 */
class ProvisionedDataProvider
{
public:
    virtual ~ProvisionedDataProvider() = default;

#ifdef SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
    /**
     * @brief Reads the test event trigger key from NVM. If the key isn't present, returns default value if defined.
     *
     * @param[out] keySpan output buffer. Must be at least large enough for 16 bytes (key length)
     * @return CHIP_ERROR
     */
    virtual CHIP_ERROR GetTestEventTriggerKey(MutableByteSpan & keySpan) = 0;
#endif // SL_MATTER_TEST_EVENT_TRIGGER_ENABLED
};

} // namespace Provision
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
