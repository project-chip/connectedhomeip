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

#include <app/app-platform/ContentApp.h>
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

// Forward declaration
std::vector<ContentApp::SupportedCluster> convert_to_cpp(JNIEnv * env, jobject supportedClusters);

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_tv_server_tvapp_AppPlatform_##METHOD_NAME

JNI_METHOD(void, nativeInit)(JNIEnv *, jobject app, jobject contentAppEndpointManager)
{
    chip::DeviceLayer::StackLock lock;
    InitVideoPlayerPlatform(contentAppEndpointManager);
}

JNI_METHOD(jint, addContentApp)
(JNIEnv *, jobject, jstring vendorName, jint vendorId, jstring appName, jint productId, jstring appVersion,
 jobject supportedClusters, jobject manager)
{
    chip::DeviceLayer::StackLock lock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    JniUtfString vName(env, vendorName);
    JniUtfString aName(env, appName);
    JniUtfString aVersion(env, appVersion);
    EndpointId epId = AddContentApp(vName.c_str(), static_cast<uint16_t>(vendorId), aName.c_str(), static_cast<uint16_t>(productId),
                                    aVersion.c_str(), convert_to_cpp(env, supportedClusters), manager);
    return static_cast<uint16_t>(epId);
}

JNI_METHOD(jint, addContentAppAtEndpoint)
(JNIEnv *, jobject, jstring vendorName, jint vendorId, jstring appName, jint productId, jstring appVersion,
 jobject supportedClusters, jint endpointId, jobject manager)
{
    chip::DeviceLayer::StackLock lock;
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    JniUtfString vName(env, vendorName);
    JniUtfString aName(env, appName);
    JniUtfString aVersion(env, appVersion);
    EndpointId epId =
        AddContentApp(vName.c_str(), static_cast<uint16_t>(vendorId), aName.c_str(), static_cast<uint16_t>(productId),
                      aVersion.c_str(), convert_to_cpp(env, supportedClusters), static_cast<EndpointId>(endpointId), manager);
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

std::vector<uint32_t> consume_and_convert_to_cpp(JNIEnv * env, jobject intArrayObject)
{
    std::vector<uint32_t> uintVector;
    if (intArrayObject != nullptr)
    {
        jsize length    = env->GetArrayLength(static_cast<jintArray>(intArrayObject));
        jint * elements = env->GetIntArrayElements(static_cast<jintArray>(intArrayObject), nullptr);
        if (elements != nullptr)
        {
            // OBS: Implicit type ambiguation from int32_t to uint32_t
            uintVector.assign(elements, elements + length);
            env->ReleaseIntArrayElements(static_cast<jintArray>(intArrayObject), elements, JNI_ABORT);
        }
        env->DeleteLocalRef(intArrayObject);
    }
    return uintVector;
}

std::vector<ContentApp::SupportedCluster> convert_to_cpp(JNIEnv * env, jobject supportedClustersObject)
{
    if (supportedClustersObject == nullptr || env == nullptr)
    {
        return {};
    }

    // Find Java classes. WARNING: Reflection
    jclass collectionClass = env->FindClass("java/util/Collection");
    jclass iteratorClass   = env->FindClass("java/util/Iterator");
    jclass clusterClass    = env->FindClass("com/matter/tv/server/tvapp/ContentAppSupportedCluster");
    if (collectionClass == nullptr || iteratorClass == nullptr || clusterClass == nullptr)
    {
        return {};
    }

    // Find Java methods. WARNING: Reflection
    jmethodID iteratorMethod = env->GetMethodID(collectionClass, "iterator", "()Ljava/util/Iterator;");
    jmethodID hasNextMethod  = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    jmethodID nextMethod     = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
    if (iteratorMethod == nullptr || hasNextMethod == nullptr || nextMethod == nullptr)
    {
        return {};
    }

    // Find Java SupportedCluster fields. WARNING: Reflection
    jfieldID clusterIdentifierField             = env->GetFieldID(clusterClass, "clusterIdentifier", "I");
    jfieldID featuresField                      = env->GetFieldID(clusterClass, "features", "I");
    jfieldID optionalCommandIdentifiersField    = env->GetFieldID(clusterClass, "optionalCommandIdentifiers", "[I");
    jfieldID optionalAttributesIdentifiersField = env->GetFieldID(clusterClass, "optionalAttributesIdentifiers", "[I");
    if (clusterIdentifierField == nullptr || featuresField == nullptr || optionalCommandIdentifiersField == nullptr ||
        optionalAttributesIdentifiersField == nullptr)
    {
        return {};
    }

    // Find Set Iterator Object
    jobject iteratorObject = env->CallObjectMethod(supportedClustersObject, iteratorMethod);
    if (iteratorObject == nullptr)
    {
        return {};
    }

    // Iterate over the Java Collection and convert each SupportedCluster
    std::vector<SupportedCluster> supportedClusters;
    while (env->CallBooleanMethod(iteratorObject, hasNextMethod))
    {
        jobject clusterObject = env->CallObjectMethod(iteratorObject, nextMethod);
        if (clusterObject != nullptr)
        {
            jint clusterIdentifier     = env->GetIntField(clusterObject, clusterIdentifierField);
            jint features              = env->GetIntField(clusterObject, featuresField);
            jobject commandIdsObject   = env->GetObjectField(clusterObject, optionalCommandIdentifiersField);
            jobject attributeIdsObject = env->GetObjectField(clusterObject, optionalAttributesIdentifiersField);
            // OBS: Implicit type ambiguation from int32_t to uint32_t
            supportedClusters.emplace_back(clusterIdentifier, features, consume_and_convert_to_cpp(env, commandIdsObject),
                                           consume_and_convert_to_cpp(env, attributeIdsObject));
            env->DeleteLocalRef(clusterObject);
        }
    }
    env->DeleteLocalRef(iteratorObject);

    return supportedClusters;
}
