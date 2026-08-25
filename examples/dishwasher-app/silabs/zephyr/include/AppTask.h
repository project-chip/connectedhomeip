/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "AppTaskZephyr.h"

#include <app/clusters/operational-state-server/operational-state-server.h>
#include <lib/core/ClusterEnums.h>

class AppTask : public chip::Zephyr::App::AppTaskZephyr
{
public:
    using OperationalStateEnum = chip::app::Clusters::OperationalState::OperationalStateEnum;

    ~AppTask() override{};
    void PreInitMatterStack(void) override;
    void PostInitMatterStack(void) override;
    void PostInitMatterServerInstance(void) override;

    static AppTask & GetDefaultInstance();

    static void ActionInitiated(OperationalStateEnum action);
    static void ActionCompleted();

private:
};

chip::Zephyr::App::AppTaskBase & GetAppTask();

/**
 * Drive LED1 from dishwasher operational state (on=Running, blink patterns for Pause/Error).
 * No-op if LED1 is not available on the board.
 */
void UpdateOperationalStateLed(chip::app::Clusters::OperationalState::OperationalStateEnum state);

/**
 * Apply fake EPM readings for the given operational state (endpoint 2 electrical sensor).
 */
void UpdateEpmAttributesForOperationalState(chip::app::Clusters::OperationalState::OperationalStateEnum state);
