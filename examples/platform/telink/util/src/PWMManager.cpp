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

#include "PWMManager.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(PwmManager, CONFIG_CHIP_APP_LOG_LEVEL);

PwmManager & PwmManager::getInstance()
{
    static PwmManager instance;

    return instance;
}

PwmManager::PwmManager() : m_pwms{}, m_backend(NULL) {}

void PwmManager::setPwm(EAppPwm appPwm, bool state)
{
    if (!m_backend)
    {
        return;
    }

    for (auto it = m_pwms.begin(); it != m_pwms.end(); ++it)
    {
        if (it->appPwm == appPwm)
        {
            m_backend->setPwmHW(it->pwm, state);
        }
    }
}

void PwmManager::setPwm(EAppPwm appPwm, uint32_t permille)
{
    if (!m_backend)
    {
        return;
    }

    for (auto it = m_pwms.begin(); it != m_pwms.end(); ++it)
    {
        if (it->appPwm == appPwm)
        {
            m_backend->setPwmHW(it->pwm, permille);
        }
    }
}

void PwmManager::setPwmBlink(EAppPwm appPwm, size_t onMs, size_t offMs)
{
    if (!m_backend)
    {
        return;
    }

    for (auto it = m_pwms.begin(); it != m_pwms.end(); ++it)
    {
        if (it->appPwm == appPwm)
        {
            m_backend->setPwmHWBlink(it->pwm, onMs, offMs);
        }
    }
}

void PwmManager::setPwmBreath(EAppPwm appPwm, size_t BrathMs)
{
    if (!m_backend)
    {
        return;
    }

    for (auto it = m_pwms.begin(); it != m_pwms.end(); ++it)
    {
        if (it->appPwm == appPwm)
        {
            m_backend->setPwmHWBreath(it->pwm, BrathMs);
        }
    }
}

void PwmManager::linkPwm(EAppPwm appPwm, size_t pwm)
{
    PwmLink link = { .appPwm = appPwm, .pwm = pwm };

    m_pwms.insert(link);
}

void PwmManager::unlinkPwm(EAppPwm appPwm)
{
    for (auto it = m_pwms.begin(); it != m_pwms.end();)
    {
        if (it->appPwm == appPwm)
        {
            it = m_pwms.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void PwmManager::unlinkPwm(size_t pwm)
{
    for (auto it = m_pwms.begin(); it != m_pwms.end();)
    {
        if (it->pwm == pwm)
        {
            it = m_pwms.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void PwmManager::linkBackend(PwmBackend & backend)
{
    if (backend.linkHW())
    {
        m_backend = &backend;
    }
    else
    {
        LOG_ERR("PWM backend not inited!");
    }
}

#if CONFIG_WS2812_STRIP_GPIO_TELINK

#include <zephyr_ws2812.h>

static WS2812_LED_DEFINE(led_strip);

Ws2812Strip & Ws2812Strip::getInstance()
{
    static Ws2812Strip instance;

    return instance;
}

bool Ws2812Strip::linkHW()
{
    bool result = false;

    if (ws2812_led_init(&led_strip))
    {
        LOG_INF("WS2812 LED inited");
        result = true;
    }
    else
    {
        LOG_ERR("WS2812 LED not inited!");
    }
    return result;
}

void Ws2812Strip::setPwmHW(size_t pwm, bool state)
{
    LOG_INF("PWM %u turn %s", pwm, state ? "on" : "off");
    if (!ws2812_led_set(&led_strip, pwm, state ? WS2812_LED_ON : WS2812_LED_OFF))
    {
        LOG_WRN("WS2812 LED set pwm %u failed!", pwm);
    }
}

void Ws2812Strip::setPwmHW(size_t pwm, uint32_t permille)
{
    LOG_INF("PWM %u set %u", pwm, permille);
    if (!ws2812_led_set(&led_strip, pwm, WS2812_LED_FIXED, permille))
    {
        LOG_WRN("WS2812 LED set pwm %u to %u permille failed!", pwm, permille);
    }
}

void Ws2812Strip::setPwmHWBlink(size_t pwm, size_t onMs, size_t offMs)
{
    LOG_WRN("WS2812 LED setPwmHWBlink not supported");
}

void Ws2812Strip::setPwmHWBreath(size_t pwm, size_t breathMs)
{

    LOG_WRN("WS2812 LED setPwmHWBreath not supported");
}

#elif CONFIG_PWM

#include <zephyr_pwm_pool.h>

static PWM_POOL_DEFINE(pwm_pool);

PwmPool & PwmPool::getInstance()
{
    static PwmPool instance;

    return instance;
}

bool PwmPool::linkHW()
{
    bool result = false;

    if (pwm_pool_init(&pwm_pool))
    {
        LOG_INF("PWM pool inited");
        result = true;
    }
    else
    {
        LOG_ERR("PWM pool not inited!");
    }
    return result;
}

void PwmPool::setPwmHW(size_t pwm, bool state)
{
    if (!pwm_pool_set(&pwm_pool, pwm, state ? PWM_ON : PWM_OFF))
    {
        LOG_WRN("PWM pool set pwm %u failed!", pwm);
    }
}

void PwmPool::setPwmHW(size_t pwm, uint32_t permille)
{
    if (!pwm_pool_set(&pwm_pool, pwm, PWM_FIXED, permille))
    {
        LOG_WRN("PWM pool set pwm %u to %u permille failed!", pwm, permille);
    }
}

void PwmPool::setPwmHWBlink(size_t pwm, size_t onMs, size_t offMs)
{
    if (!pwm_pool_set(&pwm_pool, pwm, PWM_BLINK, K_MSEC(onMs), K_MSEC(offMs)))
    {
        LOG_WRN("PWM pool set pwm %u blink to (%u-%u)mS failed!", pwm, onMs, offMs);
    }
}

void PwmPool::setPwmHWBreath(size_t pwm, size_t breathMs)
{
    if (!pwm_pool_set(&pwm_pool, pwm, PWM_BREATH, K_MSEC(breathMs)))
    {
        LOG_WRN("PWM pool set pwm %u breath to %umS failed!", pwm, breathMs);
    }
}

#else
// Dummy implementation
PwmDummy & PwmDummy::getInstance()
{
    static PwmDummy instance;

    return instance;
}

bool PwmDummy::linkHW()
{
    LOG_INF("PWM Dummy inited");

    return true;
}

void PwmDummy::setPwmHW(size_t pwm, bool state)
{
    LOG_INF("PWM Dummy %u turn %s", pwm, state ? "on" : "off");
}

void PwmDummy::setPwmHW(size_t pwm, uint32_t permille)
{
    LOG_INF("PWM Dummy %u set %u", pwm, permille);
}

void PwmDummy::setPwmHWBlink(size_t pwm, size_t onMs, size_t offMs)
{
    LOG_WRN("PWM Dummy setPwmHWBlink not supported");
}

void PwmDummy::setPwmHWBreath(size_t pwm, size_t breathMs)
{
    LOG_WRN("PWM Dummy setPwmHWBreath not supported");
}

#endif // CONFIG_WS2812_STRIP_GPIO_TELINK
