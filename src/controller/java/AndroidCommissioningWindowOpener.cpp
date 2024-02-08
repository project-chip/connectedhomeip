/*
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include "AndroidCommissioningWindowOpener.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <protocols/secure_channel/PASESession.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

using namespace chip::app::Clusters;
using namespace chip::System::Clock;

namespace chip {
namespace Controller {

AndroidCommissioningWindowOpener::AndroidCommissioningWindowOpener(DeviceController * controller, jobject jCallbackObject) :
    CommissioningWindowOpener(controller), mOnOpenCommissioningWindowCallback(OnOpenCommissioningWindowResponse, this),
    mOnOpenBasicCommissioningWindowCallback(OnOpenBasicCommissioningWindowResponse, this)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    if (mJavaCallback.Init(jCallbackObject) != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to create global reference for mJavaCallback");
        return;
    }

    jclass callbackClass = env->GetObjectClass(jCallbackObject);

    mOnSuccessMethod = env->GetMethodID(callbackClass, "onSuccess", "(JLjava/lang/String;Ljava/lang/String;)V");
    if (mOnSuccessMethod == nullptr)
    {
        ChipLogError(Controller, "Failed to access callback 'onSuccess' method");
        env->ExceptionClear();
    }

    mOnErrorMethod = env->GetMethodID(callbackClass, "onError", "(IJ)V");
    if (mOnErrorMethod == nullptr)
    {
        ChipLogError(Controller, "Failed to access callback 'onError' method");
        env->ExceptionClear();
    }
}

CHIP_ERROR AndroidCommissioningWindowOpener::OpenBasicCommissioningWindow(DeviceController * controller, NodeId deviceId,
                                                                          Seconds16 timeout, jobject jcallback)
{
    // Not using Platform::New because we want to keep our constructor private.
    auto * opener = new AndroidCommissioningWindowOpener(controller, jcallback);
    if (opener == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = opener->CommissioningWindowOpener::OpenBasicCommissioningWindow(
        deviceId, timeout, &opener->mOnOpenBasicCommissioningWindowCallback);
    if (err != CHIP_NO_ERROR)
    {
        delete opener;
    }
    // Else will clean up when the callback is called.
    return err;
}

CHIP_ERROR AndroidCommissioningWindowOpener::OpenCommissioningWindow(DeviceController * controller, NodeId deviceId,
                                                                     Seconds16 timeout, uint32_t iteration, uint16_t discriminator,
                                                                     Optional<uint32_t> setupPIN, Optional<ByteSpan> salt,
                                                                     jobject jcallback, SetupPayload & payload,
                                                                     bool readVIDPIDAttributes)
{
    // Not using Platform::New because we want to keep our constructor private.
    auto * opener = new AndroidCommissioningWindowOpener(controller, jcallback);
    if (opener == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = opener->CommissioningWindowOpener::OpenCommissioningWindow(
        deviceId, timeout, iteration, discriminator, setupPIN, salt, &opener->mOnOpenCommissioningWindowCallback, payload,
        readVIDPIDAttributes);
    if (err != CHIP_NO_ERROR)
    {
        delete opener;
    }
    // Else will clean up when the callback is called.
    return err;
}

void AndroidCommissioningWindowOpener::OnOpenCommissioningWindowResponse(void * context, NodeId deviceId, CHIP_ERROR status,
                                                                         chip::SetupPayload payload)
{
    auto * self  = static_cast<AndroidCommissioningWindowOpener *>(context);
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
    JniLocalReferenceScope scope(env);

    VerifyOrExit(self->mJavaCallback.HasValidObjectRef(), ChipLogError(Controller, "mJavaCallback is not allocated."));

    if (status == CHIP_NO_ERROR)
    {
        std::string QRCode;
        std::string manualPairingCode;

        SuccessOrExit(status = ManualSetupPayloadGenerator(payload).payloadDecimalStringRepresentation(manualPairingCode));
        SuccessOrExit(status = QRCodeSetupPayloadGenerator(payload).payloadBase38Representation(QRCode));

        if (self->mOnSuccessMethod != nullptr)
        {
            UtfString jManualPairingCode(env, manualPairingCode.c_str());
            UtfString jQRCode(env, QRCode.c_str());
            env->CallVoidMethod(self->mJavaCallback.ObjectRef(), self->mOnSuccessMethod, static_cast<jlong>(deviceId),
                                jManualPairingCode.jniValue(), jQRCode.jniValue());
        }
    }
    else
    {
        if (self->mOnErrorMethod != nullptr)
        {
            env->CallVoidMethod(self->mJavaCallback.ObjectRef(), self->mOnErrorMethod, static_cast<jint>(status.GetValue()),
                                static_cast<jlong>(deviceId));
        }
    }
exit:
    delete self;
}

void AndroidCommissioningWindowOpener::OnOpenBasicCommissioningWindowResponse(void * context, NodeId deviceId, CHIP_ERROR status)
{
    auto * self = static_cast<AndroidCommissioningWindowOpener *>(context);
    if (self->mJavaCallback.HasValidObjectRef())
    {
        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
        JniLocalReferenceScope scope(env);
        if (status == CHIP_NO_ERROR)
        {
            if (self->mOnSuccessMethod != nullptr)
            {
                UtfString jManualPairingCode(env, "");
                UtfString jQRCode(env, "");
                env->CallVoidMethod(self->mJavaCallback.ObjectRef(), self->mOnSuccessMethod, static_cast<jlong>(deviceId),
                                    jManualPairingCode.jniValue(), jQRCode.jniValue());
            }
        }
        else
        {
            if (self->mOnErrorMethod != nullptr)
            {
                env->CallVoidMethod(self->mJavaCallback.ObjectRef(), self->mOnErrorMethod, static_cast<jint>(status.GetValue()),
                                    static_cast<jlong>(deviceId));
            }
        }
    }

    delete self;
}

} // namespace Controller
} // namespace chip
