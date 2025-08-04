/*
 *
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

#include <ButtonManager.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ButtonManager, CONFIG_CHIP_APP_LOG_LEVEL);

ButtonManager & ButtonManager::getInstance()
{
    static ButtonManager instance;

    return instance;
}

ButtonManager::ButtonManager() : m_events{} {}

void ButtonManager::addCallback(void (*callback)(void), size_t button, bool pressed)
{
    Event event = { .button = button, .pressed = pressed, .callback = callback };

    m_events.insert(event);
}

void ButtonManager::rmCallback(void (*callback)(void))
{
    for (auto it = m_events.begin(); it != m_events.end();)
    {
        if (it->callback == callback)
        {
            it = m_events.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ButtonManager::rmCallback(size_t button, bool pressed)
{
    for (auto it = m_events.begin(); it != m_events.end();)
    {
        if (it->button == button && it->pressed == pressed)
        {
            it = m_events.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ButtonManager::linkBackend(ButtonBackend & backend)
{
    if (!backend.linkHW(onButton, this))
    {
        LOG_ERR("Button backend not inited!");
    }
}

void ButtonManager::onButton(size_t button, bool pressed, void * buttonMgr)
{
    ButtonManager * buttonManager = static_cast<ButtonManager *>(buttonMgr);

    for (auto it = buttonManager->m_events.begin(); it != buttonManager->m_events.end(); ++it)
    {
        if (it->button == button && it->pressed == pressed && it->callback)
        {
            it->callback();
        }
    }
}

#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE

#include <zephyr_key_pool.h>

static KEY_POOL_DEFINE(key_pool);

ButtonPool & ButtonPool::getInstance()
{
    static ButtonPool instance;

    return instance;
}

bool ButtonPool::linkHW(void (*on_button_change)(size_t button, bool pressed, void * context), void * context)
{
    bool result = false;

    if (key_pool_init(&key_pool))
    {
        key_pool_set_callback(&key_pool, on_button_change, context);
        LOG_INF("Key pool inited");
        result = true;
    }
    else
    {
        LOG_ERR("Key pool not inited!");
    }
    return result;
}

#else

#include <zephyr_key_matrix.h>

static KEY_MATRIX_DEFINE(key_matrix);

ButtonMatrix & ButtonMatrix::getInstance()
{
    static ButtonMatrix instance;

    return instance;
}

bool ButtonMatrix::linkHW(void (*on_button_change)(size_t button, bool pressed, void * context), void * context)
{
    bool result = false;

    if (key_matrix_init(&key_matrix))
    {
        key_matrix_set_callback(&key_matrix, on_button_change, context);
        LOG_INF("Key matrix inited");
        result = true;
    }
    else
    {
        LOG_ERR("Key matrix not inited!");
    }
    return result;
}

#endif // CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
