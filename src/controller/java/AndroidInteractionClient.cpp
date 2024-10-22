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

/**
 *    @file
 *      Implementation of Interaction Client API for Android Platform
 *
 */
#include "AndroidInteractionClient.h"

#include "AndroidCallbacks.h"

#include <lib/support/jsontlv/JsonToTlv.h>

#include <string>

using namespace chip;
using namespace chip::Controller;

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

CHIP_ERROR subscribe(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList,
                     jobject eventPathList, jobject dataVersionFilterList, jint minInterval, jint maxInterval,
                     jboolean keepSubscriptions, jboolean isFabricFiltered, jint imTimeoutMs, jobject eventMin, jboolean isPeerLIT)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err               = CHIP_NO_ERROR;
    app::ReadClient * readClient = nullptr;
    size_t numAttributePaths     = 0;
    size_t numEventPaths         = 0;
    size_t numDataVersionFilters = 0;
    auto callback                = reinterpret_cast<ReportCallback *>(callbackHandle);
    DeviceProxy * device         = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return CHIP_ERROR_INCORRECT_STATE;
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
        jint jNumAttributePaths = 0;
        SuccessOrExit(err = JniReferences::GetInstance().GetListSize(attributePathList, jNumAttributePaths));
        numAttributePaths = static_cast<size_t>(jNumAttributePaths);
    }

    if (numAttributePaths > 0)
    {
        std::unique_ptr<chip::app::AttributePathParams[]> attributePaths(new chip::app::AttributePathParams[numAttributePaths]);
        for (size_t i = 0; i < numAttributePaths; i++)
        {
            jobject attributePathItem = nullptr;
            SuccessOrExit(err =
                              JniReferences::GetInstance().GetListItem(attributePathList, static_cast<jint>(i), attributePathItem));

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
        jint jNumDataVersionFilters = 0;
        SuccessOrExit(err = JniReferences::GetInstance().GetListSize(dataVersionFilterList, jNumDataVersionFilters));
        numDataVersionFilters = static_cast<size_t>(jNumDataVersionFilters);
    }

    if (numDataVersionFilters > 0)
    {
        std::unique_ptr<chip::app::DataVersionFilter[]> dataVersionFilters(new chip::app::DataVersionFilter[numDataVersionFilters]);
        for (size_t i = 0; i < numDataVersionFilters; i++)
        {
            jobject dataVersionFilterItem = nullptr;
            SuccessOrExit(
                err = JniReferences::GetInstance().GetListItem(dataVersionFilterList, static_cast<jint>(i), dataVersionFilterItem));

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

    params.mIsPeerLIT = (isPeerLIT == JNI_TRUE);
    ChipLogProgress(Controller, "Peer ICD type is set to %s", params.mIsPeerLIT ? "LIT-ICD" : "non LIT-ICD");

    if (eventPathList != nullptr)
    {
        jint jNumEventPaths = 0;
        SuccessOrExit(err = JniReferences::GetInstance().GetListSize(eventPathList, jNumEventPaths));
        numEventPaths = static_cast<size_t>(jNumEventPaths);
    }

    if (numEventPaths > 0)
    {
        std::unique_ptr<chip::app::EventPathParams[]> eventPaths(new chip::app::EventPathParams[numEventPaths]);
        for (size_t i = 0; i < numEventPaths; i++)
        {
            jobject eventPathItem = nullptr;
            SuccessOrExit(err = JniReferences::GetInstance().GetListItem(eventPathList, static_cast<jint>(i), eventPathItem));

            EndpointId endpointId;
            ClusterId clusterId;
            EventId eventId;
            bool isUrgent;
            SuccessOrExit(err = ParseEventPath(eventPathItem, endpointId, clusterId, eventId, isUrgent));
            eventPaths[i] = chip::app::EventPathParams(endpointId, clusterId, eventId, isUrgent);
        }

        params.mpEventPathParamsList    = eventPaths.get();
        params.mEventPathParamsListSize = static_cast<size_t>(numEventPaths);
        eventPaths.release();
    }

    readClient = Platform::New<app::ReadClient>(app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                callback->mClusterCacheAdapter.GetBufferedCallback(),
                                                app::ReadClient::InteractionType::Subscribe);

    SuccessOrExit(err = readClient->SendAutoResubscribeRequest(std::move(params)));
    callback->mReadClient = readClient;

    return CHIP_NO_ERROR;
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
    return err;
}

CHIP_ERROR read(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributePathList, jobject eventPathList,
                jobject dataVersionFilterList, jboolean isFabricFiltered, jint imTimeoutMs, jobject eventMin)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto callback = reinterpret_cast<ReportCallback *>(callbackHandle);
    std::vector<app::AttributePathParams> attributePathParamsList;
    std::vector<app::EventPathParams> eventPathParamsList;
    std::vector<app::DataVersionFilter> versionList;
    app::ReadClient * readClient = nullptr;
    DeviceProxy * device         = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    app::ReadPrepareParams params(device->GetSecureSession().Value());

    SuccessOrExit(err = ParseAttributePathList(attributePathList, attributePathParamsList));
    SuccessOrExit(err = ParseEventPathList(eventPathList, eventPathParamsList));
    SuccessOrExit(err = ParseDataVersionFilterList(dataVersionFilterList, versionList));
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
            readClient = nullptr;
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
            callback = nullptr;
        }
    }

    return err;
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

CHIP_ERROR write(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, jobject attributeList,
                 jint timedRequestTimeoutMs, jint imTimeoutMs)
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

    for (jint i = 0; i < listSize; i++)
    {
        jobject attributeItem             = nullptr;
        jmethodID getEndpointIdMethod     = nullptr;
        jmethodID getClusterIdMethod      = nullptr;
        jmethodID getAttributeIdMethod    = nullptr;
        jmethodID hasDataVersionMethod    = nullptr;
        jmethodID getDataVersionMethod    = nullptr;
        jmethodID getTlvByteArrayMethod   = nullptr;
        jmethodID getJsonStringMethod     = nullptr;
        jlong endpointIdObj               = 0;
        jlong clusterIdObj                = 0;
        jlong attributeIdObj              = 0;
        jbyteArray tlvBytesObj            = nullptr;
        bool hasDataVersion               = false;
        Optional<DataVersion> dataVersion = Optional<DataVersion>();

        bool isGroupSession = false;

        SuccessOrExit(err = JniReferences::GetInstance().GetListItem(attributeList, i, attributeItem));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getEndpointId", "(J)J", &getEndpointIdMethod));
        SuccessOrExit(err =
                          JniReferences::GetInstance().FindMethod(env, attributeItem, "getClusterId", "(J)J", &getClusterIdMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getAttributeId", "(J)J", &getAttributeIdMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "hasDataVersion", "()Z", &hasDataVersionMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getDataVersion", "()I", &getDataVersionMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, attributeItem, "getTlvByteArray", "()[B", &getTlvByteArrayMethod));

        isGroupSession = device->GetSecureSession().Value()->IsGroupSession();

        if (isGroupSession)
        {
            endpointIdObj = static_cast<jlong>(kInvalidEndpointId);
        }
        else
        {
            endpointIdObj = env->CallLongMethod(attributeItem, getEndpointIdMethod, static_cast<jlong>(kInvalidEndpointId));
            VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
        }

        clusterIdObj = env->CallLongMethod(attributeItem, getClusterIdMethod, static_cast<jlong>(kInvalidClusterId));
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

        attributeIdObj = env->CallLongMethod(attributeItem, getAttributeIdMethod, static_cast<jlong>(kInvalidAttributeId));
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

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
        app::ConcreteDataAttributePath path(static_cast<EndpointId>(endpointIdObj), static_cast<ClusterId>(clusterIdObj),
                                            static_cast<AttributeId>(attributeIdObj), dataVersion);
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
            std::string jsonString = std::string(jsonUtfJniString.c_str(), static_cast<size_t>(jsonUtfJniString.size()));

            // Context: Chunk write is supported in sdk, oversized list could be chunked in multiple message. When transforming
            // JSON to TLV, we need know the actual size for tlv blob when handling JsonToTlv
            // TODO: Implement memory auto-grow to get the actual size needed for tlv blob when transforming tlv to json.
            // Workaround: Allocate memory using json string's size, which is large enough to hold the corresponding tlv blob
            Platform::ScopedMemoryBufferWithSize<uint8_t> tlvBytes;
            size_t length = static_cast<size_t>(jsonUtfJniString.size());
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
            writeClient = nullptr;
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
            callback = nullptr;
        }
    }
    return err;
}

CHIP_ERROR PutPreencodedInvokeRequest(app::CommandSender & commandSender, app::CommandPathParams & path, const ByteSpan & data)
{
    // PrepareCommand does not create the struct container with kFields and copycontainer below sets the
    // kFields container already
    ReturnErrorOnFailure(commandSender.PrepareCommand(path, false /* aStartDataStruct */));
    TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    TLV::TLVReader reader;
    reader.Init(data);
    ReturnErrorOnFailure(reader.Next());
    return writer->CopyContainer(TLV::ContextTag(app::CommandDataIB::Tag::kFields), reader);
}

CHIP_ERROR PutPreencodedInvokeRequest(app::CommandSender & commandSender, app::CommandPathParams & path, const ByteSpan & data,
                                      app::CommandSender::PrepareCommandParameters & prepareCommandParams)
{
    // PrepareCommand does not create the struct container with kFields and copycontainer below sets the
    // kFields container already
    ReturnErrorOnFailure(commandSender.PrepareCommand(path, prepareCommandParams));
    TLV::TLVWriter * writer = commandSender.GetCommandDataIBTLVWriter();
    VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    TLV::TLVReader reader;
    reader.Init(data);
    ReturnErrorOnFailure(reader.Next());
    return writer->CopyContainer(TLV::ContextTag(app::CommandDataIB::Tag::kFields), reader);
}

CHIP_ERROR extendableInvoke(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, jobject invokeElementList,
                            jint timedRequestTimeoutMs, jint imTimeoutMs)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    auto callback                           = reinterpret_cast<ExtendableInvokeCallback *>(callbackHandle);
    app::CommandSender * commandSender      = nullptr;
    uint16_t groupId                        = 0;
    bool isEndpointIdValid                  = false;
    bool isGroupIdValid                     = false;
    jint listSize                           = 0;
    uint16_t convertedTimedRequestTimeoutMs = static_cast<uint16_t>(timedRequestTimeoutMs);
    app::CommandSender::ConfigParameters config;

    ChipLogDetail(Controller, "IM extendableInvoke() called");

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);

    VerifyOrExit(invokeElementList != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    SuccessOrExit(err = JniReferences::GetInstance().GetListSize(invokeElementList, listSize));

    if ((listSize > 1) && (device->GetSecureSession().Value()->IsGroupSession()))
    {
        ChipLogError(Controller, "Not allow group session for InvokeRequests that has more than 1 CommandDataIB)");
        err = CHIP_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    commandSender = Platform::New<app::CommandSender>(callback, device->GetExchangeManager(), timedRequestTimeoutMs != 0);
    config.SetRemoteMaxPathsPerInvoke(device->GetSecureSession().Value()->GetRemoteSessionParameters().GetMaxPathsPerInvoke());
    SuccessOrExit(err = commandSender->SetCommandSenderConfig(config));

    for (jint i = 0; i < listSize; i++)
    {
        jmethodID getEndpointIdMethod     = nullptr;
        jmethodID getClusterIdMethod      = nullptr;
        jmethodID getCommandIdMethod      = nullptr;
        jmethodID getGroupIdMethod        = nullptr;
        jmethodID getTlvByteArrayMethod   = nullptr;
        jmethodID getJsonStringMethod     = nullptr;
        jmethodID isEndpointIdValidMethod = nullptr;
        jmethodID isGroupIdValidMethod    = nullptr;
        jlong endpointIdObj               = 0;
        jlong clusterIdObj                = 0;
        jlong commandIdObj                = 0;
        jobject groupIdObj                = nullptr;
        jbyteArray tlvBytesObj            = nullptr;
        jobject invokeElement             = nullptr;
        SuccessOrExit(err = JniReferences::GetInstance().GetListItem(invokeElementList, i, invokeElement));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getEndpointId", "(J)J", &getEndpointIdMethod));
        SuccessOrExit(err =
                          JniReferences::GetInstance().FindMethod(env, invokeElement, "getClusterId", "(J)J", &getClusterIdMethod));
        SuccessOrExit(err =
                          JniReferences::GetInstance().FindMethod(env, invokeElement, "getCommandId", "(J)J", &getCommandIdMethod));
        SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getGroupId", "()Ljava/util/Optional;",
                                                                    &getGroupIdMethod));
        SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "isEndpointIdValid", "()Z",
                                                                    &isEndpointIdValidMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, invokeElement, "isGroupIdValid", "()Z", &isGroupIdValidMethod));
        SuccessOrExit(
            err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getTlvByteArray", "()[B", &getTlvByteArrayMethod));

        isEndpointIdValid = (env->CallBooleanMethod(invokeElement, isEndpointIdValidMethod) == JNI_TRUE);
        isGroupIdValid    = (env->CallBooleanMethod(invokeElement, isGroupIdValidMethod) == JNI_TRUE);

        if (isEndpointIdValid)
        {
            endpointIdObj = env->CallLongMethod(invokeElement, getEndpointIdMethod, static_cast<jlong>(kInvalidEndpointId));
            VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
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

        clusterIdObj = env->CallLongMethod(invokeElement, getClusterIdMethod, static_cast<jlong>(kInvalidClusterId));
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

        commandIdObj = env->CallLongMethod(invokeElement, getCommandIdMethod, static_cast<jlong>(kInvalidCommandId));
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

        tlvBytesObj = static_cast<jbyteArray>(env->CallObjectMethod(invokeElement, getTlvByteArrayMethod));
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

        app::CommandSender::PrepareCommandParameters prepareCommandParams;
        prepareCommandParams.commandRef.SetValue(static_cast<uint16_t>(i));

        {
            uint16_t id = isEndpointIdValid ? static_cast<uint16_t>(endpointIdObj) : groupId;
            app::CommandPathFlags flag =
                isEndpointIdValid ? app::CommandPathFlags::kEndpointIdValid : app::CommandPathFlags::kGroupIdValid;
            app::CommandPathParams path(id, static_cast<ClusterId>(clusterIdObj), static_cast<CommandId>(commandIdObj), flag);

            if (tlvBytesObj != nullptr)
            {
                JniByteArray tlvBytesObjBytes(env, tlvBytesObj);
                SuccessOrExit(
                    err = PutPreencodedInvokeRequest(*commandSender, path, tlvBytesObjBytes.byteSpan(), prepareCommandParams));
            }
            else
            {
                SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getJsonString",
                                                                            "()Ljava/lang/String;", &getJsonStringMethod));
                jstring jsonJniString = static_cast<jstring>(env->CallObjectMethod(invokeElement, getJsonStringMethod));
                VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
                VerifyOrExit(jsonJniString != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
                JniUtfString jsonUtfJniString(env, jsonJniString);
                // The invoke does not support chunk, kMaxSecureSduLengthBytes should be enough for command json blob
                uint8_t tlvBytes[chip::app::kMaxSecureSduLengthBytes] = { 0 };
                MutableByteSpan tlvEncodingLocal{ tlvBytes };
                SuccessOrExit(err = JsonToTlv(std::string(jsonUtfJniString.c_str(), static_cast<size_t>(jsonUtfJniString.size())),
                                              tlvEncodingLocal));
                SuccessOrExit(err = PutPreencodedInvokeRequest(*commandSender, path, tlvEncodingLocal, prepareCommandParams));
            }
        }

        app::CommandSender::FinishCommandParameters finishCommandParams(convertedTimedRequestTimeoutMs != 0
                                                                            ? Optional<uint16_t>(convertedTimedRequestTimeoutMs)
                                                                            : Optional<uint16_t>::Missing());

        finishCommandParams.commandRef = prepareCommandParams.commandRef;
        SuccessOrExit(err = commandSender->FinishCommand(finishCommandParams));
    }
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
        app::CommandSender::ErrorData errorData;
        errorData.error = err;
        callback->OnError(nullptr, errorData);
        if (commandSender != nullptr)
        {
            Platform::Delete(commandSender);
            commandSender = nullptr;
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
            callback = nullptr;
        }
    }
    return err;
}

CHIP_ERROR shutdownSubscriptions(JNIEnv * env, jlong handle, jobject fabricIndex, jobject peerNodeId, jobject subscriptionId)
{
    chip::DeviceLayer::StackLock lock;
    if (fabricIndex == nullptr && peerNodeId == nullptr && subscriptionId == nullptr)
    {
        app::InteractionModelEngine::GetInstance()->ShutdownAllSubscriptions();
        return CHIP_NO_ERROR;
    }

    if (fabricIndex != nullptr && peerNodeId != nullptr && subscriptionId == nullptr)
    {
        jint jFabricIndex = chip::JniReferences::GetInstance().IntegerToPrimitive(fabricIndex);
        jlong jPeerNodeId = chip::JniReferences::GetInstance().LongToPrimitive(peerNodeId);
        app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(static_cast<chip::FabricIndex>(jFabricIndex),
                                                                          static_cast<chip::NodeId>(jPeerNodeId));
        return CHIP_NO_ERROR;
    }

    if (fabricIndex != nullptr && peerNodeId == nullptr && subscriptionId == nullptr)
    {
        jint jFabricIndex = chip::JniReferences::GetInstance().IntegerToPrimitive(fabricIndex);
        app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(static_cast<chip::FabricIndex>(jFabricIndex));
        return CHIP_NO_ERROR;
    }

    if (fabricIndex != nullptr && peerNodeId != nullptr && subscriptionId != nullptr)
    {
        jint jFabricIndex     = chip::JniReferences::GetInstance().IntegerToPrimitive(fabricIndex);
        jlong jPeerNodeId     = chip::JniReferences::GetInstance().LongToPrimitive(peerNodeId);
        jlong jSubscriptionId = chip::JniReferences::GetInstance().LongToPrimitive(subscriptionId);
        app::InteractionModelEngine::GetInstance()->ShutdownSubscription(
            chip::ScopedNodeId(static_cast<chip::NodeId>(jPeerNodeId), static_cast<chip::FabricIndex>(jFabricIndex)),
            static_cast<chip::SubscriptionId>(jSubscriptionId));
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR invoke(JNIEnv * env, jlong handle, jlong callbackHandle, jlong devicePtr, jobject invokeElement,
                  jint timedRequestTimeoutMs, jint imTimeoutMs)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    auto callback                           = reinterpret_cast<InvokeCallback *>(callbackHandle);
    app::CommandSender * commandSender      = nullptr;
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
    jlong endpointIdObj                     = 0;
    jlong clusterIdObj                      = 0;
    jlong commandIdObj                      = 0;
    jobject groupIdObj                      = nullptr;
    jbyteArray tlvBytesObj                  = nullptr;
    uint16_t convertedTimedRequestTimeoutMs = static_cast<uint16_t>(timedRequestTimeoutMs);
    ChipLogDetail(Controller, "IM invoke() called");

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);

    commandSender = Platform::New<app::CommandSender>(callback, device->GetExchangeManager(), timedRequestTimeoutMs != 0);

    SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getEndpointId", "(J)J", &getEndpointIdMethod));
    SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getClusterId", "(J)J", &getClusterIdMethod));
    SuccessOrExit(err = JniReferences::GetInstance().FindMethod(env, invokeElement, "getCommandId", "(J)J", &getCommandIdMethod));
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
        endpointIdObj = env->CallLongMethod(invokeElement, getEndpointIdMethod, static_cast<jlong>(kInvalidEndpointId));
        VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
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

    clusterIdObj = env->CallLongMethod(invokeElement, getClusterIdMethod, static_cast<jlong>(kInvalidClusterId));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

    commandIdObj = env->CallLongMethod(invokeElement, getCommandIdMethod, static_cast<jlong>(kInvalidCommandId));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);

    tlvBytesObj = static_cast<jbyteArray>(env->CallObjectMethod(invokeElement, getTlvByteArrayMethod));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
    {
        uint16_t id = isEndpointIdValid ? static_cast<uint16_t>(endpointIdObj) : groupId;
        app::CommandPathFlags flag =
            isEndpointIdValid ? app::CommandPathFlags::kEndpointIdValid : app::CommandPathFlags::kGroupIdValid;
        app::CommandPathParams path(id, static_cast<ClusterId>(clusterIdObj), static_cast<CommandId>(commandIdObj), flag);
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
            SuccessOrExit(err = JsonToTlv(std::string(jsonUtfJniString.c_str(), static_cast<size_t>(jsonUtfJniString.size())),
                                          tlvEncodingLocal));
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
            commandSender = nullptr;
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
            callback = nullptr;
        }
    }
    return err;
}

jlong getRemoteDeviceId(jlong devicePtr)
{
    OperationalDeviceProxy * chipDevice = reinterpret_cast<OperationalDeviceProxy *>(devicePtr);
    if (chipDevice == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return static_cast<jlong>(chip::kUndefinedNodeId);
    }

    return static_cast<jlong>(chipDevice->GetDeviceId());
}

jint getFabricIndex(jlong devicePtr)
{
    OperationalDeviceProxy * chipDevice = reinterpret_cast<OperationalDeviceProxy *>(devicePtr);
    if (chipDevice == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return static_cast<jint>(chip::kUndefinedFabricIndex);
    }

    return static_cast<jint>(chipDevice->GetPeerScopedNodeId().GetFabricIndex());
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

    for (jint i = 0; i < listSize; i++)
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
    ReturnErrorOnFailure(
        JniReferences::GetInstance().FindMethod(env, attributePath, "getEndpointId", "(J)J", &getEndpointIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, attributePath, "getClusterId", "(J)J", &getClusterIdMethod));
    ReturnErrorOnFailure(
        JniReferences::GetInstance().FindMethod(env, attributePath, "getAttributeId", "(J)J", &getAttributeIdMethod));

    jlong endpointIdObj  = env->CallLongMethod(attributePath, getEndpointIdMethod, static_cast<jlong>(kInvalidEndpointId));
    jlong clusterIdObj   = env->CallLongMethod(attributePath, getClusterIdMethod, static_cast<jlong>(kInvalidClusterId));
    jlong attributeIdObj = env->CallLongMethod(attributePath, getAttributeIdMethod, static_cast<jlong>(kInvalidAttributeId));

    outEndpointId  = static_cast<EndpointId>(endpointIdObj);
    outClusterId   = static_cast<ClusterId>(clusterIdObj);
    outAttributeId = static_cast<AttributeId>(attributeIdObj);

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

    for (jint i = 0; i < listSize; i++)
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

    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, eventPath, "getEndpointId", "(J)J", &getEndpointIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, eventPath, "getClusterId", "(J)J", &getClusterIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, eventPath, "getEventId", "(J)J", &getEventIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, eventPath, "isUrgent", "()Z", &isUrgentMethod));

    jlong endpointIdObj = env->CallLongMethod(eventPath, getEndpointIdMethod, static_cast<jlong>(kInvalidEndpointId));
    jlong clusterIdObj  = env->CallLongMethod(eventPath, getClusterIdMethod, static_cast<jlong>(kInvalidClusterId));
    jlong eventIdObj    = env->CallLongMethod(eventPath, getEventIdMethod, static_cast<jlong>(kInvalidEventId));
    jboolean isUrgent   = env->CallBooleanMethod(eventPath, isUrgentMethod);

    outEndpointId = static_cast<EndpointId>(endpointIdObj);
    outClusterId  = static_cast<ClusterId>(clusterIdObj);
    outEventId    = static_cast<EventId>(eventIdObj);
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

    for (jint i = 0; i < listSize; i++)
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

    ReturnErrorOnFailure(
        JniReferences::GetInstance().FindMethod(env, versionFilter, "getEndpointId", "(J)J", &getEndpointIdMethod));
    ReturnErrorOnFailure(JniReferences::GetInstance().FindMethod(env, versionFilter, "getClusterId", "(J)J", &getClusterIdMethod));
    ReturnErrorOnFailure(
        JniReferences::GetInstance().FindMethod(env, versionFilter, "getDataVersion", "()J", &getDataVersionMethod));

    jlong endpointIdObj = env->CallLongMethod(versionFilter, getEndpointIdMethod, static_cast<jlong>(kInvalidEndpointId));
    outEndpointId       = static_cast<EndpointId>(endpointIdObj);
    jlong clusterIdObj  = env->CallLongMethod(versionFilter, getClusterIdMethod, static_cast<jlong>(kInvalidClusterId));
    outClusterId        = static_cast<ClusterId>(clusterIdObj);

    outDataVersion = static_cast<DataVersion>(env->CallLongMethod(versionFilter, getDataVersionMethod));
    return CHIP_NO_ERROR;
}
