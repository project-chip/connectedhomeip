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

class ClosureUI
{
public:
    // MainState enum values matching the ClosureControl cluster MainStateEnum
    enum MainState
    {
        STATE_STOPPED            = 0x00,
        STATE_MOVING             = 0x01,
        STATE_WAITING_FOR_MOTION = 0x02,
        STATE_ERROR              = 0x03,
        STATE_CALIBRATING        = 0x04,
        STATE_PROTECTED          = 0x05,
        STATE_DISENGAGED         = 0x06,
        STATE_SETUP_REQUIRED     = 0x07,
        STATE_UNKNOWN            = 0x08,
    };

    static void DrawUI(GLIB_Context_t * glibContext);
    static void SetMainState(uint8_t state);
    static void SetOverallCurrentState(const char * positionText, const char * latchText, const char * secureText,
                                       const char * speedText);

private:
    static void DrawHeader(GLIB_Context_t * glibContext);
    static void DrawFooter(GLIB_Context_t * glibContext);
    static void DrawMainState(GLIB_Context_t * glibContext);
    static void DrawOverallCurrentState(GLIB_Context_t * glibContext);
    static void DrawStateIcon(GLIB_Context_t * glibContext, MainState state);
    static void DrawFont(GLIB_Context_t * glibContext, uint8_t initial_x, uint8_t initial_y, uint8_t width, uint8_t * data,
                         uint32_t size);

    // Static variables to store the current closure state
    static MainState sMainState;
    static char sPositionText[24]; // "Position: Pedestrian" = 21 chars + null + safety margin
    static char sLatchText[20];    // "Latch: Engaged" = 14 chars + null + safety margin
    static char sSecureText[20];   // "Secure: Unknown" = 15 chars + null + safety margin
    static char sSpeedText[18];    // "Speed: Unknown" = 14 chars + null + safety margin
};
