/*
 *   Copyright (c) 2023 Project CHIP Authors
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

#include "CastingPlayerDiscovery-JNI.h"

#include "../support/Converters-JNI.h"
#include "../support/RotatingDeviceIdUniqueIdProvider-JNI.h"
#include "core/CastingApp.h"             // from tv-casting-common
#include "core/CastingPlayerDiscovery.h" // from tv-casting-common

#include <app/clusters/bindings/BindingManager.h>
#include <app/server/Server.h>
#include <jni.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

#define JNI_METHOD(RETURN, METHOD_NAME)                                                                                            \
    extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_casting_core_MatterCastingPlayerDiscovery_##METHOD_NAME

namespace matter {
namespace casting {
namespace core {

/**
 * @brief React to CastingPlayer discovery results with this singleton
 */
class DiscoveryDelegateImpl : public DiscoveryDelegate
{
private:
    DiscoveryDelegateImpl() {}
    static DiscoveryDelegateImpl * discoveryDelegateImplSingletonInstance;
    DiscoveryDelegateImpl(DiscoveryDelegateImpl & other) = delete;
    void operator=(const DiscoveryDelegateImpl &)        = delete;

public:
    JniGlobalReference castingPlayerChangeListenerJavaObject;
    jmethodID onAddedCallbackJavaMethodID   = nullptr;
    jmethodID onChangedCallbackJavaMethodID = nullptr;
    // jmethodID onRemovedCallbackJavaMethodID = nullptr;

    static DiscoveryDelegateImpl * GetInstance()
    {
        if (DiscoveryDelegateImpl::discoveryDelegateImplSingletonInstance == nullptr)
        {
            DiscoveryDelegateImpl::discoveryDelegateImplSingletonInstance = new DiscoveryDelegateImpl();
        }
        return DiscoveryDelegateImpl::discoveryDelegateImplSingletonInstance;
    }

    void HandleOnAdded(matter::casting::memory::Strong<CastingPlayer> player) override
    {
        ChipLogProgress(AppServer,
                        "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnAdded() called with CastingPlayer, ID: %s",
                        player->GetId());

        VerifyOrReturn(castingPlayerChangeListenerJavaObject.HasValidObjectRef(),
                       ChipLogError(AppServer,
                                    "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnAdded() Not set, "
                                    "CastingPlayerChangeListener == nullptr"));
        VerifyOrReturn(onAddedCallbackJavaMethodID != nullptr,
                       ChipLogError(AppServer,
                                    "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnAdded() Not set, "
                                    "onAddedCallbackJavaMethodID == nullptr"));

        jobject matterCastingPlayerJavaObject = support::convertCastingPlayerFromCppToJava(player);
        VerifyOrReturn(matterCastingPlayerJavaObject != nullptr,
                       ChipLogError(AppServer,
                                    "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnAdded() Could not create "
                                    "CastingPlayer jobject"));

        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        chip::DeviceLayer::StackUnlock unlock;
        env->CallVoidMethod(castingPlayerChangeListenerJavaObject.ObjectRef(), onAddedCallbackJavaMethodID,
                            matterCastingPlayerJavaObject);
    }

    void HandleOnUpdated(matter::casting::memory::Strong<CastingPlayer> player) override
    {
        ChipLogProgress(AppServer,
                        "CastingPlayerDiscovery-JNI DiscoveryDelegateImpl::HandleOnUpdated() called with CastingPlayer, ID: %s",
                        player->GetId());

        VerifyOrReturn(castingPlayerChangeListenerJavaObject.HasValidObjectRef(),
                       ChipLogError(AppServer,
                                    "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnUpdated() Not set, "
                                    "CastingPlayerChangeListener == nullptr"));
        VerifyOrReturn(onChangedCallbackJavaMethodID != nullptr,
                       ChipLogError(AppServer,
                                    "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnUpdated() Not set, "
                                    "onChangedCallbackJavaMethodID == nullptr"));

        jobject matterCastingPlayerJavaObject = support::convertCastingPlayerFromCppToJava(player);
        VerifyOrReturn(matterCastingPlayerJavaObject != nullptr,
                       ChipLogError(AppServer,
                                    "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnUpdated() Could not "
                                    "create CastingPlayer jobject"));

        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        chip::DeviceLayer::StackUnlock unlock;
        env->CallVoidMethod(castingPlayerChangeListenerJavaObject.ObjectRef(), onChangedCallbackJavaMethodID,
                            matterCastingPlayerJavaObject);
    }

    // TODO: In following PRs. Implement HandleOnRemoved after implemented in tv-casting-commom CastingPlayerDiscovery.h/cpp
    // void HandleOnRemoved(matter::casting::memory::Strong<CastingPlayer> player) override
    // {
    //     ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI DiscoveryDelegateImpl::HandleOnRemoved() called with
    //     CastingPlayer, ID: %s", player->GetId());
    // }
};

// Initialize the static instance to nullptr
DiscoveryDelegateImpl * DiscoveryDelegateImpl::discoveryDelegateImplSingletonInstance = nullptr;

JNI_METHOD(jobject, startDiscovery)(JNIEnv * env, jobject, jobject targetDeviceTypeLong = nullptr)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::startDiscovery() called");
    CastingPlayerDiscovery::GetInstance()->SetDelegate(DiscoveryDelegateImpl::GetInstance());

    // Start CastingPlayer discovery
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (targetDeviceTypeLong == nullptr)
    {
        ChipLogProgress(AppServer,
                        "CastingPlayerDiscovery-JNI::startDiscovery() received null target device type. Using default type.");
        err = CastingPlayerDiscovery::GetInstance()->StartDiscovery();
    }
    else
    {
        // Get the long value from the Java Long object
        jclass longClass          = env->GetObjectClass(targetDeviceTypeLong);
        jmethodID longValueMethod = env->GetMethodID(longClass, "longValue", "()J");
        jlong jTargetDeviceType   = env->CallLongMethod(targetDeviceTypeLong, longValueMethod);
        env->DeleteLocalRef(longClass);

        ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::startDiscovery() discovery target device type: %u",
                        static_cast<uint32_t>(jTargetDeviceType));
        err = CastingPlayerDiscovery::GetInstance()->StartDiscovery(static_cast<uint32_t>(jTargetDeviceType));
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CastingPlayerDiscovery-JNI startDiscovery() err: %" CHIP_ERROR_FORMAT, err.Format());
        return support::convertMatterErrorFromCppToJava(err);
    }

    return support::convertMatterErrorFromCppToJava(CHIP_NO_ERROR);
}

JNI_METHOD(jobject, stopDiscovery)(JNIEnv * env, jobject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::stopDiscovery() called");

    // Stop CastingPlayer discovery
    CHIP_ERROR err = CastingPlayerDiscovery::GetInstance()->StopDiscovery();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CastingPlayerDiscovery-JNI::StopDiscovery() err: %" CHIP_ERROR_FORMAT, err.Format());
        return support::convertMatterErrorFromCppToJava(err);
    }

    return support::convertMatterErrorFromCppToJava(CHIP_NO_ERROR);
}

JNI_METHOD(jobject, addCastingPlayerChangeListener)(JNIEnv * env, jobject, jobject castingPlayerChangeListenerJavaObject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::addCastingPlayerChangeListener() called");
    VerifyOrReturnValue(castingPlayerChangeListenerJavaObject != nullptr,
                        support::convertMatterErrorFromCppToJava(CHIP_ERROR_INCORRECT_STATE));

    if (DiscoveryDelegateImpl::GetInstance()->castingPlayerChangeListenerJavaObject.HasValidObjectRef())
    {
        ChipLogError(AppServer,
                     "CastingPlayerDiscovery-JNI::addCastingPlayerChangeListener() Call removeCastingPlayerChangeListener "
                     "before adding a new one");
        return support::convertMatterErrorFromCppToJava(CHIP_ERROR_INCORRECT_STATE);
    }

    // Get the class and method IDs for the CastingPlayerChangeListener Java class
    jclass castingPlayerChangeListenerJavaClass = env->GetObjectClass(castingPlayerChangeListenerJavaObject);
    VerifyOrReturnValue(castingPlayerChangeListenerJavaClass != nullptr,
                        support::convertMatterErrorFromCppToJava(CHIP_ERROR_INCORRECT_STATE));

    jmethodID onAddedJavaMethodID =
        env->GetMethodID(castingPlayerChangeListenerJavaClass, "_onAdded", "(Lcom/matter/casting/core/CastingPlayer;)V");
    VerifyOrReturnValue(onAddedJavaMethodID != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INCORRECT_STATE));
    jmethodID onChangedJavaMethodID =
        env->GetMethodID(castingPlayerChangeListenerJavaClass, "_onChanged", "(Lcom/matter/casting/core/CastingPlayer;)V");
    VerifyOrReturnValue(onChangedJavaMethodID != nullptr, support::convertMatterErrorFromCppToJava(CHIP_ERROR_INCORRECT_STATE));
    // jmethodID onRemovedJavaMethodID = env->GetMethodID(castingPlayerChangeListenerJavaClass, "_onRemoved",
    // "(Lcom/matter/casting/core/CastingPlayer;)V"); VerifyOrReturnValue(onRemovedJavaMethodID != nullptr,
    // support::convertMatterErrorFromCppToJava(CHIP_ERROR_INCORRECT_STATE));

    // Set Java callbacks in the DiscoveryDelegateImpl Singleton
    CHIP_ERROR err =
        DiscoveryDelegateImpl::GetInstance()->castingPlayerChangeListenerJavaObject.Init(castingPlayerChangeListenerJavaObject);
    if (err != CHIP_NO_ERROR)
    {
        return support::convertMatterErrorFromCppToJava(err);
    }

    DiscoveryDelegateImpl::GetInstance()->onAddedCallbackJavaMethodID   = onAddedJavaMethodID;
    DiscoveryDelegateImpl::GetInstance()->onChangedCallbackJavaMethodID = onChangedJavaMethodID;
    // DiscoveryDelegateImpl::GetInstance()->onRemovedCallbackJavaMethodID = onRemovedJavaMethodID;

    return support::convertMatterErrorFromCppToJava(CHIP_NO_ERROR);
}

JNI_METHOD(jobject, removeCastingPlayerChangeListener)(JNIEnv * env, jobject, jobject castingPlayerChangeListenerJavaObject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::removeCastingPlayerChangeListener() called");

    // Check if the passed object is the same as the one added in addCastingPlayerChangeListener() JNI method
    jboolean isSameObject =
        env->IsSameObject(castingPlayerChangeListenerJavaObject,
                          DiscoveryDelegateImpl::GetInstance()->castingPlayerChangeListenerJavaObject.ObjectRef());

    if ((bool) isSameObject)
    {
        DiscoveryDelegateImpl::GetInstance()->castingPlayerChangeListenerJavaObject.Reset();
        // No explicit cleanup required
        DiscoveryDelegateImpl::GetInstance()->onAddedCallbackJavaMethodID   = nullptr;
        DiscoveryDelegateImpl::GetInstance()->onChangedCallbackJavaMethodID = nullptr;
        // DiscoveryDelegateImpl::GetInstance()->onRemovedCallbackJavaMethodID = nullptr;

        return support::convertMatterErrorFromCppToJava(CHIP_NO_ERROR);
    }
    else if (!DiscoveryDelegateImpl::GetInstance()->castingPlayerChangeListenerJavaObject.HasValidObjectRef())
    {
        ChipLogError(
            AppServer,
            "CastingPlayerDiscovery-JNI::removeCastingPlayerChangeListener() Cannot remove listener. No listener was added");
        return support::convertMatterErrorFromCppToJava(CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(AppServer,
                     "CastingPlayerDiscovery-JNI::removeCastingPlayerChangeListener() Cannot remove listener. Received a different "
                     "CastingPlayerChangeListener object");
        return support::convertMatterErrorFromCppToJava(CHIP_ERROR_INCORRECT_STATE);
    }
}

JNI_METHOD(jobject, getCastingPlayers)(JNIEnv * env, jobject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::getCastingPlayers() called");

    // Create a new ArrayList
    jclass arrayListClass          = env->FindClass("java/util/ArrayList");
    jmethodID arrayListConstructor = env->GetMethodID(arrayListClass, "<init>", "()V");
    jobject arrayList              = env->NewObject(arrayListClass, arrayListConstructor);
    jmethodID addMethod            = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");

    std::vector<memory::Strong<CastingPlayer>> castingPlayersList = CastingPlayerDiscovery::GetInstance()->GetCastingPlayers();

    for (const auto & player : castingPlayersList)
    {
        jobject matterCastingPlayerJavaObject = support::convertCastingPlayerFromCppToJava(player);
        if (matterCastingPlayerJavaObject != nullptr)
        {
            jboolean added = env->CallBooleanMethod(arrayList, addMethod, matterCastingPlayerJavaObject);
            if (!((bool) added))
            {
                ChipLogError(AppServer, "CastingPlayerDiscovery-JNI::getCastingPlayers() Unable to add CastingPlayer with ID: %s",
                             player->GetId());
            }
        }
    }

    return arrayList;
}

}; // namespace core
}; // namespace casting
}; // namespace matter
