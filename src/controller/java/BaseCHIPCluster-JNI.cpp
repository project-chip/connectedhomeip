#include <controller/CHIPCluster.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <platform/PlatformManager.h>

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
