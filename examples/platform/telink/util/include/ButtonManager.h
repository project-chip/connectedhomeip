/*
 *    Copyright (c) 2021-2023 Project CHIP Authors
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

#include <cstddef>
#include <set>

class ButtonBackend
{
public:
    virtual bool linkHW(void (*on_button_change)(size_t button, bool pressed, void * context), void * context) = 0;
};

class ButtonManager
{
public:
    static ButtonManager & getInstance();
    void addCallback(void (*callback)(void), size_t button, bool pressed);
    void rmCallback(void (*callback)(void));
    void rmCallback(size_t button, bool pressed);
    void linkBackend(ButtonBackend & backend);

    ButtonManager(ButtonManager const &)  = delete;
    void operator=(ButtonManager const &) = delete;

private:
    struct Event
    {
        size_t button;
        bool pressed;
        void (*callback)(void);

        friend bool operator<(const Event & lhs, const Event & rhs)
        {
            if (lhs.button < rhs.button)
            {
                return true;
            }
            else if (lhs.button > rhs.button)
            {
                return false;
            }
            else if (lhs.pressed < rhs.pressed)
            {
                return true;
            }
            else if (lhs.pressed > rhs.pressed)
            {
                return false;
            }
            else if (lhs.callback < rhs.callback)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        friend bool operator>(const Event & lhs, const Event & rhs)
        {
            if (lhs.button > rhs.button)
            {
                return true;
            }
            else if (lhs.button < rhs.button)
            {
                return false;
            }
            else if (lhs.pressed > rhs.pressed)
            {
                return true;
            }
            else if (lhs.pressed < rhs.pressed)
            {
                return false;
            }
            else if (lhs.callback > rhs.callback)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    ButtonManager();

    static void onButton(size_t button, bool pressed, void * buttonMgr);

    std::set<Event> m_events;
};

#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE

class ButtonPool : public ButtonBackend
{
public:
    static ButtonPool & getInstance();
    bool linkHW(void (*on_button_change)(size_t button, bool pressed, void * context), void * context);

    ButtonPool(ButtonPool const &)     = delete;
    void operator=(ButtonPool const &) = delete;

private:
    ButtonPool(){};
};

#else

class ButtonMatrix : public ButtonBackend
{
public:
    static ButtonMatrix & getInstance();
    bool linkHW(void (*on_button_change)(size_t button, bool pressed, void * context), void * context);

    ButtonMatrix(ButtonMatrix const &)   = delete;
    void operator=(ButtonMatrix const &) = delete;

private:
    ButtonMatrix(){};
};

#endif // CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
