/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <FreeRTOS.h>
#include <LEDWidget.h>
#include <WindowApp.h>
#include <queue.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <sl_simple_button_instances.h>
#include <string>
#include <task.h>
#include <timers.h>
#ifdef DISPLAY_ENABLED
#include <LcdPainter.h>
#endif

class WindowAppImpl : public WindowApp
{
public:
    static WindowAppImpl sInstance;

    WindowAppImpl();
    CHIP_ERROR Init() override;
    CHIP_ERROR Start() override;
    void Finish() override;
    void PostEvent(const WindowApp::Event & event) override;
    void PostAttributeChange(chip::EndpointId endpoint, chip::AttributeId attributeId) override;
    friend void sl_button_on_change(const sl_button_t * handle);

protected:
    struct Timer : public WindowApp::Timer
    {
        TimerHandle_t mHandler = nullptr;

        Timer(const char * name, uint32_t timeoutInMs, Callback callback, void * context);
        void IsrStart();
        void Start() override;
        void Stop() override;

    private:
        static void TimerCallback(TimerHandle_t xTimer);
    };

    struct Button : public WindowApp::Button
    {
        Button(Button::Id id, const char * name);
    };

    WindowApp::Timer * CreateTimer(const char * name, uint32_t timeoutInMs, WindowApp::Timer::Callback callback,
                                   void * context) override;
    WindowApp::Button * CreateButton(WindowApp::Button::Id id, const char * name) override;
    void OnButtonChange(const sl_button_t * handle);
    void ProcessEvents();
    void DispatchEvent(const WindowApp::Event & event) override;
    void UpdateLEDs();
    void UpdateLCD();
    void OnMainLoop() override;

    static void OnTaskCallback(void * parameter);
    static void OnIconTimeout(WindowApp::Timer & timer);

private:
    void DispatchEventAttributeChange(chip::EndpointId endpoint, chip::AttributeId attribute);
    TaskHandle_t mHandle = nullptr;
    QueueHandle_t mQueue = nullptr;
    LEDWidget mStatusLED;
    LEDWidget mActionLED;

    // Get QR Code and emulate its content using NFC tag
    char mQRCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
#ifdef DISPLAY_ENABLED
    Timer mIconTimer;
    LcdIcon mIcon = LcdIcon::None;
#endif
};
