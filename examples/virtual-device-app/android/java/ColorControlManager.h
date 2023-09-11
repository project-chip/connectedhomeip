/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <jni.h>

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
    jobject mColorControlManagerObject              = nullptr;
    jmethodID mHandleCurrentHueChangedMethod        = nullptr;
    jmethodID mHandleCurrentSaturationChangedMethod = nullptr;
    jmethodID mHandleColorTemperatureChangedMethod  = nullptr;
    jmethodID mHandleColorModeChangedMethod         = nullptr;
    jmethodID mHandleEnhancedColorModeChangedMethod = nullptr;
};
