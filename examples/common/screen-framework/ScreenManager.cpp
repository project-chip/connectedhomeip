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

#include <lib/support/CHIPMem.h>

#include <cassert>
#include <string>
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

constexpr int kVLEDWidth  = 16;
constexpr int kVLEDHeight = 16;

SemaphoreHandle_t mutex;

struct Lock
{
    Lock() { xSemaphoreTakeRecursive(mutex, portMAX_DELAY); }
    ~Lock() { xSemaphoreGiveRecursive(mutex); }
};

struct VLED
{
    color_t color;
    color_t color_off;
    bool on;
    VLED(color_t color) : color(color), on(false)
    {
        color_off = color;
        color_off.r &= 0x1F;
        color_off.g &= 0x1F;
        color_off.b &= 0x1F;
    }
};

std::vector<VLED> vleds;

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
    TFT_print(s, x - (TFT_getStringWidth(s) / 2), y);
}

// Print button text in appropriate location (1 to 3).
void DisplayButtonText(int id, const char * s)
{
    tft_fg = ScreenButtonColor;
    int x  = (DisplayWidth / 2) + (id - 2) * (DisplayWidth * 3 / 10);
    PrintCentered(s, x, DisplayHeight - (ScreenTitleSafeBottom / 2)); // within ScreenTitleSafeBottom
}

void DisplayVLED(int id)
{
    TFT_fillRect(0, ScreenFontHeight * 3 / 2 + id * (kVLEDHeight + 2), kVLEDWidth, kVLEDHeight,
                 vleds[id].on ? vleds[id].color : vleds[id].color_off);
}

} // namespace

namespace ScreenManager {

void Init()
{
    mutex = xSemaphoreCreateRecursiveMutex();

    // https://github.com/loboris/ESP32_TFT_library/issues/48
    TFT_setFont(kButtonFont, nullptr);
    ScreenTitleSafeBottom = TFT_getfontheight() * 2;
    TFT_setFont(kMainFont, nullptr);
    ScreenFontHeight   = TFT_getfontheight();
    ScreenTitleSafeTop = ScreenFontHeight * 5 / 2;
}

void Display()
{
    Lock lock;

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
    TFT_print(title.c_str(), ScreenTitleSafeTop, ScreenFontHeight / 2); // within ScreenTitleSafeTop
    TFT_drawRect(ScreenTitleSafeTop, ScreenFontHeight * 3 / 2, TFT_getStringWidth(title.c_str()), 2, ScreenNormalColor);

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

    for (int i = 0; i < vleds.size(); ++i)
    {
        DisplayVLED(i);
    }

    screens.back()->Display();
}

void ButtonPressed(int id)
{
    Lock lock;
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

void PushScreen(Screen * screen)
{
    Lock lock;
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

void PopScreen()
{
    Lock lock;
    LazyDisplay lazy;

    if (screens.empty())
    {
        return;
    }

    Screen * screen = screens.back();
    screens.pop_back(); // screen is popped immediately before last exit
    screen->Exit(true); // screen is not top when exit/popped
    chip::Platform::Delete(screen);

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

void FocusBack()
{
    Lock lock;
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

int AddVLED(color_t color)
{
    Lock lock;
    int id = vleds.size();
    vleds.emplace_back(color);
    DisplayVLED(id);
    return id;
}

void SetVLED(int id, bool on)
{
    Lock lock;
    if (vleds[id].on == on)
    {
        return;
    }

    vleds[id].on = on;
    DisplayVLED(id);
    WakeDisplay();
}

void ToggleVLED(int id)
{
    Lock lock;
    vleds[id].on = !vleds[id].on;
    DisplayVLED(id);
    WakeDisplay();
}

} // namespace ScreenManager

#endif // CONFIG_HAVE_DISPLAY
