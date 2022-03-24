/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include "AndroidCallbacks.h"

#include <controller/java/AndroidClusterExceptions.h>
#include <controller/java/CHIPAttributeTLVValueDecoder.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/jsontlv/TlvJson.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <type_traits>

namespace chip {
namespace Controller {

GetConnectedDeviceCallback::GetConnectedDeviceCallback(jobject wrapperCallback, jobject javaCallback) :
    mOnSuccess(OnDeviceConnectedFn, this), mOnFailure(OnDeviceConnectionFailureFn, this)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    mWrapperCallbackRef = env->NewGlobalRef(wrapperCallback);
    if (mWrapperCallbackRef == nullptr)
    {
        ChipLogError(Controller, "Could not create global reference for Java callback");
    }
    mJavaCallbackRef = env->NewGlobalRef(javaCallback);
    if (mJavaCallbackRef == nullptr)
    {
        ChipLogError(Controller, "Could not create global reference for Java callback");
    }
}

GetConnectedDeviceCallback::~GetConnectedDeviceCallback()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    env->DeleteGlobalRef(mJavaCallbackRef);
}

void GetConnectedDeviceCallback::OnDeviceConnectedFn(void * context, OperationalDeviceProxy * device)
{
    JNIEnv * env         = JniReferences::GetInstance().GetEnvForCurrentThread();
    auto * self          = static_cast<GetConnectedDeviceCallback *>(context);
    jobject javaCallback = self->mJavaCallbackRef;

    // Release global ref so application can clean up.
    env->DeleteGlobalRef(self->mWrapperCallbackRef);

    jclass getConnectedDeviceCallbackCls = nullptr;
    JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/GetConnectedDeviceCallbackJni$GetConnectedDeviceCallback",
                                             getConnectedDeviceCallbackCls);
    VerifyOrReturn(getConnectedDeviceCallbackCls != nullptr,
                   ChipLogError(Controller, "Could not find GetConnectedDeviceCallback class"));
    JniClass getConnectedDeviceCallbackJniCls(getConnectedDeviceCallbackCls);

    jmethodID successMethod;
    JniReferences::GetInstance().FindMethod(env, javaCallback, "onDeviceConnected", "(J)V", &successMethod);
    VerifyOrReturn(successMethod != nullptr, ChipLogError(Controller, "Could not find onDeviceConnected method"));

    static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a Java handle");
    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, successMethod, reinterpret_cast<jlong>(device));
}

void GetConnectedDeviceCallback::OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error)
{
    JNIEnv * env         = JniReferences::GetInstance().GetEnvForCurrentThread();
    auto * self          = static_cast<GetConnectedDeviceCallback *>(context);
    jobject javaCallback = self->mJavaCallbackRef;

    // Release global ref so application can clean up.
    env->DeleteGlobalRef(self->mWrapperCallbackRef);

    jclass getConnectedDeviceCallbackCls = nullptr;
    JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/GetConnectedDeviceCallbackJni$GetConnectedDeviceCallback",
                                             getConnectedDeviceCallbackCls);
    VerifyOrReturn(getConnectedDeviceCallbackCls != nullptr,
                   ChipLogError(Controller, "Could not find GetConnectedDeviceCallback class"));
    JniClass getConnectedDeviceCallbackJniCls(getConnectedDeviceCallbackCls);

    jmethodID failureMethod;
    JniReferences::GetInstance().FindMethod(env, javaCallback, "onConnectionFailure", "(JLjava/lang/Exception;)V", &failureMethod);
    VerifyOrReturn(failureMethod != nullptr, ChipLogError(Controller, "Could not find onConnectionFailure method"));

    // Create the exception to return.
    jclass controllerExceptionCls;
    CHIP_ERROR err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/ChipDeviceControllerException",
                                                              controllerExceptionCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find exception type for onConnectionFailure"));
    JniClass controllerExceptionJniCls(controllerExceptionCls);

    jmethodID exceptionConstructor = env->GetMethodID(controllerExceptionCls, "<init>", "(ILjava/lang/String;)V");
    jobject exception = env->NewObject(controllerExceptionCls, exceptionConstructor, error, env->NewStringUTF(ErrorStr(error)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, failureMethod, peerId.GetNodeId(), exception);
}

ReportCallback::ReportCallback(jobject wrapperCallback, jobject subscriptionEstablishedCallback, jobject reportCallback) :
    mBufferedReadAdapter(*this)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    if (subscriptionEstablishedCallback != nullptr)
    {
        mSubscriptionEstablishedCallbackRef = env->NewGlobalRef(subscriptionEstablishedCallback);
        if (mSubscriptionEstablishedCallbackRef == nullptr)
        {
            ChipLogError(Controller, "Could not create global reference for Java callback");
        }
    }
    mReportCallbackRef = env->NewGlobalRef(reportCallback);
    if (mReportCallbackRef == nullptr)
    {
        ChipLogError(Controller, "Could not create global reference for Java callback");
    }
    mWrapperCallbackRef = env->NewGlobalRef(wrapperCallback);
    if (mWrapperCallbackRef == nullptr)
    {
        ChipLogError(Controller, "Could not create global reference for Java callback");
    }
}

ReportCallback::~ReportCallback()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    if (mSubscriptionEstablishedCallbackRef != nullptr)
    {
        env->DeleteGlobalRef(mSubscriptionEstablishedCallbackRef);
    }
    env->DeleteGlobalRef(mReportCallbackRef);
    if (mReadClient != nullptr)
    {
        Platform::Delete(mReadClient);
    }
}

void ReportCallback::OnReportBegin()
{
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/NodeState", mNodeStateCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not get NodeState class"));
    jmethodID nodeStateCtor = env->GetMethodID(mNodeStateCls, "<init>", "(Ljava/util/Map;)V");
    VerifyOrReturn(nodeStateCtor != nullptr, ChipLogError(Controller, "Could not find NodeState constructor"));

    jobject map = nullptr;
    err         = JniReferences::GetInstance().CreateHashMap(map);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not create HashMap"));
    mNodeStateObj = env->NewObject(mNodeStateCls, nodeStateCtor, map);
}

void ReportCallback::OnReportEnd()
{
    // Transform C++ jobject pair list to a Java HashMap, and call onReport() on the Java callback.
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID onReportMethod;
    err = JniReferences::GetInstance().FindMethod(env, mReportCallbackRef, "onReport", "(Lchip/devicecontroller/model/NodeState;)V",
                                                  &onReportMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onReport method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mReportCallbackRef, onReportMethod, mNodeStateObj);
}

void ReportCallback::OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                     const app::StatusIB & aStatus)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    JNIEnv * env             = JniReferences::GetInstance().GetEnvForCurrentThread();
    jobject attributePathObj = nullptr;
    err                      = CreateChipAttributePath(aPath, attributePathObj);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create Java ChipAttributePath: %s", ErrorStr(err)));

    if (aPath.IsListItemOperation())
    {
        ReportError(attributePathObj, CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    if (aStatus.mStatus != Protocols::InteractionModel::Status::Success)
    {
        ReportError(attributePathObj, aStatus.mStatus);
        return;
    }

    if (apData == nullptr)
    {
        ReportError(attributePathObj, CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    TLV::TLVReader readerForJavaObject;
    TLV::TLVReader readerForJavaTLV;
    TLV::TLVReader readerForJson;
    readerForJavaObject.Init(*apData);
    readerForJavaTLV.Init(*apData);
    readerForJson.Init(*apData);

    jobject value = DecodeAttributeValue(aPath, readerForJavaObject, &err);
    // If we don't know this attribute, just skip it.
    VerifyOrReturn(err != CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(attributePathObj, err));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe(),
                   ReportError(attributePathObj, CHIP_JNI_ERROR_EXCEPTION_THROWN));

    // Create TLV byte array to pass to Java layer
    size_t bufferLen                  = readerForJavaTLV.GetRemainingLength() + readerForJavaTLV.GetLengthRead();
    std::unique_ptr<uint8_t[]> buffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferLen]);
    uint32_t size                     = 0;
    // The TLVReader's read head is not pointing to the first element in the container, instead of the container itself, use
    // a TLVWriter to get a TLV with a normalized TLV buffer (Wrapped with an anonymous tag, no extra "end of container" tag
    // at the end.)
    TLV::TLVWriter writer;
    writer.Init(buffer.get(), bufferLen);
    err = writer.CopyElement(TLV::AnonymousTag(), readerForJavaTLV);
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(attributePathObj, err));
    size = writer.GetLengthWritten();
    chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), size);

    // Convert TLV to JSON
    Json::Value json;
    err = TlvToJson(readerForJson, json);
    UtfString jsonString(env, JsonToString(json).c_str());

    // Create AttributeState object
    jclass attributeStateCls;
    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/AttributeState", attributeStateCls);
    VerifyOrReturn(attributeStateCls != nullptr, ChipLogError(Controller, "Could not find AttributeState class"));
    chip::JniClass attributeStateJniCls(attributeStateCls);
    jmethodID attributeStateCtor = env->GetMethodID(attributeStateCls, "<init>", "(Ljava/lang/Object;[BLjava/lang/String;)V");
    VerifyOrReturn(attributeStateCtor != nullptr, ChipLogError(Controller, "Could not find AttributeState constructor"));
    jobject attributeStateObj =
        env->NewObject(attributeStateCls, attributeStateCtor, value, jniByteArray.jniValue(), jsonString.jniValue());
    VerifyOrReturn(attributeStateObj != nullptr, ChipLogError(Controller, "Could not create AttributeState object"));

    // Add AttributeState to NodeState
    jmethodID addAttributeMethod;
    err = JniReferences::GetInstance().FindMethod(env, mNodeStateObj, "addAttribute",
                                                  "(IJJLchip/devicecontroller/model/AttributeState;)V", &addAttributeMethod);
    VerifyOrReturnError(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find addAttribute method"));
    env->CallVoidMethod(mNodeStateObj, addAttributeMethod, static_cast<jint>(aPath.mEndpointId),
                        static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mAttributeId), attributeStateObj);
    VerifyOrReturnError(!env->ExceptionCheck(), env->ExceptionDescribe());
}

CHIP_ERROR ReportCallback::CreateChipAttributePath(const app::ConcreteDataAttributePath & aPath, jobject & outObj)
{
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    jclass attributePathCls = nullptr;
    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/ChipAttributePath", attributePathCls);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    JniClass attributePathJniCls(attributePathCls);

    jmethodID attributePathCtor =
        env->GetStaticMethodID(attributePathCls, "newInstance", "(JJJ)Lchip/devicecontroller/model/ChipAttributePath;");
    VerifyOrReturnError(attributePathCtor != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    outObj =
        env->CallStaticObjectMethod(attributePathCls, attributePathCtor, aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
    VerifyOrReturnError(outObj != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    return err;
}

void ReportCallback::OnError(CHIP_ERROR aError)
{
    ReportError(nullptr, aError);
}

void ReportCallback::OnDone()
{
    JniReferences::GetInstance().GetEnvForCurrentThread()->DeleteGlobalRef(mWrapperCallbackRef);
}

void ReportCallback::OnSubscriptionEstablished(uint64_t aSubscriptionId)
{
    JniReferences::GetInstance().CallSubscriptionEstablished(mSubscriptionEstablishedCallbackRef);
}

void ReportCallback::ReportError(jobject attributePath, CHIP_ERROR err)
{
    ReportError(attributePath, ErrorStr(err), err.AsInteger());
}

void ReportCallback::ReportError(jobject attributePath, Protocols::InteractionModel::Status status)
{
    ReportError(attributePath, "IM Status", static_cast<std::underlying_type_t<Protocols::InteractionModel::Status>>(status));
}

void ReportCallback::ReportError(jobject attributePath, const char * message, ChipError::StorageType errorCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    jthrowable exception;
    err = AndroidClusterExceptions::GetInstance().CreateIllegalStateException(env, message, errorCode, exception);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create IllegalStateException: %s", ErrorStr(err)));

    jmethodID onErrorMethod;
    err = JniReferences::GetInstance().FindMethod(env, mReportCallbackRef, "onError",
                                                  "(Lchip/devicecontroller/model/ChipAttributePath;Ljava/lang/Exception;)V",
                                                  &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mReportCallbackRef, onErrorMethod, attributePath, exception);
}

} // namespace Controller
} // namespace chip
