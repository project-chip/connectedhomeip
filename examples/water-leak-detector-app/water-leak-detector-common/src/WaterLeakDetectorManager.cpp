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
    Status status = chip::app::Clusters::BooleanState::Attributes::StateValue::Set(mEndpoint, detected);
    VerifyOrReturn(Status::Success == status, ChipLogError(NotSpecified, "Failed to set BooleanState StateValue attribute"));
    ChipLogDetail(NotSpecified, "Leak status updated to: %d", detected);
}
