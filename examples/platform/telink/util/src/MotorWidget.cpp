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

#include "MotorWidget.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/pm/policy.h>

// LOG_MODULE_REGISTER(MotorWidget);
LOG_MODULE_DECLARE(Motor, CONFIG_CHIP_APP_LOG_LEVEL);

#define MOTOR_MAX_ON_TIME 3000 // 5s

// const struct gpio_dt_spec sMotorSleepDt = GPIO_DT_SPEC_GET(DT_ALIAS(motorsleep), gpios);
// const struct gpio_dt_spec sMotorIn1Dt = GPIO_DT_SPEC_GET(DT_ALIAS(motorin1), gpios);
// const struct gpio_dt_spec sMotorIn2Dt = GPIO_DT_SPEC_GET(DT_ALIAS(motorin2), gpios);
static const struct gpio_dt_spec sMotorSleepDt    = GPIO_DT_SPEC_GET(DT_ALIAS(motorsleep), gpios);
static const struct gpio_dt_spec sMotorStallingDt = GPIO_DT_SPEC_GET(DT_ALIAS(motordetect), gpios);
static const struct gpio_dt_spec sMotorLimte1Dt   = GPIO_DT_SPEC_GET(DT_ALIAS(motorlimited1), gpios);
static const struct gpio_dt_spec sMotorLimte2Dt   = GPIO_DT_SPEC_GET(DT_ALIAS(motorlimited2), gpios);

// static const struct gpio_dt_spec sMotorButton = GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios);

// static gpio_dt_spec sMotorIn1Dt = GPIO_DT_SPEC_GET(DT_ALIAS(motorin1), gpios);
static const struct pwm_dt_spec sMotorIn1Dt = PWM_DT_SPEC_GET(DT_ALIAS(pwmmotorin1));
// static gpio_dt_spec sMotorIn2Dt = GPIO_DT_SPEC_GET(DT_ALIAS(motorin2), gpios);
static const struct pwm_dt_spec sMotorIn2Dt = PWM_DT_SPEC_GET(DT_ALIAS(pwmmotorin2));

static MotorWidget::MotorWidgetStateUpdateHandler sStateUpdateCallback;
static MotorWidget::MotorWidgetStateUpdateHandler sStopCallback;

// 给新加的计时器使用
static MotorWidget::MotorWidgetStateUpdateHandler sMotorStopCallback;

// static bool skipDetectStalling = false;
static bool motorState = true;
bool isMotorStopped    = true;

static PWMDevice mPwmMotorIn1;
static PWMDevice mPwmMotorIn2;

MotorWidget MotorWidget::sInstance;

// void stalling_detected(const struct device * dev, struct gpio_callback * cb, uint32_t pins);

void MotorWidget::SetCallback(MotorWidgetStateUpdateHandler stateUpdateCb, MotorWidgetStopHandler stopCb)
{
    LOG_INF("SetCallback");
    if (stopCb)
        sStopCallback = stopCb;
    if (stateUpdateCb)
        sStateUpdateCallback = stateUpdateCb;
}

void MotorWidget::Init(void)
{
    mState      = false;
    isMotorStop = true;

    int ret = gpio_pin_configure_dt(&sMotorSleepDt, GPIO_OUTPUT_ACTIVE);
    if (ret < 0)
    {
        LOG_ERR("GPIO pin %d configure - fail. Status%d\n", sMotorSleepDt.pin, ret);
    }

    // Stalling detect
    ret = gpio_pin_configure_dt(&sMotorStallingDt, GPIO_INPUT | GPIO_PULL_DOWN);
    if (ret < 0)
    {
        LOG_ERR("Config in pin err: %d", ret);
        return ret;
    }
    ret = gpio_pin_interrupt_configure_dt(&sMotorStallingDt, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Config irq pin err: %d", ret);
        return ret;
    }

    // Limit detected
    ret = gpio_pin_configure_dt(&sMotorLimte1Dt, GPIO_INPUT | GPIO_PULL_DOWN);
    if (ret < 0)
    {
        LOG_ERR("Config in pin err: %d", ret);
        return ret;
    }
    ret = gpio_pin_interrupt_configure_dt(&sMotorLimte1Dt, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Config irq pin err: %d", ret);
        return ret;
    }

    ret = gpio_pin_configure_dt(&sMotorLimte2Dt, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0)
    {
        LOG_ERR("Config in pin err: %d", ret);
        return ret;
    }
    ret = gpio_pin_interrupt_configure_dt(&sMotorLimte2Dt, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret < 0)
    {
        LOG_ERR("Config irq pin err: %d", ret);
        return ret;
    }

    if (gpio_pin_get_dt(&sMotorLimte2Dt) == 1)
    {
        motorState = false;
        LOG_INF("---motorState = false;");
    }
    ret = gpio_pin_configure_dt(&sMotorLimte2Dt, GPIO_INPUT | GPIO_PULL_DOWN);
    if (ret < 0)
    {
        LOG_ERR("Config in pin err: %d", ret);
        return ret;
    }
    // 判断完限位开关的状态将电位拉低，防止功耗异常

    // gpio_init_callback(&mLimit2_cb_data, &MotorWidget::stalling_detected, BIT(sMotorLimte2Dt.pin));
    // ret = gpio_add_callback(sMotorLimte2Dt.port, &mLimit2_cb_data);
    // if (ret < 0)
    // {
    //     LOG_ERR("Config gpio_init_callback err: %d", ret);
    //     return ret;
    // }

    // if (gpio_pin_get_dt(&sMotorLimte2Dt) == 1)
    // {
    //     motorState = false;
    //     LOG_INF("---motorState = false;");
    // }

    CHIP_ERROR err;
    err = mPwmMotorIn1.Init(&sMotorIn1Dt, 80, 200, 80);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Green IDENTIFY PWM Device Init fail");
        return err;
    }
    mPwmMotorIn1.SetCallbacks(nullptr, nullptr, ActionMotorStateUpdateHandler);

    err = mPwmMotorIn2.Init(&sMotorIn2Dt, 80, 200, 80);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Green IDENTIFY PWM Device Init fail");
        return err;
    }
    mPwmMotorIn2.SetCallbacks(nullptr, nullptr, ActionMotorStateUpdateHandler);

    k_timer_init(&mMotorTimer, &MotorWidget::MotorStopTimerHandler, nullptr);
    k_timer_user_data_set(&mMotorTimer, this);
    // k_timer_init(&mSkipDetectStallTimer, &MotorWidget::SkipDetectStallTimerHandler, nullptr);
    // k_timer_user_data_set(&mSkipDetectStallTimer, this);

    // 初始化轮询使电机停转的计时器
    // k_timer_init(&mMotorStopTimer, &MotorWidget::MotorStopPollTimerHandler, nullptr);
    // k_timer_user_data_set(&mMotorStopTimer, this);

    Sleep(false);
}

void MotorWidget::stalling_detected(const struct device * dev, struct gpio_callback * cb, uint32_t pins)
{
    return;
    if ((BIT(sMotorLimte1Dt.pin) & pins) && (dev == sMotorLimte1Dt.port))
    {
        // MotorWidget & sInstance = MotorWidgetInst();
        // sInstance.MotorStop();
        LOG_INF("Motor limited Up detected!");
        if (sStopCallback)
            sStopCallback(&sInstance);

        int ret = gpio_pin_configure_dt(&sMotorLimte1Dt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }

        ret = gpio_pin_configure_dt(&sMotorStallingDt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }
    }
    else if ((BIT(sMotorLimte2Dt.pin) & pins) && (dev == sMotorLimte2Dt.port))
    {
        // MotorWidget & sInstance = MotorWidgetInst();
        // sInstance.MotorStop();
        LOG_INF("Motor limited Down detected!");
        if (sStopCallback)
            sStopCallback(&sInstance);

        int ret = gpio_pin_configure_dt(&sMotorLimte2Dt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }

        ret = gpio_pin_configure_dt(&sMotorStallingDt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }
    }
    // else if ((BIT(sMotorStallingDt.pin) & pins) && (dev == sMotorStallingDt.port))
    // {
    //     // MotorWidget & sInstance = MotorWidgetInst();
    //     // sInstance.MotorStop();
    //     LOG_INF("Stalling_detected!");
    //     if (sStopCallback)
    //         sStopCallback(&sInstance);
    // }
}

void MotorWidget::PollIRQ(const struct device * dev, uint32_t pins)
{
    LOG_INF("PollIRQ: %d", pins);
    if ((BIT(sMotorStallingDt.pin) & pins) && (dev == sMotorStallingDt.port))
    {
        LOG_INF("Stalling dectect: %d", pins);
    }
    MotorWidget & sInstance = MotorWidgetInst();
    if (sStopCallback)
        sStopCallback(&sInstance);
}

void MotorWidget::Sleep(bool enable) // False to sleep
{
    int ret = gpio_pin_set_dt(&sMotorSleepDt, !enable);
    if (ret < 0)
    {
        LOG_ERR("GPIO pin %d set -fail. Status: %d\n", sMotorSleepDt.pin, ret);
    }
}

void MotorWidget::Stop(void) // False to sleep
{
    mPwmMotorIn1.StopAction();
    mPwmMotorIn2.StopAction();
}

void MotorWidget::Coast(void) // False to sleep
{
    Stop();
}

void MotorWidget::Break(void)
{
    Sleep(true);
    Stop();
}

void MotorWidget::Forward(void)
{
    int ret = gpio_pin_configure_dt(&sMotorLimte1Dt, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0)
    {
        LOG_ERR("Config in pin err: %d", ret);
        return ret;
    }
    ret = gpio_pin_configure_dt(&sMotorStallingDt, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0)
    {
        LOG_ERR("Config in pin err: %d", ret);
        return ret;
    }
    if (gpio_pin_get_dt(&sMotorLimte1Dt) == 0)
    {
        // skipDetectStalling = true;
        // k_timer_stop(&mSkipDetectStallTimer);
        // k_timer_start(&mSkipDetectStallTimer, K_MSEC(20), K_NO_WAIT);
        Sleep(true);
        mPwmMotorIn1.StopAction();
        mPwmMotorIn2.InitiateMotorAction(800);
    }
    else
    {
        ret = gpio_pin_configure_dt(&sMotorLimte1Dt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }
        ret = gpio_pin_configure_dt(&sMotorStallingDt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }
    }
}

void MotorWidget::Reverse(void)
{
    int ret = gpio_pin_configure_dt(&sMotorLimte2Dt, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0)
    {
        LOG_ERR("Config in pin err: %d", ret);
        return ret;
    }
    ret = gpio_pin_configure_dt(&sMotorStallingDt, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0)
    {
        LOG_ERR("Config in pin err: %d", ret);
        return ret;
    }
    if (gpio_pin_get_dt(&sMotorLimte2Dt) == 0)
    {
        // skipDetectStalling = true;
        // k_timer_stop(&mSkipDetectStallTimer);
        // k_timer_start(&mSkipDetectStallTimer, K_MSEC(20), K_NO_WAIT);
        Sleep(true);
        mPwmMotorIn2.StopAction();
        mPwmMotorIn1.InitiateMotorAction(800);
    }
    else
    {
        ret = gpio_pin_configure_dt(&sMotorLimte2Dt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }
        ret = gpio_pin_configure_dt(&sMotorStallingDt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }
    }
}

void MotorWidget::Start(bool direction)
{
    if (motorState == direction)
    {
        LOG_INF("----> motorState == direction, return <----");
        return;
        // direction = !motorState;
    }
    if (sInstance.isMotorStop == false)
    {
        LOG_INF("---- sInstance.isMotorStopped == false, return ----");
        // return;
    }

    sInstance.isStopFlag  = false;
    sInstance.isMotorStop = false;

    // 开关运行期间把开关电位拉低
    // int ret = gpio_pin_configure_dt(&sMotorButton, GPIO_INPUT | GPIO_PULL_DOWN);
    // if (ret < 0)
    // {
    //     LOG_ERR("Config in pin err: %d", ret);
    //     return ret;
    // }
    // LOG_INF("sMotorButton  --->   DOWN");
    motorState = direction;
    if (isMotorStopped)
    {
        pm_policy_state_lock_get(PM_STATE_STANDBY, PM_ALL_SUBSTATES);
        pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
    }

    isMotorStopped = false;

    // 作为电机停转的标志位
    sInstance.mState = direction;

    LOG_INF("---- Motor start: %d\n", direction);
    k_timer_stop(&mMotorTimer);

    DoSet(direction);

    //     k_timer_start(&mMotorStopTimer, K_NO_WAIT, K_MSEC(100));

    // ScheduleStateChange();
}

void MotorWidget::ScheduleStateChange()
{
    LOG_INF("Schedule stop timer\n");
    // k_timer_start(&mMotorTimer, K_MSEC(MOTOR_MAX_ON_TIME), K_NO_WAIT);
}

void MotorWidget::DoSet(bool state)
{
    mState = state;
    if (state)
    {
        Forward();
    }
    else
    {
        Reverse();
    }
}

void MotorWidget::UpdateState()
{
    // LOG_INF("++++ UpdateState ++++");
    mPwmMotorIn1.UpdateMotorAction();
    mPwmMotorIn2.UpdateMotorAction();

    // LOG_INF("gpio_pin_get_dt(&sMotorLimte1Dt) = %d", gpio_pin_get_dt(&sMotorLimte1Dt));
    // LOG_INF("gpio_pin_get_dt(&sMotorLimte2Dt) = %d", gpio_pin_get_dt(&sMotorLimte2Dt));
    // LOG_INF("sInstance.mState = %d", sInstance.mState);

    // 代替定时器
    if (sInstance.mMotorRunTime < MOTOR_MAX_ON_TIME)
    {
        sInstance.mMotorRunTime += 20;
        // LOG_INF("====> sInstance.mMotorRunTime = %d <====", sInstance.mMotorRunTime);
    }
    else
    {
        LOG_INF("====> sInstance.mMotorRunTime > MOTOR_MAX_ON_TIME <====");
        sInstance.mMotorRunTime = 0;
        if (sStopCallback)
            sStopCallback(&sInstance);
    }

    if (gpio_pin_get_dt(&sMotorLimte1Dt) == 1 && sInstance.mState == true)
    {
        int ret = gpio_pin_configure_dt(&sMotorLimte1Dt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }
        LOG_INF("ON, sInstance.mState = %d", sInstance.mState);

        sInstance.mMotorRunTime = 0;

        if (sStopCallback)
            sStopCallback(&sInstance);
    }
    else if (gpio_pin_get_dt(&sMotorLimte2Dt) == 1 && sInstance.mState == false)
    {
        int ret = gpio_pin_configure_dt(&sMotorLimte2Dt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }
        LOG_INF("OFF, sInstance.mState = %d", sInstance.mState);

        sInstance.mMotorRunTime = 0;

        if (sStopCallback)
            sStopCallback(&sInstance);
    }

    // 阻转中断
    // else if (gpio_pin_get_dt(&sMotorStallingDt) == 1)
    // {
    // int ret = gpio_pin_configure_dt(&sMotorLimte1Dt, GPIO_INPUT | GPIO_PULL_DOWN);
    // if (ret < 0)
    // {
    //     LOG_ERR("Config in pin err: %d", ret);
    //     return ret;
    // }
    // ret = gpio_pin_configure_dt(&sMotorLimte2Dt, GPIO_INPUT | GPIO_PULL_DOWN);
    // if (ret < 0)
    // {
    //     LOG_ERR("Config in pin err: %d", ret);
    //     return ret;
    // }
    // ret = gpio_pin_configure_dt(&sMotorStallingDt, GPIO_INPUT | GPIO_PULL_DOWN);
    // if (ret < 0)
    // {
    //     LOG_ERR("Config in pin err: %d", ret);
    //     return ret;
    // }
    // LOG_INF("==========> STOP UP <==========");
    // if (sStopCallback)
    //     sStopCallback(&sInstance);
    // }
}

void MotorWidget::MotorStop()
{
    LOG_INF("Motor Stop");
    sInstance.isMotorStop = true;

    if (sInstance.mState == 0 && sInstance.isStopFlag == false)
    {
        LOG_INF("==== sInstance.mState == 0 && isStopFlag == false ====");
        sInstance.isStopFlag = true;

        Coast();
        // Sleep(false);
        // Sleep(true);

        LOG_INF("====> Emergency braking <====");

        // mPwmMotorIn1.StopAction();
        mPwmMotorIn1.InitiateMotorStopAction(400);
        mPwmMotorIn2.InitiateMotorStopAction(400);
        // mPwmMotorIn1.Set(true);
        // mPwmMotorIn2.Set(false);
        LOG_INF("==== wait 85ms start ====");
        k_sleep(K_MSEC(85));
        LOG_INF("==== wait 85ms over ====");
        // Sleep(true);
    }

    if (sInstance.mState == 1 && sInstance.isStopFlag == false)
    {
        LOG_INF("==== sInstance.mState == 1 && isStopFlag == false ====");
        sInstance.isStopFlag = true;

        Coast();
        // Sleep(false);
        // Sleep(true);

        LOG_INF("====> Emergency braking <====");

        // mPwmMotorIn2.StopAction();
        mPwmMotorIn2.InitiateMotorStopAction(400);
        mPwmMotorIn1.InitiateMotorStopAction(400);
        // mPwmMotorIn2.Set(true);
        // mPwmMotorIn1.Set(false);
        LOG_INF("==== wait 85ms start ====");
        k_sleep(K_MSEC(85));
        LOG_INF("==== wait 85ms over ====");
        // Sleep(true);
    }

    // mPwmMotorIn1.StopAction();
    // mPwmMotorIn2.StopAction();
    // k_timer_stop(&mMotorStopTimer);
    // LOG_INF("k_timer_stop(&mMotorStopTimer);");

    Coast();
    Sleep(false);
    // k_timer_stop(&mMotorTimer);
    if (!isMotorStopped)
    {
        pm_policy_state_lock_put(PM_STATE_STANDBY, PM_ALL_SUBSTATES);
        pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
    }

    isMotorStopped = true;
}

void MotorWidget::ActionMotorStateUpdateHandler(k_timer * timer)
{
    if (sStateUpdateCallback)
    {
        // LOG_INF("####ActionMotorStateUpdateHandler####");
        sStateUpdateCallback(reinterpret_cast<MotorWidget *>(timer->user_data));
    }
}

void MotorWidget::MotorStopTimerHandler(k_timer * timer)
{
    if (sStopCallback)
        sStopCallback(reinterpret_cast<MotorWidget *>(timer->user_data));
}

void MotorWidget::MotorStopPollTimerHandler(k_timer * timer)
{
    LOG_INF("----MotorStopPollTimerHandler----");

    // gpio_pin_get_dt(&sMotorLimte1Dt)高电平时为0，低电平时为1，当state = true时电机forward()
    if (gpio_pin_get_dt(&sMotorLimte1Dt) == 1 && sInstance.mState == true)
    {
        int ret = gpio_pin_configure_dt(&sMotorLimte1Dt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }
        LOG_INF("ON, sInstance.mState = %d", sInstance.mState);

        if (sStopCallback)
            sStopCallback(&sInstance);
    }
    else if (gpio_pin_get_dt(&sMotorLimte2Dt) == 1 && sInstance.mState == false)
    // else if (gpio_pin_get_dt(&sMotorLimte2Dt) == 1)
    {
        int ret = gpio_pin_configure_dt(&sMotorLimte2Dt, GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0)
        {
            LOG_ERR("Config in pin err: %d", ret);
            return ret;
        }
        LOG_INF("OFF, sInstance.mState = %d", sInstance.mState);

        // sInstance.MotorStop();

        if (sStopCallback)
            sStopCallback(&sInstance);

        // MotorStop();
    }

    if (sMotorStopCallback)
        sMotorStopCallback(reinterpret_cast<MotorWidget *>(timer->user_data));
}

// void MotorWidget::SkipDetectStallTimerHandler(k_timer * timer)
//{
//     skipDetectStalling = false;
// }

bool MotorWidget::GetMotorState()
{
    return motorState;
}
