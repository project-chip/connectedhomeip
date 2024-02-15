/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/attribute-storage.h>
#include <jni.h>
#include <lib/support/JniReferences.h>

/**
 * @brief Handles interfacing between java code and C++ code for the purposes of PowerSource clusters.
 */
class PowerSourceManager
{
public:
    // installed a bridege for a PowerSource cluster endpoint and java object
    static void NewManager(jint endpoint, jobject manager);
    static jboolean SetBatPercentRemaining(jint endpoint, jint value);

private:
    // init with java objects
    CHIP_ERROR InitializeWithObjects(jobject managerObject);
    chip::JniGlobalReference mPowerSourceManagerObject;
};
