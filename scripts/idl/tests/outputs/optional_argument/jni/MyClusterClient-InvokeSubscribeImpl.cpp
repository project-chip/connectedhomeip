#include <controller/java/zap-generated/CHIPCallbackTypes.h>
#include <controller/java/zap-generated/CHIPInvokeCallbacks.h>
#include <controller/java/zap-generated/CHIPReadCallbacks.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <zap-generated/CHIPClientCallbacks.h>
#include <zap-generated/CHIPClusters.h>

#include <controller/java/AndroidCallbacks.h>
#include <controller/java/AndroidClusterExceptions.h>
#include <controller/java/CHIPDefaultCallbacks.h>
#include <jni.h>
#include <lib/support/CHIPListUtils.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/Span.h>
#include <platform/PlatformManager.h>
#include <vector>

#define JNI_METHOD(RETURN, CLASS_NAME, METHOD_NAME)                                                                                \
    extern "C" JNIEXPORT RETURN JNICALL Java_chip_devicecontroller_ChipClusters_00024##CLASS_NAME##_##METHOD_NAME

using namespace chip;
using namespace chip::Controller;

JNI_METHOD(jlong, MyClusterCluster, initWithDevice)(JNIEnv * env, jobject self, jlong devicePtr, jint endpointId)
{
    chip::DeviceLayer::StackLock lock;
    MyClusterCluster * cppCluster = new MyClusterCluster();

    cppCluster->Associate(reinterpret_cast<DeviceProxy *>(devicePtr), endpointId);
    return reinterpret_cast<jlong>(cppCluster);
}

JNI_METHOD(void, MyClusterCluster, 
  foo)(JNIEnv * env, jobject self, jlong clusterPtr, jobject callback,jobject argument,jobject timedInvokeTimeoutMs)
{
    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = CHIP_NO_ERROR;
    MyClusterCluster * cppCluster;
    
    ListFreer listFreer;
    chip::app::Clusters::MyCluster::Commands::Foo::Type request;

    std::vector<Platform::UniquePtr<JniByteArray>> cleanupByteArrays;
    std::vector<Platform::UniquePtr<JniUtfString>> cleanupStrings;{
    jobject optionalValue_0 = nullptr;
    chip::JniReferences::GetInstance().GetOptionalValue(argument, optionalValue_0);
    if (optionalValue_0) {
      auto & definedValue_0 = request.argument.Emplace();
      cleanupByteArrays.push_back(chip::Platform::MakeUnique<chip::JniByteArray>(env, static_cast<jbyteArray>(optionalValue_0)));
        definedValue_0 = cleanupByteArrays.back()->byteSpan();
    }
  }

  
    std::unique_ptr<CHIPDefaultSuccessCallback, void (*)(CHIPDefaultSuccessCallback *)> onSuccess(
        Platform::New<CHIPDefaultSuccessCallback>(callback), Platform::Delete<CHIPDefaultSuccessCallback>);
    std::unique_ptr<CHIPDefaultFailureCallback, void (*)(CHIPDefaultFailureCallback *)> onFailure(Platform::New<CHIPDefaultFailureCallback>(callback), Platform::Delete<CHIPDefaultFailureCallback>);
    VerifyOrReturn(onSuccess.get() != nullptr, AndroidClusterExceptions::GetInstance().ReturnIllegalStateException(env, callback, "Error creating native callback", CHIP_ERROR_NO_MEMORY));
    VerifyOrReturn(onFailure.get() != nullptr, AndroidClusterExceptions::GetInstance().ReturnIllegalStateException(env, callback, "Error creating native callback", CHIP_ERROR_NO_MEMORY));

    cppCluster = reinterpret_cast<MyClusterCluster *>(clusterPtr);
    VerifyOrReturn(cppCluster != nullptr, AndroidClusterExceptions::GetInstance().ReturnIllegalStateException(env, callback, "Error getting native cluster", CHIP_ERROR_INCORRECT_STATE));

    auto successFn = chip::Callback::Callback<CHIPDefaultSuccessCallbackType>::FromCancelable(onSuccess->Cancel());
    auto failureFn = chip::Callback::Callback<CHIPDefaultFailureCallbackType>::FromCancelable(onFailure->Cancel());

  if (timedInvokeTimeoutMs == nullptr) {
        err = cppCluster->InvokeCommand(request, onSuccess->mContext, successFn->mCall, failureFn->mCall);
    } else {
        err = cppCluster->InvokeCommand(request, onSuccess->mContext, successFn->mCall, failureFn->mCall, chip::JniReferences::GetInstance().IntegerToPrimitive(timedInvokeTimeoutMs));
    }
    VerifyOrReturn(err == CHIP_NO_ERROR, AndroidClusterExceptions::GetInstance().ReturnIllegalStateException(env, callback, "Error invoking command", CHIP_ERROR_INCORRECT_STATE));

    onSuccess.release();
    onFailure.release();
}
