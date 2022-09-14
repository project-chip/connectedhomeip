/*
 *
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

/**
 * @brief Facilitates communication with the application handlers in Java
 */

#pragma once

#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <jni.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/JniReferences.h>

namespace chip {
namespace AppPlatform {

class ContentAppAttributeDelegate
{
public:
    ContentAppAttributeDelegate(jobject manager)
    {
        if (manager == nullptr)
        {
            // To support the existing hardcoded sample apps.
            return;
        }
        InitializeJNIObjects(manager);
    }

    ~ContentAppAttributeDelegate()
    {
        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for ContentAppEndpointManager"));
        env->DeleteGlobalRef(mContentAppEndpointManager);
    }

    const char * Read(const chip::app::ConcreteReadAttributePath & aPath);

private:
    void InitializeJNIObjects(jobject manager)
    {
        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for ContentAppEndpointManager"));

        mContentAppEndpointManager = env->NewGlobalRef(manager);
        VerifyOrReturn(mContentAppEndpointManager != nullptr,
                       ChipLogError(Zcl, "Failed to NewGlobalRef ContentAppEndpointManager"));

        jclass ContentAppEndpointManagerClass = env->GetObjectClass(manager);
        VerifyOrReturn(ContentAppEndpointManagerClass != nullptr,
                       ChipLogError(Zcl, "Failed to get ContentAppEndpointManager Java class"));

        mReadAttributeMethod = env->GetMethodID(ContentAppEndpointManagerClass, "readAttribute", "(III)Ljava/lang/String;");
        if (mReadAttributeMethod == nullptr)
        {
            ChipLogError(Zcl, "Failed to access ContentAppEndpointManager 'readAttribute' method");
            env->ExceptionClear();
        }
    }

    jobject mContentAppEndpointManager = nullptr;
    jmethodID mReadAttributeMethod     = nullptr;
};

} // namespace AppPlatform
} // namespace chip
