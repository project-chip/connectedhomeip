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

JNI_METHOD(jobject, verifyOrEstablishConnection)
(JNIEnv * env, jobject thiz, jlong commissioningWindowTimeoutSec, jobject desiredEndpointFilterJavaObject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::verifyOrEstablishConnection() called with a timeout of: %ld seconds",
                    static_cast<long>(commissioningWindowTimeoutSec));

    CastingPlayer * castingPlayer = support::convertCastingPlayerFromJavaToCpp(thiz);
    VerifyOrReturnValue(
        castingPlayer != nullptr, nullptr,
        ChipLogError(AppServer, "MatterCastingPlayer-JNI::verifyOrEstablishConnection() Invalid CastingPlayer argument"));

    // Create a new Java CompletableFuture
    jclass completableFutureClass          = env->FindClass("java/util/concurrent/CompletableFuture");
    jmethodID completableFutureConstructor = env->GetMethodID(completableFutureClass, "<init>", "()V");
    jobject completableFutureObj           = env->NewObject(completableFutureClass, completableFutureConstructor);
    jobject completableFutureObjGlobalRef  = env->NewGlobalRef(completableFutureObj);
    VerifyOrReturnValue(
        completableFutureObjGlobalRef != nullptr, nullptr,
        ChipLogError(AppServer,
                     "MatterCastingPlayer-JNI::verifyOrEstablishConnection() Could not create completableFutureObjGlobalRef"));

    ConnectCallback callback = [completableFutureObjGlobalRef](CHIP_ERROR err, CastingPlayer * playerPtr) {
        ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::verifyOrEstablishConnection() ConnectCallback called");

        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "ConnectCallback, env == nullptr"));

        // Ensures proper cleanup of local references to Java objects.
        JniLocalReferenceManager manager(env);
        VerifyOrReturn(completableFutureObjGlobalRef != nullptr,
                       ChipLogError(AppServer, "ConnectCallback, completableFutureObjGlobalRef was nullptr"));

        jclass completableFutureClass = env->FindClass("java/util/concurrent/CompletableFuture");
        VerifyOrReturn(completableFutureClass != nullptr,
                       ChipLogError(AppServer, "ConnectCallback, completableFutureClass == nullptr"));

        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(AppServer, "ConnectCallback, Connected to Casting Player with device ID: %s", playerPtr->GetId());
            jmethodID completeMethod = env->GetMethodID(completableFutureClass, "complete", "(Ljava/lang/Object;)Z");
            VerifyOrReturn(completeMethod != nullptr, ChipLogError(AppServer, "ConnectCallback, completeMethod == nullptr"));

            chip::DeviceLayer::StackUnlock unlock;
            env->CallBooleanMethod(completableFutureObjGlobalRef, completeMethod, nullptr);
        }
        else
        {
            ChipLogError(AppServer, "ConnectCallback, connection error: %" CHIP_ERROR_FORMAT, err.Format());
            jmethodID completeExceptionallyMethod =
                env->GetMethodID(completableFutureClass, "completeExceptionally", "(Ljava/lang/Throwable;)Z");
            VerifyOrReturn(completeExceptionallyMethod != nullptr,
                           ChipLogError(AppServer, "ConnectCallback, completeExceptionallyMethod == nullptr"));

            // Create a Throwable object (e.g., RuntimeException) to pass to completeExceptionallyMethod
            jclass throwableClass = env->FindClass("java/lang/RuntimeException");
            VerifyOrReturn(throwableClass != nullptr, ChipLogError(AppServer, "ConnectCallback, throwableClass == nullptr"));
            jmethodID throwableConstructor = env->GetMethodID(throwableClass, "<init>", "(Ljava/lang/String;)V");
            VerifyOrReturn(throwableConstructor != nullptr,
                           ChipLogError(AppServer, "ConnectCallback, throwableConstructor == nullptr"));
            jstring errorMessage = env->NewStringUTF(err.Format());
            VerifyOrReturn(errorMessage != nullptr, ChipLogError(AppServer, "ConnectCallback, errorMessage == nullptr"));
            jobject throwableObject = env->NewObject(throwableClass, throwableConstructor, errorMessage);
            VerifyOrReturn(throwableObject != nullptr, ChipLogError(AppServer, "ConnectCallback, throwableObject == nullptr"));

            chip::DeviceLayer::StackUnlock unlock;
            env->CallBooleanMethod(completableFutureObjGlobalRef, completeExceptionallyMethod, throwableObject);
        }
    };

    if (desiredEndpointFilterJavaObject == nullptr)
    {
        ConnectionContextJNI * context         = new ConnectionContextJNI();
        context->castingPlayer                 = castingPlayer;
        context->callback                      = callback;
        context->commissioningWindowTimeoutSec = static_cast<unsigned long long int>(commissioningWindowTimeoutSec);

        chip::DeviceLayer::SystemLayer().ScheduleWork(MatterCastingPlayerJNI::VerifyOrEstablishConnectionTask, context);
    }
    else
    {
        // Convert the EndpointFilter Java class to a C++ EndpointFilter
        jclass endpointFilterJavaClass = env->GetObjectClass(desiredEndpointFilterJavaObject);
        jfieldID vendorIdFieldId       = env->GetFieldID(endpointFilterJavaClass, "vendorId", "Ljava/lang/Integer;");
        jfieldID productIdFieldId      = env->GetFieldID(endpointFilterJavaClass, "productId", "Ljava/lang/Integer;");
        jobject vendorIdIntegerObject  = env->GetObjectField(desiredEndpointFilterJavaObject, vendorIdFieldId);
        jobject productIdIntegerObject = env->GetObjectField(desiredEndpointFilterJavaObject, productIdFieldId);
        // jfieldID requiredDeviceTypesFieldId = env->GetFieldID(endpointFilterJavaClass, "requiredDeviceTypes",
        // "Ljava/util/List;");

        matter::casting::core::EndpointFilter desiredEndpointFilter;
        // Value of 0 means unspecified
        desiredEndpointFilter.vendorId  = vendorIdIntegerObject != nullptr
             ? static_cast<uint16_t>(env->CallIntMethod(
                  vendorIdIntegerObject, env->GetMethodID(env->GetObjectClass(vendorIdIntegerObject), "intValue", "()I")))
             : 0;
        desiredEndpointFilter.productId = productIdIntegerObject != nullptr
            ? static_cast<uint16_t>(env->CallIntMethod(
                  productIdIntegerObject, env->GetMethodID(env->GetObjectClass(productIdIntegerObject), "intValue", "()I")))
            : 0;
        ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::VerifyOrEstablishConnection() desiredEndpointFilter.vendorId: %d",
                        desiredEndpointFilter.vendorId);
        ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::VerifyOrEstablishConnection() desiredEndpointFilter.productId: %d",
                        desiredEndpointFilter.productId);
        // TODO: In following PRs. Translate the Java requiredDeviceTypes list to a C++ requiredDeviceTypes vector. For now we're
        // passing an empty list of DeviceTypeStruct.

        ChipLogProgress(AppServer,
                        "MatterCastingPlayer-JNI::verifyOrEstablishConnection() calling "
                        "CastingPlayer::VerifyOrEstablishConnection() on Casting Player with device ID: %s",
                        castingPlayer->GetId());

        ConnectionContextJNI * context         = new ConnectionContextJNI();
        context->castingPlayer                 = castingPlayer;
        context->callback                      = callback;
        context->commissioningWindowTimeoutSec = static_cast<unsigned long long int>(commissioningWindowTimeoutSec);
        context->desiredEndpointFilter         = desiredEndpointFilter;

        chip::DeviceLayer::SystemLayer().ScheduleWork(MatterCastingPlayerJNI::VerifyOrEstablishConnectionTask, context);
    }

    return completableFutureObjGlobalRef;
}

void MatterCastingPlayerJNI::VerifyOrEstablishConnectionTask(chip::System::Layer * aSystemLayer, void * context)
{
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::VerifyOrEstablishConnectionTask() called");
    if (context != nullptr)
    {
        ConnectionContextJNI * _context = static_cast<ConnectionContextJNI *>(context);
        _context->castingPlayer->VerifyOrEstablishConnection(_context->callback, _context->commissioningWindowTimeoutSec,
                                                             _context->desiredEndpointFilter);
        delete _context;
    }
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
