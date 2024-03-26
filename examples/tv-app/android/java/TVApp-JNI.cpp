/*
 *   Copyright (c) 2021 Project CHIP Authors
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

#include "TvApp-JNI.h"
#include "ChannelManager.h"
#include "CommissionerMain.h"
#include "ContentLauncherManager.h"
#include "DeviceCallbacks.h"
#include "JNIDACProvider.h"
#include "KeypadInputManager.h"
#include "LevelManager.h"
#include "LowPowerManager.h"
#include "MediaInputManager.h"
#include "MediaPlaybackManager.h"
#include "MessagesManager.h"
#include "MyUserPrompter-JNI.h"
#include "OnOffManager.h"
#include "WakeOnLanManager.h"
#include "credentials/DeviceAttestationCredsProvider.h"
#include <app/app-platform/ContentAppPlatform.h>
#include <app/server/Dnssd.h>
#include <app/server/java/AndroidAppServerWrapper.h>
#include <controller/CHIPCluster.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::AppPlatform;
using namespace chip::Credentials;

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_com_matter_tv_server_tvapp_TvApp_##METHOD_NAME

TvAppJNI TvAppJNI::sInstance;

void TvAppJNI::InitializeWithObjects(jobject app)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for TvAppJNI"));

    VerifyOrReturn(mTvAppObject.Init(app) == CHIP_NO_ERROR, ChipLogError(Zcl, "Failed to init mTvAppObject"));

    jclass managerClass = env->GetObjectClass(app);
    VerifyOrReturn(managerClass != nullptr, ChipLogError(Zcl, "Failed to get TvAppJNI Java class"));

    mPostClusterInitMethod = env->GetMethodID(managerClass, "postClusterInit", "(JI)V");
    if (mPostClusterInitMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ChannelManager 'postClusterInit' method");
        env->ExceptionClear();
    }
}

void TvAppJNI::PostClusterInit(int clusterId, int endpoint)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for TvAppJNI::PostClusterInit"));
    VerifyOrReturn(mTvAppObject.HasValidObjectRef(), ChipLogError(Zcl, "TvAppJNI::mTvAppObject is not valid"));
    VerifyOrReturn(mPostClusterInitMethod != nullptr, ChipLogError(Zcl, "TvAppJNI::mPostClusterInitMethod null"));

    env->CallVoidMethod(mTvAppObject.ObjectRef(), mPostClusterInitMethod, static_cast<jlong>(clusterId),
                        static_cast<jint>(endpoint));
    if (env->ExceptionCheck())
    {
        ChipLogError(Zcl, "Failed to call TvAppJNI 'postClusterInit' method");
        env->ExceptionClear();
    }
}

jint JNI_OnLoad(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnLoad(jvm, reserved);
}

void JNI_OnUnload(JavaVM * jvm, void * reserved)
{
    return AndroidAppServerJNI_OnUnload(jvm, reserved);
}

JNI_METHOD(void, nativeInit)(JNIEnv *, jobject app)
{
    TvAppJNIMgr().InitializeWithObjects(app);
}

JNI_METHOD(void, initializeCommissioner)(JNIEnv *, jobject app, jobject prompter)
{
    TvAppJNIMgr().InitializeCommissioner(new JNIMyUserPrompter(prompter));
}

JNI_METHOD(void, setKeypadInputManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    KeypadInputManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setWakeOnLanManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    WakeOnLanManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setMediaInputManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    MediaInputManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setContentLaunchManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    ContentLauncherManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setLowPowerManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    LowPowerManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setMediaPlaybackManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    MediaPlaybackManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setMessagesManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    MessagesManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setChannelManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    ChannelManager::NewManager(endpoint, manager);
}

JNI_METHOD(void, setDACProvider)(JNIEnv *, jobject, jobject provider)
{
    if (!chip::Credentials::IsDeviceAttestationCredentialsProviderSet())
    {
        JNIDACProvider * p = new JNIDACProvider(provider);
        chip::Credentials::SetDeviceAttestationCredentialsProvider(p);
    }
}

JNI_METHOD(void, preServerInit)(JNIEnv *, jobject app)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(Zcl, "TvAppJNI::preServerInit");
}

JNI_METHOD(void, postServerInit)(JNIEnv *, jobject app, jobject contentAppEndpointManager)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(Zcl, "TvAppJNI::postServerInit");
}

JNI_METHOD(void, setOnOffManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    OnOffManager::NewManager(endpoint, manager);
}

JNI_METHOD(jboolean, setOnOff)(JNIEnv *, jobject, jint endpoint, jboolean value)
{
    return OnOffManager::SetOnOff(endpoint, value);
}

JNI_METHOD(void, setLevelManager)(JNIEnv *, jobject, jint endpoint, jobject manager)
{
    LevelManager::NewManager(endpoint, manager);
}

JNI_METHOD(jboolean, setCurrentLevel)(JNIEnv *, jobject, jint endpoint, jboolean value)
{
    return LevelManager::SetLevel(endpoint, value);
}

JNI_METHOD(void, setChipDeviceEventProvider)(JNIEnv *, jobject, jobject provider)
{
    DeviceCallbacks::NewManager(provider);
}

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
class MyPincodeService : public PasscodeService
{
    void LookupTargetContentApp(uint16_t vendorId, uint16_t productId, chip::CharSpan rotatingId,
                                chip::Protocols::UserDirectedCommissioning::TargetAppInfo & info) override
    {
        uint32_t passcode;
        bool foundApp = ContentAppPlatform::GetInstance().HasTargetContentApp(vendorId, productId, rotatingId, info, passcode);
        if (!foundApp)
        {
            info.checkState = chip::Controller::TargetAppCheckState::kAppNotFound;
        }
        else if (passcode != 0)
        {
            info.checkState = chip::Controller::TargetAppCheckState::kAppFoundPasscodeReturned;
        }
        else
        {
            info.checkState = chip::Controller::TargetAppCheckState::kAppFoundNoPasscode;
        }
        CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
        if (cdc != nullptr)
        {
            cdc->HandleTargetContentAppCheck(info, passcode);
        }
    }

    uint32_t GetCommissionerPasscode(uint16_t vendorId, uint16_t productId, chip::CharSpan rotatingId) override
    {
        // TODO: randomly generate this value
        return 12345678;
    }

    void FetchCommissionPasscodeFromContentApp(uint16_t vendorId, uint16_t productId, CharSpan rotatingId) override
    {
        uint32_t passcode = ContentAppPlatform::GetInstance().GetPasscodeFromContentApp(vendorId, productId, rotatingId);
        CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
        if (cdc != nullptr)
        {
            cdc->HandleContentAppPasscodeResponse(passcode);
        }
    }
};
MyPincodeService gMyPincodeService;

class MyPostCommissioningListener : public PostCommissioningListener
{
    void CommissioningCompleted(uint16_t vendorId, uint16_t productId, NodeId nodeId, Messaging::ExchangeManager & exchangeMgr,
                                const SessionHandle & sessionHandle) override
    {
        // read current binding list
        chip::Controller::ClusterBase cluster(exchangeMgr, sessionHandle, kTargetBindingClusterEndpointId);

        cacheContext(vendorId, productId, nodeId, exchangeMgr, sessionHandle);

        CHIP_ERROR err =
            cluster.ReadAttribute<Binding::Attributes::Binding::TypeInfo>(this, OnReadSuccessResponse, OnReadFailureResponse);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Controller, "Failed in reading binding. Error %s", ErrorStr(err));
            clearContext();
        }
    }

    /* Callback when command results in success */
    static void
    OnReadSuccessResponse(void * context,
                          const app::DataModel::DecodableList<Binding::Structs::TargetStruct::DecodableType> & responseData)
    {
        ChipLogProgress(Controller, "OnReadSuccessResponse - Binding Read Successfully");

        MyPostCommissioningListener * listener = static_cast<MyPostCommissioningListener *>(context);
        listener->finishTargetConfiguration(responseData);
    }

    /* Callback when command results in failure */
    static void OnReadFailureResponse(void * context, CHIP_ERROR error)
    {
        ChipLogProgress(Controller, "OnReadFailureResponse - Binding Read Failed");

        MyPostCommissioningListener * listener = static_cast<MyPostCommissioningListener *>(context);
        listener->clearContext();

        CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
        if (cdc != nullptr)
        {
            cdc->PostCommissioningFailed(error);
        }
    }

    /* Callback when command results in success */
    static void OnSuccessResponse(void * context)
    {
        ChipLogProgress(Controller, "OnSuccessResponse - Binding Add Successfully");
        CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
        if (cdc != nullptr)
        {
            cdc->PostCommissioningSucceeded();
        }
    }

    /* Callback when command results in failure */
    static void OnFailureResponse(void * context, CHIP_ERROR error)
    {
        ChipLogProgress(Controller, "OnFailureResponse - Binding Add Failed");
        CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
        if (cdc != nullptr)
        {
            cdc->PostCommissioningFailed(error);
        }
    }

    void
    finishTargetConfiguration(const app::DataModel::DecodableList<Binding::Structs::TargetStruct::DecodableType> & responseList)
    {
        std::vector<app::Clusters::Binding::Structs::TargetStruct::Type> bindings;
        NodeId localNodeId = GetDeviceCommissioner()->GetNodeId();

        auto iter = responseList.begin();
        while (iter.Next())
        {
            auto & binding = iter.GetValue();
            ChipLogProgress(Controller, "Binding found nodeId=0x" ChipLogFormatX64 " my nodeId=0x" ChipLogFormatX64,
                            ChipLogValueX64(binding.node.ValueOr(0)), ChipLogValueX64(localNodeId));
            if (binding.node.ValueOr(0) != localNodeId)
            {
                ChipLogProgress(Controller, "Found a binding for a different node, preserving");
                bindings.push_back(binding);
            }
            else
            {
                ChipLogProgress(Controller, "Found a binding for a matching node, dropping");
            }
        }

        Optional<SessionHandle> opt   = mSecureSession.Get();
        SessionHandle & sessionHandle = opt.Value();
        ContentAppPlatform::GetInstance().ManageClientAccess(*mExchangeMgr, sessionHandle, mVendorId, mProductId, localNodeId,
                                                             bindings, OnSuccessResponse, OnFailureResponse);
        clearContext();
    }

    void cacheContext(uint16_t vendorId, uint16_t productId, NodeId nodeId, Messaging::ExchangeManager & exchangeMgr,
                      const SessionHandle & sessionHandle)
    {
        mVendorId    = vendorId;
        mProductId   = productId;
        mNodeId      = nodeId;
        mExchangeMgr = &exchangeMgr;
        mSecureSession.ShiftToSession(sessionHandle);
    }

    void clearContext()
    {
        mVendorId    = 0;
        mProductId   = 0;
        mNodeId      = 0;
        mExchangeMgr = nullptr;
        mSecureSession.SessionReleased();
    }
    uint16_t mVendorId                        = 0;
    uint16_t mProductId                       = 0;
    NodeId mNodeId                            = 0;
    Messaging::ExchangeManager * mExchangeMgr = nullptr;
    SessionHolder mSecureSession;
};

MyPostCommissioningListener gMyPostCommissioningListener;

void TvAppJNI::InitializeCommissioner(JNIMyUserPrompter * userPrompter)
{
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    chip::DeviceLayer::StackLock lock;
    CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
    if (cdc != nullptr && userPrompter != nullptr)
    {
        cdc->SetPasscodeService(&gMyPincodeService);
        cdc->SetUserPrompter(userPrompter);
        cdc->SetPostCommissioningListener(&gMyPostCommissioningListener);
    }

    ChipLogProgress(AppServer, "Starting commissioner");
    InitCommissioner(CHIP_PORT + 2 + 10, CHIP_UDC_PORT);
    ChipLogProgress(AppServer, "Started commissioner");

#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
}
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
