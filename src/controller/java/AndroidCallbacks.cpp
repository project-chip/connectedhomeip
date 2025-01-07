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
#include <controller/java/AndroidConnectionFailureExceptions.h>
#include <controller/java/AndroidControllerExceptions.h>
#ifdef USE_JAVA_TLV_ENCODE_DECODE
#include <controller/java/CHIPAttributeTLVValueDecoder.h>
#include <controller/java/CHIPEventTLVValueDecoder.h>
#endif
#include <app/EventLoggingTypes.h>
#include <jni.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/jsontlv/JsonToTlv.h>
#include <lib/support/jsontlv/TlvToJson.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>
#include <string>
#include <type_traits>

namespace chip {
namespace Controller {

static const int MILLIS_SINCE_BOOT  = 0;
static const int MILLIS_SINCE_EPOCH = 1;
// Add the bytes for attribute tag(1:control + 8:tag + 8:length) and structure(1:struct + 1:close container)
static const int EXTRA_SPACE_FOR_ATTRIBUTE_TAG = 19;

jobject DecodeGeneralTLVValue(JNIEnv * env, TLV::TLVReader & readerForGeneralValueObject, CHIP_ERROR & err);

GetConnectedDeviceCallback::GetConnectedDeviceCallback(jobject wrapperCallback, jobject javaCallback,
                                                       const char * callbackClassSignature) :
    mOnSuccess(OnDeviceConnectedFn, this),
    mOnFailure(OnDeviceConnectionFailureFn, this), mCallbackClassSignature(callbackClassSignature)
{
    VerifyOrReturn(mWrapperCallbackRef.Init(wrapperCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mWrapperCallbackRef in %s", __func__));
    VerifyOrReturn(mJavaCallbackRef.Init(javaCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mJavaCallbackRef in %s", __func__));
}

GetConnectedDeviceCallback::~GetConnectedDeviceCallback() {}

void GetConnectedDeviceCallback::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                     const SessionHandle & sessionHandle)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    auto * self = static_cast<GetConnectedDeviceCallback *>(context);
    VerifyOrReturn(self->mJavaCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mJavaCallbackRef is not valid in %s", __func__));
    jobject javaCallback = self->mJavaCallbackRef.ObjectRef();
    JniLocalReferenceScope scope(env);
    // Release wrapper's global ref so application can clean up the actual callback underneath.
    VerifyOrReturn(self->mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    JniGlobalReference globalRef(std::move(self->mWrapperCallbackRef));

    jclass getConnectedDeviceCallbackCls = nullptr;
    CHIP_ERROR err =
        JniReferences::GetInstance().GetLocalClassRef(env, self->mCallbackClassSignature, getConnectedDeviceCallbackCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "GetLocalClassRef Error! : %" CHIP_ERROR_FORMAT, err.Format()));
    VerifyOrReturn(getConnectedDeviceCallbackCls != nullptr,
                   ChipLogError(Controller, "Could not find GetConnectedDeviceCallback class"));

    jmethodID successMethod;
    err = JniReferences::GetInstance().FindMethod(env, javaCallback, "onDeviceConnected", "(J)V", &successMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "FindMethod Error! : %" CHIP_ERROR_FORMAT, err.Format()));
    VerifyOrReturn(successMethod != nullptr, ChipLogError(Controller, "Could not find onDeviceConnected method"));

    static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a Java handle");

    OperationalDeviceProxy * device = new OperationalDeviceProxy(&exchangeMgr, sessionHandle);
    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, successMethod, reinterpret_cast<jlong>(device));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void GetConnectedDeviceCallback::OnDeviceConnectionFailureFn(void * context,
                                                             const OperationalSessionSetup::ConnectionFailureInfo & failureInfo)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    auto * self = static_cast<GetConnectedDeviceCallback *>(context);
    VerifyOrReturn(self->mJavaCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mJavaCallbackRef is not valid in %s", __func__));
    jobject javaCallback = self->mJavaCallbackRef.ObjectRef();
    JniLocalReferenceScope scope(env);

    jclass getConnectedDeviceCallbackCls = nullptr;
    CHIP_ERROR err =
        JniReferences::GetInstance().GetLocalClassRef(env, self->mCallbackClassSignature, getConnectedDeviceCallbackCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "GetLocalClassRef Error! : %" CHIP_ERROR_FORMAT, err.Format()));
    VerifyOrReturn(getConnectedDeviceCallbackCls != nullptr,
                   ChipLogError(Controller, "Could not find GetConnectedDeviceCallback class"));

    jmethodID failureMethod;
    err = JniReferences::GetInstance().FindMethod(env, javaCallback, "onConnectionFailure", "(JLjava/lang/Exception;)V",
                                                  &failureMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "FindMethod Error! : %" CHIP_ERROR_FORMAT, err.Format()));
    VerifyOrReturn(failureMethod != nullptr, ChipLogError(Controller, "Could not find onConnectionFailure method"));

    jthrowable exception;
    err = AndroidConnectionFailureExceptions::GetInstance().CreateAndroidConnectionFailureException(
        env, failureInfo.error.Format(), failureInfo.error.AsInteger(), failureInfo.sessionStage, exception);
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(Controller,
                     "Unable to create AndroidControllerException on GetConnectedDeviceCallback::OnDeviceConnectionFailureFn: %s",
                     ErrorStr(err)));
    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, failureMethod, failureInfo.peerId.GetNodeId(), exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

CHIP_ERROR CreateOptional(chip::Optional<uint8_t> value, jobject & outObj)
{

    return CHIP_NO_ERROR;
}
jobject GetNodeStateObj(JNIEnv * env, const char * nodeStateClassSignature, jobject wrapperCallback)
{
    jmethodID getNodeStateMethod;
    CHIP_ERROR err =
        JniReferences::GetInstance().FindMethod(env, wrapperCallback, "getNodeState", nodeStateClassSignature, &getNodeStateMethod);
    VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogError(Controller, "Could not find getNodeState method"));

    jobject ret = env->CallObjectMethod(wrapperCallback, getNodeStateMethod);
    VerifyOrReturnValue(!env->ExceptionCheck(), nullptr, env->ExceptionDescribe());

    return ret;
}

ReportCallback::ReportCallback(jobject wrapperCallback, jobject subscriptionEstablishedCallback,
                               jobject resubscriptionAttemptCallback, const char * nodeStateClassSignature) :
    mClusterCacheAdapter(*this, Optional<EventNumber>::Missing(), false /*cacheData*/),
    mNodeStateClassSignature(nodeStateClassSignature)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    if (subscriptionEstablishedCallback != nullptr)
    {
        VerifyOrReturn(mSubscriptionEstablishedCallbackRef.Init(subscriptionEstablishedCallback) == CHIP_NO_ERROR,
                       ChipLogError(Controller, "Could not init mSubscriptionEstablishedCallbackRef in %s", __func__));
    }

    VerifyOrReturn(mWrapperCallbackRef.Init(wrapperCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mWrapperCallbackRef in %s", __func__));

    if (resubscriptionAttemptCallback != nullptr)
    {
        VerifyOrReturn(mResubscriptionAttemptCallbackRef.Init(resubscriptionAttemptCallback) == CHIP_NO_ERROR,
                       ChipLogError(Controller, "Could not init mResubscriptionAttemptCallbackRef in %s", __func__));
    }
}

ReportCallback::~ReportCallback()
{
    if (mReadClient != nullptr)
    {
        Platform::Delete(mReadClient);
    }
}

void ReportCallback::OnReportBegin()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    JniLocalReferenceScope scope(env);

    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mReportCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    jmethodID onReportBeginMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onReportBegin", "()V", &onReportBeginMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onReportBegin method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(wrapperCallback, onReportBeginMethod);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void ReportCallback::OnDeallocatePaths(app::ReadPrepareParams && aReadPrepareParams)
{
    if (aReadPrepareParams.mpAttributePathParamsList != nullptr && aReadPrepareParams.mAttributePathParamsListSize != 0)
    {
        delete[] aReadPrepareParams.mpAttributePathParamsList;
        aReadPrepareParams.mpAttributePathParamsList    = nullptr;
        aReadPrepareParams.mAttributePathParamsListSize = 0;
    }

    if (aReadPrepareParams.mpEventPathParamsList != nullptr && aReadPrepareParams.mEventPathParamsListSize != 0)
    {
        delete[] aReadPrepareParams.mpEventPathParamsList;
        aReadPrepareParams.mpEventPathParamsList    = nullptr;
        aReadPrepareParams.mEventPathParamsListSize = 0;
    }

    if (aReadPrepareParams.mpDataVersionFilterList != nullptr && aReadPrepareParams.mDataVersionFilterListSize != 0)
    {
        delete[] aReadPrepareParams.mpDataVersionFilterList;
        aReadPrepareParams.mpDataVersionFilterList    = nullptr;
        aReadPrepareParams.mDataVersionFilterListSize = 0;
    }
}

void ReportCallback::OnReportEnd()
{
    UpdateClusterDataVersion();

    // Transform C++ jobject pair list to a Java HashMap, and call onReport() on the Java callback.
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    JniLocalReferenceScope scope(env);
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    jmethodID onReportEndMethod;
    err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onReportEnd", "()V", &onReportEndMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onReportEnd method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(wrapperCallback, onReportEndMethod);
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
    DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    VerifyOrReturn(!aPath.IsListItemOperation(), ChipLogError(Controller, "Expect non-list item operation"); aPath.LogPath());

    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mReportCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();

    jobject nodeState = GetNodeStateObj(env, mNodeStateClassSignature, wrapperCallback);

    {
        // Add Attribute Status to wrapperCallback
        jmethodID addAttributeStatusMethod = nullptr;
        err = JniReferences::GetInstance().FindMethod(env, nodeState, "addAttributeStatus", "(IJJILjava/lang/Integer;)V",
                                                      &addAttributeStatusMethod);
        VerifyOrReturn(
            err == CHIP_NO_ERROR,
            ChipLogError(Controller, "Could not find addAttributeStatus method with error %" CHIP_ERROR_FORMAT, err.Format()));

        jobject jClusterState = nullptr;
        if (aStatus.mClusterStatus.HasValue())
        {
            err = JniReferences::GetInstance().CreateBoxedObject<jint>(
                "java/lang/Integer", "(I)V", static_cast<jint>(aStatus.mClusterStatus.Value()), jClusterState);
            VerifyOrReturn(err == CHIP_NO_ERROR,
                           ChipLogError(Controller, "Could not CreateBoxedObject with error %" CHIP_ERROR_FORMAT, err.Format()));
        }

        env->CallVoidMethod(nodeState, addAttributeStatusMethod, static_cast<jint>(aPath.mEndpointId),
                            static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mAttributeId),
                            static_cast<jint>(aStatus.mStatus), jClusterState);
        VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
    }

    VerifyOrReturn(aStatus.IsSuccess(), ChipLogError(Controller, "Receive bad status %s", ErrorStr(aStatus.ToChipError()));
                   aPath.LogPath());
    VerifyOrReturn(apData != nullptr, ChipLogError(Controller, "Receive empty apData"); aPath.LogPath());

    TLV::TLVReader readerForJavaTLV;
    readerForJavaTLV.Init(*apData);

    jobject value = nullptr;
    TLV::TLVReader readerForJavaObject;
    readerForJavaObject.Init(*apData);
#ifdef USE_JAVA_TLV_ENCODE_DECODE
    value = DecodeAttributeValue(aPath, readerForJavaObject, &err);
    // If we don't know this attribute, suppress it.
    if (err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB)
    {
        TLV::TLVReader readerForGeneralValueObject;
        readerForGeneralValueObject.Init(*apData);
        value = DecodeGeneralTLVValue(env, readerForGeneralValueObject, err);
        err   = CHIP_NO_ERROR;
    }

    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Fail to decode attribute with error %s", ErrorStr(err));
                   aPath.LogPath());
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
#else
    value = DecodeGeneralTLVValue(env, readerForJavaObject, err);
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
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Fail to copy tlv element with error %s", ErrorStr(err));
                   aPath.LogPath());
    size = writer.GetLengthWritten();
    chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), static_cast<jint>(size));

    // Convert TLV to JSON
    std::string json;
    err = ConvertReportTlvToJson(static_cast<uint32_t>(aPath.mAttributeId), *apData, json);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Fail to convert report tlv to json with error %s", ErrorStr(err));
                   aPath.LogPath());
    UtfString jsonString(env, json.c_str());

    // Add AttributeState to wrapperCallback
    jmethodID addAttributeMethod;
    err = JniReferences::GetInstance().FindMethod(env, nodeState, "addAttribute", "(IJJLjava/lang/Object;[BLjava/lang/String;)V",
                                                  &addAttributeMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not find addAttribute method with error %s", ErrorStr(err)));
    env->CallVoidMethod(nodeState, addAttributeMethod, static_cast<jint>(aPath.mEndpointId), static_cast<jlong>(aPath.mClusterId),
                        static_cast<jlong>(aPath.mAttributeId), value, jniByteArray.jniValue(), jsonString.jniValue());
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());

    UpdateClusterDataVersion();
}

void ReportCallback::UpdateClusterDataVersion()
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
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

    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mReportCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    jobject nodeState       = GetNodeStateObj(env, mNodeStateClassSignature, wrapperCallback);

    // SetDataVersion to NodeState
    jmethodID setDataVersionMethod;
    CHIP_ERROR err = JniReferences::GetInstance().FindMethod(env, nodeState, "setDataVersion", "(IJJ)V", &setDataVersionMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find setDataVersion method"));
    env->CallVoidMethod(nodeState, setDataVersionMethod, static_cast<jint>(lastConcreteClusterPath.mEndpointId),
                        static_cast<jlong>(lastConcreteClusterPath.mClusterId), static_cast<jlong>(committedDataVersion.Value()));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void ReportCallback::OnEventData(const app::EventHeader & aEventHeader, TLV::TLVReader * apData, const app::StatusIB * apStatus)
{
    DeviceLayer::StackUnlock unlock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mReportCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    jobject nodeState       = GetNodeStateObj(env, mNodeStateClassSignature, wrapperCallback);
    if (apStatus != nullptr)
    {
        // Add Event Status to NodeState
        jmethodID addEventStatusMethod;
        err = JniReferences::GetInstance().FindMethod(env, nodeState, "addEventStatus", "(IJJILjava/lang/Integer;)V",
                                                      &addEventStatusMethod);
        VerifyOrReturn(
            err == CHIP_NO_ERROR,
            ChipLogError(Controller, "Could not find addEventStatus method with error %" CHIP_ERROR_FORMAT, err.Format()));

        jobject jClusterState = nullptr;
        if (apStatus->mClusterStatus.HasValue())
        {
            err = JniReferences::GetInstance().CreateBoxedObject<jint>(
                "java/lang/Integer", "(I)V", static_cast<jint>(apStatus->mClusterStatus.Value()), jClusterState);
            VerifyOrReturn(err == CHIP_NO_ERROR,
                           ChipLogError(Controller, "Could not CreateBoxedObject with error %" CHIP_ERROR_FORMAT, err.Format()));
        }

        env->CallVoidMethod(nodeState, addEventStatusMethod, static_cast<jint>(aEventHeader.mPath.mEndpointId),
                            static_cast<jlong>(aEventHeader.mPath.mClusterId), static_cast<jlong>(aEventHeader.mPath.mEventId),
                            static_cast<jint>(apStatus->mStatus), jClusterState);
        VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
        return;
    }
    VerifyOrReturn(apData != nullptr, ChipLogError(Controller, "Receive empty apData"); aEventHeader.LogPath());

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
        aEventHeader.LogPath();
        return;
    }

    jobject value = nullptr;
    TLV::TLVReader readerForJavaObject;
    readerForJavaObject.Init(*apData);
#ifdef USE_JAVA_TLV_ENCODE_DECODE
    value = DecodeEventValue(aEventHeader.mPath, readerForJavaObject, &err);
    // If we don't know this event, just skip it.
    if (err == CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB)
    {
        TLV::TLVReader readerForGeneralValueObject;
        readerForGeneralValueObject.Init(*apData);
        value = DecodeGeneralTLVValue(env, readerForGeneralValueObject, err);
        err   = CHIP_NO_ERROR;
    }
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Fail to decode event with error %s", ErrorStr(err));
                   aEventHeader.LogPath());
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
#else
    value = DecodeGeneralTLVValue(env, readerForJavaObject, err);
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

    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Fail to copy element with error %s", ErrorStr(err));
                   aEventHeader.LogPath());
    size = writer.GetLengthWritten();
    chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), static_cast<jint>(size));

    // Convert TLV to JSON
    std::string json;
    err = ConvertReportTlvToJson(static_cast<uint32_t>(aEventHeader.mPath.mEventId), *apData, json);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Fail to convert report tlv to Json with error %s", ErrorStr(err));
                   aEventHeader.LogPath());
    UtfString jsonString(env, json.c_str());

    jmethodID addEventMethod;
    err = JniReferences::GetInstance().FindMethod(env, nodeState, "addEvent", "(IJJJIIJLjava/lang/Object;[BLjava/lang/String;)V",
                                                  &addEventMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find addEvent method with error %s", ErrorStr(err));
                   aEventHeader.LogPath());
    env->CallVoidMethod(nodeState, addEventMethod, static_cast<jint>(aEventHeader.mPath.mEndpointId),
                        static_cast<jlong>(aEventHeader.mPath.mClusterId), static_cast<jlong>(aEventHeader.mPath.mEventId),
                        eventNumber, priorityLevel, timestampType, timestampValue, value, jniByteArray.jniValue(),
                        jsonString.jniValue());
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe(); aEventHeader.LogPath());
}

void ReportCallback::OnError(CHIP_ERROR aError)
{
    ReportError(nullptr, nullptr, aError);
}

void ReportCallback::OnDone(app::ReadClient *)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jmethodID onDoneMethod;
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    JniGlobalReference globalRef(std::move(mWrapperCallbackRef));

    err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    if (mReadClient != nullptr)
    {
        Platform::Delete(mReadClient);
    }
    mReadClient = nullptr;

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(wrapperCallback, onDoneMethod);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void ReportCallback::OnSubscriptionEstablished(SubscriptionId aSubscriptionId)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    DeviceLayer::StackUnlock unlock;
    VerifyOrReturn(mSubscriptionEstablishedCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, " mSubscriptionEstablishedCallbackRef is not valid in %s", __func__));
    CHIP_ERROR err =
        JniReferences::GetInstance().CallSubscriptionEstablished(mSubscriptionEstablishedCallbackRef.ObjectRef(), aSubscriptionId);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(Controller, "CallSubscriptionEstablished error! : %" CHIP_ERROR_FORMAT, err.Format()));
}

CHIP_ERROR ReportCallback::OnResubscriptionNeeded(app::ReadClient * apReadClient, CHIP_ERROR aTerminationCause)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);
    ReturnErrorOnFailure(app::ReadClient::Callback::OnResubscriptionNeeded(apReadClient, aTerminationCause));
    JniLocalReferenceScope scope(env);
    jmethodID onResubscriptionAttemptMethod;
    VerifyOrReturnLogError(mResubscriptionAttemptCallbackRef.HasValidObjectRef(), CHIP_ERROR_INCORRECT_STATE);
    jobject resubscriptionAttemptCallbackRef = mResubscriptionAttemptCallbackRef.ObjectRef();
    ReturnLogErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, resubscriptionAttemptCallbackRef, "onResubscriptionAttempt", "(JJ)V", &onResubscriptionAttemptMethod));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(resubscriptionAttemptCallbackRef, onResubscriptionAttemptMethod,
                        static_cast<jlong>(aTerminationCause.AsInteger()),
                        static_cast<jlong>(apReadClient->ComputeTimeTillNextSubscription()));
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
    return CHIP_NO_ERROR;
}

void ReportCallback::ReportError(const app::ConcreteAttributePath * attributePath, const app::ConcreteEventPath * eventPath,
                                 CHIP_ERROR err)
{
    ReportError(attributePath, eventPath, ErrorStr(err), err.AsInteger());
}

void ReportCallback::ReportError(const app::ConcreteAttributePath * attributePath, const app::ConcreteEventPath * eventPath,
                                 Protocols::InteractionModel::Status status)
{
    ReportError(attributePath, eventPath, "IM Status",
                static_cast<std::underlying_type_t<Protocols::InteractionModel::Status>>(status));
}

void ReportCallback::ReportError(const app::ConcreteAttributePath * attributePath, const app::ConcreteEventPath * eventPath,
                                 const char * message, ChipError::StorageType errorCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    jthrowable exception;
    err = AndroidControllerExceptions::GetInstance().CreateAndroidControllerException(env, message, errorCode, exception);
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(Controller, "Unable to create AndroidControllerException on ReportCallback::ReportError: %s", ErrorStr(err)));
    jmethodID onErrorMethod;
    err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onError", "(ZIJJZIJJLjava/lang/Exception;)V",
                                                  &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;

    jboolean isAttributePath = JNI_FALSE;
    jint attributeEndpointId = static_cast<jint>(kInvalidEndpointId);
    jlong attributeClusterId = static_cast<jlong>(kInvalidClusterId);
    jlong attributeId        = static_cast<jlong>(kInvalidAttributeId);

    jboolean isEventPath = JNI_FALSE;
    jint eventEndpointId = static_cast<jint>(kInvalidEndpointId);
    jlong eventClusterId = static_cast<jlong>(kInvalidClusterId);
    jlong eventId        = static_cast<jlong>(kInvalidAttributeId);

    if (attributePath != nullptr)
    {
        isAttributePath     = JNI_TRUE;
        attributeEndpointId = static_cast<jint>(attributePath->mEndpointId);
        attributeClusterId  = static_cast<jlong>(attributePath->mClusterId);
        attributeId         = static_cast<jlong>(attributePath->mAttributeId);
    }

    if (eventPath != nullptr)
    {
        isEventPath     = JNI_TRUE;
        eventEndpointId = static_cast<jint>(eventPath->mEndpointId);
        eventClusterId  = static_cast<jlong>(eventPath->mClusterId);
        eventId         = static_cast<jlong>(eventPath->mEventId);
    }
    env->CallVoidMethod(wrapperCallback, onErrorMethod, isAttributePath, attributeEndpointId, attributeClusterId, attributeId,
                        isEventPath, eventEndpointId, eventClusterId, eventId, exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

WriteAttributesCallback::WriteAttributesCallback(jobject wrapperCallback) : mChunkedWriteCallback(this)
{
    VerifyOrReturn(mWrapperCallbackRef.Init(wrapperCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mWrapperCallbackRef for WriteAttributesCallback"));
}

WriteAttributesCallback::~WriteAttributesCallback()
{
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
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    jmethodID onResponseMethod;
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onResponse", "(IJJILjava/lang/Integer;)V",
                                                  &onResponseMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    jobject jClusterState = nullptr;
    if (aStatus.mClusterStatus.HasValue())
    {
        err = JniReferences::GetInstance().CreateBoxedObject<jint>(
            "java/lang/Integer", "(I)V", static_cast<jint>(aStatus.mClusterStatus.Value()), jClusterState);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(Controller, "Could not CreateBoxedObject with error %" CHIP_ERROR_FORMAT, err.Format()));
    }

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(wrapperCallback, onResponseMethod, static_cast<jint>(aPath.mEndpointId),
                        static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mAttributeId), aStatus.mStatus,
                        jClusterState);
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
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jmethodID onDoneMethod;
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    JniGlobalReference globalRef(std::move(mWrapperCallbackRef));

    err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(wrapperCallback, onDoneMethod);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void WriteAttributesCallback::ReportError(const app::ConcreteAttributePath * attributePath, CHIP_ERROR err)
{
    ReportError(attributePath, ErrorStr(err), err.AsInteger());
}

void WriteAttributesCallback::ReportError(const app::ConcreteAttributePath * attributePath,
                                          Protocols::InteractionModel::Status status)
{
    ReportError(attributePath, "IM Status", static_cast<std::underlying_type_t<Protocols::InteractionModel::Status>>(status));
}

void WriteAttributesCallback::ReportError(const app::ConcreteAttributePath * attributePath, const char * message,
                                          ChipError::StorageType errorCode)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    ChipLogError(Controller, "WriteAttributesCallback::ReportError is called with %u", errorCode);
    jthrowable exception;
    err = AndroidControllerExceptions::GetInstance().CreateAndroidControllerException(env, message, errorCode, exception);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(Controller,
                                "Unable to create AndroidControllerException on WriteAttributesCallback::ReportError: %s",
                                ErrorStr(err)));
    jmethodID onErrorMethod;
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onError", "(ZIJJLjava/lang/Exception;)V", &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    jboolean isAttributePath = JNI_FALSE;
    jint attributeEndpointId = static_cast<jint>(kInvalidEndpointId);
    jlong attributeClusterId = static_cast<jlong>(kInvalidClusterId);
    jlong attributeId        = static_cast<jlong>(kInvalidAttributeId);

    if (attributePath != nullptr)
    {
        isAttributePath     = JNI_TRUE;
        attributeEndpointId = static_cast<jint>(attributePath->mEndpointId);
        attributeClusterId  = static_cast<jint>(attributePath->mClusterId);
        attributeId         = static_cast<jint>(attributePath->mAttributeId);
    }

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(wrapperCallback, onErrorMethod, isAttributePath, attributeEndpointId, attributeClusterId, attributeId,
                        exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

InvokeCallback::InvokeCallback(jobject wrapperCallback)
{
    VerifyOrReturn(mWrapperCallbackRef.Init(wrapperCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mWrapperCallbackRef for InvokeCallback"));
}

InvokeCallback::~InvokeCallback()
{
    if (mCommandSender != nullptr)
    {
        Platform::Delete(mCommandSender);
        mCommandSender = nullptr;
    }
}

void InvokeCallback::OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aPath,
                                const app::StatusIB & aStatusIB, TLV::TLVReader * apData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    jmethodID onResponseMethod;
    JniLocalReferenceScope scope(env);
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallbackRef = mWrapperCallbackRef.ObjectRef();
    err = JniReferences::GetInstance().FindMethod(env, wrapperCallbackRef, "onResponse", "(IJJ[BLjava/lang/String;J)V",
                                                  &onResponseMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onResponse method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;

    if (apData != nullptr)
    {
        TLV::TLVReader readerForJavaTLV;
        TLV::TLVReader readerForJson;
        readerForJavaTLV.Init(*apData);

        // Create TLV byte array to pass to Java layer
        size_t bufferLen                  = readerForJavaTLV.GetRemainingLength() + readerForJavaTLV.GetLengthRead();
        std::unique_ptr<uint8_t[]> buffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferLen]);
        uint32_t size                     = 0;

        TLV::TLVWriter writer;
        writer.Init(buffer.get(), bufferLen);
        err = writer.CopyElement(TLV::AnonymousTag(), readerForJavaTLV);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Failed CopyElement: %" CHIP_ERROR_FORMAT, err.Format()));
        size = writer.GetLengthWritten();

        chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), static_cast<jint>(size));

        // Convert TLV to JSON
        std::string json;
        readerForJson.Init(buffer.get(), size);
        err = readerForJson.Next();
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(Controller, "Failed readerForJson next: %" CHIP_ERROR_FORMAT, err.Format()));
        err = TlvToJson(readerForJson, json);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Failed TlvToJson: %" CHIP_ERROR_FORMAT, err.Format()));
        UtfString jsonString(env, json.c_str());

        env->CallVoidMethod(wrapperCallbackRef, onResponseMethod, static_cast<jint>(aPath.mEndpointId),
                            static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mCommandId), jniByteArray.jniValue(),
                            jsonString.jniValue(),
                            aStatusIB.mClusterStatus.HasValue() ? static_cast<jlong>(aStatusIB.mClusterStatus.Value())
                                                                : static_cast<jlong>(Protocols::InteractionModel::Status::Success));
    }
    else
    {
        env->CallVoidMethod(wrapperCallbackRef, onResponseMethod, static_cast<jint>(aPath.mEndpointId),
                            static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mCommandId), nullptr, nullptr,
                            aStatusIB.mClusterStatus.HasValue() ? static_cast<jlong>(aStatusIB.mClusterStatus.Value())
                                                                : static_cast<jlong>(Protocols::InteractionModel::Status::Success));
    }

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
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    jmethodID onDoneMethod;
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    JniGlobalReference globalRef(std::move(mWrapperCallbackRef));

    err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(wrapperCallback, onDoneMethod);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
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
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    ChipLogError(Controller, "InvokeCallback::ReportError is called with %u", errorCode);
    jthrowable exception;
    err = AndroidControllerExceptions::GetInstance().CreateAndroidControllerException(env, message, errorCode, exception);
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(Controller, "Unable to create AndroidControllerException: %s on InvokeCallback::ReportError", ErrorStr(err)));

    jmethodID onErrorMethod;
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onError", "(Ljava/lang/Exception;)V", &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(wrapperCallback, onErrorMethod, exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

jobject DecodeGeneralTLVValue(JNIEnv * env, TLV::TLVReader & readerForGeneralValueObject, CHIP_ERROR & err)
{
    jobject retValue = nullptr;

    switch (readerForGeneralValueObject.GetType())
    {
    case TLV::kTLVType_SignedInteger: {
        int64_t signedValue = 0;
        VerifyOrReturnValue(readerForGeneralValueObject.Get(signedValue) == CHIP_NO_ERROR, nullptr,
                            ChipLogProgress(Controller, "Get TLV Value fail!"));
        err = JniReferences::GetInstance().CreateBoxedObject<jlong>("java/lang/Long", "(J)V", static_cast<jlong>(signedValue),
                                                                    retValue);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogProgress(Controller, "Create Boxed Object fail!"));
        return retValue;
    }
    case TLV::kTLVType_UnsignedInteger: {
        uint64_t unsignedValue = 0;
        VerifyOrReturnValue(readerForGeneralValueObject.Get(unsignedValue) == CHIP_NO_ERROR, nullptr,
                            ChipLogProgress(Controller, "Get TLV Value fail!"));
        err = JniReferences::GetInstance().CreateBoxedObject<jlong>("java/lang/Long", "(J)V", static_cast<jlong>(unsignedValue),
                                                                    retValue);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogProgress(Controller, "Create Boxed Object fail!"));
        return retValue;
    }
    case TLV::kTLVType_Boolean: {
        bool booleanValue = false;
        VerifyOrReturnValue(readerForGeneralValueObject.Get(booleanValue) == CHIP_NO_ERROR, nullptr,
                            ChipLogProgress(Controller, "Get TLV Value fail!"));
        err = JniReferences::GetInstance().CreateBoxedObject<jboolean>("java/lang/Boolean", "(Z)V",
                                                                       static_cast<jboolean>(booleanValue), retValue);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogProgress(Controller, "Create Boxed Object fail!"));
        return retValue;
    }
    case TLV::kTLVType_FloatingPointNumber: {
        double doubleValue = 0.0;
        VerifyOrReturnValue(readerForGeneralValueObject.Get(doubleValue) == CHIP_NO_ERROR, nullptr,
                            ChipLogProgress(Controller, "Get TLV Value fail!"));
        err = JniReferences::GetInstance().CreateBoxedObject<jdouble>("java/lang/Double", "(D)V", static_cast<jdouble>(doubleValue),
                                                                      retValue);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogProgress(Controller, "Create Boxed Object fail!"));
        return retValue;
    }
    case TLV::kTLVType_UTF8String: {
        uint32_t bufferLen             = readerForGeneralValueObject.GetLength();
        std::unique_ptr<char[]> buffer = std::unique_ptr<char[]>(new char[bufferLen + 1]);
        err                            = readerForGeneralValueObject.GetString(buffer.get(), bufferLen + 1);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogProgress(Controller, "Get TLV Value fail!"));
        chip::CharSpan valueSpan(buffer.get(), bufferLen);
        chip::JniReferences::GetInstance().CharToStringUTF(valueSpan, retValue);
        return retValue;
    }
    case TLV::kTLVType_ByteString: {
        uint32_t bufferLen                = readerForGeneralValueObject.GetLength();
        std::unique_ptr<uint8_t[]> buffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferLen + 1]);
        err                               = readerForGeneralValueObject.GetBytes(buffer.get(), bufferLen + 1);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr, ChipLogProgress(Controller, "Get TLV Value fail!"));

        jbyteArray valueByteArray = env->NewByteArray(static_cast<jsize>(bufferLen));
        env->SetByteArrayRegion(valueByteArray, 0, static_cast<jsize>(bufferLen), reinterpret_cast<const jbyte *>(buffer.get()));

        return static_cast<jobject>(valueByteArray);
    }
    case TLV::kTLVType_Array: {
        TLV::TLVType containerType;
        err = readerForGeneralValueObject.EnterContainer(containerType);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogProgress(Controller, "EnterContainer fail! : %" CHIP_ERROR_FORMAT, err.Format()));
        err = chip::JniReferences::GetInstance().CreateArrayList(retValue);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogProgress(Controller, "CreateArrayList fail! : %" CHIP_ERROR_FORMAT, err.Format()));
        while (readerForGeneralValueObject.Next() == CHIP_NO_ERROR)
        {
            jobject inValue = DecodeGeneralTLVValue(env, readerForGeneralValueObject, err);
            VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                                ChipLogProgress(Controller, "Can't parse general value : %" CHIP_ERROR_FORMAT, err.Format()));
            err = chip::JniReferences::GetInstance().AddToList(retValue, inValue);
        }
        return retValue;
    }
    case TLV::kTLVType_List: {
        TLV::TLVType containerType;
        err = readerForGeneralValueObject.EnterContainer(containerType);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogProgress(Controller, "EnterContainer fail! : %" CHIP_ERROR_FORMAT, err.Format()));
        err = chip::JniReferences::GetInstance().CreateArrayList(retValue);
        VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                            ChipLogProgress(Controller, "CreateArrayList fail! : %" CHIP_ERROR_FORMAT, err.Format()));
        while (readerForGeneralValueObject.Next() == CHIP_NO_ERROR)
        {
            jobject inValue = DecodeGeneralTLVValue(env, readerForGeneralValueObject, err);
            VerifyOrReturnValue(err == CHIP_NO_ERROR, nullptr,
                                ChipLogProgress(Controller, "Can't parse general value : %" CHIP_ERROR_FORMAT, err.Format()));
            err = chip::JniReferences::GetInstance().AddToList(retValue, inValue);
        }
        return retValue;
    }
    case TLV::kTLVType_Null: {
        return nullptr;
    }
    default:
        err = CHIP_ERROR_WRONG_TLV_TYPE;
        return nullptr;
    }
}

ExtendableInvokeCallback::ExtendableInvokeCallback(jobject wrapperCallback)
{
    VerifyOrReturn(mWrapperCallbackRef.Init(wrapperCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mWrapperCallbackRef for ExtendableInvokeCallback"));
}

ExtendableInvokeCallback::~ExtendableInvokeCallback()
{
    if (mCommandSender != nullptr)
    {
        Platform::Delete(mCommandSender);
        mCommandSender = nullptr;
    }
}

void ExtendableInvokeCallback::OnResponse(app::CommandSender * apCommandSender,
                                          const app::CommandSender::ResponseData & aResponseData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    jmethodID onResponseMethod;
    JniLocalReferenceScope scope(env);
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallbackRef = mWrapperCallbackRef.ObjectRef();
    DeviceLayer::StackUnlock unlock;

    jobject jCommandRef = nullptr;
    if (aResponseData.commandRef.HasValue())
    {
        err = JniReferences::GetInstance().CreateBoxedObject<jint>(
            "java/lang/Integer", "(I)V", static_cast<jint>(aResponseData.commandRef.Value()), jCommandRef);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(Controller, "Could not CreateBoxedObject with error %" CHIP_ERROR_FORMAT, err.Format()));
    }

    if (aResponseData.data != nullptr)
    {
        TLV::TLVReader readerForJavaTLV;
        TLV::TLVReader readerForJson;
        readerForJavaTLV.Init(*(aResponseData.data));

        // Create TLV byte array to pass to Java layer
        size_t bufferLen                  = readerForJavaTLV.GetRemainingLength() + readerForJavaTLV.GetLengthRead();
        std::unique_ptr<uint8_t[]> buffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferLen]);
        uint32_t size                     = 0;

        TLV::TLVWriter writer;
        writer.Init(buffer.get(), bufferLen);
        err = writer.CopyElement(TLV::AnonymousTag(), readerForJavaTLV);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Failed CopyElement: %" CHIP_ERROR_FORMAT, err.Format()));
        size = writer.GetLengthWritten();

        chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), static_cast<jint>(size));

        // Convert TLV to JSON
        std::string json;
        readerForJson.Init(buffer.get(), size);
        err = readerForJson.Next();
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(Controller, "Failed readerForJson next: %" CHIP_ERROR_FORMAT, err.Format()));
        err = TlvToJson(readerForJson, json);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Failed TlvToJson: %" CHIP_ERROR_FORMAT, err.Format()));
        UtfString jsonString(env, json.c_str());

        err = JniReferences::GetInstance().FindMethod(env, wrapperCallbackRef, "onResponse",
                                                      "(IJJLjava/lang/Integer;[BLjava/lang/String;)V", &onResponseMethod);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onResponse method: %s", ErrorStr(err)));

        env->CallVoidMethod(wrapperCallbackRef, onResponseMethod, static_cast<jint>(aResponseData.path.mEndpointId),
                            static_cast<jlong>(aResponseData.path.mClusterId), static_cast<jlong>(aResponseData.path.mCommandId),
                            jCommandRef, jniByteArray.jniValue(), jsonString.jniValue());
    }
    else
    {
        err = JniReferences::GetInstance().FindMethod(env, wrapperCallbackRef, "onResponse",
                                                      "(IJJLjava/lang/Integer;ILjava/lang/Integer;)V", &onResponseMethod);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onResponse method: %s", ErrorStr(err)));

        jobject jClusterState = nullptr;
        if (aResponseData.statusIB.mClusterStatus.HasValue())
        {
            err = JniReferences::GetInstance().CreateBoxedObject<jint>(
                "java/lang/Integer", "(I)V", static_cast<jint>(aResponseData.statusIB.mClusterStatus.Value()), jClusterState);
            VerifyOrReturn(err == CHIP_NO_ERROR,
                           ChipLogError(Controller, "Could not CreateBoxedObject with error %" CHIP_ERROR_FORMAT, err.Format()));
        }

        env->CallVoidMethod(wrapperCallbackRef, onResponseMethod, static_cast<jint>(aResponseData.path.mEndpointId),
                            static_cast<jlong>(aResponseData.path.mClusterId), static_cast<jlong>(aResponseData.path.mCommandId),
                            jCommandRef, aResponseData.statusIB.mStatus, jClusterState);
    }

    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void ExtendableInvokeCallback::OnNoResponse(app::CommandSender * commandSender,
                                            const app::CommandSender::NoResponseData & aNoResponseData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    jmethodID onNoResponseMethod;
    JniLocalReferenceScope scope(env);
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallbackRef = mWrapperCallbackRef.ObjectRef();
    DeviceLayer::StackUnlock unlock;

    err = JniReferences::GetInstance().FindMethod(env, wrapperCallbackRef, "onNoResponse", "(I)V", &onNoResponseMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Unable to find onNoResponse method: %" CHIP_ERROR_FORMAT, err.Format()));
    env->CallVoidMethod(wrapperCallbackRef, onNoResponseMethod, static_cast<jint>(aNoResponseData.commandRef));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void ExtendableInvokeCallback::OnError(const app::CommandSender * apCommandSender, const app::CommandSender::ErrorData & aErrorData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    ChipLogError(Controller, "ExtendableInvokeCallback::OnError is called with %u", aErrorData.error.AsInteger());
    jthrowable exception;
    err = AndroidControllerExceptions::GetInstance().CreateAndroidControllerException(env, ErrorStr(aErrorData.error),
                                                                                      aErrorData.error.AsInteger(), exception);
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(Controller, "Unable to create AndroidControllerException with error: %" CHIP_ERROR_FORMAT, err.Format()));

    jmethodID onErrorMethod;
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onError", "(Ljava/lang/Exception;)V", &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Unable to find onError method:  %" CHIP_ERROR_FORMAT, err.Format()));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(wrapperCallback, onErrorMethod, exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void ExtendableInvokeCallback::OnDone(app::CommandSender * apCommandSender)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    jmethodID onDoneMethod;
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    jobject wrapperCallback = mWrapperCallbackRef.ObjectRef();
    JniGlobalReference globalRef(std::move(mWrapperCallbackRef));

    err = JniReferences::GetInstance().FindMethod(env, wrapperCallback, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(wrapperCallback, onDoneMethod);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

jlong newConnectedDeviceCallback(JNIEnv * env, jobject self, jobject callback)
{
    chip::DeviceLayer::StackLock lock;
    GetConnectedDeviceCallback * connectedDeviceCallback = chip::Platform::New<GetConnectedDeviceCallback>(self, callback);
    return reinterpret_cast<jlong>(connectedDeviceCallback);
}

void deleteConnectedDeviceCallback(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    GetConnectedDeviceCallback * connectedDeviceCallback = reinterpret_cast<GetConnectedDeviceCallback *>(callbackHandle);
    VerifyOrReturn(connectedDeviceCallback != nullptr, ChipLogError(Controller, "GetConnectedDeviceCallback handle is nullptr"));
    chip::Platform::Delete(connectedDeviceCallback);
}

jlong newReportCallback(JNIEnv * env, jobject self, jobject subscriptionEstablishedCallbackJava,
                        jobject resubscriptionAttemptCallbackJava, const char * nodeStateClassSignature)
{
    chip::DeviceLayer::StackLock lock;
    ReportCallback * reportCallback = chip::Platform::New<ReportCallback>(
        self, subscriptionEstablishedCallbackJava, resubscriptionAttemptCallbackJava, nodeStateClassSignature);
    return reinterpret_cast<jlong>(reportCallback);
}

void deleteReportCallback(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    ReportCallback * reportCallback = reinterpret_cast<ReportCallback *>(callbackHandle);
    VerifyOrReturn(reportCallback != nullptr, ChipLogError(Controller, "ReportCallback handle is nullptr"));
    chip::Platform::Delete(reportCallback);
}

jlong newWriteAttributesCallback(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    WriteAttributesCallback * writeAttributesCallback = chip::Platform::New<WriteAttributesCallback>(self);
    return reinterpret_cast<jlong>(writeAttributesCallback);
}

void deleteWriteAttributesCallback(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    WriteAttributesCallback * writeAttributesCallback = reinterpret_cast<WriteAttributesCallback *>(callbackHandle);
    VerifyOrReturn(writeAttributesCallback != nullptr, ChipLogError(Controller, "WriteAttributesCallback handle is nullptr"));
    chip::Platform::Delete(writeAttributesCallback);
}

jlong newInvokeCallback(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    InvokeCallback * invokeCallback = chip::Platform::New<InvokeCallback>(self);
    return reinterpret_cast<jlong>(invokeCallback);
}

void deleteInvokeCallback(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    InvokeCallback * invokeCallback = reinterpret_cast<InvokeCallback *>(callbackHandle);
    VerifyOrReturn(invokeCallback != nullptr, ChipLogError(Controller, "InvokeCallback handle is nullptr"));
    chip::Platform::Delete(invokeCallback);
}

jlong newExtendableInvokeCallback(JNIEnv * env, jobject self)
{
    chip::DeviceLayer::StackLock lock;
    ExtendableInvokeCallback * invokeCallback = chip::Platform::New<ExtendableInvokeCallback>(self);
    return reinterpret_cast<jlong>(invokeCallback);
}

void deleteExtendableInvokeCallback(JNIEnv * env, jobject self, jlong callbackHandle)
{
    chip::DeviceLayer::StackLock lock;
    ExtendableInvokeCallback * invokeCallback = reinterpret_cast<ExtendableInvokeCallback *>(callbackHandle);
    VerifyOrReturn(invokeCallback != nullptr, ChipLogError(Controller, "ExtendableInvokeCallback handle is nullptr"));
    chip::Platform::Delete(invokeCallback);
}
} // namespace Controller
} // namespace chip
