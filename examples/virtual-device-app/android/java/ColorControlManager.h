/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <jni.h>
#include <lib/support/JniReferences.h>

class ColorControlManager
{
public:
    static void NewManager(jint endpoint, jobject manager);

    static void PostCurrentHueChanged(chip::EndpointId endpoint, int currentHue);

    static void PostCurrentSaturationChanged(chip::EndpointId endpoint, int currentSaturation);

    static void PostColorTemperatureChanged(chip::EndpointId endpoint, int colorTemperature);

    static void PostColorModeChanged(chip::EndpointId endpoint, int colorMode);

    static void PostEnhancedColorModeChanged(chip::EndpointId endpoint, int enhancedColorMode);

    void HandleCurrentHueChanged(int value);

    void HandleCurrentSaturationChanged(int value);

    void HandleColorTemperatureChanged(int value);

    void HandleColorModeChanged(int value);

    void HandleEnhancedColorModeChanged(int value);

private:
    CHIP_ERROR InitializeWithObjects(jobject managerObject);
    chip::JniGlobalReference mColorControlManagerObject;
    jmethodID mHandleCurrentHueChangedMethod        = nullptr;
    jmethodID mHandleCurrentSaturationChangedMethod = nullptr;
    jmethodID mHandleColorTemperatureChangedMethod  = nullptr;
    jmethodID mHandleColorModeChangedMethod         = nullptr;
    jmethodID mHandleEnhancedColorModeChangedMethod = nullptr;
};
