#include <controller/CHIPCluster.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <platform/PlatformManager.h>

#include "AndroidCallbacks.h"

#define JNI_METHOD(RETURN, CLASS_NAME, METHOD_NAME)                                                                                \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipClusters_00024##CLASS_NAME##_##METHOD_NAME

JNI_METHOD(void, BaseChipCluster, deleteCluster)(JNIEnv * env, jobject self, jlong clusterPtr)
{
    chip::DeviceLayer::StackLock lock;
    chip::Controller::ClusterBase * cluster = reinterpret_cast<chip::Controller::ClusterBase *>(clusterPtr);
    if (cluster != nullptr)
    {
        delete cluster;
    }
}

JNI_METHOD(jobject, BaseChipCluster, getCommandTimeout)
(JNIEnv * env, jobject self, jlong clusterPtr)
{
    chip::DeviceLayer::StackLock lock;
    chip::Controller::ClusterBase * cluster = reinterpret_cast<chip::Controller::ClusterBase *>(clusterPtr);

    chip::Optional<chip::System::Clock::Timeout> timeout = cluster->GetCommandTimeout();
    if (!timeout.HasValue())
    {
        jobject emptyOptional = nullptr;
        chip::JniReferences::GetInstance().CreateOptional(nullptr, emptyOptional);
        return emptyOptional;
    }

    jobject timeoutOptional = nullptr;
    jobject timeoutBoxed    = nullptr;
    jlong timeoutPrimitive  = static_cast<jlong>(timeout.Value().count());
    chip::JniReferences::GetInstance().CreateBoxedObject("java/lang/Long", "(J)V", timeoutPrimitive, timeoutBoxed);
    chip::JniReferences::GetInstance().CreateOptional(timeoutBoxed, timeoutOptional);
    return timeoutOptional;
}

JNI_METHOD(void, BaseChipCluster, setCommandTimeout)
(JNIEnv * env, jobject self, jlong clusterPtr, jobject timeoutOptional)
{
    chip::DeviceLayer::StackLock lock;
    chip::Controller::ClusterBase * cluster = reinterpret_cast<chip::Controller::ClusterBase *>(clusterPtr);

    jobject boxedLong = nullptr;
    chip::JniReferences::GetInstance().GetOptionalValue(timeoutOptional, boxedLong);

    if (boxedLong != nullptr)
    {
        jlong timeoutMillis = chip::JniReferences::GetInstance().LongToPrimitive(boxedLong);
        cluster->SetCommandTimeout(chip::MakeOptional(chip::System::Clock::Milliseconds32(static_cast<uint64_t>(timeoutMillis))));
    }
    else
    {
        cluster->SetCommandTimeout(chip::NullOptional);
    }
}
// (long callbackHandle, long deviceControllerPtr, int endpointId, long attributeId, boolean isFabricFiltered, int imTimeoutMs);
JNI_METHOD(void, BaseChipCluster, readAttribute)
(JNIEnv * env, jobject self, jlong callbackHandle, jlong devicePtr, jint jEndpointId, jlong jClusterId, jlong jAttributeId, jboolean isFabricFiltered, jint imTimeoutMs)
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
