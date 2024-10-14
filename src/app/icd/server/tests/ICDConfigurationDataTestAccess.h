/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/icd/server/ICDConfigurationData.h>

namespace chip {
namespace Test {
/**
 * @brief Class acts as an accessor to private methods of the ICDConfigurationData class without needing to give friend access to
 *        each individual test.
 *        This design is necessary because certain tests validate specific edge cases around specific configurations.
 *        See ICDConfigurationData.h for more details why SetModeDurations cannot be a public API.
 */
class ICDConfigurationDataTestAccess
{
public:
    ICDConfigurationDataTestAccess() = delete;
    ICDConfigurationDataTestAccess(ICDConfigurationData * data) : mData(data) {}

    CHIP_ERROR SetModeDurations(Optional<System::Clock::Milliseconds32> activeModeDuration,
                                Optional<System::Clock::Milliseconds32> idleModeDuration)
    {
        return mData->SetModeDurations(activeModeDuration, idleModeDuration);
    }

private:
    ICDConfigurationData * mData = nullptr;
};

} // namespace Test
} // namespace chip
