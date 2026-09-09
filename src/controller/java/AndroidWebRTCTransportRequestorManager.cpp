/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "AndroidWebRTCTransportRequestorManager.h" // Your JNI Wrapper header

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/logging/CHIPLogging.h>

// Stores the JavaVM pointer globally to obtain JNIEnv in C++ background threads.
static JavaVM * gJvm = nullptr;

namespace chip {
namespace Controller {

void AndroidWebRTCTransportRequestorManager::Init(JNIEnv * env, jobject javaCallbackObj)
{
    ChipLogDetail(Controller, "AndroidWebRTCTransportRequestorManager::Init called");

    // Acquires the JVM pointer for future use in background threads.
    env->GetJavaVM(&gJvm);

    // Releases the previously registered callback object if it exists.
    if (mJavaCallbackObj != nullptr)
    {
        env->DeleteGlobalRef(mJavaCallbackObj);
    }

    // Creates a global reference so the Java object is not destroyed by the GC.
    mJavaCallbackObj = env->NewGlobalRef(javaCallbackObj);
    VerifyOrReturn(mJavaCallbackObj != nullptr, ChipLogError(Controller, "Failed to create global ref for WebRTC delegate"));

    jclass cbClass = env->GetObjectClass(mJavaCallbackObj);
    VerifyOrReturn(cbClass != nullptr, ChipLogError(Controller, "Failed to get WebRTC delegate class"));

    // Caches the IDs of the callback methods to be implemented on the Java side.
    // Changed return type from 'V' (void) to 'I' (integer)
    mOnOfferMethod  = env->GetMethodID(cbClass, "onOffer", "(ILjava/lang/String;)I");
    mOnAnswerMethod = env->GetMethodID(cbClass, "onAnswer", "(ILjava/lang/String;)I");
    mOnEndMethod    = env->GetMethodID(cbClass, "onEnd", "(II)I");

    // Assumes a Java method signature that receives an array of IceCandidate objects and returns an integer.
    mOnICECandidatesMethod = env->GetMethodID(cbClass, "onIceCandidates", "(I[Lchip/devicecontroller/IceCandidate;)I");

    if (mOnOfferMethod == nullptr || mOnAnswerMethod == nullptr || mOnEndMethod == nullptr || mOnICECandidatesMethod == nullptr)
    {
        ChipLogError(Controller, "Failed to find one or more callback method IDs");
    }
    else
    {
        ChipLogProgress(Controller, "Successfully initialized JNI callback methods");
    }

    env->DeleteLocalRef(cbClass);

    jclass localIceCandidateClass = env->FindClass("chip/devicecontroller/IceCandidate");
    if (localIceCandidateClass != nullptr)
    {
        mIceCandidateClass = static_cast<jclass>(env->NewGlobalRef(localIceCandidateClass));
        env->DeleteLocalRef(localIceCandidateClass);
        ChipLogProgress(Controller, "Successfully cached IceCandidate class");
    }
    else
    {
        ChipLogError(Controller, "Failed to find Java IceCandidate class in Init");
    }

    // Registers the JNI static callback functions with the original C++ manager.

    VerifyOrReturn(mJavaCallbackObj != nullptr && mOnOfferMethod != nullptr && mOnAnswerMethod != nullptr &&
                       mOnEndMethod != nullptr && mOnICECandidatesMethod != nullptr && mIceCandidateClass != nullptr,
                   ChipLogError(Controller, "WebRTC Transport Requestor JNI callbacks not initialized; skipping init"));
    WebRTCTransportRequestorManager::Instance().InitCallbacks(OnOffer, OnAnswer, OnICECandidates, OnEnd);
    WebRTCTransportRequestorManager::Instance().Init();
}

void AndroidWebRTCTransportRequestorManager::Shutdown(JNIEnv * env)
{
    ChipLogDetail(Controller, "AndroidWebRTCTransportRequestorManager::Shutdown called");

    if (mJavaCallbackObj != nullptr)
    {
        env->DeleteGlobalRef(mJavaCallbackObj);
        mJavaCallbackObj = nullptr;
    }

    if (mIceCandidateClass != nullptr)
    {
        env->DeleteGlobalRef(mIceCandidateClass);
        mIceCandidateClass = nullptr;
    }

    WebRTCTransportRequestorManager::Instance().Shutdown();
}

CHIP_ERROR AndroidWebRTCTransportRequestorManager::OnOffer(uint16_t sessionId, const char * offer)
{
    ChipLogProgress(Controller, "OnOffer called for sessionId: %u", sessionId);

    VerifyOrReturnError(gJvm != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(Instance().mJavaCallbackObj != nullptr && Instance().mOnOfferMethod != nullptr, CHIP_ERROR_INCORRECT_STATE);
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_ERROR_INTERNAL);

    jstring jOffer = env->NewStringUTF(offer);
    jint status    = env->CallIntMethod(Instance().mJavaCallbackObj, Instance().mOnOfferMethod, sessionId, jOffer);
    env->DeleteLocalRef(jOffer);
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_ERROR_INTERNAL);

    return (status == 0) ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR AndroidWebRTCTransportRequestorManager::OnAnswer(uint16_t sessionId, const char * answer)
{
    ChipLogProgress(Controller, "OnAnswer called for sessionId: %u", sessionId);

    JNIEnv * env = nullptr;
    if (gJvm->AttachCurrentThread(&env, nullptr) != JNI_OK)
    {
        ChipLogError(Controller, "Failed to attach current thread for OnAnswer");
        return CHIP_ERROR_INTERNAL;
    }

    jstring jAnswer = env->NewStringUTF(answer);
    jint status     = env->CallIntMethod(Instance().mJavaCallbackObj, Instance().mOnAnswerMethod, sessionId, jAnswer);
    env->DeleteLocalRef(jAnswer);

    return (status == 0) ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR AndroidWebRTCTransportRequestorManager::OnICECandidates(uint16_t sessionId, const IceCandidate * candidates,
                                                                   int candidateCount)
{
    ChipLogProgress(Controller, "OnICECandidates called for sessionId: %u, count: %d", sessionId, candidateCount);

    JNIEnv * env = nullptr;
    if (gJvm->AttachCurrentThread(&env, nullptr) != JNI_OK)
    {
        ChipLogError(Controller, "Failed to attach current thread for OnICECandidates");
        return CHIP_ERROR_INTERNAL;
    }

    // Finds the Java IceCandidate class and its constructor.
    jclass iceCandidateClass = Instance().mIceCandidateClass;
    if (iceCandidateClass == nullptr)
    {
        ChipLogError(Controller, "Cached Java IceCandidate class is null");
        return CHIP_ERROR_INTERNAL;
    }

    jmethodID constructorId = env->GetMethodID(iceCandidateClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;I)V");

    // Creates a Java object array to pass.
    jobjectArray jCandidatesArray = env->NewObjectArray(candidateCount, iceCandidateClass, nullptr);

    for (int i = 0; i < candidateCount; ++i)
    {
        jstring jCandidateStr = env->NewStringUTF(candidates[i].candidate);
        jstring jSdpMidStr    = candidates[i].sdpMid ? env->NewStringUTF(candidates[i].sdpMid) : nullptr;

        jobject jCandidateObj =
            env->NewObject(iceCandidateClass, constructorId, jCandidateStr, jSdpMidStr, candidates[i].sdpMLineIndex);
        env->SetObjectArrayElement(jCandidatesArray, i, jCandidateObj);

        env->DeleteLocalRef(jCandidateStr);
        if (jSdpMidStr)
            env->DeleteLocalRef(jSdpMidStr);
        env->DeleteLocalRef(jCandidateObj);
    }

    jint status = env->CallIntMethod(Instance().mJavaCallbackObj, Instance().mOnICECandidatesMethod, sessionId, jCandidatesArray);

    env->DeleteLocalRef(jCandidatesArray);

    return (status == 0) ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR AndroidWebRTCTransportRequestorManager::OnEnd(uint16_t sessionId, uint8_t reason)
{
    ChipLogProgress(Controller, "OnEnd called for sessionId: %u, reason: %u", sessionId, reason);

    JNIEnv * env = nullptr;
    if (gJvm->AttachCurrentThread(&env, nullptr) != JNI_OK)
    {
        ChipLogError(Controller, "Failed to attach current thread for OnEnd");
        return CHIP_ERROR_INTERNAL;
    }

    jint status = env->CallIntMethod(Instance().mJavaCallbackObj, Instance().mOnEndMethod, sessionId, static_cast<int>(reason));

    return (status == 0) ? CHIP_NO_ERROR : CHIP_ERROR_INCORRECT_STATE;
}

} // namespace Controller
} // namespace chip
