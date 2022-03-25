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

/**
 * @brief class to manage and bridge the level cluster status to the java layer
 */
class LevelManager
{
public:
    static void NewManager(jint endpoint, jobject manager);
    static jboolean SetLevel(jint endpoint, jint value);
    static void PostLevelChanged(chip::EndpointId endpoint, uint8_t value);
    CHIP_ERROR InitializeWithObjects(jobject managerObject);
    void HandleLevelChanged(uint8_t value);

private:
    jobject mLevelManagerObject         = nullptr;
    jmethodID mHandleLevelChangedMethod = nullptr;
};
