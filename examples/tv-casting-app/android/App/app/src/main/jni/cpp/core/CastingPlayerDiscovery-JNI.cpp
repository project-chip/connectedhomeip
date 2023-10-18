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

#include "../JNIDACProvider.h"
#include "../support/CastingPlayerConverter-JNI.h"
#include "../support/ErrorConverter-JNI.h"
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
 * @brief React to CastingPlayer discovery results with this sigleton
 */
class DiscoveryDelegateImpl : public DiscoveryDelegate
{
private:
    DiscoveryDelegateImpl() {}
    static DiscoveryDelegateImpl * discoveryDelegateImplSingletonInstance;
    DiscoveryDelegateImpl(DiscoveryDelegateImpl & other) = delete;
    void operator=(const DiscoveryDelegateImpl &)        = delete;

public:
    jobject castingPlayerChangeListenerJavaObject = nullptr;
    jmethodID onAddedCallbackJavaMethodID         = nullptr;
    jmethodID onChangedCallbackJavaMethodID       = nullptr;
    // jmethodID onRemovedCallbackJavaMethodID = nullptr;

    static DiscoveryDelegateImpl * GetInstance()
    {
        ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::GetInstance() called");
        if (DiscoveryDelegateImpl::discoveryDelegateImplSingletonInstance == nullptr)
        {
            ChipLogProgress(AppServer,
                            "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::GetInstance() creating new singleton instance");
            DiscoveryDelegateImpl::discoveryDelegateImplSingletonInstance = new DiscoveryDelegateImpl();
        }
        return DiscoveryDelegateImpl::discoveryDelegateImplSingletonInstance;
    }

    void HandleOnAdded(matter::casting::memory::Strong<CastingPlayer> player) override
    {
        ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnAdded() called with CastingPlayer:");
        player->LogDetail();

        if (castingPlayerChangeListenerJavaObject == nullptr || onAddedCallbackJavaMethodID == nullptr)
        {
            ChipLogError(AppServer,
                         "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnAdded() Warning: Not specified, "
                         "CastingPlayerChangeListener == nullptr");
        }
        else
        {
            JNIEnv * env                          = JniReferences::GetInstance().GetEnvForCurrentThread();
            jobject matterCastingPlayerJavaObject = support::createJCastingPlayer(player);
            if (matterCastingPlayerJavaObject == nullptr)
            {
                ChipLogError(AppServer,
                             "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnAdded() Warning: Could not create "
                             "CastingPlayer jobject");
            }
            else
            {
                env->CallVoidMethod(castingPlayerChangeListenerJavaObject, onAddedCallbackJavaMethodID,
                                    matterCastingPlayerJavaObject);
            }
        }
    }

    void HandleOnUpdated(matter::casting::memory::Strong<CastingPlayer> player) override
    {
        ChipLogProgress(AppServer,
                        "CastingPlayerDiscovery-JNI DiscoveryDelegateImpl::HandleOnUpdated() called with CastingPlayer:");
        player->LogDetail();

        if (castingPlayerChangeListenerJavaObject == nullptr || onChangedCallbackJavaMethodID == nullptr)
        {
            ChipLogError(AppServer,
                         "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnUpdated() Warning: Not specified, "
                         "CastingPlayerChangeListener == nullptr");
        }
        else
        {
            JNIEnv * env                          = JniReferences::GetInstance().GetEnvForCurrentThread();
            jobject matterCastingPlayerJavaObject = support::createJCastingPlayer(player);
            if (matterCastingPlayerJavaObject == nullptr)
            {
                ChipLogError(AppServer,
                             "CastingPlayerDiscovery-JNI::DiscoveryDelegateImpl::HandleOnUpdated() Warning: Could not create "
                             "CastingPlayer jobject");
            }
            else
            {
                env->CallVoidMethod(castingPlayerChangeListenerJavaObject, onChangedCallbackJavaMethodID,
                                    matterCastingPlayerJavaObject);
            }
        }
    }

    // TODO: In following PRs. Implement HandleOnRemoved after implemented in tv-casting-commom CastingPlayerDiscovery.h/cpp
    // void HandleOnRemoved(matter::casting::memory::Strong<CastingPlayer> player) override
    // {
    //     ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI DiscoveryDelegateImpl::HandleOnRemoved() called with
    //     CastingPlayer:"); player->LogDetail();
    // }
};

// Initialize the static instance to nullptr
DiscoveryDelegateImpl * DiscoveryDelegateImpl::discoveryDelegateImplSingletonInstance = nullptr;

JNI_METHOD(jobject, startDiscovery)(JNIEnv * env, jobject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::startDiscovery() called");
    DiscoveryDelegateImpl * delegate = DiscoveryDelegateImpl::GetInstance();
    CastingPlayerDiscovery::GetInstance()->SetDelegate(delegate);

    // Start CastingPlayer discovery
    const uint64_t kTargetPlayerDeviceType = 35; // 35 represents device type of Matter Video Player
    CHIP_ERROR err                         = CastingPlayerDiscovery::GetInstance()->StartDiscovery(kTargetPlayerDeviceType);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "CastingPlayerDiscovery-JNI startDiscovery() err: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // TODO: Verify error returned?
    VerifyOrReturnValue(err == CHIP_NO_ERROR, support::createJMatterError(CHIP_ERROR_INCORRECT_STATE));
    return support::createJMatterError(CHIP_NO_ERROR);
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
    }

    VerifyOrReturnValue(err == CHIP_NO_ERROR, support::createJMatterError(CHIP_ERROR_INCORRECT_STATE));
    return support::createJMatterError(CHIP_NO_ERROR);
}

JNI_METHOD(jobject, addCastingPlayerChangeListener)(JNIEnv * env, jobject, jobject castingPlayerChangeListenerJavaObject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::addCastingPlayerChangeListener() called");
    VerifyOrReturnValue(castingPlayerChangeListenerJavaObject != nullptr, support::createJMatterError(CHIP_ERROR_INCORRECT_STATE));

    DiscoveryDelegateImpl * delegate = DiscoveryDelegateImpl::GetInstance();
    if (delegate->castingPlayerChangeListenerJavaObject != nullptr)
    {
        ChipLogError(AppServer,
                     "CastingPlayerDiscovery-JNI::addCastingPlayerChangeListener() Warning: Call removeCastingPlayerChangeListener "
                     "before adding a new one");
        return support::createJMatterError(CHIP_ERROR_INCORRECT_STATE);
    }

    // Get the class and method IDs for the CastingPlayerChangeListener Java class
    jclass castingPlayerChangeListenerJavaClass = env->GetObjectClass(castingPlayerChangeListenerJavaObject);
    VerifyOrReturnValue(castingPlayerChangeListenerJavaClass != nullptr, support::createJMatterError(CHIP_ERROR_INCORRECT_STATE));

    jmethodID onAddedJavaMethodID =
        env->GetMethodID(castingPlayerChangeListenerJavaClass, "_onAdded", "(Lcom/matter/casting/core/CastingPlayer;)V");
    VerifyOrReturnValue(onAddedJavaMethodID != nullptr, support::createJMatterError(CHIP_ERROR_INCORRECT_STATE));
    jmethodID onChangedJavaMethodID =
        env->GetMethodID(castingPlayerChangeListenerJavaClass, "_onChanged", "(Lcom/matter/casting/core/CastingPlayer;)V");
    VerifyOrReturnValue(onChangedJavaMethodID != nullptr, support::createJMatterError(CHIP_ERROR_INCORRECT_STATE));
    // jmethodID onRemovedJavaMethodID = env->GetMethodID(castingPlayerChangeListenerJavaClass, "_onRemoved",
    // "(Lcom/matter/casting/core/CastingPlayer;)V"); VerifyOrReturnValue(onRemovedJavaMethodID != nullptr,
    // support::createJMatterError(CHIP_ERROR_INCORRECT_STATE));

    // Set Java callbacks in the DiscoveryDelegateImpl Singleton
    delegate->castingPlayerChangeListenerJavaObject = env->NewGlobalRef(castingPlayerChangeListenerJavaObject);
    delegate->onAddedCallbackJavaMethodID           = onAddedJavaMethodID;
    delegate->onChangedCallbackJavaMethodID         = onChangedJavaMethodID;
    // delegate->onRemovedCallbackJavaMethodID = onRemovedJavaMethodID;

    return support::createJMatterError(CHIP_NO_ERROR);
}

JNI_METHOD(jobject, removeCastingPlayerChangeListener)(JNIEnv * env, jobject, jobject castingPlayerChangeListenerJavaObject)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::removeCastingPlayerChangeListener() called");

    // Remove the Java callbacks in the DiscoveryDelegateImpl Singleton
    DiscoveryDelegateImpl * delegate = DiscoveryDelegateImpl::GetInstance();

    // Check if the passed object is the same as the one added in addCastingPlayerChangeListener() JNI method
    jboolean isSameObject =
        env->IsSameObject(castingPlayerChangeListenerJavaObject, delegate->castingPlayerChangeListenerJavaObject);

    if ((bool) isSameObject)
    {
        ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::removeCastingPlayerChangeListener() removing listener");

        // Delete the global reference to the Java object
        env->DeleteGlobalRef(delegate->castingPlayerChangeListenerJavaObject);
        delegate->castingPlayerChangeListenerJavaObject = nullptr;
        // No explicit cleanup required
        delegate->onAddedCallbackJavaMethodID   = nullptr;
        delegate->onChangedCallbackJavaMethodID = nullptr;
        // delegate->onRemovedCallbackJavaMethodID = nullptr;

        return support::createJMatterError(CHIP_NO_ERROR);
    }
    else
    {
        ChipLogError(AppServer,
                     "CastingPlayerDiscovery-JNI::removeCastingPlayerChangeListener() Warning: Cannot remove listener. Received a "
                     "different CastingPlayerChangeListener object");
        return support::createJMatterError(CHIP_ERROR_INCORRECT_STATE);
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
        jobject matterCastingPlayerJavaObject = support::createJCastingPlayer(player);
        if (matterCastingPlayerJavaObject != nullptr)
        {
            jboolean added = env->CallBooleanMethod(arrayList, addMethod, matterCastingPlayerJavaObject);
            if ((bool) added)
            {
                ChipLogProgress(AppServer, "CastingPlayerDiscovery-JNI::getCastingPlayers() added CastingPlayer with ID: %s",
                                player->GetId());
            }
            else
            {
                ChipLogError(AppServer,
                             "CastingPlayerDiscovery-JNI::getCastingPlayers() Warning: Unable to add CastingPlayer with ID: %s",
                             player->GetId());
            }
        }
    }

    return arrayList;
}

}; // namespace core
}; // namespace casting
}; // namespace matter
