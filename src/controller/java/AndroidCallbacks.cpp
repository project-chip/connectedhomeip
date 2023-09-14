/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
#include <controller/java/AndroidControllerExceptions.h>
#if USE_JAVA_TLV_ENCODE_DECODE
#include <controller/java/CHIPAttributeTLVValueDecoder.h>
#include <controller/java/CHIPEventTLVValueDecoder.h>
#endif
#include <app/EventLoggingTypes.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/jsontlv/JsonToTlv.h>
#include <lib/support/jsontlv/TlvToJson.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <type_traits>

namespace chip {
namespace Controller {

static const int MILLIS_SINCE_BOOT  = 0;
static const int MILLIS_SINCE_EPOCH = 1;
// Add the bytes for attribute tag(1:control + 8:tag + 8:length) and structure(1:struct + 1:close container)
static const int EXTRA_SPACE_FOR_ATTRIBUTE_TAG = 19;

class JniChipAttributePath
{
public:
    JniChipAttributePath(JNIEnv * env, const app::ConcreteDataAttributePath & aPath) : mEnv(env)
    {
        jclass attributePathCls = nullptr;
        ReturnOnFailure(mError = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/ChipAttributePath",
                                                                          attributePathCls));
        JniClass attributePathJniCls(attributePathCls);

        jmethodID attributePathCtor =
            env->GetStaticMethodID(attributePathCls, "newInstance", "(IJJ)Lchip/devicecontroller/model/ChipAttributePath;");
        VerifyOrReturn(attributePathCtor != nullptr, mError = CHIP_JNI_ERROR_METHOD_NOT_FOUND);
        mData = env->CallStaticObjectMethod(attributePathCls, attributePathCtor, static_cast<jint>(aPath.mEndpointId),
                                            static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mAttributeId));
        VerifyOrReturn(mData != nullptr, mError = CHIP_JNI_ERROR_NULL_OBJECT);
        return;
    }

    ~JniChipAttributePath() { mEnv->DeleteLocalRef(mData); }

    CHIP_ERROR GetError() { return mError; }
    jobject GetData() { return mData; }

private:
    JNIEnv * mEnv     = nullptr;
    jobject mData     = nullptr;
    CHIP_ERROR mError = CHIP_NO_ERROR;
};

class JniChipEventPath
{
public:
    JniChipEventPath(JNIEnv * env, const app::ConcreteEventPath & aPath) : mEnv(env)
    {
        jclass eventPathCls = nullptr;
        ReturnOnFailure(
            mError = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/ChipEventPath", eventPathCls));
        JniClass eventPathJniCls(eventPathCls);

        jmethodID eventPathCtor =
            env->GetStaticMethodID(eventPathCls, "newInstance", "(IJJ)Lchip/devicecontroller/model/ChipEventPath;");
        VerifyOrReturn(eventPathCtor != nullptr, mError = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

        mData = env->CallStaticObjectMethod(eventPathCls, eventPathCtor, static_cast<jint>(aPath.mEndpointId),
                                            static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mEventId));
        VerifyOrReturn(mData != nullptr, mError = CHIP_JNI_ERROR_NULL_OBJECT);
        return;
    }

    ~JniChipEventPath() { mEnv->DeleteLocalRef(mData); }

    CHIP_ERROR GetError() { return mError; }
    jobject GetData() { return mData; }

private:
    JNIEnv * mEnv     = nullptr;
    jobject mData     = nullptr;
    CHIP_ERROR mError = CHIP_NO_ERROR;
};

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
                                                     const SessionHandle & sessionHandle)
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
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
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

    jthrowable exception;
    CHIP_ERROR err = AndroidControllerExceptions::GetInstance().CreateAndroidControllerException(env, ErrorStr(error),
                                                                                                 error.AsInteger(), exception);
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(Controller,
                     "Unable to create AndroidControllerException on GetConnectedDeviceCallback::OnDeviceConnectionFailureFn: %s",
                     ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, failureMethod, peerId.GetNodeId(), exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

ReportCallback::ReportCallback(jobject wrapperCallback, jobject subscriptionEstablishedCallback, jobject reportCallback,
                               jobject resubscriptionAttemptCallback) :
    mClusterCacheAdapter(*this, Optional<EventNumber>::Missing(), false /*cacheData*/)
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

void ReportCallback::OnDeallocatePaths(app::ReadPrepareParams && aReadPrepareParams)
{
    if (aReadPrepareParams.mpAttributePathParamsList != nullptr)
    {
        delete[] aReadPrepareParams.mpAttributePathParamsList;
    }

    if (aReadPrepareParams.mpEventPathParamsList != nullptr)
    {
        delete[] aReadPrepareParams.mpEventPathParamsList;
    }
}

void ReportCallback::OnReportEnd()
{
    UpdateClusterDataVersion();

    // Transform C++ jobject pair list to a Java HashMap, and call onReport() on the Java callback.
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID onReportMethod;
    err = JniReferences::GetInstance().FindMethod(env, mReportCallbackRef, "onReport", "(Lchip/devicecontroller/model/NodeState;)V",
                                                  &onReportMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onReport method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mReportCallbackRef, onReportMethod, mNodeStateObj);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

// Convert TLV blob to Json with structure, the element's tag is replaced with the actual attributeId.
CHIP_ERROR ConvertReportTlvToJson(const uint32_t id, TLV::TLVReader & data, std::string & json)
{
    TLV::TLVWriter writer;
    TLV::TLVReader readerForJavaTLV;
    uint32_t size = 0;
    readerForJavaTLV.Init(data);
    size_t bufferLen                  = readerForJavaTLV.GetTotalLength() + EXTRA_SPACE_FOR_ATTRIBUTE_TAG;
    std::unique_ptr<uint8_t[]> buffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferLen]);
    writer.Init(buffer.get(), bufferLen);
    TLV::TLVType outer;

    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outer));
    TLV::Tag tag;
    ReturnErrorOnFailure(ConvertTlvTag(id, tag));
    ReturnErrorOnFailure(writer.CopyElement(tag, readerForJavaTLV));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    size = writer.GetLengthWritten();

    TLV::TLVReader readerForJson;
    readerForJson.Init(buffer.get(), size);
    ReturnErrorOnFailure(readerForJson.Next());
    // Convert TLV to JSON
    return TlvToJson(readerForJson, json);
}

void ReportCallback::OnAttributeData(const app::ConcreteDataAttributePath & aPath, TLV::TLVReader * apData,
                                     const app::StatusIB & aStatus)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    JniChipAttributePath attributePathObj(env, aPath);
    VerifyOrReturn(attributePathObj.GetError() == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Unable to create Java ChipAttributePath: %s", ErrorStr(err)));

    if (aPath.IsListItemOperation())
    {
        ReportError(attributePathObj.GetData(), nullptr, CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    if (aStatus.mStatus != Protocols::InteractionModel::Status::Success)
    {
        ReportError(attributePathObj.GetData(), nullptr, aStatus.mStatus);
        return;
    }

    if (apData == nullptr)
    {
        ReportError(attributePathObj.GetData(), nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    TLV::TLVReader readerForJavaTLV;
    readerForJavaTLV.Init(*apData);

    jobject value = nullptr;
#if USE_JAVA_TLV_ENCODE_DECODE
    TLV::TLVReader readerForJavaObject;
    readerForJavaObject.Init(*apData);

    value = DecodeAttributeValue(aPath, readerForJavaObject, &err);
    // If we don't know this attribute, suppress it.
    if (err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB)
    {
        err = CHIP_NO_ERROR;
    }

    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(attributePathObj.GetData(), nullptr, err));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe(),
                   ReportError(attributePathObj.GetData(), nullptr, CHIP_JNI_ERROR_EXCEPTION_THROWN));
#endif
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
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(attributePathObj.GetData(), nullptr, err));
    size = writer.GetLengthWritten();
    chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), size);

    // Convert TLV to JSON
    std::string json;
    err = ConvertReportTlvToJson(static_cast<uint32_t>(aPath.mAttributeId), *apData, json);
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(attributePathObj.GetData(), nullptr, err));
    UtfString jsonString(env, json.c_str());

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
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
    env->DeleteLocalRef(attributeStateObj);

    UpdateClusterDataVersion();
}

void ReportCallback::UpdateClusterDataVersion()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    chip::app::ConcreteClusterPath lastConcreteClusterPath;

    if (mClusterCacheAdapter.GetLastReportDataPath(lastConcreteClusterPath) != CHIP_NO_ERROR)
    {
        return;
    }

    if (!lastConcreteClusterPath.IsValidConcreteClusterPath())
    {
        return;
    }

    chip::Optional<chip::DataVersion> committedDataVersion;
    if (mClusterCacheAdapter.GetVersion(lastConcreteClusterPath, committedDataVersion) != CHIP_NO_ERROR)
    {
        return;
    }
    if (!committedDataVersion.HasValue())
    {
        return;
    }

    // SetDataVersion to NodeState
    jmethodID setDataVersionMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, mNodeStateObj, "setDataVersion", "(IJJ)V", &setDataVersionMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find setDataVersion method"));
    env->CallVoidMethod(mNodeStateObj, setDataVersionMethod, static_cast<jint>(lastConcreteClusterPath.mEndpointId),
                        static_cast<jlong>(lastConcreteClusterPath.mClusterId), static_cast<jlong>(committedDataVersion.Value()));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void ReportCallback::OnEventData(const app::EventHeader & aEventHeader, TLV::TLVReader * apData, const app::StatusIB * apStatus)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    JniChipEventPath eventPathObj(env, aEventHeader.mPath);
    VerifyOrReturn(eventPathObj.GetError() == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Unable to create Java ChipEventPath: %s", ErrorStr(err)));

    if (apData == nullptr)
    {
        ReportError(nullptr, eventPathObj.GetData(), CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    TLV::TLVReader readerForJavaTLV;
    readerForJavaTLV.Init(*apData);

    jlong eventNumber    = static_cast<jlong>(aEventHeader.mEventNumber);
    jint priorityLevel   = static_cast<jint>(aEventHeader.mPriorityLevel);
    jlong timestampValue = static_cast<jlong>(aEventHeader.mTimestamp.mValue);

    jint timestampType = 0;
    if (aEventHeader.mTimestamp.mType == app::Timestamp::Type::kSystem)
    {
        timestampType = static_cast<jint>(MILLIS_SINCE_BOOT);
    }
    else if (aEventHeader.mTimestamp.mType == app::Timestamp::Type::kEpoch)
    {
        timestampType = static_cast<jint>(MILLIS_SINCE_EPOCH);
    }
    else
    {
        ChipLogError(Controller, "Unsupported event timestamp type");
        ReportError(nullptr, eventPathObj.GetData(), CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    jobject value = nullptr;
#if USE_JAVA_TLV_ENCODE_DECODE
    TLV::TLVReader readerForJavaObject;
    readerForJavaObject.Init(*apData);
    value = DecodeEventValue(aEventHeader.mPath, readerForJavaObject, &err);
    // If we don't know this event, just skip it.
    if (err == CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(nullptr, eventPathObj.GetData(), err));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe(),
                   ReportError(nullptr, eventPathObj.GetData(), CHIP_JNI_ERROR_EXCEPTION_THROWN));
#endif

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
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(nullptr, eventPathObj.GetData(), err));
    size = writer.GetLengthWritten();
    chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), size);

    // Convert TLV to JSON
    std::string json;
    err = ConvertReportTlvToJson(static_cast<uint32_t>(aEventHeader.mPath.mEventId), *apData, json);
    VerifyOrReturn(err == CHIP_NO_ERROR, ReportError(eventPathObj.GetData(), nullptr, err));
    UtfString jsonString(env, json.c_str());

    // Create EventState object
    jclass eventStateCls;
    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/EventState", eventStateCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Failed to find EventState class"));
    VerifyOrReturn(eventStateCls != nullptr, ChipLogError(Controller, "Could not find EventState class"));
    chip::JniClass eventStateJniCls(eventStateCls);
    jmethodID eventStateCtor = env->GetMethodID(eventStateCls, "<init>", "(JIIJLjava/lang/Object;[BLjava/lang/String;)V");
    VerifyOrReturn(eventStateCtor != nullptr, ChipLogError(Controller, "Could not find EventState constructor"));
    jobject eventStateObj = env->NewObject(eventStateCls, eventStateCtor, eventNumber, priorityLevel, timestampType, timestampValue,
                                           value, jniByteArray.jniValue(), jsonString.jniValue());
    VerifyOrReturn(eventStateObj != nullptr, ChipLogError(Controller, "Could not create EventState object"));

    // Add EventState to NodeState
    jmethodID addEventMethod;
    err = JniReferences::GetInstance().FindMethod(env, mNodeStateObj, "addEvent", "(IJJLchip/devicecontroller/model/EventState;)V",
                                                  &addEventMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find addEvent method"));
    env->CallVoidMethod(mNodeStateObj, addEventMethod, static_cast<jint>(aEventHeader.mPath.mEndpointId),
                        static_cast<jlong>(aEventHeader.mPath.mClusterId), static_cast<jlong>(aEventHeader.mPath.mEventId),
                        eventStateObj);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
    env->DeleteLocalRef(eventStateObj);
}

CHIP_ERROR InvokeCallback::CreateInvokeElement(const app::ConcreteCommandPath & aPath, TLV::TLVReader * apData, jobject & outObj)
{
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    CHIP_ERROR err = CHIP_NO_ERROR;

    jclass invokeElementCls = nullptr;
    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/model/InvokeElement", invokeElementCls);
    ReturnErrorOnFailure(err);
    JniClass invokeElementJniCls(invokeElementCls);

    jmethodID invokeElementCtor = env->GetStaticMethodID(invokeElementCls, "newInstance",
                                                         "(IJJ[BLjava/lang/String;)Lchip/devicecontroller/model/InvokeElement;");
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
    VerifyOrReturnError(invokeElementCtor != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    if (apData != nullptr)
    {
        TLV::TLVReader readerForJavaTLV;
        TLV::TLVReader readerForJson;
        readerForJavaTLV.Init(*apData);

        // Create TLV byte array to pass to Java layer
        size_t bufferLen = readerForJavaTLV.GetRemainingLength() + readerForJavaTLV.GetLengthRead();
        ;
        std::unique_ptr<uint8_t[]> buffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferLen]);
        uint32_t size                     = 0;

        TLV::TLVWriter writer;
        writer.Init(buffer.get(), bufferLen);
        err = writer.CopyElement(TLV::AnonymousTag(), readerForJavaTLV);
        ReturnErrorOnFailure(err);
        size = writer.GetLengthWritten();
        chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), size);

        // Convert TLV to JSON
        std::string json;
        readerForJson.Init(buffer.get(), size);
        err = readerForJson.Next();
        ReturnErrorOnFailure(err);
        err = TlvToJson(readerForJson, json);
        ReturnErrorOnFailure(err);
        UtfString jsonString(env, json.c_str());
        outObj = env->CallStaticObjectMethod(invokeElementCls, invokeElementCtor, static_cast<jint>(aPath.mEndpointId),
                                             static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mCommandId),
                                             jniByteArray.jniValue(), jsonString.jniValue());
    }
    else
    {
        outObj = env->CallStaticObjectMethod(invokeElementCls, invokeElementCtor, static_cast<jint>(aPath.mEndpointId),
                                             static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mCommandId), nullptr,
                                             nullptr);
    }
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

    if (mReadClient != nullptr)
    {
        Platform::Delete(mReadClient);
    }
    mReadClient = nullptr;

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mReportCallbackRef, onDoneMethod);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
    JniReferences::GetInstance().GetEnvForCurrentThread()->DeleteGlobalRef(mWrapperCallbackRef);
}

void ReportCallback::OnSubscriptionEstablished(SubscriptionId aSubscriptionId)
{
    DeviceLayer::StackUnlock unlock;
    JniReferences::GetInstance().CallSubscriptionEstablished(mSubscriptionEstablishedCallbackRef, aSubscriptionId);
}

CHIP_ERROR ReportCallback::OnResubscriptionNeeded(app::ReadClient * apReadClient, CHIP_ERROR aTerminationCause)
{
    VerifyOrReturnLogError(mResubscriptionAttemptCallbackRef != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    ReturnErrorOnFailure(app::ReadClient::Callback::OnResubscriptionNeeded(apReadClient, aTerminationCause));

    jmethodID onResubscriptionAttemptMethod;
    ReturnLogErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, mResubscriptionAttemptCallbackRef, "onResubscriptionAttempt", "(JJ)V", &onResubscriptionAttemptMethod));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mResubscriptionAttemptCallbackRef, onResubscriptionAttemptMethod,
                        static_cast<jlong>(aTerminationCause.AsInteger()),
                        static_cast<jlong>(apReadClient->ComputeTimeTillNextSubscription()));
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
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

    ChipLogError(Controller, "ReportCallback::ReportError is called with %u", errorCode);
    jthrowable exception;
    err = AndroidControllerExceptions::GetInstance().CreateAndroidControllerException(env, message, errorCode, exception);
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(Controller, "Unable to create AndroidControllerException on ReportCallback::ReportError: %s", ErrorStr(err)));

    jmethodID onErrorMethod;
    err = JniReferences::GetInstance().FindMethod(
        env, mReportCallbackRef, "onError",
        "(Lchip/devicecontroller/model/ChipAttributePath;Lchip/devicecontroller/model/ChipEventPath;Ljava/lang/Exception;)V",
        &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mReportCallbackRef, onErrorMethod, attributePath, eventPath, exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

WriteAttributesCallback::WriteAttributesCallback(jobject wrapperCallback, jobject javaCallback) : mChunkedWriteCallback(this)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    mWrapperCallbackRef = env->NewGlobalRef(wrapperCallback);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
    if (mWrapperCallbackRef == nullptr)
    {
        ChipLogError(Controller, "Could not create global reference for Wrapper WriteAttributesCallback");
    }
    mJavaCallbackRef = env->NewGlobalRef(javaCallback);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
    if (mJavaCallbackRef == nullptr)
    {
        ChipLogError(Controller, "Could not create global reference for Java WriteAttributesCallback");
    }
}

WriteAttributesCallback::~WriteAttributesCallback()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    env->DeleteGlobalRef(mJavaCallbackRef);
    if (mWriteClient != nullptr)
    {
        Platform::Delete(mWriteClient);
    }
}

void WriteAttributesCallback::OnResponse(const app::WriteClient * apWriteClient, const app::ConcreteDataAttributePath & aPath,
                                         app::StatusIB aStatus)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    JniChipAttributePath attributePathObj(env, aPath);
    VerifyOrReturn(attributePathObj.GetError() == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Unable to create Java ChipAttributePath: %s", ErrorStr(err)));

    if (aStatus.mStatus != Protocols::InteractionModel::Status::Success)
    {
        ReportError(attributePathObj.GetData(), aStatus.mStatus);
        return;
    }

    jmethodID onResponseMethod;
    err = JniReferences::GetInstance().FindMethod(env, mJavaCallbackRef, "onResponse",
                                                  "(Lchip/devicecontroller/model/ChipAttributePath;)V", &onResponseMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mJavaCallbackRef, onResponseMethod, attributePathObj.GetData());
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void WriteAttributesCallback::OnError(const app::WriteClient * apWriteClient, CHIP_ERROR aError)
{
    ReportError(nullptr, aError);
}

void WriteAttributesCallback::OnDone(app::WriteClient *)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID onDoneMethod;
    err = JniReferences::GetInstance().FindMethod(env, mJavaCallbackRef, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mJavaCallbackRef, onDoneMethod);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
    JniReferences::GetInstance().GetEnvForCurrentThread()->DeleteGlobalRef(mWrapperCallbackRef);
}

void WriteAttributesCallback::ReportError(jobject attributePath, CHIP_ERROR err)
{
    ReportError(attributePath, ErrorStr(err), err.AsInteger());
}

void WriteAttributesCallback::ReportError(jobject attributePath, Protocols::InteractionModel::Status status)
{
    ReportError(attributePath, "IM Status", static_cast<std::underlying_type_t<Protocols::InteractionModel::Status>>(status));
}

void WriteAttributesCallback::ReportError(jobject attributePath, const char * message, ChipError::StorageType errorCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogError(Controller, "WriteAttributesCallback::ReportError is called with %u", errorCode);
    jthrowable exception;
    err = AndroidControllerExceptions::GetInstance().CreateAndroidControllerException(env, message, errorCode, exception);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(Controller,
                                "Unable to create AndroidControllerException on WriteAttributesCallback::ReportError: %s",
                                ErrorStr(err)));

    jmethodID onErrorMethod;
    err = JniReferences::GetInstance().FindMethod(env, mJavaCallbackRef, "onError",
                                                  "(Lchip/devicecontroller/model/ChipAttributePath;Ljava/lang/Exception;)V",
                                                  &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mJavaCallbackRef, onErrorMethod, attributePath, exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

InvokeCallback::InvokeCallback(jobject wrapperCallback, jobject javaCallback)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    mWrapperCallbackRef = env->NewGlobalRef(wrapperCallback);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
    if (mWrapperCallbackRef == nullptr)
    {
        ChipLogError(Controller, "Could not create global reference for Wrapper InvokeCallback");
    }
    mJavaCallbackRef = env->NewGlobalRef(javaCallback);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
    if (mJavaCallbackRef == nullptr)
    {
        ChipLogError(Controller, "Could not create global reference for Java InvokeCallback");
    }
}

InvokeCallback::~InvokeCallback()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    env->DeleteGlobalRef(mJavaCallbackRef);
    if (mCommandSender != nullptr)
    {
        Platform::Delete(mCommandSender);
    }
}

void InvokeCallback::OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aPath,
                                const app::StatusIB & aStatusIB, TLV::TLVReader * apData)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    JNIEnv * env             = JniReferences::GetInstance().GetEnvForCurrentThread();
    jobject invokeElementObj = nullptr;
    jmethodID onResponseMethod;

    err = CreateInvokeElement(aPath, apData, invokeElementObj);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create Java InvokeElement: %s", ErrorStr(err)));
    err = JniReferences::GetInstance().FindMethod(env, mJavaCallbackRef, "onResponse",
                                                  "(Lchip/devicecontroller/model/InvokeElement;J)V", &onResponseMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mJavaCallbackRef, onResponseMethod, invokeElementObj, static_cast<jlong>(aStatusIB.mStatus));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void InvokeCallback::OnError(const app::CommandSender * apCommandSender, CHIP_ERROR aError)
{
    ReportError(aError);
}

void InvokeCallback::OnDone(app::CommandSender * apCommandSender)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID onDoneMethod;
    err = JniReferences::GetInstance().FindMethod(env, mJavaCallbackRef, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mJavaCallbackRef, onDoneMethod);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
    JniReferences::GetInstance().GetEnvForCurrentThread()->DeleteGlobalRef(mWrapperCallbackRef);
}

void InvokeCallback::ReportError(CHIP_ERROR err)
{
    ReportError(ErrorStr(err), err.AsInteger());
}

void InvokeCallback::ReportError(Protocols::InteractionModel::Status status)
{
    ReportError("IM Status", static_cast<std::underlying_type_t<Protocols::InteractionModel::Status>>(status));
}

void InvokeCallback::ReportError(const char * message, ChipError::StorageType errorCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();

    ChipLogError(Controller, "InvokeCallback::ReportError is called with %u", errorCode);
    jthrowable exception;
    err = AndroidControllerExceptions::GetInstance().CreateAndroidControllerException(env, message, errorCode, exception);
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(Controller, "Unable to create AndroidControllerException: %s on InvokeCallback::ReportError", ErrorStr(err)));

    jmethodID onErrorMethod;
    err = JniReferences::GetInstance().FindMethod(env, mJavaCallbackRef, "onError", "(Ljava/lang/Exception;)V", &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(mJavaCallbackRef, onErrorMethod, exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

} // namespace Controller
} // namespace chip
