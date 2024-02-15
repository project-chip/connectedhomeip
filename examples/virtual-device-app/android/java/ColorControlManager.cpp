/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "ColorControlManager.h"
#include "DeviceApp-JNI.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>

using namespace chip;

static constexpr size_t kColorControlManagerTableSize = MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT;

namespace {

ColorControlManager * gColorControlManagerTable[kColorControlManagerTableSize] = { nullptr };
static_assert(kColorControlManagerTableSize <= kEmberInvalidEndpointIndex, "gColorControlManagerTable table size error");

} // namespace

void emberAfColorControlClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Device App::ColorControl::PostClusterInit");
    DeviceAppJNIMgr().PostClusterInit(chip::app::Clusters::ColorControl::Id, endpoint);
}

void ColorControlManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "Device App: ColorControlManager::NewManager");
    uint16_t ep = emberAfGetClusterServerEndpointIndex(static_cast<chip::EndpointId>(endpoint), app::Clusters::ColorControl::Id,
                                                       MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    VerifyOrReturn(ep < kColorControlManagerTableSize,
                   ChipLogError(Zcl, "Device App::ColorControl::NewManager: endpoint %d not found", endpoint));

    VerifyOrReturn(gColorControlManagerTable[ep] == nullptr,
                   ChipLogError(Zcl, "ST Device App::ColorControl::NewManager: endpoint %d already has a manager", endpoint));
    ColorControlManager * mgr = new ColorControlManager();
    CHIP_ERROR err            = mgr->InitializeWithObjects(manager);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "ST Device App::ColorControl::NewManager: failed to initialize manager for endpoint %d", endpoint);
        delete mgr;
    }
    else
    {
        gColorControlManagerTable[ep] = mgr;
    }
}

static ColorControlManager * GetColorControlManager(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, app::Clusters::ColorControl::Id,
                                                       MATTER_DM_COLOR_CONTROL_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kColorControlManagerTableSize ? nullptr : gColorControlManagerTable[ep]);
}

void ColorControlManager::PostCurrentHueChanged(chip::EndpointId endpoint, int value)
{
    ChipLogProgress(Zcl, "Device App: ColorControlManager::PostCurrentHueChanged");
    ColorControlManager * mgr = GetColorControlManager(endpoint);
    VerifyOrReturn(mgr != nullptr, ChipLogError(Zcl, "ColorControlManager null"));

    mgr->HandleCurrentHueChanged(value);
}

void ColorControlManager::PostCurrentSaturationChanged(chip::EndpointId endpoint, int value)
{
    ChipLogProgress(Zcl, "Device App: ColorControlManager::PostCurrentSaturationChanged");
    ColorControlManager * mgr = GetColorControlManager(endpoint);
    VerifyOrReturn(mgr != nullptr, ChipLogError(Zcl, "ColorControlManager null"));

    mgr->HandleCurrentSaturationChanged(value);
}

void ColorControlManager::PostColorTemperatureChanged(chip::EndpointId endpoint, int value)
{
    ChipLogProgress(Zcl, "Device App: ColorControlManager::PostColorTemperatureChanged");
    ColorControlManager * mgr = GetColorControlManager(endpoint);
    VerifyOrReturn(mgr != nullptr, ChipLogError(Zcl, "ColorControlManager null"));

    mgr->HandleColorTemperatureChanged(value);
}

void ColorControlManager::PostColorModeChanged(chip::EndpointId endpoint, int value)
{
    ChipLogProgress(Zcl, "Device App: ColorControlManager::PostColorModeChanged");
    ColorControlManager * mgr = GetColorControlManager(endpoint);
    VerifyOrReturn(mgr != nullptr, ChipLogError(Zcl, "ColorControlManager null"));

    mgr->HandleColorModeChanged(value);
}

void ColorControlManager::PostEnhancedColorModeChanged(chip::EndpointId endpoint, int value)
{
    ChipLogProgress(Zcl, "Device App: ColorControlManager::PostEnhancedColorModeChanged");
    ColorControlManager * mgr = GetColorControlManager(endpoint);
    VerifyOrReturn(mgr != nullptr, ChipLogError(Zcl, "ColorControlManager null"));

    mgr->HandleEnhancedColorModeChanged(value);
}

CHIP_ERROR ColorControlManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(env != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnLogErrorOnFailure(mColorControlManagerObject.Init(managerObject));

    jclass ColorControlManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturnLogError(ColorControlManagerClass != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mHandleCurrentHueChangedMethod = env->GetMethodID(ColorControlManagerClass, "HandleCurrentHueChanged", "(I)V");
    if (mHandleCurrentHueChangedMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ColorControlManager 'HandleCurrentHueChanged' method");
        env->ExceptionClear();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mHandleCurrentSaturationChangedMethod = env->GetMethodID(ColorControlManagerClass, "HandleCurrentSaturationChanged", "(I)V");
    if (mHandleCurrentSaturationChangedMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ColorControlManager 'HandleCurrentSaturationChanged' method");
        env->ExceptionClear();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mHandleColorTemperatureChangedMethod = env->GetMethodID(ColorControlManagerClass, "HandleColorTemperatureChanged", "(I)V");
    if (mHandleColorTemperatureChangedMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ColorControlManager 'HandleColorTemperatureChanged' method");
        env->ExceptionClear();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mHandleColorModeChangedMethod = env->GetMethodID(ColorControlManagerClass, "HandleColorModeChanged", "(I)V");
    if (mHandleColorModeChangedMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ColorControlManager 'HandleColorModeChanged' method");
        env->ExceptionClear();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mHandleEnhancedColorModeChangedMethod = env->GetMethodID(ColorControlManagerClass, "HandleEnhancedColorModeChanged", "(I)V");
    if (mHandleEnhancedColorModeChangedMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access ColorControlManager 'HandleEnhancedColorModeChanged' method");
        env->ExceptionClear();
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

void ColorControlManager::HandleCurrentHueChanged(int value)
{
    ChipLogProgress(Zcl, "ColorControlManager::HandleCurrentHueChanged");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != NULL, ChipLogProgress(Zcl, "env null"));
    VerifyOrReturn(mColorControlManagerObject.HasValidObjectRef(), ChipLogProgress(Zcl, "mColorControlManagerObject null"));
    VerifyOrReturn(mHandleCurrentHueChangedMethod != nullptr, ChipLogProgress(Zcl, "mHandleCurrentHueChangedMethod null"));

    env->ExceptionClear();
    env->CallVoidMethod(mColorControlManagerObject.ObjectRef(), mHandleCurrentHueChangedMethod, value);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in ColorControlManager::HandleCurrentHueChanged");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void ColorControlManager::HandleCurrentSaturationChanged(int value)
{
    ChipLogProgress(Zcl, "ColorControlManager::HandleCurrentSaturationChanged");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != NULL, ChipLogProgress(Zcl, "env null"));
    VerifyOrReturn(mColorControlManagerObject.HasValidObjectRef(), ChipLogProgress(Zcl, "mColorControlManagerObject null"));
    VerifyOrReturn(mHandleCurrentSaturationChangedMethod != nullptr,
                   ChipLogProgress(Zcl, "mHandleCurrentSaturationChangedMethod null"));

    env->ExceptionClear();
    env->CallVoidMethod(mColorControlManagerObject.ObjectRef(), mHandleCurrentSaturationChangedMethod, value);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in ColorControlManager::HandleCurrentSaturationChanged");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void ColorControlManager::HandleColorTemperatureChanged(int value)
{
    ChipLogProgress(Zcl, "ColorControlManager::HandleColorTemperatureChanged");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != NULL, ChipLogProgress(Zcl, "env null"));
    VerifyOrReturn(mColorControlManagerObject.HasValidObjectRef(), ChipLogProgress(Zcl, "mColorControlManagerObject null"));
    VerifyOrReturn(mHandleColorTemperatureChangedMethod != nullptr,
                   ChipLogProgress(Zcl, "mHandleColorTemperatureChangedMethod null"));

    env->ExceptionClear();
    env->CallVoidMethod(mColorControlManagerObject.ObjectRef(), mHandleColorTemperatureChangedMethod, value);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in ColorControlManager::mHandleColorTemperatureChanged");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void ColorControlManager::HandleColorModeChanged(int value)
{
    ChipLogProgress(Zcl, "ColorControlManager::HandleColorModeChanged");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != NULL, ChipLogProgress(Zcl, "env null"));
    VerifyOrReturn(mColorControlManagerObject.HasValidObjectRef(), ChipLogProgress(Zcl, "mColorControlManagerObject null"));
    VerifyOrReturn(mHandleColorModeChangedMethod != nullptr, ChipLogProgress(Zcl, "mHandleColorModeChangedMethod null"));

    env->ExceptionClear();
    env->CallVoidMethod(mColorControlManagerObject.ObjectRef(), mHandleColorModeChangedMethod, value);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in ColorControlManager::HandleColorModeChanged");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void ColorControlManager::HandleEnhancedColorModeChanged(int value)
{
    ChipLogProgress(Zcl, "ColorControlManager::HandleEnhancedColorModeChanged");

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != NULL, ChipLogProgress(Zcl, "env null"));
    VerifyOrReturn(mColorControlManagerObject.HasValidObjectRef(), ChipLogProgress(Zcl, "mColorControlManagerObject null"));
    VerifyOrReturn(mHandleEnhancedColorModeChangedMethod != nullptr,
                   ChipLogProgress(Zcl, "mHandleEnhancedColorModeChangedMethod null"));

    env->ExceptionClear();
    env->CallVoidMethod(mColorControlManagerObject.ObjectRef(), mHandleEnhancedColorModeChangedMethod, value);
    if (env->ExceptionCheck())
    {
        ChipLogError(AppServer, "Java exception in ColorControlManager::HandleEnhancedColorModeChanged");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
