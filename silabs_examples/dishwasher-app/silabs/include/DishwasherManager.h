/*
 *
 *    Copyright (c) 2019 Google LLC.
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

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af-enums.h>
#include <lib/core/ClusterEnums.h>

#include <lib/core/CHIPError.h>

using namespace chip::app::Clusters::OperationalState;

class DishwasherManager
{
public:
    CHIP_ERROR Init();
    void CycleOperationalState();
    void UpdateDishwasherLed();
    void UpdateOperationState(OperationalStateEnum state);
    OperationalStateEnum GetOperationalState();

    typedef void (*Callback_fn_initiated)(OperationalStateEnum action);
    typedef void (*Callback_fn_completed)();
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

private:
    friend DishwasherManager & DishwasherMgr(void);
    OperationalStateEnum mState;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    static DishwasherManager sDishwasher;
};

inline DishwasherManager & DishwasherMgr(void)
{
    return DishwasherManager::sDishwasher;
}
