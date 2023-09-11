/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <app-common/zap-generated/cluster-objects.h>
#include <cstdint>
#include <jni.h>

/**
 * @brief Handles interfacing between java code and C++ code for the purposes of LevelControl clusters.
 */
class LevelManager
{
public:
    // installed a bridege for a LevelControl cluster endpoint and java object
    static void NewManager(jint endpoint, jobject manager);

    // helps for java to set attributes::CurrentLevel of LevelControl cluster
    static jboolean SetLevel(jint endpoint, jint value);

    // posts a CurrentLevelChanged event to suitable LevelManager
    static void PostLevelChanged(chip::EndpointId endpoint, uint8_t value);

    // handles `Changed` callbacks by calling the java `void HandleLevelChanged()` method
    void HandleLevelChanged(uint8_t value);

private:
    // init with java objects
    CHIP_ERROR InitializeWithObjects(jobject managerObject);
    jobject mLevelManagerObject         = nullptr;
    jmethodID mHandleLevelChangedMethod = nullptr;
};
