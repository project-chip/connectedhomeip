/*
 *   Copyright (c) 2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "AppImpl.h"

#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;
using namespace chip::app;
using namespace chip::AppPlatform;
using namespace chip::Credentials;

/*
 * This file provides the native implementation of methods of the
 * com.matter.tv.server.tvapp.AppPlatform class.
 */

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_tv_server_tvapp_AppPlatform_##METHOD_NAME

JNI_METHOD(void, nativeInit)(JNIEnv *, jobject app, jobject contentAppEndpointManager)
{
    chip::DeviceLayer::StackLock lock;
    InitVideoPlayerPlatform(contentAppEndpointManager);
}

JNI_METHOD(jint, addContentApp)
(JNIEnv *, jobject, jstring vendorName, jint vendorId, jstring appName, jint productId, jstring appVersion, jobject manager)
{
    chip::DeviceLayer::StackLock lock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    JniUtfString vName(env, vendorName);
    JniUtfString aName(env, appName);
    JniUtfString aVersion(env, appVersion);
    EndpointId epId = AddContentApp(vName.c_str(), static_cast<uint16_t>(vendorId), aName.c_str(), static_cast<uint16_t>(productId),
                                    aVersion.c_str(), manager);
    return static_cast<uint16_t>(epId);
}

JNI_METHOD(jint, addContentAppAtEndpoint)
(JNIEnv *, jobject, jstring vendorName, jint vendorId, jstring appName, jint productId, jstring appVersion, jint endpointId,
 jobject manager)
{
    chip::DeviceLayer::StackLock lock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    JniUtfString vName(env, vendorName);
    JniUtfString aName(env, appName);
    JniUtfString aVersion(env, appVersion);
    EndpointId epId = AddContentApp(vName.c_str(), static_cast<uint16_t>(vendorId), aName.c_str(), static_cast<uint16_t>(productId),
                                    aVersion.c_str(), static_cast<EndpointId>(endpointId), manager);
    return static_cast<uint16_t>(epId);
}

JNI_METHOD(jint, removeContentApp)
(JNIEnv *, jobject, jint endpointId)
{
    chip::DeviceLayer::StackLock lock;
    EndpointId epId = RemoveContentApp(static_cast<EndpointId>(endpointId));
    return static_cast<uint16_t>(epId);
}

JNI_METHOD(void, reportAttributeChange)
(JNIEnv *, jobject, jint endpointId, jint clusterId, jint attributeId)
{
    chip::DeviceLayer::StackLock lock;
    ReportAttributeChange(static_cast<EndpointId>(endpointId), static_cast<chip::ClusterId>(clusterId),
                          static_cast<chip::AttributeId>(attributeId));
}

JNI_METHOD(void, addSelfVendorAsAdmin)
(JNIEnv *, jobject, jint endpointId, jint clusterId, jint attributeId)
{
    AddSelfVendorAsAdmin();
}
