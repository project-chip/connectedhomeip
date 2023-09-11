/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <jni.h>

/**
 * @brief Handles interfacing between java code and C++ code for the purposes of On/Off clusters.
 */
class OnOffManager
{
public:
    // installed a bridege for a On/Off cluster endpoint and java object
    static void NewManager(jint endpoint, jobject manager);

    // helps for java to set attributes::OnOff of OnOff cluster
    static jboolean SetOnOff(jint endpoint, bool value);

    // posts a OnOffChanged event to suitable OnOffManager
    static void PostOnOffChanged(chip::EndpointId endpoint, bool value);

    // handles `Changed` callbacks by calling the java `void HandleOnOffChanged()` method
    void HandleOnOffChanged(bool value);

private:
    // init with java objects
    CHIP_ERROR InitializeWithObjects(jobject managerObject);
    jobject mOnOffManagerObject         = nullptr;
    jmethodID mHandleOnOffChangedMethod = nullptr;
};
