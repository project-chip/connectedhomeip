/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "ThermostatUI.h"
#include "GUI.h"
#include <stdio.h>

static int8_t mCurrentTempCelsius;
static uint8_t mCurrentHumidity;

void ThermostatUI::DrawUI()
{
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
    DrawCurrentTemp(mCurrentTempCelsius);
    DrawCurrentHumidity(mCurrentHumidity);
}

void ThermostatUI::SetCurrentTemp(int8_t temp)
{
    mCurrentTempCelsius = temp;
}

void ThermostatUI::SetCurrentHumidity(uint8_t humidity)
{
    mCurrentHumidity = humidity;
}

void ThermostatUI::DrawCurrentTemp(int8_t temp)
{
    char buffer[8];
    sprintf(buffer, "%d", temp);
    GUI_DrawCircle(31, 31, 31);
    GUI_SetFont(&GUI_Font24B_1);
    GUI_DispStringAt(buffer, 10, 22);
    GUI_DispChar(176); // degree
    GUI_DispChar('C');
}

void ThermostatUI::DrawCurrentHumidity(uint8_t humidity)
{
    char buffer[8];
    sprintf(buffer, "%d", humidity);
    GUI_DrawCircle(96, 31, 31);
    GUI_SetFont(&GUI_Font24B_1);
    GUI_DispStringAt(buffer, 78, 22);
    GUI_DispChar('%');
}
