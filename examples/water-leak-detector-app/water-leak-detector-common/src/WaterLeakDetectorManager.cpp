/*
 *   Copyright (c) 2025 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "WaterLeakDetectorManager.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>

// TODO: Ideally we should not depend on the codegen integration
// It would be best if we could use generic cluster API instead
#include <app/clusters/boolean-state-server/CodegenIntegration.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::Protocols::InteractionModel;

WaterLeakDetectorManager WaterLeakDetectorManager::sInstance;

WaterLeakDetectorManager * WaterLeakDetectorManager::GetInstance()
{
    return &sInstance;
}

void WaterLeakDetectorManager::InitInstance(EndpointId endpoint)
{
    sInstance.mEndpoint = endpoint;
}

void WaterLeakDetectorManager::OnLeakDetected(bool detected)
{
    auto booleanState = BooleanState::FindClusterOnEndpoint(sInstance.mEndpoint);
    VerifyOrReturn(booleanState != nullptr);
    booleanState->SetStateValue(detected);
    ChipLogDetail(NotSpecified, "Leak status updated to: %d", detected);
}
