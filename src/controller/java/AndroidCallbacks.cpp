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
#include <type_traits>

namespace chip {
namespace Controller {

static const int MILLIS_SINCE_BOOT  = 0;
static const int MILLIS_SINCE_EPOCH = 1;
// Add the bytes for attribute tag(1:control + 8:tag + 8:length) and structure(1:struct + 1:close container)
static const int EXTRA_SPACE_FOR_ATTRIBUTE_TAG = 19;

CHIP_ERROR CreateChipAttributePath(JNIEnv * env, const app::ConcreteDataAttributePath & aPath, jobject & outObj)
{
    jclass attributePathCls = nullptr;
    ReturnErrorOnFailure(
        JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/model/ChipAttributePath", attributePathCls));
    jmethodID attributePathCtor =
        env->GetStaticMethodID(attributePathCls, "newInstance", "(IJJ)Lchip/devicecontroller/model/ChipAttributePath;");
    VerifyOrReturnError(attributePathCtor != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);
    outObj = env->CallStaticObjectMethod(attributePathCls, attributePathCtor, static_cast<jint>(aPath.mEndpointId),
                                         static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mAttributeId));
    VerifyOrReturnError(outObj != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ReportCallback::CreateChipEventPath(JNIEnv * env, const app::ConcreteEventPath & aPath, jobject & outObj)
{
    jclass eventPathCls = nullptr;
    ReturnErrorOnFailure(
        JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/model/ChipEventPath", eventPathCls));

    jmethodID eventPathCtor =
        env->GetStaticMethodID(eventPathCls, "newInstance", "(IJJ)Lchip/devicecontroller/model/ChipEventPath;");
    VerifyOrReturnError(eventPathCtor != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    outObj = env->CallStaticObjectMethod(eventPathCls, eventPathCtor, static_cast<jint>(aPath.mEndpointId),
                                         static_cast<jlong>(aPath.mClusterId), static_cast<jlong>(aPath.mEventId));
    VerifyOrReturnError(outObj != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);
    return CHIP_NO_ERROR;
}

GetConnectedDeviceCallback::GetConnectedDeviceCallback(jobject wrapperCallback, jobject javaCallback) :
    mOnSuccess(OnDeviceConnectedFn, this), mOnFailure(OnDeviceConnectionFailureFn, this)
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
    JniReferences::GetInstance().GetLocalClassRef(
        env, "chip/devicecontroller/GetConnectedDeviceCallbackJni$GetConnectedDeviceCallback", getConnectedDeviceCallbackCls);
    VerifyOrReturn(getConnectedDeviceCallbackCls != nullptr,
                   ChipLogError(Controller, "Could not find GetConnectedDeviceCallback class"));

    jmethodID successMethod;
    JniReferences::GetInstance().FindMethod(env, javaCallback, "onDeviceConnected", "(J)V", &successMethod);
    VerifyOrReturn(successMethod != nullptr, ChipLogError(Controller, "Could not find onDeviceConnected method"));

    static_assert(sizeof(jlong) >= sizeof(void *), "Need to store a pointer in a Java handle");

    OperationalDeviceProxy * device = new OperationalDeviceProxy(&exchangeMgr, sessionHandle);
    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, successMethod, reinterpret_cast<jlong>(device));
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

void GetConnectedDeviceCallback::OnDeviceConnectionFailureFn(void * context,
                                                             const OperationalSessionSetup::ConnnectionFailureInfo & failureInfo)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    auto * self = static_cast<GetConnectedDeviceCallback *>(context);
    VerifyOrReturn(self->mJavaCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mJavaCallbackRef is not valid in %s", __func__));
    jobject javaCallback = self->mJavaCallbackRef.ObjectRef();
    JniLocalReferenceScope scope(env);

    jclass getConnectedDeviceCallbackCls = nullptr;
    JniReferences::GetInstance().GetLocalClassRef(
        env, "chip/devicecontroller/GetConnectedDeviceCallbackJni$GetConnectedDeviceCallback", getConnectedDeviceCallbackCls);
    VerifyOrReturn(getConnectedDeviceCallbackCls != nullptr,
                   ChipLogError(Controller, "Could not find GetConnectedDeviceCallback class"));

    jmethodID failureMethod;
    JniReferences::GetInstance().FindMethod(env, javaCallback, "onConnectionFailure", "(JLjava/lang/Exception;)V", &failureMethod);
    VerifyOrReturn(failureMethod != nullptr, ChipLogError(Controller, "Could not find onConnectionFailure method"));

    jthrowable exception;
    CHIP_ERROR err = AndroidConnectionFailureExceptions::GetInstance().CreateAndroidConnectionFailureException(
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

ReportCallback::ReportCallback(jobject wrapperCallback, jobject subscriptionEstablishedCallback, jobject reportCallback,
                               jobject resubscriptionAttemptCallback) :
    mClusterCacheAdapter(*this, Optional<EventNumber>::Missing(), false /*cacheData*/)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    if (subscriptionEstablishedCallback != nullptr)
    {
        VerifyOrReturn(mSubscriptionEstablishedCallbackRef.Init(subscriptionEstablishedCallback) == CHIP_NO_ERROR,
                       ChipLogError(Controller, "Could not init mSubscriptionEstablishedCallbackRef in %s", __func__));
    }

    VerifyOrReturn(mReportCallbackRef.Init(reportCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mReportCallbackRef in %s", __func__));
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
    jclass nodeStateCls = nullptr;
    CHIP_ERROR err      = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/model/NodeState", nodeStateCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not get NodeState class"));
    jmethodID nodeStateCtor = env->GetMethodID(nodeStateCls, "<init>", "()V");
    VerifyOrReturn(nodeStateCtor != nullptr, ChipLogError(Controller, "Could not find NodeState constructor"));
    jobject nodeState = env->NewObject(nodeStateCls, nodeStateCtor);
    VerifyOrReturn(nodeState != nullptr, ChipLogError(Controller, "Could not create local object for nodeState"));
    VerifyOrReturn(mNodeStateObj.Init(nodeState) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mNodeStateObj in %s", __func__));
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
    VerifyOrReturn(mReportCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mReportCallbackRef is not valid in %s", __func__));
    jobject reportCallback = mReportCallbackRef.ObjectRef();
    JniGlobalReference globalRef(std::move(mNodeStateObj));
    jmethodID onReportMethod;
    err = JniReferences::GetInstance().FindMethod(env, reportCallback, "onReport", "(Lchip/devicecontroller/model/NodeState;)V",
                                                  &onReportMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onReport method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(reportCallback, onReportMethod, globalRef.ObjectRef());
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
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    jobject attributePathObj = nullptr;
    err                      = CreateChipAttributePath(env, aPath, attributePathObj);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create Java ChipAttributePath: %s", ErrorStr(err)));

    VerifyOrReturn(!aPath.IsListItemOperation(), ChipLogError(Controller, "Expect non-list item operation"); aPath.LogPath());
    VerifyOrReturn(aStatus.IsSuccess(), ChipLogError(Controller, "Receive bad status %s", ErrorStr(aStatus.ToChipError()));
                   aPath.LogPath());
    VerifyOrReturn(apData != nullptr, ChipLogError(Controller, "Receive empty apData"); aPath.LogPath());

    TLV::TLVReader readerForJavaTLV;
    readerForJavaTLV.Init(*apData);

    jobject value = nullptr;
#ifdef USE_JAVA_TLV_ENCODE_DECODE
    TLV::TLVReader readerForJavaObject;
    readerForJavaObject.Init(*apData);

    value = DecodeAttributeValue(aPath, readerForJavaObject, &err);
    // If we don't know this attribute, suppress it.
    if (err == CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB)
    {
        err = CHIP_NO_ERROR;
    }

    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Fail to decode attribute with error %s", ErrorStr(err));
                   aPath.LogPath());
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
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

    // Create AttributeState object
    jclass attributeStateCls;
    err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/model/AttributeState", attributeStateCls);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not find AttributeState class with error %s", ErrorStr(err));
                   aPath.LogPath());
    VerifyOrReturn(attributeStateCls != nullptr, ChipLogError(Controller, "Could not find AttributeState class"); aPath.LogPath());
    jmethodID attributeStateCtor = env->GetMethodID(attributeStateCls, "<init>", "(Ljava/lang/Object;[BLjava/lang/String;)V");
    VerifyOrReturn(attributeStateCtor != nullptr, ChipLogError(Controller, "Could not find AttributeState constructor");
                   aPath.LogPath());
    jobject attributeStateObj =
        env->NewObject(attributeStateCls, attributeStateCtor, value, jniByteArray.jniValue(), jsonString.jniValue());
    VerifyOrReturn(attributeStateObj != nullptr, ChipLogError(Controller, "Could not create AttributeState object");
                   aPath.LogPath());

    jobject nodeState = mNodeStateObj.ObjectRef();
    // Add AttributeState to NodeState
    jmethodID addAttributeMethod;
    err = JniReferences::GetInstance().FindMethod(env, nodeState, "addAttribute",
                                                  "(IJJLchip/devicecontroller/model/AttributeState;)V", &addAttributeMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not find addAttribute method with error %s", ErrorStr(err)));
    env->CallVoidMethod(nodeState, addAttributeMethod, static_cast<jint>(aPath.mEndpointId), static_cast<jlong>(aPath.mClusterId),
                        static_cast<jlong>(aPath.mAttributeId), attributeStateObj);
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

    jobject nodeState = mNodeStateObj.ObjectRef();

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
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    jobject eventPathObj = nullptr;
    err                  = CreateChipEventPath(env, aEventHeader.mPath, eventPathObj);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create Java ChipEventPath: %s", ErrorStr(err)));

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
#ifdef USE_JAVA_TLV_ENCODE_DECODE
    TLV::TLVReader readerForJavaObject;
    readerForJavaObject.Init(*apData);
    value = DecodeEventValue(aEventHeader.mPath, readerForJavaObject, &err);
    // If we don't know this event, just skip it.
    if (err == CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Fail to decode event with error %s", ErrorStr(err));
                   aEventHeader.LogPath());
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
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

    // Create EventState object
    jclass eventStateCls;
    err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/model/EventState", eventStateCls);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Failed to find EventState class"); aEventHeader.LogPath());
    VerifyOrReturn(eventStateCls != nullptr, ChipLogError(Controller, "Could not find EventState class"); aEventHeader.LogPath());
    jmethodID eventStateCtor = env->GetMethodID(eventStateCls, "<init>", "(JIIJLjava/lang/Object;[BLjava/lang/String;)V");
    VerifyOrReturn(eventStateCtor != nullptr, ChipLogError(Controller, "Could not find EventState constructor");
                   aEventHeader.LogPath());
    jobject eventStateObj = env->NewObject(eventStateCls, eventStateCtor, eventNumber, priorityLevel, timestampType, timestampValue,
                                           value, jniByteArray.jniValue(), jsonString.jniValue());
    VerifyOrReturn(eventStateObj != nullptr, ChipLogError(Controller, "Could not create EventState object");
                   aEventHeader.LogPath());

    // Add EventState to NodeState
    jmethodID addEventMethod;
    jobject nodeState = mNodeStateObj.ObjectRef();
    err = JniReferences::GetInstance().FindMethod(env, nodeState, "addEvent", "(IJJLchip/devicecontroller/model/EventState;)V",
                                                  &addEventMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find addEvent method with error %s", ErrorStr(err));
                   aEventHeader.LogPath());
    env->CallVoidMethod(nodeState, addEventMethod, static_cast<jint>(aEventHeader.mPath.mEndpointId),
                        static_cast<jlong>(aEventHeader.mPath.mClusterId), static_cast<jlong>(aEventHeader.mPath.mEventId),
                        eventStateObj);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe(); aEventHeader.LogPath());
}

CHIP_ERROR InvokeCallback::CreateInvokeElement(JNIEnv * env, const app::ConcreteCommandPath & aPath, TLV::TLVReader * apData,
                                               jobject & outObj)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    jclass invokeElementCls = nullptr;
    err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/model/InvokeElement", invokeElementCls);
    ReturnErrorOnFailure(err);

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

        std::unique_ptr<uint8_t[]> buffer = std::unique_ptr<uint8_t[]>(new uint8_t[bufferLen]);
        uint32_t size                     = 0;

        TLV::TLVWriter writer;
        writer.Init(buffer.get(), bufferLen);
        err = writer.CopyElement(TLV::AnonymousTag(), readerForJavaTLV);
        ReturnErrorOnFailure(err);
        size = writer.GetLengthWritten();
        chip::ByteArray jniByteArray(env, reinterpret_cast<jbyte *>(buffer.get()), static_cast<jint>(size));

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
    return CHIP_NO_ERROR;
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
    JniGlobalReference globalRef(std::move(mWrapperCallbackRef));
    jmethodID onDoneMethod;
    VerifyOrReturn(mReportCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mReportCallbackRef is not valid in %s", __func__));
    jobject reportCallback = mReportCallbackRef.ObjectRef();
    err                    = JniReferences::GetInstance().FindMethod(env, reportCallback, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    if (mReadClient != nullptr)
    {
        Platform::Delete(mReadClient);
    }
    mReadClient = nullptr;

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(reportCallback, onDoneMethod);
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
    JniReferences::GetInstance().CallSubscriptionEstablished(mSubscriptionEstablishedCallbackRef.ObjectRef(), aSubscriptionId);
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
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);
    VerifyOrReturn(mReportCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mReportCallbackRef is not valid in %s", __func__));
    jobject reportCallback = mReportCallbackRef.ObjectRef();
    jthrowable exception;
    err = AndroidControllerExceptions::GetInstance().CreateAndroidControllerException(env, message, errorCode, exception);
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(Controller, "Unable to create AndroidControllerException on ReportCallback::ReportError: %s", ErrorStr(err)));
    jmethodID onErrorMethod;
    err = JniReferences::GetInstance().FindMethod(
        env, reportCallback, "onError",
        "(Lchip/devicecontroller/model/ChipAttributePath;Lchip/devicecontroller/model/ChipEventPath;Ljava/lang/Exception;)V",
        &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(reportCallback, onErrorMethod, attributePath, eventPath, exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

WriteAttributesCallback::WriteAttributesCallback(jobject wrapperCallback, jobject javaCallback) : mChunkedWriteCallback(this)
{
    VerifyOrReturn(mWrapperCallbackRef.Init(wrapperCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mWrapperCallbackRef for WriteAttributesCallback"));
    VerifyOrReturn(mJavaCallbackRef.Init(javaCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mJavaCallbackRef in %s", __func__));
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

    jobject attributePathObj = nullptr;
    err                      = CreateChipAttributePath(env, aPath, attributePathObj);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create Java ChipAttributePath: %s", ErrorStr(err)));

    if (aStatus.mStatus != Protocols::InteractionModel::Status::Success)
    {
        ReportError(attributePathObj, aStatus.mStatus);
        return;
    }

    jmethodID onResponseMethod;
    VerifyOrReturn(mJavaCallbackRef.HasValidObjectRef(), ChipLogError(Controller, "mJavaCallbackRef is not valid in %s", __func__));
    jobject javaCallback = mJavaCallbackRef.ObjectRef();
    err                  = JniReferences::GetInstance().FindMethod(env, javaCallback, "onResponse",
                                                                   "(Lchip/devicecontroller/model/ChipAttributePath;)V", &onResponseMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, onResponseMethod, attributePathObj);
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
    JniGlobalReference globalRef(std::move(mWrapperCallbackRef));
    jmethodID onDoneMethod;
    VerifyOrReturn(mJavaCallbackRef.HasValidObjectRef(), ChipLogError(Controller, "mJavaCallbackRef is not valid in %s", __func__));
    jobject javaCallback = mJavaCallbackRef.ObjectRef();
    err                  = JniReferences::GetInstance().FindMethod(env, javaCallback, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, onDoneMethod);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
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
    VerifyOrReturn(mJavaCallbackRef.HasValidObjectRef(), ChipLogError(Controller, "mJavaCallbackRef is not valid in %s", __func__));
    jobject javaCallback = mJavaCallbackRef.ObjectRef();
    err                  = JniReferences::GetInstance().FindMethod(
        env, javaCallback, "onError", "(Lchip/devicecontroller/model/ChipAttributePath;Ljava/lang/Exception;)V", &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, onErrorMethod, attributePath, exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

InvokeCallback::InvokeCallback(jobject wrapperCallback, jobject javaCallback)
{
    VerifyOrReturn(mWrapperCallbackRef.Init(wrapperCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mWrapperCallbackRef for InvokeCallback"));
    VerifyOrReturn(mJavaCallbackRef.Init(javaCallback) == CHIP_NO_ERROR,
                   ChipLogError(Controller, "Could not init mJavaCallbackRef in %s", __func__));
}

InvokeCallback::~InvokeCallback()
{
    if (mCommandSender != nullptr)
    {
        Platform::Delete(mCommandSender);
    }
}

void InvokeCallback::OnResponse(app::CommandSender * apCommandSender, const app::ConcreteCommandPath & aPath,
                                const app::StatusIB & aStatusIB, TLV::TLVReader * apData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    JNIEnv * env   = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    jobject invokeElementObj = nullptr;
    jmethodID onResponseMethod;
    JniLocalReferenceScope scope(env);
    err = CreateInvokeElement(env, aPath, apData, invokeElementObj);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to create Java InvokeElement: %s", ErrorStr(err)));
    VerifyOrReturn(mJavaCallbackRef.HasValidObjectRef(), ChipLogError(Controller, "mJavaCallbackRef is not valid in %s", __func__));
    jobject javaCallback = mJavaCallbackRef.ObjectRef();
    err                  = JniReferences::GetInstance().FindMethod(env, javaCallback, "onResponse",
                                                                   "(Lchip/devicecontroller/model/InvokeElement;J)V", &onResponseMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onResponse method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    if (aStatusIB.mClusterStatus.HasValue())
    {
        env->CallVoidMethod(javaCallback, onResponseMethod, invokeElementObj, static_cast<jlong>(aStatusIB.mClusterStatus.Value()));
    }
    else
    {
        env->CallVoidMethod(javaCallback, onResponseMethod, invokeElementObj,
                            static_cast<jlong>(Protocols::InteractionModel::Status::Success));
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
    VerifyOrReturn(mWrapperCallbackRef.HasValidObjectRef(),
                   ChipLogError(Controller, "mWrapperCallbackRef is not valid in %s", __func__));
    JniGlobalReference globalRef(std::move(mWrapperCallbackRef));
    jmethodID onDoneMethod;
    VerifyOrReturn(mJavaCallbackRef.HasValidObjectRef(), ChipLogError(Controller, "mJavaCallbackRef is not valid in %s", __func__));
    jobject javaCallback = mJavaCallbackRef.ObjectRef();
    err                  = JniReferences::GetInstance().FindMethod(env, javaCallback, "onDone", "()V", &onDoneMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Could not find onDone method"));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, onDoneMethod);
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
    VerifyOrReturn(mJavaCallbackRef.HasValidObjectRef(), ChipLogError(Controller, "mJavaCallbackRef is not valid in %s", __func__));
    jobject javaCallback = mJavaCallbackRef.ObjectRef();
    err = JniReferences::GetInstance().FindMethod(env, javaCallback, "onError", "(Ljava/lang/Exception;)V", &onErrorMethod);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Controller, "Unable to find onError method: %s", ErrorStr(err)));

    DeviceLayer::StackUnlock unlock;
    env->CallVoidMethod(javaCallback, onErrorMethod, exception);
    VerifyOrReturn(!env->ExceptionCheck(), env->ExceptionDescribe());
}

} // namespace Controller
} // namespace chip
