/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "LowPowerManager.h"

#include <app/util/af.h>

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>

bool LowPowerManager::proxySleepRequest()
{
    // TODO: Insert code here
    return true;
}

bool emberAfLowPowerClusterSleepCallback()
{
    bool success         = LowPowerManager().proxySleepRequest();
    EmberAfStatus status = success ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
