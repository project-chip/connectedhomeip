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

#pragma once

#include "AppEvent.h"
#include "LEDWidget.h"
#include "PWMDevice.h"

#include <app/AttributePathParams.h>
#include <app/data-model-provider/ProviderChangeListener.h>
#include <platform/CHIPDeviceLayer.h>

#if CONFIG_CHIP_FACTORY_DATA
#include <platform/nrfconnect/FactoryDataProvider.h>
#else
#include <platform/nrfconnect/DeviceInstanceInfoProviderImpl.h>
#endif

#ifdef CONFIG_CHIP_PW_RPC
#include "Rpc.h"
#endif

#ifdef CONFIG_MCUMGR_TRANSPORT_BT
#include "DFUOverSMP.h"
#endif

#include <cstdint>

struct k_timer;
struct Identify;

/**
 * @brief A customized ProviderChangeListener that introduces a delay before marking attributes dirty.
 *
 * This implementation adds a randomized delay (a constant base time plus jitter) before notifying the reporting engine that an
 * attribute path is dirty. The primary goal is to delay DataReports sending, thereby reducing overall network traffic. This
 * approach has proven effective in mitigating network congestion and reducing packet retransmissions, particularly on constrained
 * networks like Thread.
 */
class LightAttributesJitterProviderChangeListener : public chip::app::DataModel::ProviderChangeListener
{
public:
    static constexpr size_t kMaxAttributePathsBufferSize    = 10;
    static constexpr int kUpdateClusterStateBaseTimeoutMs   = 1000;
    static constexpr int kUpdateClusterStateJitterTimeoutMs = 1000;

    LightAttributesJitterProviderChangeListener() { k_mutex_init(&mMutex); }
    void MarkDirty(const chip::app::AttributePathParams & path) override;

private:
    void FlushDirtyPaths();
    void TimerCallback();

    std::array<chip::app::AttributePathParams, kMaxAttributePathsBufferSize> mAttributePaths;
    size_t mCurrentIndex = 0;
    bool mTimerActive    = false;
    k_mutex mMutex;
};

class AppTask
{
public:
    static AppTask & Instance()
    {
        static AppTask sAppTask;
        return sAppTask;
    };

    CHIP_ERROR StartApp();

    void UpdateClusterState();
    PWMDevice & GetPWMDevice() { return mPWMDevice; }

    static void IdentifyStartHandler(Identify *);
    static void IdentifyStopHandler(Identify *);

private:
#ifdef CONFIG_CHIP_PW_RPC
    friend class chip::rpc::NrfButton;
#endif

    CHIP_ERROR Init();

    void CancelTimer();
    void StartTimer(uint32_t timeoutInMs);

    static void PostEvent(const AppEvent & event);
    static void DispatchEvent(const AppEvent & event);
    static void FunctionTimerEventHandler(const AppEvent & event);
    static void LightingActionEventHandler(const AppEvent & event);
    static void StartBLEAdvertisementHandler(const AppEvent & event);
    static void UpdateLedStateEventHandler(const AppEvent & event);

    static void ChipEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void ButtonEventHandler(uint32_t buttonState, uint32_t hasChanged);
    static void FunctionTimerTimeoutCallback(k_timer * timer);

    static void ActionInitiated(PWMDevice::Action_t action, int32_t actor);
    static void ActionCompleted(PWMDevice::Action_t action, int32_t actor);
    static void UpdateStatusLED();
    static void LEDStateUpdateHandler(LEDWidget & ledWidget);
    static void FunctionHandler(const AppEvent & event);
    static void StartBLEAdvertisementAndLightActionEventHandler(const AppEvent & event);

    FunctionEvent mFunction   = FunctionEvent::NoneSelected;
    bool mFunctionTimerActive = false;
    PWMDevice mPWMDevice;
    LightAttributesJitterProviderChangeListener mCustomizedProviderChangeListener;

#if CONFIG_CHIP_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider<chip::DeviceLayer::InternalFlashFactoryData> mFactoryDataProvider;
#endif
};
