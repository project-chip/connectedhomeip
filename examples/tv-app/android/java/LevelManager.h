/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <cstdint>
#include <jni.h>
#include <lib/support/JniReferences.h>

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
    chip::JniGlobalReference mLevelManagerObject;
    jmethodID mHandleLevelChangedMethod = nullptr;
};
