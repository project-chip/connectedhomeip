#include <controller/CHIPCluster.h>
#include <jni.h>
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
