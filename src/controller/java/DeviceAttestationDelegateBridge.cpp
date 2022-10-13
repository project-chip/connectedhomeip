/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "DeviceAttestationDelegateBridge.h"
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>

using namespace chip;

CHIP_ERROR N2J_AttestationInfo(JNIEnv * env, const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                               jobject & outAttestationInfo)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    jclass infoClass = nullptr;
    jmethodID constructor = nullptr;
    jbyteArray javaDAC = nullptr;
    jbyteArray javaPAI = nullptr;
    jbyteArray javaCD = nullptr;
    const ByteSpan DAC = info.dacDerBuffer();
    const ByteSpan PAI = info.paiDerBuffer();
    const Optional<ByteSpan> certificationDeclarationSpan = info.cdBuffer();

    err = JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/AttestationInfo", infoClass);
    JniClass attestationInfoClass(infoClass);
    SuccessOrExit(err);

    env->ExceptionClear();
    constructor = env->GetMethodID(infoClass, "<init>", "([B[B[B)V");
    VerifyOrExit(constructor != nullptr, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    err = JniReferences::GetInstance().N2J_ByteArray(env, DAC.data(), DAC.size(), javaDAC);
    SuccessOrExit(err);
    err = JniReferences::GetInstance().N2J_ByteArray(env, PAI.data(), PAI.size(), javaPAI);
    SuccessOrExit(err);
    if(certificationDeclarationSpan.HasValue())
    {
        err = JniReferences::GetInstance().N2J_ByteArray(env, certificationDeclarationSpan.Value().data(), certificationDeclarationSpan.Value().size(),
                                        javaCD);
        SuccessOrExit(err);
    }
    outAttestationInfo = (jobject) env->NewObject(infoClass, constructor, javaDAC, javaPAI, javaCD);
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
exit:
    return err;
}

void DeviceAttestationDelegateBridge::OnDeviceAttestationCompleted(chip::Controller::DeviceCommissioner * deviceCommissioner,
    chip::DeviceProxy * device, const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
    chip::Credentials::AttestationVerificationResult attestationResult)
{
    ChipLogProgress(Controller, "OnDeviceAttestationCompleted with result: %hu", attestationResult);

    mResult = attestationResult;
    if(mDeviceAttestationDelegate != nullptr)
    {
        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        jclass completionCallbackCls = nullptr;
        JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/DeviceAttestationDelegate$DeviceAttestationCompletionCallback",
                                                completionCallbackCls);
        VerifyOrReturn(completionCallbackCls != nullptr,
                    ChipLogError(Controller, "Could not find device attestation completion callback class."));
        jclass failureCallbackCls = nullptr;
        JniReferences::GetInstance().GetClassRef(env, "chip/devicecontroller/DeviceAttestationDelegate$DeviceAttestationFailureCallback",
                                                failureCallbackCls);
        VerifyOrReturn(failureCallbackCls != nullptr,
                    ChipLogError(Controller, "Could not find device attestation failure callback class."));

        if(env->IsInstanceOf(mDeviceAttestationDelegate, completionCallbackCls))
        {
            jmethodID onDeviceAttestationCompletedMethod;
            JniReferences::GetInstance().FindMethod(env, mDeviceAttestationDelegate, "onDeviceAttestationCompleted", "(JJLchip/devicecontroller/AttestationInfo;I)V", &onDeviceAttestationCompletedMethod);
            VerifyOrReturn(onDeviceAttestationCompletedMethod != nullptr, ChipLogError(Controller, "Could not find deviceAttestation completed method"));
            jobject javaAttestationInfo;
            CHIP_ERROR err = N2J_AttestationInfo(env, info, javaAttestationInfo);
            VerifyOrReturn(err == CHIP_NO_ERROR, 
                    ChipLogError(Controller, "Failed to create AttestationInfo, error: %s", err.AsString()));
            env->CallVoidMethod(mDeviceAttestationDelegate, onDeviceAttestationCompletedMethod, mDeviceController, reinterpret_cast<jlong>(device), javaAttestationInfo, static_cast<jint>(attestationResult));
        }
        else if((attestationResult != chip::Credentials::AttestationVerificationResult::kSuccess) &&
            env->IsInstanceOf(mDeviceAttestationDelegate, failureCallbackCls))
        {
            jmethodID onDeviceAttestationFailedMethod;
            JniReferences::GetInstance().FindMethod(env, mDeviceAttestationDelegate, "onDeviceAttestationFailed", "(JJI)V", &onDeviceAttestationFailedMethod);
            VerifyOrReturn(onDeviceAttestationFailedMethod != nullptr, ChipLogError(Controller, "Could not find deviceAttestation failed method"));
            env->CallVoidMethod(mDeviceAttestationDelegate, onDeviceAttestationFailedMethod, mDeviceController, reinterpret_cast<jlong>(device), static_cast<jint>(attestationResult));
        }
    }
    
}

DeviceAttestationDelegateBridge::~DeviceAttestationDelegateBridge()
{
    if(mDeviceAttestationDelegate != nullptr)
    {
        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
        env->DeleteGlobalRef(mDeviceAttestationDelegate);
        mDeviceAttestationDelegate = nullptr;
    }
}