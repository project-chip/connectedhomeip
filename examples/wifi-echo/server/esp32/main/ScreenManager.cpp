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
 * @file ScreenManager.cpp
 *
 * Simple screen manager.
 *
 */

#include "ScreenManager.h"

#if CONFIG_HAVE_DISPLAY

#include <cassert>
#include <vector>

uint16_t ScreenFontHeight;
uint16_t ScreenTitleSafeTop;
uint16_t ScreenTitleSafeBottom;

color_t ScreenNormalColor = { 255, 255, 255 };
color_t ScreenFocusColor  = { 128, 128, 255 };
color_t ScreenButtonColor = { 64, 64, 64 };

namespace {

constexpr int kMainFont   = DEJAVU24_FONT;
constexpr int kButtonFont = DEJAVU18_FONT;

std::vector<Screen *> screens;

bool focusBack = false;

int lazyDisplay   = 0;
bool dirtyDisplay = false;

struct LazyDisplay
{
    LazyDisplay() { ++lazyDisplay; }
    ~LazyDisplay()
    {
        if (--lazyDisplay == 0)
        {
            if (dirtyDisplay)
            {
                ScreenManager::Display();
                dirtyDisplay = false;
            }
        }
    }
};

// Print text centered horizontally at x.
void PrintCentered(const char * s, int x, int y)
{
    TFT_print((char *) s, x - (TFT_getStringWidth((char *) s) / 2), y);
}

// Print button text in appropriate location (1 to 3).
void DisplayButtonText(int id, const char * s)
{
    tft_fg = ScreenButtonColor;
    int x  = (DisplayWidth / 2) + (id - 2) * (DisplayWidth * 3 / 10);
    PrintCentered(s, x, DisplayHeight - (ScreenTitleSafeBottom / 2)); // within ScreenTitleSafeBottom
}

}; // namespace

void ScreenManager::Init()
{
    // https://github.com/loboris/ESP32_TFT_library/issues/48
    TFT_setFont(kButtonFont, nullptr);
    ScreenTitleSafeBottom = TFT_getfontheight() * 2;
    TFT_setFont(kMainFont, nullptr);
    ScreenFontHeight   = TFT_getfontheight();
    ScreenTitleSafeTop = ScreenFontHeight * 5 / 2;
}

void ScreenManager::Display()
{
    if (lazyDisplay)
    {
        dirtyDisplay = true;
        return;
    }

    TFT_fillScreen(TFT_BLACK);
    TFT_setFont(kMainFont, nullptr);

    if (screens.empty())
    {
        tft_fg = TFT_RED;
        PrintCentered("No Screen", DisplayWidth / 2, DisplayHeight / 2);
        return;
    }

    if (screens.size() > 1)
    {
        tft_fg = focusBack ? ScreenFocusColor : ScreenNormalColor;
        TFT_print("<", ScreenFontHeight, ScreenFontHeight / 2);
    }

    std::string title = screens.back()->GetTitle();
    tft_fg            = ScreenNormalColor;
    TFT_print((char *) title.c_str(), ScreenTitleSafeTop, ScreenFontHeight / 2); // within ScreenTitleSafeTop
    TFT_drawRect(ScreenTitleSafeTop, ScreenFontHeight * 3 / 2, TFT_getStringWidth((char *) title.c_str()), 2, ScreenNormalColor);

    TFT_setFont(kButtonFont, nullptr);
    if (screens.back()->IsFocusable())
    {
        DisplayButtonText(1, screens.back()->GetButtonText(1).c_str());
        DisplayButtonText(2, screens.back()->GetButtonText(2).c_str());
    }
    if (focusBack)
    {
        DisplayButtonText(3, "Back");
    }
    else if (screens.back()->IsFocusable())
    {
        DisplayButtonText(3, screens.back()->GetButtonText(3).c_str());
    }
    TFT_setFont(kMainFont, nullptr);

    screens.back()->Display();
}

void ScreenManager::ButtonPressed(int id)
{
    LazyDisplay lazy;

    if (screens.empty())
    {
        return;
    }

    if (focusBack && id == 3)
    {
        PopScreen();
    }
    else if (screens.back()->IsFocusable())
    {
        switch (id)
        {
        case 1:
            focusBack = false;
            screens.back()->Focus(Screen::FocusType::PREVIOUS);
            break;
        case 2:
            focusBack = false;
            screens.back()->Focus(Screen::FocusType::NEXT);
            break;
        case 3:
            screens.back()->Action();
            break;
        }
        Display();
    }
}

void ScreenManager::PushScreen(Screen * screen)
{
    LazyDisplay lazy;

    if (!screens.empty())
    {
        if (screens.back()->IsFocusable())
        {
            screens.back()->Focus(Screen::FocusType::BLUR);
        }
        screens.back()->Exit(false);
    }

    screen->Enter(true);       // screen is not top when enter/pushed
    screens.push_back(screen); // screen is pushed immediately after first enter

    focusBack = false;

    if (screens.back()->IsFocusable())
    {
        screens.back()->Focus(Screen::FocusType::NEXT);
    }
    else
    {
        focusBack = true;
    }

    Display();
}

void ScreenManager::PopScreen()
{
    LazyDisplay lazy;

    if (screens.empty())
    {
        return;
    }

    Screen * screen = screens.back();
    screens.pop_back(); // screen is popped immediately before last exit
    screen->Exit(true); // screen is not top when exit/popped
    delete screen;

    focusBack = false;

    if (!screens.empty())
    {
        screens.back()->Enter(false);
        if (screens.back()->IsFocusable())
        {
            screens.back()->Focus(Screen::FocusType::UNBLUR);
        }
        else
        {
            focusBack = true;
        }
    }

    Display();
}

void ScreenManager::FocusBack()
{
    if (screens.size() > 1)
    {
        focusBack = true;
        if (screens.back()->IsFocusable())
        {
            screens.back()->Focus(Screen::FocusType::NONE);
        }
    }
    else
    {
        focusBack = false;
    }
}

#endif // CONFIG_HAVE_DISPLAY
