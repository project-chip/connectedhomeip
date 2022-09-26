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
#include <controller/java/CHIPEventTLVValueDecoder.h>
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

void GetConnectedDeviceCallback::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                     SessionHandle & sessionHandle)
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

    OperationalDeviceProxy * device = new OperationalDeviceProxy(&exchangeMgr, sessionHandle);
    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, successMethod, reinterpret_cast<jlong>(device));
}

void GetConnectedDeviceCallback::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
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

ReportCallback::ReportCallback(jobject wrapperCallback, jobject subscriptionEstablishedCallback, jobject reportCallback,
                               jobject resubscriptionAttemptCallback) :
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
    if (resubscriptionAttemptCallback != nullptr)
    {
        mResubscriptionAttemptCallbackRef = env->NewGlobalRef(resubscriptionAttemptCallback);
        if (mResubscriptionAttemptCallbackRef == nullptr)
        {
            ChipLogError(Controller, "Could not create global reference for Java callback");
        }
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
        ReportError(attributePathObj, nullptr, CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    if (aStatus.mStatus != Protocols::InteractionModel::Status::Success)
    {
        ReportError(attributePathObj, nullptr, aStatus.mStatus);
        return;
    }

    if (apData == nullptr)
    {
        ReportError(attributePathObj, nullptr, CHIP_ERROR_INVALID_ARGUMENT);
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
    VerifyOrReturn(err != CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB);
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(attributePathObj, nullptr, err));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe(),
                   ReportError(attributePathObj, nullptr, CHIP_JNI_ERROR_EXCEPTION_THROWN));

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
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(attributePathObj, nullptr, err));
    size = writer.GetLengthWritten();
    chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), size);

    // Convert TLV to JSON
    Json::Value json;
    err = TlvToJson(readerForJson, json);
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(attributePathObj, nullptr, err));

    UtfString jsonString(env, JsonToString(json).c_str());

    // Create AttributeState object
    jclass attributeStateCls;
    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/AttributeState", attributeStateCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find AttributeState class"));
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
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find addAttribute method"));
    env->CallVoidMethod(mNodeStateObj, addAttributeMethod, static_cast<jint>(aPath.mEndpointId),
                        static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mAttributeId), attributeStateObj);
    VerifyOrReturnError(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void ReportCallback::OnEventData(const app::EventHeader & aEventHeader, TLV::TLVReader * apData, const app::StatusIB * apStatus)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    JNIEnv * env         = JniReferences::GetInstance().GetEnvForCurrentThread();
    jobject eventPathObj = nullptr;
    err                  = CreateChipEventPath(aEventHeader.mPath, eventPathObj);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create Java ChipEventPath: %s", ErrorStr(err)));

    if (apData == nullptr)
    {
        ReportError(nullptr, eventPathObj, CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    TLV::TLVReader readerForJavaObject;
    TLV::TLVReader readerForJavaTLV;
    TLV::TLVReader readerForJson;
    readerForJavaObject.Init(*apData);
    readerForJavaTLV.Init(*apData);
    readerForJson.Init(*apData);

    jlong eventNumber   = static_cast<jlong>(aEventHeader.mEventNumber);
    jlong priorityLevel = static_cast<jint>(aEventHeader.mPriorityLevel);
    jlong timestamp     = static_cast<jlong>(aEventHeader.mTimestamp.mValue);

    jobject value = DecodeEventValue(aEventHeader.mPath, readerForJavaObject, &err);
    // If we don't know this event, just skip it.
    VerifyOrReturn(err != CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB);
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(nullptr, eventPathObj, err));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe(),
                   ReportError(nullptr, eventPathObj, CHIP_JNI_ERROR_EXCEPTION_THROWN));

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
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(nullptr, eventPathObj, err));
    size = writer.GetLengthWritten();
    chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), size);

    // Convert TLV to JSON
    Json::Value json;
    err = TlvToJson(readerForJson, json);
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(nullptr, eventPathObj, err));

    UtfString jsonString(env, JsonToString(json).c_str());

    // Create EventState object
    jclass eventStateCls;
    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/EventState", eventStateCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find EventState class"));
    VerifyOrReturn(eventStateCls != nullptr, ChipLogError(Controller, "Could not find EventState class"));
    chip::JniClass eventStateJniCls(eventStateCls);
    jmethodID eventStateCtor = env->GetMethodID(eventStateCls, "<init>", "(JIJLjava/lang/Object;[BLjava/lang/String;)V");
    VerifyOrReturn(eventStateCtor != nullptr, ChipLogError(Controller, "Could not find EventState constructor"));
    jobject eventStateObj = env->NewObject(eventStateCls, eventStateCtor, eventNumber, priorityLevel, timestamp, value,
                                           jniByteArray.jniValue(), jsonString.jniValue());
    VerifyOrReturn(eventStateObj != nullptr, ChipLogError(Controller, "Could not create EventState object"));

    // Add EventState to NodeState
    jmethodID addEventMethod;
    err = JniReferences::GetInstance().FindMethod(env, mNodeStateObj, "addEvent", "(IJJLchip/devicecontroller/model/EventState;)V",
                                                  &addEventMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find addEvent method"));
    env->CallVoidMethod(mNodeStateObj, addEventMethod, static_cast<jint>(aEventHeader.mPath.mEndpointId),
                        static_cast<jlong>(aEventHeader.mPath.mClusterId), static_cast<jlong>(aEventHeader.mPath.mEventId),
                        eventStateObj);
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

CHIP_ERROR ReportCallback::CreateChipEventPath(const app::ConcreteEventPath & aPath, jobject & outObj)
{
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    jclass eventPathCls = nullptr;
    err                 = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/ChipEventPath", eventPathCls);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    JniClass eventPathJniCls(eventPathCls);

    jmethodID eventPathCtor =
        env->GetStaticMethodID(eventPathCls, "newInstance", "(JJJ)Lchip/devicecontroller/model/ChipEventPath;");
    VerifyOrReturnError(eventPathCtor != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    outObj = env->CallStaticObjectMethod(eventPathCls, eventPathCtor, aPath.mEndpointId, aPath.mClusterId, aPath.mEventId);
    VerifyOrReturnError(outObj != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    return err;
}

void ReportCallback::OnError(CHIP_ERROR aError)
{
    ReportError(nullptr, nullptr, aError);
}

void ReportCallback::OnDone(app::ReadClient *)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID onDoneMethod;
    err = JniReferences::GetInstance().FindMethod(env, mReportCallbackRef, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mReportCallbackRef, onDoneMethod);

    JniReferences::GetInstance().GetEnvForCurrentThread()->DeleteGlobalRef(mWrapperCallbackRef);
}

void ReportCallback::OnSubscriptionEstablished(SubscriptionId aSubscriptionId)
{
    JniReferences::GetInstance().CallSubscriptionEstablished(mSubscriptionEstablishedCallbackRef);
}

CHIP_ERROR ReportCallback::OnResubscriptionNeeded(app::ReadClient * apReadClient, CHIP_ERROR aTerminationCause)
{
    VerifyOrReturnLogError(mResubscriptionAttemptCallbackRef != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ReturnErrorOnFailure(app::ReadClient::Callback::OnResubscriptionNeeded(apReadClient, aTerminationCause));

    jmethodID onResubscriptionAttemptMethod;
    ReturnLogErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, mResubscriptionAttemptCallbackRef, "onResubscriptionAttempt", "(II)V", &onResubscriptionAttemptMethod));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mResubscriptionAttemptCallbackRef, onResubscriptionAttemptMethod, aTerminationCause.AsInteger(),
                        apReadClient->ComputeTimeTillNextSubscription());

    return CHIP_NO_ERROR;
}

void ReportCallback::ReportError(jobject attributePath, jobject eventPath, CHIP_ERROR err)
{
    ReportError(attributePath, eventPath, ErrorStr(err), err.AsInteger());
}

void ReportCallback::ReportError(jobject attributePath, jobject eventPath, Protocols::InteractionModel::Status status)
{
    ReportError(attributePath, eventPath, "IM Status",
                static_cast<std::underlying_type_t<Protocols::InteractionModel::Status>>(status));
}

void ReportCallback::ReportError(jobject attributePath, jobject eventPath, const char * message, ChipError::StorageType errorCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    jthrowable exception;
    err = AndroidClusterExceptions::GetInstance().CreateIllegalStateException(env, message, errorCode, exception);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create IllegalStateException: %s", ErrorStr(err)));

    jmethodID onErrorMethod;
    err = JniReferences::GetInstance().FindMethod(
        env, mReportCallbackRef, "onError",
        "(Lchip/devicecontroller/model/ChipAttributePath;Lchip/devicecontroller/model/ChipEventPath;Ljava/lang/Exception;)V",
        &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mReportCallbackRef, onErrorMethod, attributePath, eventPath, exception);
}

ReportEventCallback::ReportEventCallback(jobject wrapperCallback, jobject subscriptionEstablishedCallback, jobject reportCallback,
                                         jobject resubscriptionAttemptCallback) :
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
    if (resubscriptionAttemptCallback != nullptr)
    {
        mResubscriptionAttemptCallbackRef = env->NewGlobalRef(resubscriptionAttemptCallback);
        if (mResubscriptionAttemptCallbackRef == nullptr)
        {
            ChipLogError(Controller, "Could not create global reference for Java callback");
        }
    }
}

ReportEventCallback::~ReportEventCallback()
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

void ReportEventCallback::OnReportBegin()
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

void ReportEventCallback::OnReportEnd()
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

void ReportEventCallback::OnEventData(const app::EventHeader & aEventHeader, TLV::TLVReader * apData,
                                      const app::StatusIB * apStatus)
{
    CHIP_ERROR err       = CHIP_NO_ERROR;
    JNIEnv * env         = JniReferences::GetInstance().GetEnvForCurrentThread();
    jobject eventPathObj = nullptr;
    err                  = CreateChipEventPath(aEventHeader.mPath, eventPathObj);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create Java ChipEventPath: %s", ErrorStr(err)));

    if (apData == nullptr)
    {
        ReportError(eventPathObj, CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    TLV::TLVReader readerForJavaObject;
    TLV::TLVReader readerForJavaTLV;
    TLV::TLVReader readerForJson;
    readerForJavaObject.Init(*apData);
    readerForJavaTLV.Init(*apData);
    readerForJson.Init(*apData);

    jlong eventNumber   = static_cast<jlong>(aEventHeader.mEventNumber);
    jlong priorityLevel = static_cast<jint>(aEventHeader.mPriorityLevel);
    jlong timestamp     = static_cast<jlong>(aEventHeader.mTimestamp.mValue);

    jobject value = DecodeEventValue(aEventHeader.mPath, readerForJavaObject, &err);
    // If we don't know this event, just skip it.
    VerifyOrReturn(err != CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB);
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(eventPathObj, err));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe(), ReportError(eventPathObj, CHIP_JNI_ERROR_EXCEPTION_THROWN));

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
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(eventPathObj, err));
    size = writer.GetLengthWritten();
    chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), size);

    // Convert TLV to JSON
    Json::Value json;
    err = TlvToJson(readerForJson, json);
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(eventPathObj, err));

    UtfString jsonString(env, JsonToString(json).c_str());

    // Create EventState object
    jclass eventStateCls;
    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/EventState", eventStateCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find EventState class"));
    VerifyOrReturn(eventStateCls != nullptr, ChipLogError(Controller, "Could not find EventState class"));
    chip::JniClass eventStateJniCls(eventStateCls);
    jmethodID eventStateCtor = env->GetMethodID(eventStateCls, "<init>", "(JIJLjava/lang/Object;[BLjava/lang/String;)V");
    VerifyOrReturn(eventStateCtor != nullptr, ChipLogError(Controller, "Could not find EventState constructor"));
    jobject eventStateObj = env->NewObject(eventStateCls, eventStateCtor, eventNumber, priorityLevel, timestamp, value,
                                           jniByteArray.jniValue(), jsonString.jniValue());
    VerifyOrReturn(eventStateObj != nullptr, ChipLogError(Controller, "Could not create EventState object"));

    // Add EventState to NodeState
    jmethodID addEventMethod;
    err = JniReferences::GetInstance().FindMethod(env, mNodeStateObj, "addEvent", "(IJJLchip/devicecontroller/model/EventState;)V",
                                                  &addEventMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find addEvent method"));
    env->CallVoidMethod(mNodeStateObj, addEventMethod, static_cast<jint>(aEventHeader.mPath.mEndpointId),
                        static_cast<jlong>(aEventHeader.mPath.mClusterId), static_cast<jlong>(aEventHeader.mPath.mEventId),
                        eventStateObj);
    VerifyOrReturnError(!env->ExceptionCheck(), env->ExceptionDescribe());
}

CHIP_ERROR ReportEventCallback::CreateChipEventPath(const app::ConcreteEventPath & aPath, jobject & outObj)
{
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    jclass eventPathCls = nullptr;
    err                 = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/ChipEventPath", eventPathCls);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    JniClass eventPathJniCls(eventPathCls);

    jmethodID eventPathCtor =
        env->GetStaticMethodID(eventPathCls, "newInstance", "(JJJ)Lchip/devicecontroller/model/ChipEventPath;");
    VerifyOrReturnError(eventPathCtor != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    outObj = env->CallStaticObjectMethod(eventPathCls, eventPathCtor, aPath.mEndpointId, aPath.mClusterId, aPath.mEventId);
    VerifyOrReturnError(outObj != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    return err;
}

void ReportEventCallback::OnError(CHIP_ERROR aError)
{
    ReportError(nullptr, aError);
}

void ReportEventCallback::OnDone(app::ReadClient *)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID onDoneMethod;
    err = JniReferences::GetInstance().FindMethod(env, mReportCallbackRef, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mReportCallbackRef, onDoneMethod);

    JniReferences::GetInstance().GetEnvForCurrentThread()->DeleteGlobalRef(mWrapperCallbackRef);
}

void ReportEventCallback::OnSubscriptionEstablished(SubscriptionId aSubscriptionId)
{
    JniReferences::GetInstance().CallSubscriptionEstablished(mSubscriptionEstablishedCallbackRef);
}

CHIP_ERROR ReportEventCallback::OnResubscriptionNeeded(app::ReadClient * apReadClient, CHIP_ERROR aTerminationCause)
{
    VerifyOrReturnLogError(mResubscriptionAttemptCallbackRef != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ReturnErrorOnFailure(app::ReadClient::Callback::OnResubscriptionNeeded(apReadClient, aTerminationCause));

    jmethodID onResubscriptionAttemptMethod;
    ReturnLogErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, mResubscriptionAttemptCallbackRef, "onResubscriptionAttempt", "(II)V", &onResubscriptionAttemptMethod));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mResubscriptionAttemptCallbackRef, onResubscriptionAttemptMethod, aTerminationCause.AsInteger(),
                        apReadClient->ComputeTimeTillNextSubscription());

    return CHIP_NO_ERROR;
}

void ReportEventCallback::ReportError(jobject eventPath, CHIP_ERROR err)
{
    ReportError(eventPath, ErrorStr(err), err.AsInteger());
}

void ReportEventCallback::ReportError(jobject eventPath, Protocols::InteractionModel::Status status)
{
    ReportError(eventPath, "IM Status", static_cast<std::underlying_type_t<Protocols::InteractionModel::Status>>(status));
}

void ReportEventCallback::ReportError(jobject eventPath, const char * message, ChipError::StorageType errorCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    jthrowable exception;
    err = AndroidClusterExceptions::GetInstance().CreateIllegalStateException(env, message, errorCode, exception);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create IllegalStateException: %s", ErrorStr(err)));

    jmethodID onErrorMethod;
    err = JniReferences::GetInstance().FindMethod(
        env, mReportCallbackRef, "onError", "(Lchip/devicecontroller/model/ChipEventPath;Ljava/lang/Exception;)V", &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mReportCallbackRef, onErrorMethod, eventPath, exception);
}

} // namespace Controller
} // namespace chip
