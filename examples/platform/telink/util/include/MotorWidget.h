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

#include <cstdint>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <PWMDevice.h>

class MotorWidget
{
public:
    typedef void (*MotorWidgetStopHandler)(MotorWidget * MotorWidget);
    typedef void (*MotorWidgetStateUpdateHandler)(MotorWidget * MotorWidget);

    static void SetCallback(MotorWidgetStateUpdateHandler stateUpdateCb, MotorWidgetStopHandler stopCb);
    void Init(void);
    void Sleep(bool state);
    void Start(bool direction);
    void Stop();
    void Coast();
    void Break();
    void Forward();
    void Reverse();
    void UpdateState();
    void MotorStop();
    void PollIRQ(const struct device * dev, uint32_t pins);

private:

    struct gpio_callback mStalling_cb_data;
    struct gpio_callback mLimit1_cb_data;
    struct gpio_callback mLimit2_cb_data;

    bool mState;
    bool isMotorStopped = true;
    k_timer mMotorTimer;
    k_timer mSkipDetectStallTimer;

    static void MotorStopTimerHandler(k_timer * timer);
    static void ActionMotorStateUpdateHandler(k_timer * timer);
    static void SkipDetectStallTimerHandler(k_timer * timer);
    static void stalling_detected(const struct device * dev, struct gpio_callback * cb, uint32_t pins);
    static void limited1_detected(const struct device * dev, struct gpio_callback * cb, uint32_t pins);
    static void limited2_detected(const struct device * dev, struct gpio_callback * cb, uint32_t pins);

    void DoSet(bool state);
    void ScheduleStateChange();

    //static void ActionIdentifyStateUpdateHandler(k_timer * timer);
    //static void UpdateIdentifyStateEventHandler(AppEvent * aEvent);

    friend MotorWidget & MotorWidgetInst(void);

    static MotorWidget sInstance;
};

/**
 * Returns the KeyManager singleton object.
 */
inline MotorWidget & MotorWidgetInst(void)
{
    return MotorWidget::sInstance;
}
