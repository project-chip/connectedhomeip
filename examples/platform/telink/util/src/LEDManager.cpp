/*
 *
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

#include "LEDManager.h"
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LedManager, CONFIG_CHIP_APP_LOG_LEVEL);

LedManager & LedManager::getInstance()
{
    static LedManager instance;

    return instance;
}

LedManager::LedManager() : m_leds{}, m_backend(NULL) {}

void LedManager::setLed(EAppLed appLed, bool state)
{
    if (!m_backend)
    {
        return;
    }

    for (auto it = m_leds.begin(); it != m_leds.end(); ++it)
    {
        if (it->appLed == appLed)
        {
            m_backend->setLedHW(it->led, state);
        }
    }
}

void LedManager::setLed(EAppLed appLed, size_t onMs, size_t offMs)
{
    if (!m_backend)
    {
        return;
    }

    for (auto it = m_leds.begin(); it != m_leds.end(); ++it)
    {
        if (it->appLed == appLed)
        {
            m_backend->setLedHW(it->led, onMs, offMs);
        }
    }
}

void LedManager::linkLed(EAppLed appLed, size_t led)
{
    LedLink link = { .appLed = appLed, .led = led };

    m_leds.insert(link);
}

void LedManager::unlinkLed(EAppLed appLed)
{
    for (auto it = m_leds.begin(); it != m_leds.end();)
    {
        if (it->appLed == appLed)
        {
            it = m_leds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void LedManager::unlinkLed(size_t led)
{
    for (auto it = m_leds.begin(); it != m_leds.end();)
    {
        if (it->led == led)
        {
            it = m_leds.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void LedManager::linkBackend(LedBackend & backend)
{
    if (backend.linkHW())
    {
        m_backend = &backend;
    }
    else
    {
        LOG_ERR("LED backend not inited!");
    }
}

#include <zephyr_led_pool.h>

static LED_POOL_DEFINE(led_pool);

LedPool & LedPool::getInstance()
{
    static LedPool instance;

    return instance;
}

bool LedPool::linkHW()
{
    bool result = false;

    if (led_pool_init(&led_pool))
    {
        LOG_INF("LED pool inited");
        result = true;
    }
    else
    {
        LOG_ERR("LED pool not inited!");
    }
    return result;
}

void LedPool::setLedHW(size_t led, bool state)
{
    if (!led_pool_set(&led_pool, led, state ? LED_ON : LED_OFF))
    {
        LOG_WRN("LED pool set led %u failed!", led);
    }
}

void LedPool::setLedHW(size_t led, size_t onMs, size_t offMs)
{
    if (!led_pool_set(&led_pool, led, LED_BLINK, K_MSEC(onMs), K_MSEC(offMs)))
    {
        LOG_WRN("LED pool set led %u failed!", led);
    }
}
