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

/**
 * @file ListScreen.cpp
 *
 * Simple list screen.
 *
 */

#include "ListScreen.h"

#if CONFIG_HAVE_DISPLAY

#include <algorithm>

namespace {

const char * buttonText[] = { "Up", "Down", "Action" };

};

std::string ListScreen::GetButtonText(int id)
{
    return buttonText[id - 1];
}

void ListScreen::Display()
{
    int i     = 0;
    int items = (DisplayHeight - ScreenTitleSafeTop - ScreenTitleSafeBottom) / ScreenFontHeight;
    if (items < model->GetItemCount())
    {
        i = std::max(0, focusIndex - items + (focusIndex == model->GetItemCount() - 1 ? 1 : 2));
    }

    for (int count = 0, y = ScreenTitleSafeTop; i < model->GetItemCount() && count < items; ++i, ++count, y += ScreenFontHeight)
    {
        tft_fg = focusIndex == i ? ScreenFocusColor : ScreenNormalColor;
        TFT_print(model->GetItemText(i).c_str(), ScreenTitleSafeTop, y);
    }
}

void ListScreen::Focus(FocusType focus)
{
    switch (focus)
    {
    case FocusType::NONE:
        hasFocus   = false;
        focusIndex = -1;
        break;
    case FocusType::BLUR:
        hasFocus = false;
        // leave focus index alone
        break;
    case FocusType::UNBLUR:
        hasFocus = true;
        // leave focus index alone
        break;
    case FocusType::NEXT:
        hasFocus = true;
        if (focusIndex == -1)
        {
            focusIndex = 0;
            break;
        }
        focusIndex = (focusIndex + 1) % model->GetItemCount(); // wraparound
        if (focusIndex == 0)
        {
            ScreenManager::FocusBack(); // try focus back if it did wrap
        }
        break;
    case FocusType::PREVIOUS:
        hasFocus = true;
        if (focusIndex == -1)
        {
            focusIndex = model->GetItemCount() - 1;
            break;
        }
        focusIndex = (focusIndex + model->GetItemCount() - 1) % model->GetItemCount(); // wraparound
        if (focusIndex == model->GetItemCount() - 1)
        {
            ScreenManager::FocusBack(); // try focus back if it did wrap
        }
        break;
    }
}

#endif // CONFIG_HAVE_DISPLAY
