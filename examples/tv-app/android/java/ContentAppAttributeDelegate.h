/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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

    std::string Read(const chip::app::ConcreteReadAttributePath & aPath);

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

        mReadAttributeMethod = env->GetMethodID(ContentAppEndpointManagerClass, "readAttribute", "(IJJ)Ljava/lang/String;");
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
