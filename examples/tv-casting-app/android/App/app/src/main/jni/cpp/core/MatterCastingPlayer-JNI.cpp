/*
 *   Copyright (c) 2024 Project CHIP Authors
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

#include "MatterCastingPlayer-JNI.h"

#include "../JNIDACProvider.h"
#include "../support/Converters-JNI.h"
#include "../support/RotatingDeviceIdUniqueIdProvider-JNI.h"
#include "core/CastingApp.h"             // from tv-casting-common
#include "core/CastingPlayer.h"          // from tv-casting-common
#include "core/CastingPlayerDiscovery.h" // from tv-casting-common

#include <app/clusters/bindings/BindingManager.h>
#include <app/server/Server.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_casting_core_MatterCastingPlayer_##METHOD_NAME

namespace matter {
namespace casting {
namespace core {

MatterCastingPlayerJNI MatterCastingPlayerJNI::sInstance;

JNI_METHOD(jobject, verifyOrEstablishConnection)
(JNIEnv * env, jobject thiz, jlong commissioningWindowTimeoutSec, jobject desiredEndpointFilterJavaObject, jobject jSuccessCallback,
 jobject jFailureCallback)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::verifyOrEstablishConnection() called with a timeout of: %ld seconds",
                    static_cast<long>(commissioningWindowTimeoutSec));

    CastingPlayer * castingPlayer = support::convertCastingPlayerFromJavaToCpp(thiz);
    VerifyOrReturnValue(castingPlayer != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INVALID_ARGUMENT));

    matter::casting::core::EndpointFilter desiredEndpointFilter;
    if (desiredEndpointFilterJavaObject != nullptr)
    {
        // Convert the EndpointFilter Java class to a C++ EndpointFilter
        jclass endpointFilterJavaClass = env->GetObjectClass(desiredEndpointFilterJavaObject);
        jfieldID vendorIdFieldId       = env->GetFieldID(endpointFilterJavaClass, "vendorId", "Ljava/lang/Integer;");
        jfieldID productIdFieldId      = env->GetFieldID(endpointFilterJavaClass, "productId", "Ljava/lang/Integer;");
        jobject vendorIdIntegerObject  = env->GetObjectField(desiredEndpointFilterJavaObject, vendorIdFieldId);
        jobject productIdIntegerObject = env->GetObjectField(desiredEndpointFilterJavaObject, productIdFieldId);
        // jfieldID requiredDeviceTypesFieldId = env->GetFieldID(endpointFilterJavaClass, "requiredDeviceTypes",
        // "Ljava/util/List;");

        // Value of 0 means unspecified
        desiredEndpointFilter.vendorId  = vendorIdIntegerObject != nullptr
             ? static_cast<uint16_t>(env->CallIntMethod(
                  vendorIdIntegerObject, env->GetMethodID(env->GetObjectClass(vendorIdIntegerObject), "intValue", "()I")))
             : 0;
        desiredEndpointFilter.productId = productIdIntegerObject != nullptr
            ? static_cast<uint16_t>(env->CallIntMethod(
                  productIdIntegerObject, env->GetMethodID(env->GetObjectClass(productIdIntegerObject), "intValue", "()I")))
            : 0;
        // TODO: In following PRs. Translate the Java requiredDeviceTypes list to a C++ requiredDeviceTypes vector. For now we're
        // passing an empty list of DeviceTypeStruct.
    }

    MatterCastingPlayerJNIMgr().mConnectionSuccessHandler.SetUp(env, jSuccessCallback);
    MatterCastingPlayerJNIMgr().mConnectionFailureHandler.SetUp(env, jFailureCallback);
    castingPlayer->VerifyOrEstablishConnection(
        [](CHIP_ERROR err, CastingPlayer * playerPtr) {
            ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::verifyOrEstablishConnection() ConnectCallback called");
            if (err == CHIP_NO_ERROR)
            {
                ChipLogProgress(AppServer, "MatterCastingPlayer-JNI:: Connected to Casting Player with device ID: %s",
                                playerPtr->GetId());
                MatterCastingPlayerJNIMgr().mConnectionSuccessHandler.Handle(nullptr);
            }
            else
            {
                ChipLogError(AppServer, "MatterCastingPlayer-JNI:: ConnectCallback, connection error: %" CHIP_ERROR_FORMAT,
                             err.Format());
                MatterCastingPlayerJNIMgr().mConnectionFailureHandler.Handle(err);
            }
        },
        static_cast<unsigned long long int>(commissioningWindowTimeoutSec), desiredEndpointFilter);
    return support::convertMatterErrorFromCppToJava(CHIP_NO_ERROR);
}

JNI_METHOD(void, disconnect)
(JNIEnv * env, jobject thiz)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::disconnect()");

    core::CastingPlayer * castingPlayer = support::convertCastingPlayerFromJavaToCpp(thiz);
    VerifyOrReturn(castingPlayer != nullptr,
                   ChipLogError(AppServer, "MatterCastingPlayer-JNI::disconnect() castingPlayer == nullptr"));

    castingPlayer->Disconnect();
}

JNI_METHOD(jobject, getEndpoints)
(JNIEnv * env, jobject thiz)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::getEndpoints() called");

    CastingPlayer * castingPlayer = support::convertCastingPlayerFromJavaToCpp(thiz);
    VerifyOrReturnValue(castingPlayer != nullptr, nullptr,
                        ChipLogError(AppServer, "MatterCastingPlayer-JNI::getEndpoints() castingPlayer == nullptr"));

    const std::vector<memory::Strong<Endpoint>> endpoints = castingPlayer->GetEndpoints();
    jobject jEndpointList                                 = nullptr;
    chip::JniReferences::GetInstance().CreateArrayList(jEndpointList);
    for (memory::Strong<Endpoint> endpoint : endpoints)
    {
        jobject matterEndpointJavaObject = support::convertEndpointFromCppToJava(endpoint);
        VerifyOrReturnValue(matterEndpointJavaObject != nullptr, jEndpointList,
                            ChipLogError(AppServer, "MatterCastingPlayer-JNI::getEndpoints(): Could not create Endpoint jobject"));
        chip::JniReferences::GetInstance().AddToList(jEndpointList, matterEndpointJavaObject);
    }
    return jEndpointList;
}

}; // namespace core
}; // namespace casting
}; // namespace matter
