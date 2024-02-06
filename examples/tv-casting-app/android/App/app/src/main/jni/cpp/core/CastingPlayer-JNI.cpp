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

#include "CastingPlayer-JNI.h"

#include "../JNIDACProvider.h"
#include "../support/CastingPlayerConverter-JNI.h"
#include "../support/ErrorConverter-JNI.h"
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

JNI_METHOD(jobject, VerifyOrEstablishConnection)
(JNIEnv * env, jobject thiz, jlong commissioningWindowTimeoutSec, jobject desiredEndpointFilterJavaObject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "CastingPlayer-JNI::VerifyOrEstablishConnection() called with a timeout of: %ld seconds",
                    static_cast<long>(commissioningWindowTimeoutSec));

    // Convert the CastingPlayer jlong to a CastingPlayer pointer
    jclass castingPlayerClass         = env->GetObjectClass(thiz);
    jfieldID _cppCastingPlayerFieldId = env->GetFieldID(castingPlayerClass, "_cppCastingPlayer", "J");
    VerifyOrReturnValue(
        _cppCastingPlayerFieldId != nullptr, nullptr,
        ChipLogError(AppServer, "CastingPlayer-JNI::VerifyOrEstablishConnection() _cppCastingPlayerFieldId == nullptr"));

    jlong _cppCastingPlayerValue  = env->GetLongField(thiz, _cppCastingPlayerFieldId);
    CastingPlayer * castingPlayer = reinterpret_cast<CastingPlayer *>(_cppCastingPlayerValue);
    VerifyOrReturnValue(castingPlayer != nullptr, nullptr,
                        ChipLogError(AppServer, "CastingPlayer-JNI::VerifyOrEstablishConnection() castingPlayer == nullptr"));

    // Create a new Java CompletableFuture
    jclass completableFutureClass          = env->FindClass("java/util/concurrent/CompletableFuture");
    jmethodID completableFutureConstructor = env->GetMethodID(completableFutureClass, "<init>", "()V");
    jobject completableFutureObj           = env->NewObject(completableFutureClass, completableFutureConstructor);
    jobject completableFutureObjGlobalRef  = env->NewGlobalRef(completableFutureObj);
    VerifyOrReturnValue(
        completableFutureObjGlobalRef != nullptr, nullptr,
        ChipLogError(AppServer, "CastingPlayer-JNI::VerifyOrEstablishConnection() completableFutureObjGlobalRef == nullptr"));

    ConnectCallback callback = [completableFutureObjGlobalRef](CHIP_ERROR err, CastingPlayer * playerPtr) {
        ChipLogProgress(AppServer, "CastingPlayer-JNI::VerifyOrEstablishConnection() ConnectCallback called");
        VerifyOrReturn(completableFutureObjGlobalRef != nullptr,
                       ChipLogError(AppServer, "ConnectCallback, completableFutureObjGlobalRef == nullptr"));

        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        VerifyOrReturn(env != nullptr, ChipLogError(AppServer, "ConnectCallback, env == nullptr"));
        // Ensures proper cleanup of local references to Java objects.
        JniLocalReferenceScope scope(env);
        // Ensures proper cleanup of global references to Java objects.
        JniGlobalRefWrapper globalRefWrapper(completableFutureObjGlobalRef);

        jclass completableFutureClass = env->FindClass("java/util/concurrent/CompletableFuture");
        VerifyOrReturn(completableFutureClass != nullptr,
                       ChipLogError(AppServer, "ConnectCallback, completableFutureClass == nullptr");
                       env->DeleteGlobalRef(completableFutureObjGlobalRef););

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
        ChipLogProgress(AppServer,
                        "CastingPlayer-JNI::VerifyOrEstablishConnection() calling CastingPlayer::VerifyOrEstablishConnection() on "
                        "Casting Player with device ID: %s",
                        castingPlayer->GetId());
        castingPlayer->VerifyOrEstablishConnection(callback, static_cast<unsigned long long int>(commissioningWindowTimeoutSec));
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
        ChipLogProgress(AppServer, "CastingPlayer-JNI::VerifyOrEstablishConnection() desiredEndpointFilter.vendorId: %d",
                        desiredEndpointFilter.vendorId);
        ChipLogProgress(AppServer, "CastingPlayer-JNI::VerifyOrEstablishConnection() desiredEndpointFilter.productId: %d",
                        desiredEndpointFilter.productId);
        // TODO: In following PRs. Translate the Java requiredDeviceTypes list to a C++ requiredDeviceTypes vector. For now we're
        // passing an empty list of DeviceTypeStruct.

        ChipLogProgress(AppServer,
                        "CastingPlayer-JNI::VerifyOrEstablishConnection() calling "
                        "CastingPlayer::VerifyOrEstablishConnection() on Casting Player with device ID: %s",
                        castingPlayer->GetId());
        castingPlayer->VerifyOrEstablishConnection(callback, static_cast<unsigned long long int>(commissioningWindowTimeoutSec),
                                                   desiredEndpointFilter);
    }

    return completableFutureObjGlobalRef;
}

}; // namespace core
}; // namespace casting
}; // namespace matter
