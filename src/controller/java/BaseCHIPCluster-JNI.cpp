#include <controller/CHIPCluster.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <platform/PlatformManager.h>

#include "AndroidCallbacks.h"

#define JNI_METHOD(RETURN, CLASS_NAME, METHOD_NAME)                                                                                \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipClusters_00024##CLASS_NAME##_##METHOD_NAME

// JNI_METHOD(void, BaseChipCluster, deleteCluster)(JNIEnv * env, jobject self, jlong clusterPtr)
// {
//     chip::DeviceLayer::StackLock lock;
//     chip::Controller::ClusterBase * cluster = reinterpret_cast<chip::Controller::ClusterBase *>(clusterPtr);
//     if (cluster != nullptr)
//     {
//         delete cluster;
//     }
// }

// JNI_METHOD(jobject, BaseChipCluster, getCommandTimeout)
// (JNIEnv * env, jobject self, jlong clusterPtr)
// {
//     chip::DeviceLayer::StackLock lock;
//     chip::Controller::ClusterBase * cluster = reinterpret_cast<chip::Controller::ClusterBase *>(clusterPtr);

//     chip::Optional<chip::System::Clock::Timeout> timeout = cluster->GetCommandTimeout();
//     if (!timeout.HasValue())
//     {
//         jobject emptyOptional = nullptr;
//         chip::JniReferences::GetInstance().CreateOptional(nullptr, emptyOptional);
//         return emptyOptional;
//     }

//     jobject timeoutOptional = nullptr;
//     jobject timeoutBoxed    = nullptr;
//     jlong timeoutPrimitive  = static_cast<jlong>(timeout.Value().count());
//     chip::JniReferences::GetInstance().CreateBoxedObject("java/lang/Long", "(J)V", timeoutPrimitive, timeoutBoxed);
//     chip::JniReferences::GetInstance().CreateOptional(timeoutBoxed, timeoutOptional);
//     return timeoutOptional;
// }

// JNI_METHOD(void, BaseChipCluster, setCommandTimeout)
// (JNIEnv * env, jobject self, jlong clusterPtr, jobject timeoutOptional)
// {
//     chip::DeviceLayer::StackLock lock;
//     chip::Controller::ClusterBase * cluster = reinterpret_cast<chip::Controller::ClusterBase *>(clusterPtr);

//     jobject boxedLong = nullptr;
//     chip::JniReferences::GetInstance().GetOptionalValue(timeoutOptional, boxedLong);

//     if (boxedLong != nullptr)
//     {
//         jlong timeoutMillis = chip::JniReferences::GetInstance().LongToPrimitive(boxedLong);
//         cluster->SetCommandTimeout(chip::MakeOptional(chip::System::Clock::Milliseconds32(static_cast<uint64_t>(timeoutMillis))));
//     }
//     else
//     {
//         cluster->SetCommandTimeout(chip::NullOptional);
//     }
// }

JNI_METHOD(void, BaseChipCluster, readAttribute)
(JNIEnv * env, jobject self, jlong callbackHandle, jlong devicePtr, jint jEndpointId, jlong jClusterId, jlong jAttributeId, jboolean isFabricFiltered, jlong imTimeoutMs)
{
    using namespace chip;
    using namespace chip::Controller;

    ChipLogError(Controller, "BaseChipCluster:readAttribute");
    DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;

    auto callback = reinterpret_cast<ReportCallback *>(callbackHandle);
    std::vector<app::AttributePathParams> attributePathParamsList;
    app::ReadClient * readClient = nullptr;
    DeviceProxy * device         = reinterpret_cast<DeviceProxy *>(devicePtr);
    if (device == nullptr)
    {
        ChipLogProgress(Controller, "Could not cast device pointer to Device object");
        return;
    }
    chip::app::ReadPrepareParams params(device->GetSecureSession().Value());

    EndpointId endpointId = static_cast<EndpointId>(jEndpointId);
    ClusterId clusterId = static_cast<ClusterId>(jClusterId);
    AttributeId attributeId = static_cast<AttributeId>(jAttributeId);
    attributePathParamsList.push_back(app::AttributePathParams(endpointId, clusterId, attributeId));

    params.mpAttributePathParamsList    = attributePathParamsList.data();
    params.mAttributePathParamsListSize = 1;

    params.mIsFabricFiltered = (isFabricFiltered != JNI_FALSE);
    params.mTimeout          = imTimeoutMs != 0 ? System::Clock::Milliseconds32(imTimeoutMs) : System::Clock::kZero;

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
        }
        if (callback != nullptr)
        {
            Platform::Delete(callback);
        }
    }
}

JNI_METHOD(void, BaseChipCluster, writeAttribute)
(JNIEnv * env, jobject self, jlong callbackHandle, jlong devicePtr, jint jEndpointId, jlong jClusterId, jlong jAttributeId, jbyteArray tlvBytes, jint timedRequestTimeoutMs, jlong imTimeoutMs)
{
    using namespace chip;
    using namespace chip::Controller;

    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    auto callback                           = reinterpret_cast<WriteAttributesCallback *>(callbackHandle);
    app::WriteClient * writeClient          = nullptr;
    uint16_t convertedTimedRequestTimeoutMs = static_cast<uint16_t>(timedRequestTimeoutMs);

    ChipLogDetail(Controller, "IM write() called");

    if (tlvBytes == nullptr) {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(Controller, "JNI IM Write Error: %s", err.AsString());
        return;
    }

    EndpointId endpointId = 0;
    ClusterId clusterId = static_cast<ClusterId>(jClusterId);
    AttributeId attributeId = static_cast<AttributeId>(jAttributeId);
    bool isGroupSession = false;
    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    TLV::TLVReader reader;

    isGroupSession = device->GetSecureSession().Value()->IsGroupSession();

    if (isGroupSession)
    {
        endpointId = kInvalidEndpointId;
    }
    else
    {
        endpointId = static_cast<EndpointId>(jEndpointId);
    }

    app::ConcreteDataAttributePath path(static_cast<EndpointId>(endpointId), static_cast<ClusterId>(clusterId),
                                        static_cast<AttributeId>(attributeId), Optional<DataVersion>());

    writeClient = Platform::New<app::WriteClient>(
        device->GetExchangeManager(), callback->GetChunkedWriteCallback(),
        convertedTimedRequestTimeoutMs != 0 ? Optional<uint16_t>(convertedTimedRequestTimeoutMs) : Optional<uint16_t>::Missing());

    JniByteArray tlvByteArray(env, tlvBytes);

    VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);
    VerifyOrExit(writeClient != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    reader.Init(tlvByteArray.byteSpan());
    SuccessOrExit(err = reader.Next());
    SuccessOrExit(err = writeClient->PutPreencodedAttribute(path, reader));

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

JNI_METHOD(void, BaseChipCluster, subscribeAttribute)
(JNIEnv * env, jobject self, jlong callbackHandle, jlong devicePtr, jint jEndpointId, jlong jClusterId, jlong jAttributeId, jint minInterval, jint maxInterval, jboolean keepSubscriptions, jboolean isFabricFiltered, jlong imTimeoutMs)
{
    using namespace chip;
    using namespace chip::Controller;

    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err               = CHIP_NO_ERROR;
    app::ReadClient * readClient = nullptr;
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

    EndpointId endpointId = static_cast<EndpointId>(jEndpointId);
    ClusterId clusterId = static_cast<ClusterId>(jClusterId);
    AttributeId attributeId = static_cast<AttributeId>(jAttributeId);

    std::unique_ptr<chip::app::AttributePathParams[]> attributePaths(new chip::app::AttributePathParams[1]);
    attributePaths[0] = chip::app::AttributePathParams(endpointId, clusterId, attributeId);

    params.mpAttributePathParamsList    = attributePaths.get();
    params.mAttributePathParamsListSize = 1;

    readClient = Platform::New<app::ReadClient>(app::InteractionModelEngine::GetInstance(), device->GetExchangeManager(),
                                                callback->mClusterCacheAdapter.GetBufferedCallback(),
                                                app::ReadClient::InteractionType::Subscribe);

    attributePaths.release();

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

JNI_METHOD(void, BaseChipCluster, invoke)
(JNIEnv * env, jobject self, jlong callbackHandle, jlong devicePtr, jint jEndpointId, jlong jClusterId, jlong jCommandId, jbyteArray tlvBytes, jint timedRequestTimeoutMs, jlong imTimeoutMs)
{
    using namespace chip;
    using namespace chip::Controller;

    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    auto callback                           = reinterpret_cast<InvokeCallback *>(callbackHandle);
    app::CommandSender * commandSender      = nullptr;
    EndpointId endpointId = static_cast<EndpointId>(jEndpointId);
    ClusterId clusterId = static_cast<ClusterId>(jClusterId);
    CommandId commandId = static_cast<CommandId>(jCommandId);
    uint16_t convertedTimedRequestTimeoutMs = static_cast<uint16_t>(timedRequestTimeoutMs);
    ChipLogDetail(Controller, "IM invoke() called");

    DeviceProxy * device = reinterpret_cast<DeviceProxy *>(devicePtr);
    VerifyOrExit(device != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(device->GetSecureSession().HasValue(), err = CHIP_ERROR_MISSING_SECURE_SESSION);

    commandSender = Platform::New<app::CommandSender>(callback, device->GetExchangeManager(), convertedTimedRequestTimeoutMs != 0);
    {
        app::CommandPathParams path(endpointId, clusterId, commandId, app::CommandPathFlags::kEndpointIdValid);
        JniByteArray tlvBytesObjBytes(env, tlvBytes);
        TLV::TLVWriter * writer = nullptr;
        TLV::TLVReader reader;

        SuccessOrExit(err = commandSender->PrepareCommand(path, false /* aStartDataStruct */));

        writer = commandSender->GetCommandDataIBTLVWriter();
        VerifyOrExit(writer != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        reader.Init(tlvBytesObjBytes.byteSpan());
        SuccessOrExit(err = reader.Next());
        SuccessOrExit(err = writer->CopyContainer(TLV::ContextTag(app::CommandDataIB::Tag::kFields), reader));
    }
    SuccessOrExit(err = commandSender->FinishCommand(convertedTimedRequestTimeoutMs != 0
                                                         ? Optional<uint16_t>(convertedTimedRequestTimeoutMs)
                                                         : Optional<uint16_t>::Missing()));

    SuccessOrExit(err = commandSender->SendCommandRequest(device->GetSecureSession().Value(), imTimeoutMs != 0 ? MakeOptional(System::Clock::Milliseconds32(imTimeoutMs)) : Optional<System::Clock::Timeout>::Missing()));

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

JNI_METHOD(jbyteArray, BaseChipCluster, encodeToTlv)(JNIEnv * env, jclass clazz, jobject value)
{
    return nullptr;
}

JNI_METHOD(jobject, BaseChipCluster, decodeFromTlv)(JNIEnv * env, jclass clazz, jbyteArray tlv)
{
    return nullptr;
}