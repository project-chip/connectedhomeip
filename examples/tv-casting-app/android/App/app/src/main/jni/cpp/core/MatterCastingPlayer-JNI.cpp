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

#include "../support/Converters-JNI.h"
#include "../support/RotatingDeviceIdUniqueIdProvider-JNI.h"
#include "core/CastingApp.h"                       // from tv-casting-common
#include "core/CastingPlayer.h"                    // from tv-casting-common
#include "core/CastingPlayerDiscovery.h"           // from tv-casting-common
#include "core/CommissionerDeclarationHandler.h"   // from tv-casting-common
#include "core/ConnectionCallbacks.h"              // from tv-casting-common
#include "core/IdentificationDeclarationOptions.h" // from tv-casting-common

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
(JNIEnv * env, jobject thiz, jobject jconnectionCallbacks, jlong commissioningWindowTimeoutSec,
 jobject jIdentificationDeclarationOptions)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::verifyOrEstablishConnection() called with a timeout of: %d seconds",
                    static_cast<int>(commissioningWindowTimeoutSec));

    CastingPlayer * castingPlayer = support::convertCastingPlayerFromJavaToCpp(thiz);
    VerifyOrReturnValue(castingPlayer != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INVALID_ARGUMENT));

    // Find the ConnectionCallbacks class, get the field IDs of the connection callbacks and extract the callback objects.
    jclass connectionCallbacksClass = env->GetObjectClass(jconnectionCallbacks);
    VerifyOrReturnValue(
        connectionCallbacksClass != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INVALID_ARGUMENT),
        ChipLogError(AppServer, "MatterCastingPlayer-JNI::verifyOrEstablishConnection() connectionCallbacksClass == nullptr "));

    jfieldID successCallbackFieldID =
        env->GetFieldID(connectionCallbacksClass, "onSuccess", "Lcom/matter/casting/support/MatterCallback;");
    jfieldID failureCallbackFieldID =
        env->GetFieldID(connectionCallbacksClass, "onFailure", "Lcom/matter/casting/support/MatterCallback;");
    jfieldID commissionerDeclarationCallbackFieldID =
        env->GetFieldID(connectionCallbacksClass, "onCommissionerDeclaration", "Lcom/matter/casting/support/MatterCallback;");

    jobject jSuccessCallback                 = env->GetObjectField(jconnectionCallbacks, successCallbackFieldID);
    jobject jFailureCallback                 = env->GetObjectField(jconnectionCallbacks, failureCallbackFieldID);
    jobject jCommissionerDeclarationCallback = env->GetObjectField(jconnectionCallbacks, commissionerDeclarationCallbackFieldID);

    VerifyOrReturnValue(
        jSuccessCallback != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INVALID_ARGUMENT),
        ChipLogError(AppServer,
                     "MatterCastingPlayer-JNI::verifyOrEstablishConnection() jSuccessCallback == nullptr but is mandatory "));
    VerifyOrReturnValue(
        jFailureCallback != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INVALID_ARGUMENT),
        ChipLogError(AppServer,
                     "MatterCastingPlayer-JNI::verifyOrEstablishConnection() jFailureCallback == nullptr but is mandatory "));

    // jIdentificationDeclarationOptions is optional
    matter::casting::core::IdentificationDeclarationOptions * idOptions = nullptr;
    if (jIdentificationDeclarationOptions == nullptr)
    {
        ChipLogProgress(AppServer,
                        "MatterCastingPlayer-JNI::verifyOrEstablishConnection() Optional jIdentificationDeclarationOptions not "
                        "provided by the client");
    }
    else
    {
        ChipLogProgress(
            AppServer,
            "MatterCastingPlayer-JNI::verifyOrEstablishConnection() jIdentificationDeclarationOptions was provided by client");
        idOptions = support::convertIdentificationDeclarationOptionsFromJavaToCpp(jIdentificationDeclarationOptions);
        VerifyOrReturnValue(idOptions != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INVALID_ARGUMENT),
                            ChipLogError(AppServer,
                                         "MatterCastingPlayer-JNI::verifyOrEstablishConnection() "
                                         "convertIdentificationDeclarationOptionsFromJavaToCpp() error"));
        idOptions->LogDetail();
    }

    MatterCastingPlayerJNIMgr().mConnectionSuccessHandler.SetUp(env, jSuccessCallback);
    MatterCastingPlayerJNIMgr().mConnectionFailureHandler.SetUp(env, jFailureCallback);

    // jCommissionerDeclarationCallback is optional
    if (jCommissionerDeclarationCallback == nullptr)
    {
        ChipLogProgress(AppServer,
                        "MatterCastingPlayer-JNI::verifyOrEstablishConnection() optional jCommissionerDeclarationCallback was not "
                        "provided by the client");
    }
    else
    {
        MatterCastingPlayerJNIMgr().mCommissionerDeclarationHandler.SetUp(env, jCommissionerDeclarationCallback);
    }

    matter::casting::core::ConnectionCallbacks connectionCallbacks;
    connectionCallbacks.mOnConnectionComplete = MatterCastingPlayerJNI::getInstance().getConnectCallback();
    connectionCallbacks.mCommissionerDeclarationCallback =
        MatterCastingPlayerJNI::getInstance().getCommissionerDeclarationCallback();

    castingPlayer->VerifyOrEstablishConnection(connectionCallbacks, static_cast<uint16_t>(commissioningWindowTimeoutSec),
                                               *idOptions);

    return support::convertMatterErrorFromCppToJava(CHIP_NO_ERROR);
}

JNI_METHOD(jobject, continueConnectingNative)
(JNIEnv * env, jobject thiz)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::continueConnecting()");

    CastingPlayer * castingPlayer = support::convertCastingPlayerFromJavaToCpp(thiz);
    VerifyOrReturnValue(castingPlayer != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INVALID_ARGUMENT));

    return support::convertMatterErrorFromCppToJava(castingPlayer->ContinueConnecting());
}

JNI_METHOD(jobject, stopConnecting)
(JNIEnv * env, jobject thiz)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::stopConnecting()");

    CastingPlayer * castingPlayer = support::convertCastingPlayerFromJavaToCpp(thiz);
    VerifyOrReturnValue(castingPlayer != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INVALID_ARGUMENT));

    return support::convertMatterErrorFromCppToJava(castingPlayer->StopConnecting());
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

JNI_METHOD(jstring, getConnectionStateNative)
(JNIEnv * env, jobject thiz)
{
    char error_str[50];
    jobject jstr_obj = nullptr;

    if (NULL == env)
    {
        LogErrorOnFailure(
            chip::JniReferences::GetInstance().CharToStringUTF(CharSpan::fromCharString("JNIEnv interface is NULL"), jstr_obj));
        return static_cast<jstring>(jstr_obj);
    }

    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::getConnectionState()");

    CastingPlayer * castingPlayer = support::convertCastingPlayerFromJavaToCpp(thiz);
    VerifyOrReturnValue(castingPlayer != nullptr, env->NewStringUTF("Cast Player is nullptr"));

    matter::casting::core::ConnectionState state = castingPlayer->GetConnectionState();
    switch (state)
    {
    case matter::casting::core::ConnectionState::CASTING_PLAYER_NOT_CONNECTED:
        LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(CharSpan::fromCharString("NOT_CONNECTED"), jstr_obj));
        break;
    case matter::casting::core::ConnectionState::CASTING_PLAYER_CONNECTING:
        LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(CharSpan::fromCharString("CONNECTING"), jstr_obj));
        break;
    case matter::casting::core::ConnectionState::CASTING_PLAYER_CONNECTED:
        LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(CharSpan::fromCharString("CONNECTED"), jstr_obj));
        break;
    default:
        snprintf(error_str, sizeof(error_str), "Unsupported Connection State: %d", state);
        LogErrorOnFailure(chip::JniReferences::GetInstance().CharToStringUTF(CharSpan::fromCharString(error_str), jstr_obj));
        break;
    }
    return static_cast<jstring>(jstr_obj);
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

void MatterCastingPlayerJNI::ConnectCallback(CHIP_ERROR err, CastingPlayer * playerPtr)
{
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::verifyOrEstablishConnection() ConnectCallback()");
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer,
                        "MatterCastingPlayer-JNI::verifyOrEstablishConnection() ConnectCallback() Connected to Casting Player "
                        "with device ID: %s",
                        playerPtr->GetId());
        // The Java jSuccessCallback is expecting a Void v callback parameter which translates to a nullptr. When calling the
        // Java method from C++ via JNI, passing nullptr is equivalent to passing a Void object in Java.
        MatterCastingPlayerJNIMgr().mConnectionSuccessHandler.Handle(nullptr);
    }
    else
    {
        ChipLogError(
            AppServer,
            "MatterCastingPlayer-JNI::verifyOrEstablishConnection() ConnectCallback() Connection error: %" CHIP_ERROR_FORMAT,
            err.Format());
        MatterCastingPlayerJNIMgr().mConnectionFailureHandler.Handle(err);
    }
}

void MatterCastingPlayerJNI::CommissionerDeclarationCallback(const chip::Transport::PeerAddress & source,
                                                             chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cd)
{
    ChipLogProgress(AppServer, "MatterCastingPlayer-JNI::verifyOrEstablishConnection() CommissionerDeclarationCallback()");
    cd.DebugLog();

    char addressStr[chip::Transport::PeerAddress::kMaxToStringSize];
    source.ToString(addressStr, sizeof(addressStr));
    ChipLogProgress(AppServer,
                    "MatterCastingPlayer-JNI::verifyOrEstablishConnection() CommissionerDeclarationCallback() source: %s",
                    addressStr);

    // Call the Java CommissionerDeclarationCallback if it was provided by the client.
    if (!MatterCastingPlayerJNIMgr().mCommissionerDeclarationHandler.IsSetUp())
    {
        ChipLogError(AppServer,
                     "MatterCastingPlayer-JNI::verifyOrEstablishConnection() CommissionerDeclarationCallback() received from "
                     "but Java callback is not set");
    }
    else
    {
        MatterCastingPlayerJNIMgr().mCommissionerDeclarationHandler.Handle(cd);
    }
}

}; // namespace core
}; // namespace casting
}; // namespace matter
