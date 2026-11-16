/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "AppFactoryData.h"

namespace chip::app::AllDevices {

template <class Derived>
class NxpAppTask : public AppTaskBase<Derived>
{
public:
    // Registers the NXP factory data providers when CONFIG_CHIP_FACTORY_DATA=y,
    // otherwise the example DAC and device instance info providers.
    CHIP_ERROR InitCredentials() { return chip::NXP::App::AppFactoryData_PostMatterStackInit(); }
};

class AppTask final : public NxpAppTask<AppTask>
{
};

} // namespace chip::app::AllDevices
