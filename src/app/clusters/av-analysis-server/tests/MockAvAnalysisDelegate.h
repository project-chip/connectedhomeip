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

#include <app/clusters/av-analysis-server/AvAnalysisCluster.h>

namespace chip {
namespace app {
namespace Clusters {

class MockAvAnalysisDelegate : public AvAnalysisDelegate
{
public:
    void ShutdownApp() override {}

    CHIP_ERROR VerifyZoneIDsAreValid(const std::vector<uint16_t> & aZoneIDs) override { return CHIP_NO_ERROR; }

    bool CanAddContextTriggers() override { return true; }

    void ActiveAmbientContextTriggersUpdated() override {}

    CHIP_ERROR PersistentAttributesLoadedCallback() override { return CHIP_NO_ERROR; }
};

} // namespace Clusters
} // namespace app
} // namespace chip
