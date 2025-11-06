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

#include "ClosureUIStrings.h"
#include "glib.h"
#include "lcd.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/closure-control-server/closure-control-cluster-objects.h>
#include <app/data-model/Nullable.h>

/**
 * @brief Structure to hold closure data needed for UI display
 */
struct ClosureUIData
{
    chip::app::Clusters::ClosureControl::MainStateEnum mainState;
    chip::app::DataModel::Nullable<chip::app::Clusters::ClosureControl::GenericOverallCurrentState> overallCurrentState;
};

struct ClosureUITextInitializer;

class ClosureUI
{
public:
    static void DrawUI(GLIB_Context_t * glibContext);
    static void SetMainState(chip::app::Clusters::ClosureControl::MainStateEnum state);
    static void SetOverallCurrentState(const char * positionText, const char * latchText, const char * secureText,
                                       const char * speedText);

    static void FormatAndSetPosition(const char * suffix);
    static void FormatAndSetLatch(const char * suffix);
    static void FormatAndSetSecure(const char * suffix);
    static void FormatAndSetSpeed(const char * suffix);

private:
    friend struct ClosureUITextInitializer;
    static void DrawHeader(GLIB_Context_t * glibContext);
    static void DrawFooter(GLIB_Context_t * glibContext);
    static void DrawMainState(GLIB_Context_t * glibContext);
    static void DrawOverallCurrentState(GLIB_Context_t * glibContext);

    // Static variables to store the current closure state
    static chip::app::Clusters::ClosureControl::MainStateEnum sMainState;
    static char sPositionText[ClosureUIStrings::BUFFER_SIZE_POSITION];
    static char sLatchText[ClosureUIStrings::BUFFER_SIZE_LATCH];
    static char sSecureText[ClosureUIStrings::BUFFER_SIZE_SECURE];
    static char sSpeedText[ClosureUIStrings::BUFFER_SIZE_SPEED];
    static char sStateText[ClosureUIStrings::BUFFER_SIZE_STATE];
};
