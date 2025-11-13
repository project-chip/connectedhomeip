/*
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <cstdint>
#include <set>

class PwmBackend
{
public:
    virtual bool linkHW() = 0;

    virtual void setPwmHW(size_t pwm, bool state)                     = 0;
    virtual void setPwmHW(size_t pwm, uint32_t permille)              = 0;
    virtual void setPwmHWBlink(size_t pwm, size_t onMs, size_t offMs) = 0;
    virtual void setPwmHWBreath(size_t pwm, size_t onMs)              = 0;
};

class PwmManager
{
public:
    enum EAppPwm
    {
        EAppPwm_Indication = 0,
        EAppPwm_Red,
        EAppPwm_Green,
        EAppPwm_Blue,
    };

    static PwmManager & getInstance();

    void setPwm(EAppPwm appPwm, bool state);
    void setPwm(EAppPwm appPwm, uint32_t permille);
    void setPwmBlink(EAppPwm appPwm, size_t onMs, size_t offMs);
    void setPwmBreath(EAppPwm appPwm, size_t BrathMs);

    void linkPwm(EAppPwm appPwm, size_t pwm);
    void unlinkPwm(EAppPwm appPwm);
    void unlinkPwm(size_t pwm);
    void linkBackend(PwmBackend & backend);

    PwmManager(PwmManager const &)     = delete;
    void operator=(PwmManager const &) = delete;

private:
    struct PwmLink
    {
        enum EAppPwm appPwm;
        size_t pwm;

        friend bool operator<(const PwmLink & lhs, const PwmLink & rhs)
        {
            if (lhs.appPwm < rhs.appPwm)
            {
                return true;
            }
            else if (lhs.appPwm > rhs.appPwm)
            {
                return false;
            }
            else if (lhs.pwm < rhs.pwm)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        friend bool operator>(const PwmLink & lhs, const PwmLink & rhs)
        {
            if (lhs.appPwm > rhs.appPwm)
            {
                return true;
            }
            else if (lhs.appPwm < rhs.appPwm)
            {
                return false;
            }
            else if (lhs.pwm > rhs.pwm)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };

    PwmManager();

    std::set<PwmLink> m_pwms;
    PwmBackend * m_backend;
};

#if CONFIG_WS2812_STRIP_GPIO_TELINK

class Ws2812Strip : public PwmBackend
{
public:
    static Ws2812Strip & getInstance();
    bool linkHW();

    void setPwmHW(size_t pwm, bool state);
    void setPwmHW(size_t pwm, uint32_t permille);
    void setPwmHWBlink(size_t pwm, size_t onMs, size_t offMs);
    void setPwmHWBreath(size_t pwm, size_t breathMs);

    Ws2812Strip(Ws2812Strip const &)    = delete;
    void operator=(Ws2812Strip const &) = delete;

private:
    Ws2812Strip(){};
};

#elif CONFIG_PWM

class PwmPool : public PwmBackend
{
public:
    static PwmPool & getInstance();
    bool linkHW();

    void setPwmHW(size_t pwm, bool state);
    void setPwmHW(size_t pwm, uint32_t permille);
    void setPwmHWBlink(size_t pwm, size_t onMs, size_t offMs);
    void setPwmHWBreath(size_t pwm, size_t breathMs);

    PwmPool(PwmPool const &)        = delete;
    void operator=(PwmPool const &) = delete;

private:
    PwmPool(){};
};

#else

class PwmDummy : public PwmBackend
{
public:
    static PwmDummy & getInstance();
    bool linkHW();

    void setPwmHW(size_t pwm, bool state);
    void setPwmHW(size_t pwm, uint32_t permille);
    void setPwmHWBlink(size_t pwm, size_t onMs, size_t offMs);
    void setPwmHWBreath(size_t pwm, size_t breathMs);

    PwmDummy(PwmDummy const &)       = delete;
    void operator=(PwmDummy const &) = delete;

private:
    PwmDummy(){};
};

#endif // CONFIG_WS2812_STRIP_GPIO_TELINK
