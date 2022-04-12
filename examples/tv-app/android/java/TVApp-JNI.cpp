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
#include "AppImpl.h"
#include "ChannelManager.h"
#include "CommissionerMain.h"
#include "ContentLauncherManager.h"
#include "JNIDACProvider.h"
#include "KeypadInputManager.h"
#include "LevelManager.h"
#include "LowPowerManager.h"
#include "MediaInputManager.h"
#include "MediaPlaybackManager.h"
#include "OnOffManager.h"
#include "WakeOnLanManager.h"
#include "credentials/DeviceAttestationCredsProvider.h"
#include <app/server/Dnssd.h>
#include <app/server/java/AndroidAppServerWrapper.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>

using namespace chip;
using namespace chip::app;
using namespace chip::AppPlatform;
using namespace chip::Credentials;

#define EXTENDED_DISCOVERY_TIMEOUT_SEC 20

#define JNI_METHOD(RETURN, METHOD_NAME) extern "C" JNIEXPORT RETURN JNICALL Java_com_tcl_chip_tvapp_TvApp_##METHOD_NAME

TvAppJNI TvAppJNI::sInstance;

void TvAppJNI::InitializeWithObjects(jobject app)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for TvAppJNI"));

    mTvAppObject = env->NewGlobalRef(app);
    VerifyOrReturn(mTvAppObject != nullptr, ChipLogError(Zcl, "Failed to NewGlobalRef TvAppJNI"));

    jclass managerClass = env->GetObjectClass(mTvAppObject);
    VerifyOrReturn(managerClass != nullptr, ChipLogError(Zcl, "Failed to get TvAppJNI Java class"));

    mPostClusterInitMethod = env->GetMethodID(managerClass, "postClusterInit", "(II)V");
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
    VerifyOrReturn(mTvAppObject != nullptr, ChipLogError(Zcl, "TvAppJNI::mTvAppObject null"));
    VerifyOrReturn(mPostClusterInitMethod != nullptr, ChipLogError(Zcl, "TvAppJNI::mPostClusterInitMethod null"));

    env->CallVoidMethod(mTvAppObject, mPostClusterInitMethod, static_cast<jint>(clusterId), static_cast<jint>(endpoint));
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

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
class MyUserPrompter : public UserPrompter
{
    // tv should override this with a dialog prompt
    inline void PromptForCommissionOKPermission(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override
    {
        /*
         *   Called to prompt the user for consent to allow the given commissioneeName/vendorId/productId to be commissioned.
         * For example "[commissioneeName] is requesting permission to cast to this TV, approve?"
         *
         * If user responds with OK then implementor should call CommissionerRespondOk();
         * If user responds with Cancel then implementor should call CommissionerRespondCancel();
         *
         */
        GetCommissionerDiscoveryController()->Ok();

        /**
         * For Demo: Launch Prime Video App
         */
        return;
    }

    // tv should override this with a dialog prompt
    inline void PromptForCommissionPincode(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override
    {
        /*
         *   Called to prompt the user to enter the setup pincode displayed by the given commissioneeName/vendorId/productId to be
         * commissioned. For example "Please enter pin displayed in casting app."
         *
         * If user enters with pin then implementor should call CommissionerRespondPincode(uint32_t pincode);
         * If user responds with Cancel then implementor should call CommissionerRespondCancel();
         */

        GetCommissionerDiscoveryController()->CommissionWithPincode(20202021); // dummy pin code

        /**
         * For Demo: Launch Prime Video App
         */
        return;
    }

    // tv should override this with a dialog prompt
    inline void PromptCommissioningSucceeded(uint16_t vendorId, uint16_t productId, const char * commissioneeName) override
    {
        return;
    }

    // tv should override this with a dialog prompt
    inline void PromptCommissioningFailed(const char * commissioneeName, CHIP_ERROR error) override { return; }
};

MyUserPrompter gMyUserPrompter;
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
class MyPincodeService : public PincodeService
{
    uint32_t FetchCommissionPincodeFromContentApp(uint16_t vendorId, uint16_t productId, CharSpan rotatingId) override
    {
        return ContentAppPlatform::GetInstance().GetPincodeFromContentApp(vendorId, productId, rotatingId);
    }
};
MyPincodeService gMyPincodeService;

class MyPostCommissioningListener : public PostCommissioningListener
{
    void CommissioningCompleted(uint16_t vendorId, uint16_t productId, NodeId nodeId, OperationalDeviceProxy * device) override
    {

        ContentAppPlatform::GetInstance().ManageClientAccess(device, vendorId, GetDeviceCommissioner()->GetNodeId(),
                                                             OnSuccessResponse, OnFailureResponse);
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
};

MyPostCommissioningListener gMyPostCommissioningListener;
ContentAppFactoryImpl gFactory;

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

JNI_METHOD(void, postInit)(JNIEnv *, jobject app)
{
    chip::DeviceLayer::StackLock lock;
    ChipLogProgress(Zcl, "TvAppJNI::PostInit");

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    ContentAppPlatform::GetInstance().SetupAppPlatform();
    ContentAppPlatform::GetInstance().SetContentAppFactory(&gFactory);
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
    if (cdc != nullptr)
    {
        cdc->SetPincodeService(&gMyPincodeService);
        cdc->SetUserPrompter(&gMyUserPrompter);
        cdc->SetPostCommissioningListener(&gMyPostCommissioningListener);
    }

    ChipLogProgress(AppServer, "Starting commissioner");
    VerifyOrReturn(InitCommissioner(CHIP_PORT + 2 + 10, CHIP_UDC_PORT) == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Started commissioner");

#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
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
