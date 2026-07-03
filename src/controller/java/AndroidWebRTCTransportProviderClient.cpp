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

#include "AndroidWebRTCTransportProviderClient.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/logging/CHIPLogging.h>

#include <controller/webrtc/WebRTCTransportRequestorManager.h>

namespace chip {
namespace Controller {

AndroidWebRTCTransportProviderClient::AndroidWebRTCTransportProviderClient(jobject javaCallbackObject, chip::CommandId commandId)
    : mCommandId(commandId)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    // Initialize the global reference for the Java callback object.
    CHIP_ERROR err = mJavaCallback.Init(javaCallbackObject);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to initialize JNI global reference for callback");
        return;
    }

    jclass callbackClass = env->GetObjectClass(javaCallbackObject);
    if (mCommandId == Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Id)
    {
        mOnResponseMethod = env->GetMethodID(callbackClass, "onResponse", "(ILjava/lang/Integer;Ljava/lang/Integer;)V");
    }
    else if (mCommandId == Clusters::WebRTCTransportProvider::Commands::SolicitOffer::Id)
    {
        mOnResponseMethod = env->GetMethodID(callbackClass, "onResponse", "(IZLjava/lang/Integer;Ljava/lang/Integer;)V");
    }
    mOnErrorMethod       = env->GetMethodID(callbackClass, "onError", "(I)V");
}

AndroidWebRTCTransportProviderClient::~AndroidWebRTCTransportProviderClient()
{
    // mJavaCallback is automatically released by the JniGlobalReference destructor.
}

CHIP_ERROR AndroidWebRTCTransportProviderClient::ProvideOffer(DeviceController * controller, NodeId deviceId, EndpointId endpointId, Optional<DataModel::Nullable<uint16_t>> videoStreamID, Optional<DataModel::Nullable<uint16_t>> audioStreamID, const CharSpan & offerSdp, jobject jcallback)
{
    auto * client = new AndroidWebRTCTransportProviderClient(jcallback, Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Id);
    VerifyOrReturnError(client != nullptr, CHIP_ERROR_NO_MEMORY);

    client->InitCallbacks(HandleCommandResponse, HandleCommandError, HandleCommandDone);
    client->Init(deviceId, controller->GetFabricIndex(), endpointId);

    Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Type value;

    value.webRTCSessionID = app::DataModel::NullNullable;
    value.sdp = offerSdp;
    value.streamUsage = StreamUsageEnum::kLiveView;
    value.originatingEndpointID = WebRTCTransportRequestorManager::Instance().GetEndpointId();
    value.videoStreamID = videoStreamID;
    value.audioStreamID = audioStreamID;
    value.ICEServers = NullOptional;
    value.metadataEnabled = NullOptional;
    value.SFrameConfig = NullOptional;
    value.videoStreams = NullOptional;
    value.audioStreams = NullOptional;

    size_t requiredBufferSize = offerSdp.size() + 256;

    chip::Platform::ScopedMemoryBuffer<uint8_t> payloadBuffer;
    if (!payloadBuffer.Alloc(requiredBufferSize))
    {
        client->Cleanup();
        return CHIP_ERROR_NO_MEMORY;
    }

    chip::TLV::TLVWriter writer;
    writer.Init(payloadBuffer.Get(), requiredBufferSize);
    CHIP_ERROR err = chip::app::DataModel::Encode(writer, chip::TLV::AnonymousTag(), value);
    if (err == CHIP_NO_ERROR)
    {
        err = writer.Finalize();
    }

    if (err != CHIP_NO_ERROR)
    {
        client->Cleanup();
        return err;
    }

    err = client->SendCommand(client, 
                              endpointId, 
                              Clusters::WebRTCTransportProvider::Id, 
                              Clusters::WebRTCTransportProvider::Commands::ProvideOffer::Id, 
                              payloadBuffer.Get(), 
                              writer.GetLengthWritten());

    if (err != CHIP_NO_ERROR)
    {
        client->Cleanup();
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AndroidWebRTCTransportProviderClient::SolicitOffer(DeviceController * controller, NodeId deviceId, EndpointId endpointId, jobject jcallback)
{
    auto * client = new AndroidWebRTCTransportProviderClient(jcallback, Clusters::WebRTCTransportProvider::Commands::SolicitOffer::Id);
    VerifyOrReturnError(client != nullptr, CHIP_ERROR_NO_MEMORY);

    client->InitCallbacks(HandleCommandResponse, HandleCommandError, HandleCommandDone);
    client->Init(deviceId, controller->GetFabricIndex(), endpointId);

    Clusters::WebRTCTransportProvider::Commands::SolicitOffer::Type value;

    value.streamUsage = StreamUsageEnum::kLiveView;
    value.originatingEndpointID = WebRTCTransportRequestorManager::Instance().GetEndpointId();
    value.videoStreamID = NullOptional;
    value.audioStreamID = NullOptional;
    value.ICEServers = NullOptional;
    value.metadataEnabled = NullOptional;
    value.SFrameConfig = NullOptional;
    value.videoStreams = NullOptional;
    value.audioStreams = NullOptional;

    constexpr size_t kPayloadSize = 256; 
    chip::Platform::ScopedMemoryBuffer<uint8_t> payloadBuffer;
    if (!payloadBuffer.Alloc(kPayloadSize))
    {
        client->Cleanup();
        return CHIP_ERROR_NO_MEMORY;
    }
    
    chip::TLV::TLVWriter writer;
    writer.Init(payloadBuffer.Get(), kPayloadSize);

    CHIP_ERROR err = chip::app::DataModel::Encode(writer, chip::TLV::AnonymousTag(), value);
    if (err == CHIP_NO_ERROR)
    {
        err = writer.Finalize();
    }

    if (err != CHIP_NO_ERROR)
    {
        client->Cleanup();
        return err;
    }

    err = client->SendCommand(client, 
                              endpointId, 
                              Clusters::WebRTCTransportProvider::Id, 
                              Clusters::WebRTCTransportProvider::Commands::SolicitOffer::Id, 
                              payloadBuffer.Get(), 
                              writer.GetLengthWritten());

    if (err != CHIP_NO_ERROR)
    {
        client->Cleanup();
        return err;
    }

    return CHIP_NO_ERROR;
}

void AndroidWebRTCTransportProviderClient::HandleCommandResponse(void * appContext, chip::EndpointId endpointId, chip::ClusterId clusterId,
                                                                 chip::CommandId commandId, size_t index,
                                                                 chip::Protocols::InteractionModel::Status status,
                                                                 chip::ClusterStatus clusterStatus, const uint8_t * payload, uint32_t length)
{
    auto * self = static_cast<AndroidWebRTCTransportProviderClient *>(appContext);
    VerifyOrReturn(self != nullptr);

    if (payload == nullptr || length == 0)
    {
        self->NotifyError(CHIP_ERROR_INVALID_ARGUMENT);
        return;
    }

    chip::TLV::TLVReader reader;
    reader.Init(payload, length);
    ReturnOnFailure(reader.Next(), ChipLogError(Controller, "Wrong TLV Payload."));

    if (commandId == Clusters::WebRTCTransportProvider::Commands::ProvideOfferResponse::Id)
    {
        Clusters::WebRTCTransportProvider::Commands::ProvideOfferResponse::DecodableType response;
        CHIP_ERROR err = chip::app::DataModel::Decode(reader, response);
        if (err == CHIP_NO_ERROR)
        {
            self->NotifyProvideOfferSuccess(response.webRTCSessionID, response.videoStreamID, response.audioStreamID);
        }
        else
        {
            ChipLogError(Controller, "ProvideOfferResponse : %" CHIP_ERROR_FORMAT, err.Format());
            self->NotifyError(err);
        }
    }
    else if (commandId == Clusters::WebRTCTransportProvider::Commands::SolicitOfferResponse::Id)
    {
        Clusters::WebRTCTransportProvider::Commands::SolicitOfferResponse::DecodableType response;
        CHIP_ERROR err = chip::app::DataModel::Decode(reader, response);
        if (err == CHIP_NO_ERROR)
        {
            self->NotifySolicitOfferSuccess(response.webRTCSessionID, response.deferredOffer, response.videoStreamID, response.audioStreamID);
        }
        else
        {
            ChipLogError(Controller, "SolicitOfferResponse : %" CHIP_ERROR_FORMAT, err.Format());
            self->NotifyError(err);
        }
    }
    else
    {
        self->NotifyError(CHIP_ERROR_INVALID_MESSAGE_TYPE);
    }
}

void AndroidWebRTCTransportProviderClient::HandleCommandError(void * appContext, chip::Protocols::InteractionModel::Status status,
                                                              chip::ClusterStatus clusterStatus, CHIP_ERROR error)
{
    auto * self = static_cast<AndroidWebRTCTransportProviderClient *>(appContext);
    if (self != nullptr)
    {
        self->NotifyError(error);
    }
}

void AndroidWebRTCTransportProviderClient::HandleCommandDone(void * appContext)
{
    auto * self = static_cast<AndroidWebRTCTransportProviderClient *>(appContext);
    if (self != nullptr)
    {
        self->Cleanup();
    }
}

static jobject CreateJavaInteger(JNIEnv * env, Optional<DataModel::Nullable<uint16_t>> value)
{
    if (!value.HasValue() || !value.Value().IsNull())
    {
        return nullptr;
    }
    jclass integerClass = env->FindClass("java/lang/Integer");
    jmethodID valueOfMethod = env->GetStaticMethodID(integerClass, "valueOf", "(I)Ljava/lang/Integer;");
    jobject integerObj = env->CallStaticObjectMethod(integerClass, valueOfMethod, static_cast<jint>(value.Value().Value()));
    env->DeleteLocalRef(integerClass);
    return integerObj;
}

void AndroidWebRTCTransportProviderClient::NotifyProvideOfferSuccess(uint16_t webRTCSessionID, Optional<DataModel::Nullable<uint16_t>> videoStreamID, Optional<DataModel::Nullable<uint16_t>> audioStreamID)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    if (mJavaCallback.HasValidObjectRef() && mOnResponseMethod != nullptr)
    {
        jobject jVideoStreamID = CreateJavaInteger(env, videoStreamID);
        jobject jAudioStreamID = CreateJavaInteger(env, audioStreamID);

        env->CallVoidMethod(mJavaCallback.ObjectRef(), mOnResponseMethod, 
                            static_cast<jint>(webRTCSessionID), jVideoStreamID, jAudioStreamID);

        if (jVideoStreamID != nullptr) env->DeleteLocalRef(jVideoStreamID);
        if (jAudioStreamID != nullptr) env->DeleteLocalRef(jAudioStreamID);
    }
}

void AndroidWebRTCTransportProviderClient::NotifySolicitOfferSuccess(uint16_t webRTCSessionID, bool deferredOffer, Optional<DataModel::Nullable<uint16_t>> videoStreamID, Optional<DataModel::Nullable<uint16_t>> audioStreamID)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    if (mJavaCallback.HasValidObjectRef() && mOnResponseMethod != nullptr)
    {
        jobject jVideoStreamID = CreateJavaInteger(env, videoStreamID);
        jobject jAudioStreamID = CreateJavaInteger(env, audioStreamID);

        env->CallVoidMethod(mJavaCallback.ObjectRef(), mOnResponseMethod, 
                            static_cast<jint>(webRTCSessionID), static_cast<jboolean>(deferredOffer), jVideoStreamID, jAudioStreamID);

        if (jVideoStreamID != nullptr) env->DeleteLocalRef(jVideoStreamID);
        if (jAudioStreamID != nullptr) env->DeleteLocalRef(jAudioStreamID);
    }
}

void AndroidWebRTCTransportProviderClient::NotifyError(CHIP_ERROR error)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));

    if (mJavaCallback.HasValidObjectRef() && mOnErrorMethod != nullptr)
    {
        // Passes the CHIP_ERROR integer value to the Java onError method.
        env->CallVoidMethod(mJavaCallback.ObjectRef(), mOnErrorMethod, static_cast<jint>(error.AsInteger()));
    }
}

void AndroidWebRTCTransportProviderClient::Cleanup()
{
    // Deletes the current instance to prevent memory leaks after the async operation finishes.
    delete this;
}

} // namespace Controller
} // namespace chip