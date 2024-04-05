/*
 *    Copyright (c) 2021 Project CHIP Authors
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

class LedBackend
{
public:
    virtual bool linkHW()                                        = 0;
    virtual void setLedHW(size_t led, bool state)                = 0;
    virtual void setLedHW(size_t led, size_t onMs, size_t offMs) = 0;
};

class LedManager
{
public:
    enum EAppLed
    {
        EAppLed_Status = 0,
        EAppLed_App0,
        EAppLed_App1,
    };

    static LedManager & getInstance();

    void setLed(EAppLed appLed, bool state);
    void setLed(EAppLed appLed, size_t onMs, size_t offMs);

    void linkLed(EAppLed appLed, size_t led);
    void unlinkLed(EAppLed appLed);
    void unlinkLed(size_t led);
    void linkBackend(LedBackend & backend);

    LedManager(LedManager const &)     = delete;
    void operator=(LedManager const &) = delete;

private:
    struct LedLink
    {
        enum EAppLed appLed;
        size_t led;

        friend bool operator<(const LedLink & lhs, const LedLink & rhs)
        {
            if (lhs.appLed < rhs.appLed)
            {
                return true;
            }
            else if (lhs.appLed > rhs.appLed)
            {
                return false;
            }
            else if (lhs.led < rhs.led)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        friend bool operator>(const LedLink & lhs, const LedLink & rhs)
        {
            if (lhs.appLed > rhs.appLed)
            {
                return true;
            }
            else if (lhs.appLed < rhs.appLed)
            {
                return false;
            }
            else if (lhs.led > rhs.led)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    LedManager();

    std::set<LedLink> m_leds;
    LedBackend * m_backend;
};

class LedPool : public LedBackend
{
public:
    static LedPool & getInstance();
    bool linkHW();
    void setLedHW(size_t led, bool state);
    void setLedHW(size_t led, size_t onMs, size_t offMs);

    LedPool(LedPool const &)        = delete;
    void operator=(LedPool const &) = delete;

private:
    LedPool(){};
};
