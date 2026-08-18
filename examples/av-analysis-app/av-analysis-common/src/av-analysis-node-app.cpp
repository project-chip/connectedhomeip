/*
 *
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

#include "av-analysis-node-app.h"

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

CHIP_ERROR AvAnalysisNodeApp::Init()
{
    ChipLogProgress(AppServer, "AvAnalysisNodeApp: Init on endpoint %u", mEndpointId);

    return CHIP_NO_ERROR;
}

void AvAnalysisNodeApp::Shutdown()
{
    ChipLogProgress(AppServer, "AvAnalysisNodeApp: Shutdown");
}

} // namespace app
} // namespace chip
