/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "LightingManager.h"
#include "qvIO.h"
#include <lib/support/logging/CHIPLogging.h>

// initialization values for Blue in XY color space
constexpr XyColor_t kBlueXY  = { 9830, 3932 };
constexpr XyColor_t kWhiteXY = { 20495, 21563 };

// initialization values for Blue in HSV color space
constexpr HsvColor_t kBlueHSV  = { 240, 100, 255 };
constexpr HsvColor_t kWhiteHSV = { 0, 0, 255 };

// default initialization value for the light level after start
constexpr uint8_t kDefaultLevel = 1;

LightingManager LightingManager::sLight;

CHIP_ERROR LightingManager::Init()
{
    mState = kState_Off;
    mLevel = kDefaultLevel;
    mXY    = kWhiteXY;
    mHSV   = kWhiteHSV;
    mRGB   = XYToRgb(mLevel, mXY.x, mXY.y);

    return CHIP_NO_ERROR;
}

bool LightingManager::IsTurnedOn()
{
    return mState == kState_On;
}

uint8_t LightingManager::GetLevel()
{
    return mLevel;
}

void LightingManager::SetCallbacks(LightingCallback_fn aActionInitiated_CB, LightingCallback_fn aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool LightingManager::InitiateAction(Action_t aAction, int32_t aActor, uint16_t size, uint8_t * value)
{
    // TODO: this function is called InitiateAction because we want to implement some features such as ramping up here.
    bool action_initiated = false;
    State_t new_state;
    XyColor_t xy;
    HsvColor_t hsv;
    CtColor_t ct;

    switch (aAction)
    {
    case ON_ACTION:
        ChipLogProgress(NotSpecified, "LightMgr:ON: %s->ON", mState == kState_On ? "ON" : "OFF");
        break;
    case OFF_ACTION:
        ChipLogProgress(NotSpecified, "LightMgr:OFF: %s->OFF", mState == kState_On ? "ON" : "OFF");
        break;
    case LEVEL_ACTION:
        ChipLogProgress(NotSpecified, "LightMgr:LEVEL: lev:%u->%u", mLevel, *value);
        break;
    case COLOR_ACTION_XY:
        xy = *reinterpret_cast<XyColor_t *>(value);
        ChipLogProgress(NotSpecified, "LightMgr:COLOR: xy:%u|%u->%u|%u", mXY.x, mXY.y, xy.x, xy.y);
        break;
    case COLOR_ACTION_HSV:
        hsv = *reinterpret_cast<HsvColor_t *>(value);
        ChipLogProgress(NotSpecified, "LightMgr:COLOR: hsv:%u|%u->%u|%u", mHSV.h, mHSV.s, hsv.h, hsv.s);
        break;
    case COLOR_ACTION_CT:
        ct.ctMireds = *reinterpret_cast<uint16_t *>(value);
        ChipLogProgress(NotSpecified, "LightMgr:COLOR: ct:%u->%u", mCT.ctMireds, ct.ctMireds);
        break;
    default:
        ChipLogProgress(NotSpecified, "LightMgr:Unknown");
        break;
    }

    // Initiate On/Off Action only when the previous one is complete.
    if (mState == kState_Off && aAction == ON_ACTION)
    {
        action_initiated = true;
        new_state        = kState_On;
    }
    else if (mState == kState_On && aAction == OFF_ACTION)
    {
        action_initiated = true;
        new_state        = kState_Off;
    }
    else if (aAction == LEVEL_ACTION && *value != mLevel)
    {
        action_initiated = true;
        if (*value == 0)
        {
            new_state = kState_Off;
        }
        else
        {
            new_state = kState_On;
        }
    }
    else if (aAction == COLOR_ACTION_XY)
    {
        action_initiated = true;
        if (xy.x == 0 && xy.y == 0)
        {
            new_state = kState_Off;
        }
        else
        {
            new_state = kState_On;
        }
    }
    else if (aAction == COLOR_ACTION_HSV)
    {
        action_initiated = true;
        if (hsv.h == 0 && hsv.s == 0)
        {
            new_state = kState_Off;
        }
        else
        {
            new_state = kState_On;
        }
    }

    if (action_initiated)
    {
        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction);
        }
        if (aAction == LEVEL_ACTION)
        {
            SetLevel(*value);
        }
        else if (aAction == COLOR_ACTION_XY)
        {
            SetColor(xy.x, xy.y);
        }
        else if (aAction == COLOR_ACTION_HSV)
        {
            SetColor(hsv.h, hsv.s);
        }
        else if (aAction == COLOR_ACTION_CT)
        {
            SetColorTemperature(ct);
        }
        else
        {
            Set(new_state == kState_On);
        }

        if (mActionCompleted_CB)
        {
            mActionCompleted_CB(aAction);
        }
    }

    return action_initiated;
}

void LightingManager::SetLevel(uint8_t aLevel)
{
    mLevel = aLevel;
    mRGB   = XYToRgb(mLevel, mXY.x, mXY.y);
    UpdateLight();
}

void LightingManager::SetColor(uint16_t x, uint16_t y)
{
    mXY.x = x;
    mXY.y = y;
    mRGB  = XYToRgb(mLevel, mXY.x, mXY.y);
    UpdateLight();
}

void LightingManager::SetColor(uint8_t hue, uint8_t saturation)
{
    mHSV.h = hue;
    mHSV.s = saturation;
    mHSV.v = mLevel; // use level from Level Cluster as Vibrance parameter
    mRGB   = HsvToRgb(mHSV);
    UpdateLight();
}

void LightingManager::SetColorTemperature(CtColor_t ct)
{
    mCT  = ct;
    mRGB = CTToRgb(ct);
    UpdateLight();
}

void LightingManager::Set(bool aOn)
{
    if (aOn)
    {
        mState = kState_On;
    }
    else
    {
        mState = kState_Off;
    }
    UpdateLight();
}

void LightingManager::UpdateLight()
{
    ChipLogProgress(NotSpecified, "UpdateLight: %d L:%d R:%d G:%d B:%d", mState, mLevel, mRGB.r, mRGB.g, mRGB.b);
    qvIO_PWMSetColor(mRGB.r, mRGB.g, mRGB.b);
    qvIO_PWMColorOnOff(mState == kState_On);
}
