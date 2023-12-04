/*
 *   Copyright (c) 2020-2022 Project CHIP Authors
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

/**
 *    @file
 *      Implementation of JNI bridge for CHIP Device Controller for Android apps
 *
 */
#include "AndroidCallbacks.h"
#include "AndroidCommissioningWindowOpener.h"
#include "AndroidCurrentFabricRemover.h"
#include "AndroidDeviceControllerWrapper.h"
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

#include <app/AttributePathParams.h>
#include <app/DataVersionFilter.h>
#include <app/InteractionModelEngine.h>
#include <app/ReadClient.h>
#include <app/WriteClient.h>
#include <app/util/error-mapping.h>
#include <atomic>
#include <ble/BleUUID.h>
#include <controller/CHIPDeviceController.h>
#include <controller/CommissioningWindowOpener.h>
#include <controller/java/AndroidClusterExceptions.h>
#include <controller/java/GroupDeviceProxy.h>
#include <credentials/CHIPCert.h>
#include <jni.h>
#include <lib/core/ErrorStr.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/jsontlv/JsonToTlv.h>
#include <lib/support/jsontlv/TlvToJson.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/Protocols.h>
#include <pthread.h>
#include <system/SystemClock.h>
#include <vector>

#ifdef CHIP_DEVICE_CONFIG_DYNAMIC_SERVER
#include <app/dynamic_server/AccessControl.h>
#endif // CHIP_DEVICE_CONFIG_DYNAMIC_SERVER

#ifdef JAVA_MATTER_CONTROLLER_TEST
#include <controller/ExampleOperationalCredentialsIssuer.h>
#else
#include <platform/android/AndroidChipPlatform-JNI.h>
#endif

// Choose an approximation of PTHREAD_NULL if pthread.h doesn't define one.
#ifndef PTHREAD_NULL
#define PTHREAD_NULL 0
#endif // PTHREAD_NULL

using namespace chip;
using namespace chip::Inet;
using namespace chip::Controller;
using namespace chip::Credentials;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipDeviceController_##METHOD_NAME

static CHIP_ERROR GetChipPathIdValue(jobject chipPathId, uint32_t wildcardValue, uint32_t & outValue);
static CHIP_ERROR ParseAttributePathList(jobject attributePathList,
                                         std::vector<app::AttributePathParams> & outAttributePathParamsList);
CHIP_ERROR ParseAttributePath(jobject attributePath, EndpointId & outEndpointId, ClusterId & outClusterId,
                              AttributeId & outAttributeId);
static CHIP_ERROR ParseEventPathList(jobject eventPathList, std::vector<app::EventPathParams> & outEventPathParamsList);
CHIP_ERROR ParseEventPath(jobject eventPath, EndpointId & outEndpointId, ClusterId & outClusterId, EventId & outEventId,
                          bool & outIsUrgent);
CHIP_ERROR ParseDataVersionFilter(jobject dataVersionFilter, EndpointId & outEndpointId, ClusterId & outClusterId,
                                  DataVersion & outDataVersion);
static CHIP_ERROR ParseDataVersionFilterList(jobject dataVersionFilterList,
                                             std::vector<app::DataVersionFilter> & outDataVersionFilterList);
static CHIP_ERROR IsWildcardChipPathId(jobject chipPathId, bool & isWildcard);

JNI_METHOD(void, subscribe)
(JNIEnv * env, jclass clz, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jobject dataVersionFilterList, jint minInterval, jint maxInterval, jboolean keepSubscriptions, jboolean isFabricFiltered,
 jint imTimeoutMs, jobject eventMin)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err               = CHIP_NO_ERROR;
    app::ReadClient * readClient = nullptr;
    jint numAttributePaths       = 0;
    jint numEventPaths           = 0;
    jint numDataVersionFilters   = 0;
    auto callback                = reinterpret_cast<ReportCallback *>(callbackHandle);
    DeviceProxy * device         = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return;
    }

    app::ReadPrepareParams params(device->GetSecureSession().Value());

    uint16_t aImTimeoutMs             = static_cast<uint16_t>(imTimeoutMs);
    params.mMinIntervalFloorSeconds   = static_cast<uint16_t>(minInterval);
    params.mMaxIntervalCeilingSeconds = static_cast<uint16_t>(maxInterval);
    params.mKeepSubscriptions         = (keepSubscriptions != JNI_FALSE);
    params.mIsFabricFiltered          = (isFabricFiltered != JNI_FALSE);
    params.mTimeout                   = aImTimeoutMs != 0 ? System::Clock::Milliseconds32(aImTimeoutMs) : System::Clock::kZero;

    if (attributePathList != nullptr)
    {
        SuccessOrExit(err = JniReferences::GetInstance().GetListSize(attributePathList, numAttributePaths));
    }

    if (numAttributePaths > 0)
    {
        std::unique_ptr<chip::app::AttributePathParams[]> attributePaths(new chip::app::AttributePathParams[numAttributePaths]);
        for (uint8_t i = 0; i < numAttributePaths; i++)
        {
            jobject attributePathItem = nullptr;
            SuccessOrExit(err = JniReferences::GetInstance().GetListItem(attributePathList, i, attributePathItem));

            EndpointId endpointId;
            ClusterId clusterId;
            AttributeId attributeId;
            SuccessOrExit(err = ParseAttributePath(attributePathItem, endpointId, clusterId, attributeId));
            attributePaths[i] = chip::app::AttributePathParams(endpointId, clusterId, attributeId);
        }
        params.mpAttributePathParamsList    = attributePaths.get();
        params.mAttributePathParamsListSize = numAttributePaths;
        attributePaths.release();
    }

    if (dataVersionFilterList != nullptr)
    {
        SuccessOrExit(err = JniReferences::GetInstance().GetListSize(dataVersionFilterList, numDataVersionFilters));
    }

    if (numDataVersionFilters > 0)
    {
        std::unique_ptr<chip::app::DataVersionFilter[]> dataVersionFilters(new chip::app::DataVersionFilter[numDataVersionFilters]);
        for (uint8_t i = 0; i < numDataVersionFilters; i++)
        {
            jobject dataVersionFilterItem = nullptr;
            SuccessOrExit(err = JniReferences::GetInstance().GetListItem(dataVersionFilterList, i, dataVersionFilterItem));

            EndpointId endpointId;
            ClusterId clusterId;
            DataVersion dataVersion;
            SuccessOrExit(err = ParseDataVersionFilter(dataVersionFilterItem, endpointId, clusterId, dataVersion));
            dataVersionFilters[i] = chip::app::DataVersionFilter(endpointId, clusterId, dataVersion);
        }
        params.mpDataVersionFilterList    = dataVersionFilters.get();
        params.mDataVersionFilterListSize = numDataVersionFilters;
        dataVersionFilters.release();
    }

    if (eventMin != nullptr)
    {
        params.mEventNumber.SetValue(static_cast<chip::EventNumber>(JniReferences::GetInstance().LongToPrimitive(eventMin)));
    }

    if (eventPathList != nullptr)
    {
        SuccessOrExit(err = JniReferences::GetInstance().GetListSize(eventPathList, numEventPaths));
    }

    if (numEventPaths > 0)
    {
        std::unique_ptr<chip::app::EventPathParams[]> eventPaths(new chip::app::EventPathParams[numEventPaths]);
        for (uint8_t i = 0; i < numEventPaths; i++)
        {
            jobject eventPathItem = nullptr;
            SuccessOrExit(err = JniReferences::GetInstance().GetListItem(eventPathList, i, eventPathItem));

            EndpointId endpointId;
            ClusterId clusterId;
            EventId eventId;
            bool isUrgent;
            SuccessOrExit(err = ParseEventPath(eventPathItem, endpointId, clusterId, eventId, isUrgent));
            eventPaths[i] = chip::app::EventPathParams(endpointId, clusterId, eventId, isUrgent);
        }

        params.mpEventPathParamsList    = eventPaths.get();
        params.mEventPathParamsListSize = numEventPaths;
        eventPaths.release();
    }

    readClient = Platform::New<app::ReadClient>(app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                callback->mClusterCacheAdapter.GetBufferedCallback(),
                                                app::ReadClient::InteractionType::Subscribe);

    SuccessOrExit(err = readClient->SendAutoResubscribeRequest(std::move(params)));
    callback->mReadClient = readClient;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Subscribe Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(err);
        if (readClient != nullptr)
        {
            Platform::Delete(readClient);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

CHIP_ERROR read(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, std::vector<app::AttributePathParams> &attributePathParamsList, std::vector<app::EventPathParams> &eventPathParamsList, std::vector<app::DataVersionFilter> &versionList, jboolean isFabricFiltered, jint imTimeoutMs, jobject eventMin)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err               = CHIP_NO_ERROR;

    app::ReadClient * readClient = nullptr;
    auto callback                = reinterpret_cast<ReportCallback *>(callbackHandle);
    DeviceProxy * device         = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    app::ReadPrepareParams params(device->GetSecureSession().Value());

    VerifyOrExit(attributePathParamsList.size() != 0 || eventPathParamsList.size() != 0, err = CHIP_ERROR_INVALID_ARGUMENT);
    params.mpAttributePathParamsList    = attributePathParamsList.data();
    params.mAttributePathParamsListSize = attributePathParamsList.size();
    params.mpEventPathParamsList        = eventPathParamsList.data();
    params.mEventPathParamsListSize     = eventPathParamsList.size();
    if (versionList.size() != 0)
    {
        params.mpDataVersionFilterList    = versionList.data();
        params.mDataVersionFilterListSize = versionList.size();
    }

    params.mIsFabricFiltered = (isFabricFiltered != JNI_FALSE);
    params.mTimeout          = imTimeoutMs != 0 ? System::Clock::Milliseconds32(imTimeoutMs) : System::Clock::kZero;

    if (eventMin != nullptr)
    {
        params.mEventNumber.SetValue(static_cast<chip::EventNumber>(JniReferences::GetInstance().LongToPrimitive(eventMin)));
    }

    readClient = Platform::New<app::ReadClient>(app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                callback->mClusterCacheAdapter.GetBufferedCallback(),
                                                app::ReadClient::InteractionType::Read);

    SuccessOrExit(err = readClient->SendRequest(params));
    callback->mReadClient = readClient;

    return CHIP_NO_ERROR;
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Read Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(err);
        if (readClient != nullptr)
        {
            Platform::Delete(readClient);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }

    return err;
}

JNI_METHOD(void, read)
(JNIEnv * env, jclass clz, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
 jobject dataVersionFilterList, jboolean isFabricFiltered, jint imTimeoutMs, jobject eventMin)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::vector<app::AttributePathParams> attributePathParamsList;
    std::vector<app::EventPathParams> eventPathParamsList;
    std::vector<app::DataVersionFilter> versionList;

    SuccessOrExit(err = ParseAttributePathList(attributePathList, attributePathParamsList));
    SuccessOrExit(err = ParseEventPathList(eventPathList, eventPathParamsList));
    SuccessOrExit(err = ParseDataVersionFilterList(dataVersionFilterList, versionList));

    SuccessOrExit(err = read(env, handle, callbackHandle, devicePtr, attributePathParamsList, eventPathParamsList, versionList, isFabricFiltered, imTimeoutMs, eventMin));
    return;
exit:
    ChipLogError(Controller, "JNI IM Read Error: %s", err.AsString());
}

// Convert Json to Tlv, and remove the outer structure
CHIP_ERROR ConvertJsonToTlvWithoutStruct(const std::string & json, MutableByteSpan & data)
{
    Platform::ScopedMemoryBufferWithSize<uint8_t> buf;
    VerifyOrReturnError(buf.Calloc(data.size()), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan dataWithStruct(buf.Get(), buf.AllocatedSize());
    ReturnErrorOnFailure(JsonToTlv(json, dataWithStruct));
    TLV::TLVReader tlvReader;
    TLV::TLVType outerContainer = TLV::kTLVType_Structure;
    tlvReader.Init(dataWithStruct);
    ReturnErrorOnFailure(tlvReader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(outerContainer));
    ReturnErrorOnFailure(tlvReader.Next());

    TLV::TLVWriter tlvWrite;
    tlvWrite.Init(data);
    ReturnErrorOnFailure(tlvWrite.CopyElement(TLV::AnonymousTag(), tlvReader));
    ReturnErrorOnFailure(tlvWrite.Finalize());
    data.reduce_size(tlvWrite.GetLengthWritten());
    return CHIP_NO_ERROR;
}

CHIP_ERROR PutPreencodedWriteAttribute(app::WriteClient & writeClient, app::ConcreteDataAttributePath & path, const ByteSpan & data)
{
    TLV::TLVReader reader;
    reader.Init(data);
    ReturnErrorOnFailure(reader.Next());
    return writeClient.PutPreencodedAttribute(path, reader);
}

JNI_METHOD(void, write)
(JNIEnv * env, jclass clz, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributeList, jint timedRequestTimeoutMs,
 jint imTimeoutMs)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    jint listSize                           = 0;
    auto callback                           = reinterpret_cast<WriteAttributesCallback *>(callbackHandle);
    app::WriteClient * writeClient          = nullptr;
    uint16_t convertedTimedRequestTimeoutMs = static_cast<uint16_t>(timedRequestTimeoutMs);

    ChipLogDetail(Controller, "IM write() called");

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);
    VerifyOrExit(attributeList != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    SuccessOrExit(err = JniReferences::GetInstance().GetListSize(attributeList, listSize));

    writeClient = Platform::New<app::WriteClient>(
        device->GetExchangeManager(), callback->GetChunkedWriteCallback(),
        convertedTimedRequestTimeoutMs != 0 ? Optional<uint16_t>(convertedTimedRequestTimeoutMs) : Optional<uint16_t>::Missing());

    for (uint8_t i = 0; i < listSize; i++)
    {
        jobject attributeItem             = nullptr;
        uint32_t endpointId               = 0;
        uint32_t clusterId                = 0;
        uint32_t attributeId              = 0;
        jmethodID getEndpointIdMethod     = nullptr;
        jmethodID getClusterIdMethod      = nullptr;
        jmethodID getAttributeIdMethod    = nullptr;
        jmethodID hasDataVersionMethod    = nullptr;
        jmethodID getDataVersionMethod    = nullptr;
        jmethodID getTlvByteArrayMethod   = nullptr;
        jmethodID getJsonStringMethod     = nullptr;
        jobject endpointIdObj             = nullptr;
        jobject clusterIdObj              = nullptr;
        jobject attributeIdObj            = nullptr;
        jbyteArray tlvBytesObj            = nullptr;
        bool hasDataVersion               = false;
        Optional<DataVersion> dataVersion = Optional<DataVersion>();

        bool isGroupSession = false;

        SuccessOrExit(err = JniReferences::GetInstance().GetListItem(attributeList, i, attributeItem));
        SuccessOrExit(err = JniReferences::GetInstance().FindMethod(
                          env, attributeItem, "getEndpointId", "()Lchip/devicecontroller/model/ChipPathId;", &getEndpointIdMethod));
        SuccessOrExit(err = JniReferences::GetInstance().FindMethod(
                          env, attributeItem, "getClusterId", "()Lchip/devicecontroller/model/ChipPathId;", &getClusterIdMethod));
        SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getAttributeId",
                                                                    "()Lchip/devicecontroller/model/ChipPathId;",
                                                                    &getAttributeIdMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "hasDataVersion", "()Z", &hasDataVersionMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getDataVersion", "()I", &getDataVersionMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getTlvByteArray", "()[B", &getTlvByteArrayMethod));

        isGroupSession = device->GetSecureSession().Value()->IsGroupSession();

        if (isGroupSession)
        {
            endpointId = kInvalidEndpointId;
        }
        else
        {
            endpointIdObj = env->CallObjectMethod(attributeItem, getEndpointIdMethod);
            VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
            VerifyOrExit(endpointIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

            SuccessOrExit(err = GetChipPathIdValue(endpointIdObj, kInvalidEndpointId, endpointId));
        }

        clusterIdObj = env->CallObjectMethod(attributeItem, getClusterIdMethod);
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        VerifyOrExit(clusterIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        attributeIdObj = env->CallObjectMethod(attributeItem, getAttributeIdMethod);
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        VerifyOrExit(attributeIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        SuccessOrExit(err = GetChipPathIdValue(clusterIdObj, kInvalidClusterId, clusterId));
        SuccessOrExit(err = GetChipPathIdValue(attributeIdObj, kInvalidAttributeId, attributeId));

        hasDataVersion = static_cast<bool>(env->CallBooleanMethod(attributeItem, hasDataVersionMethod));
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        if (hasDataVersion)
        {
            DataVersion dataVersionVal = static_cast<DataVersion>(env->CallIntMethod(attributeItem, getDataVersionMethod));
            VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
            dataVersion.SetValue(dataVersionVal);
        }

        tlvBytesObj = static_cast<jbyteArray>(env->CallObjectMethod(attributeItem, getTlvByteArrayMethod));
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        app::ConcreteDataAttributePath path(static_cast<EndpointId>(endpointId), static_cast<ClusterId>(clusterId),
                                            static_cast<AttributeId>(attributeId), dataVersion);
        if (tlvBytesObj != nullptr)
        {
            JniByteArray tlvByteArray(env, tlvBytesObj);
            SuccessOrExit(err = PutPreencodedWriteAttribute(*writeClient, path, tlvByteArray.byteSpan()));
        }
        else
        {
            SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getJsonString", "()Ljava/lang/String;",
                                                                        &getJsonStringMethod));
            jstring jsonJniString = static_cast<jstring>(env->CallObjectMethod(attributeItem, getJsonStringMethod));
            VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
            VerifyOrExit(jsonJniString != nullptr, err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
            JniUtfString jsonUtfJniString(env, jsonJniString);
            std::string jsonString = std::string(jsonUtfJniString.c_str(), jsonUtfJniString.size());

            // Context: Chunk write is supported in sdk, oversized list could be chunked in multiple message. When transforming
            // JSON to TLV, we need know the actual size for tlv blob when handling JsonToTlv
            // TODO: Implement memory auto-grow to get the actual size needed for tlv blob when transforming tlv to json.
            // Workaround: Allocate memory using json string's size, which is large enough to hold the corresponding tlv blob
            Platform::ScopedMemoryBufferWithSize<uint8_t> tlvBytes;
            size_t length = jsonUtfJniString.size();
            VerifyOrExit(tlvBytes.Calloc(length), err = CHIP_ERROR_NO_MEMORY);
            MutableByteSpan data(tlvBytes.Get(), tlvBytes.AllocatedSize());
            SuccessOrExit(err = ConvertJsonToTlvWithoutStruct(jsonString, data));
            SuccessOrExit(err = PutPreencodedWriteAttribute(*writeClient, path, data));
        }
    }

    err = writeClient->SendWriteRequest(device->GetSecureSession().Value(),
                                        imTimeoutMs != 0 ? System::Clock::Milliseconds32(imTimeoutMs) : System::Clock::kZero);
    SuccessOrExit(err);
    callback->mWriteClient = writeClient;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Write Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(writeClient, err);
        if (writeClient != nullptr)
        {
            Platform::Delete(writeClient);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

CHIP_ERROR PutPreencodedInvokeRequest(app::CommandSender & commandSender, app::CommandPathParams & path, const ByteSpan & data)
{
    // PrepareCommand does nott create the struct container with kFields and copycontainer below sets the
    // kFields container already
    ReturnErrorOnFailure(commandSender.PrepareCommand(path, false /* aStartDataStruct */));
    TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    TLV::TLVReader reader;
    reader.Init(data);
    ReturnErrorOnFailure(reader.Next());
    return writer->CopyContainer(TLV::ContextTag(app::CommandDataIB::Tag::kFields), reader);
}

JNI_METHOD(void, invoke)
(JNIEnv * env, jclass clz, jlong handle, jlong callbackHandle, jlong devicePtr, jobject invokeElement, jint timedRequestTimeoutMs,
 jint imTimeoutMs)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    auto callback                           = reinterpret_cast<InvokeCallback *>(callbackHandle);
    app::CommandSender * commandSender      = nullptr;
    uint32_t endpointId                     = 0;
    uint32_t clusterId                      = 0;
    uint32_t commandId                      = 0;
    uint16_t groupId                        = 0;
    bool isEndpointIdValid                  = false;
    bool isGroupIdValid                     = false;
    jmethodID getEndpointIdMethod           = nullptr;
    jmethodID getClusterIdMethod            = nullptr;
    jmethodID getCommandIdMethod            = nullptr;
    jmethodID getGroupIdMethod              = nullptr;
    jmethodID getTlvByteArrayMethod         = nullptr;
    jmethodID getJsonStringMethod           = nullptr;
    jmethodID isEndpointIdValidMethod       = nullptr;
    jmethodID isGroupIdValidMethod          = nullptr;
    jobject endpointIdObj                   = nullptr;
    jobject clusterIdObj                    = nullptr;
    jobject commandIdObj                    = nullptr;
    jobject groupIdObj                      = nullptr;
    jbyteArray tlvBytesObj                  = nullptr;
    uint16_t convertedTimedRequestTimeoutMs = static_cast<uint16_t>(timedRequestTimeoutMs);
    ChipLogDetail(Controller, "IM invoke() called");

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);

    commandSender = Platform::New<app::CommandSender>(callback, device->GetExchangeManager(), timedRequestTimeoutMs != 0);

    SuccessOrExit(err = JniReferences::GetInstance().FindMethod(
                      env, invokeElement, "getEndpointId", "()Lchip/devicecontroller/model/ChipPathId;", &getEndpointIdMethod));
    SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getClusterId",
                                                                "()Lchip/devicecontroller/model/ChipPathId;", &getClusterIdMethod));
    SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getCommandId",
                                                                "()Lchip/devicecontroller/model/ChipPathId;", &getCommandIdMethod));
    SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getGroupId", "()Ljava/util/Optional;",
                                                                &getGroupIdMethod));
    SuccessOrExit(
        err = JniReferences::GetInstance().FindMethod(env, invokeElement, "isEndpointIdValid", "()Z", &isEndpointIdValidMethod));
    SuccessOrExit(err =
                      JniReferences::GetInstance().FindMethod(env, invokeElement, "isGroupIdValid", "()Z", &isGroupIdValidMethod));
    SuccessOrExit(
        err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getTlvByteArray", "()[B", &getTlvByteArrayMethod));

    isEndpointIdValid = (env->CallBooleanMethod(invokeElement, isEndpointIdValidMethod) == JNI_TRUE);
    isGroupIdValid    = (env->CallBooleanMethod(invokeElement, isGroupIdValidMethod) == JNI_TRUE);

    if (isEndpointIdValid)
    {
        endpointIdObj = env->CallObjectMethod(invokeElement, getEndpointIdMethod);
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        VerifyOrExit(endpointIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        SuccessOrExit(err = GetChipPathIdValue(endpointIdObj, kInvalidEndpointId, endpointId));
    }

    if (isGroupIdValid)
    {
        VerifyOrExit(device->GetSecureSession().Value()->IsGroupSession(), err = CHIP_ERROR_INVALID_ARGUMENT);
        groupIdObj = env->CallObjectMethod(invokeElement, getGroupIdMethod);
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        VerifyOrExit(groupIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

        jobject boxedGroupId = nullptr;

        SuccessOrExit(err = JniReferences::GetInstance().GetOptionalValue(groupIdObj, boxedGroupId));
        VerifyOrExit(boxedGroupId != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
        groupId = static_cast<uint16_t>(JniReferences::GetInstance().IntegerToPrimitive(boxedGroupId));
    }

    clusterIdObj = env->CallObjectMethod(invokeElement, getClusterIdMethod);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
    VerifyOrExit(clusterIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    commandIdObj = env->CallObjectMethod(invokeElement, getCommandIdMethod);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
    VerifyOrExit(commandIdObj != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    SuccessOrExit(err = GetChipPathIdValue(clusterIdObj, kInvalidClusterId, clusterId));
    SuccessOrExit(err = GetChipPathIdValue(commandIdObj, kInvalidCommandId, commandId));

    tlvBytesObj = static_cast<jbyteArray>(env->CallObjectMethod(invokeElement, getTlvByteArrayMethod));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
    {
        uint16_t id = isEndpointIdValid ? static_cast<uint16_t>(endpointId) : groupId;
        app::CommandPathFlags flag =
            isEndpointIdValid ? app::CommandPathFlags::kEndpointIdValid : app::CommandPathFlags::kGroupIdValid;
        app::CommandPathParams path(id, static_cast<ClusterId>(clusterId), static_cast<CommandId>(commandId), flag);
        if (tlvBytesObj != nullptr)
        {
            JniByteArray tlvBytesObjBytes(env, tlvBytesObj);
            SuccessOrExit(err = PutPreencodedInvokeRequest(*commandSender, path, tlvBytesObjBytes.byteSpan()));
        }
        else
        {
            SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getJsonString", "()Ljava/lang/String;",
                                                                        &getJsonStringMethod));
            jstring jsonJniString = static_cast<jstring>(env->CallObjectMethod(invokeElement, getJsonStringMethod));
            VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
            VerifyOrExit(jsonJniString != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
            JniUtfString jsonUtfJniString(env, jsonJniString);
            // The invoke does not support chunk, kMaxSecureSduLengthBytes should be enough for command json blob
            uint8_t tlvBytes[chip::app::kMaxSecureSduLengthBytes] = { 0 };
            MutableByteSpan tlvEncodingLocal{ tlvBytes };
            SuccessOrExit(err = JsonToTlv(std::string(jsonUtfJniString.c_str(), jsonUtfJniString.size()), tlvEncodingLocal));
            SuccessOrExit(err = PutPreencodedInvokeRequest(*commandSender, path, tlvEncodingLocal));
        }
    }
    SuccessOrExit(err = commandSender->FinishCommand(convertedTimedRequestTimeoutMs != 0
                                                         ? Optional<uint16_t>(convertedTimedRequestTimeoutMs)
                                                         : Optional<uint16_t>::Missing()));

    SuccessOrExit(err = device->GetSecureSession().Value()->IsGroupSession()
                      ? commandSender->SendGroupCommandRequest(device->GetSecureSession().Value())
                      : commandSender->SendCommandRequest(device->GetSecureSession().Value(),
                                                          imTimeoutMs != 0
                                                              ? MakeOptional(System::Clock::Milliseconds32(imTimeoutMs))
                                                              : Optional<System::Clock::Timeout>::Missing()));

    callback->mCommandSender = commandSender;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "JNI IM Invoke Error: %s", err.AsString());
        if (err == CHIP_JNI_ERROR_EXCEPTION_THROWN)
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        callback->OnError(nullptr, err);
        if (commandSender != nullptr)
        {
            Platform::Delete(commandSender);
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

/**
 * Takes objects in attributePathList, converts them to app:AttributePathParams, and appends them to outAttributePathParamsList.
 */
CHIP_ERROR ParseAttributePathList(jobject attributePathList, std::vector<app::AttributePathParams> & outAttributePathParamsList)
{
    jint listSize;

    if (attributePathList == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(JniReferences::GetInstance().GetListSize(attributePathList, listSize));

    for (uint8_t i = 0; i < listSize; i++)
    {
        jobject attributePathItem = nullptr;
        ReturnErrorOnFailure(JniReferences::GetInstance().GetListItem(attributePathList, i, attributePathItem));

        EndpointId endpointId;
        ClusterId clusterId;
        AttributeId attributeId;
        ReturnErrorOnFailure(ParseAttributePath(attributePathItem, endpointId, clusterId, attributeId));
        outAttributePathParamsList.push_back(app::AttributePathParams(endpointId, clusterId, attributeId));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ParseAttributePath(jobject attributePath, EndpointId & outEndpointId, ClusterId & outClusterId,
                              AttributeId & outAttributeId)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID getEndpointIdMethod  = nullptr;
    jmethodID getClusterIdMethod   = nullptr;
    jmethodID getAttributeIdMethod = nullptr;
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, attributePath, "getEndpointId", "()Lchip/devicecontroller/model/ChipPathId;", &getEndpointIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, attributePath, "getClusterId", "()Lchip/devicecontroller/model/ChipPathId;", &getClusterIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, attributePath, "getAttributeId", "()Lchip/devicecontroller/model/ChipPathId;", &getAttributeIdMethod));

    jobject endpointIdObj = env->CallObjectMethod(attributePath, getEndpointIdMethod);
    VerifyOrReturnError(endpointIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject clusterIdObj = env->CallObjectMethod(attributePath, getClusterIdMethod);
    VerifyOrReturnError(clusterIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject attributeIdObj = env->CallObjectMethod(attributePath, getAttributeIdMethod);
    VerifyOrReturnError(attributeIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint32_t endpointId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(endpointIdObj, kInvalidEndpointId, endpointId));
    uint32_t clusterId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(clusterIdObj, kInvalidClusterId, clusterId));
    uint32_t attributeId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(attributeIdObj, kInvalidAttributeId, attributeId));

    outEndpointId  = static_cast<EndpointId>(endpointId);
    outClusterId   = static_cast<ClusterId>(clusterId);
    outAttributeId = static_cast<AttributeId>(attributeId);

    return CHIP_NO_ERROR;
}

/**
 * Takes objects in eventPathList, converts them to app:EventPathParams, and appends them to outEventPathParamsList.
 */
CHIP_ERROR ParseEventPathList(jobject eventPathList, std::vector<app::EventPathParams> & outEventPathParamsList)
{
    jint listSize;

    if (eventPathList == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(JniReferences::GetInstance().GetListSize(eventPathList, listSize));

    for (uint8_t i = 0; i < listSize; i++)
    {
        jobject eventPathItem = nullptr;
        ReturnErrorOnFailure(JniReferences::GetInstance().GetListItem(eventPathList, i, eventPathItem));

        EndpointId endpointId;
        ClusterId clusterId;
        EventId eventId;
        bool isUrgent;
        ReturnErrorOnFailure(ParseEventPath(eventPathItem, endpointId, clusterId, eventId, isUrgent));
        outEventPathParamsList.push_back(app::EventPathParams(endpointId, clusterId, eventId, isUrgent));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ParseEventPath(jobject eventPath, EndpointId & outEndpointId, ClusterId & outClusterId, EventId & outEventId,
                          bool & outIsUrgent)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID getEndpointIdMethod = nullptr;
    jmethodID getClusterIdMethod  = nullptr;
    jmethodID getEventIdMethod    = nullptr;
    jmethodID isUrgentMethod      = nullptr;

    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, eventPath, "getEndpointId", "()Lchip/devicecontroller/model/ChipPathId;", &getEndpointIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, eventPath, "getClusterId", "()Lchip/devicecontroller/model/ChipPathId;", &getClusterIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, eventPath, "getEventId",
                                                                 "()Lchip/devicecontroller/model/ChipPathId;", &getEventIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, eventPath, "isUrgent", "()Z", &isUrgentMethod));

    jobject endpointIdObj = env->CallObjectMethod(eventPath, getEndpointIdMethod);
    VerifyOrReturnError(endpointIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject clusterIdObj = env->CallObjectMethod(eventPath, getClusterIdMethod);
    VerifyOrReturnError(clusterIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jobject eventIdObj = env->CallObjectMethod(eventPath, getEventIdMethod);
    VerifyOrReturnError(eventIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    jboolean isUrgent = env->CallBooleanMethod(eventPath, isUrgentMethod);

    uint32_t endpointId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(endpointIdObj, kInvalidEndpointId, endpointId));
    uint32_t clusterId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(clusterIdObj, kInvalidClusterId, clusterId));
    uint32_t eventId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(eventIdObj, kInvalidEventId, eventId));

    outEndpointId = static_cast<EndpointId>(endpointId);
    outClusterId  = static_cast<ClusterId>(clusterId);
    outEventId    = static_cast<EventId>(eventId);
    outIsUrgent   = (isUrgent == JNI_TRUE);

    return CHIP_NO_ERROR;
}

/**
 * Takes objects in dataVersionFilterList, converts them to app:DataVersionFilter, and appends them to outDataVersionFilterList.
 */
CHIP_ERROR ParseDataVersionFilterList(jobject dataVersionFilterList, std::vector<app::DataVersionFilter> & outDataVersionFilterList)
{
    jint listSize;

    if (dataVersionFilterList == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(JniReferences::GetInstance().GetListSize(dataVersionFilterList, listSize));

    for (uint8_t i = 0; i < listSize; i++)
    {
        jobject dataVersionFilterItem = nullptr;
        ReturnErrorOnFailure(JniReferences::GetInstance().GetListItem(dataVersionFilterList, i, dataVersionFilterItem));

        EndpointId endpointId;
        ClusterId clusterId;
        DataVersion dataVersion;
        ReturnErrorOnFailure(ParseDataVersionFilter(dataVersionFilterItem, endpointId, clusterId, dataVersion));
        outDataVersionFilterList.push_back(app::DataVersionFilter(endpointId, clusterId, dataVersion));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ParseDataVersionFilter(jobject versionFilter, EndpointId & outEndpointId, ClusterId & outClusterId,
                                  DataVersion & outDataVersion)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    jmethodID getEndpointIdMethod  = nullptr;
    jmethodID getClusterIdMethod   = nullptr;
    jmethodID getDataVersionMethod = nullptr;

    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, versionFilter, "getEndpointId", "()Lchip/devicecontroller/model/ChipPathId;", &getEndpointIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, versionFilter, "getClusterId", "()Lchip/devicecontroller/model/ChipPathId;", &getClusterIdMethod));
    ReturnErrorOnFailure(
        JniReferences::GetInstance().FindMethod(env, versionFilter, "getDataVersion", "()J", &getDataVersionMethod));

    jobject endpointIdObj = env->CallObjectMethod(versionFilter, getEndpointIdMethod);
    VerifyOrReturnError(endpointIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    uint32_t endpointId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(endpointIdObj, kInvalidEndpointId, endpointId));
    outEndpointId        = static_cast<EndpointId>(endpointId);
    jobject clusterIdObj = env->CallObjectMethod(versionFilter, getClusterIdMethod);
    VerifyOrReturnError(clusterIdObj != nullptr, CHIP_ERROR_INCORRECT_STATE);
    uint32_t clusterId = 0;
    ReturnErrorOnFailure(GetChipPathIdValue(clusterIdObj, kInvalidClusterId, clusterId));
    outClusterId = static_cast<ClusterId>(clusterId);

    outDataVersion = static_cast<DataVersion>(env->CallLongMethod(versionFilter, getDataVersionMethod));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GetChipPathIdValue(jobject chipPathId, uint32_t wildcardValue, uint32_t & outValue)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();

    bool idIsWildcard = false;
    ReturnErrorOnFailure(IsWildcardChipPathId(chipPathId, idIsWildcard));

    if (idIsWildcard)
    {
        outValue = wildcardValue;
        return CHIP_NO_ERROR;
    }

    jmethodID getIdMethod = nullptr;
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, chipPathId, "getId", "()J", &getIdMethod));
    outValue = static_cast<uint32_t>(env->CallLongMethod(chipPathId, getIdMethod));
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);

    return CHIP_NO_ERROR;
}

CHIP_ERROR IsWildcardChipPathId(jobject chipPathId, bool & isWildcard)
{
    JNIEnv * env            = JniReferences::GetInstance().GetEnvForCurrentThread();
    jmethodID getTypeMethod = nullptr;
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(
        env, chipPathId, "getType", "()Lchip/devicecontroller/model/ChipPathId$IdType;", &getTypeMethod));

    jobject idType = env->CallObjectMethod(chipPathId, getTypeMethod);
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
    VerifyOrReturnError(idType != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    jmethodID nameMethod = nullptr;
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, idType, "name", "()Ljava/lang/String;", &nameMethod));

    jstring typeNameString = static_cast<jstring>(env->CallObjectMethod(idType, nameMethod));
    VerifyOrReturnError(!env->ExceptionCheck(), CHIP_JNI_ERROR_EXCEPTION_THROWN);
    VerifyOrReturnError(typeNameString != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);

    JniUtfString typeNameJniString(env, typeNameString);

    isWildcard = strncmp(typeNameJniString.c_str(), "WILDCARD", 8) == 0;

    return CHIP_NO_ERROR;
}
