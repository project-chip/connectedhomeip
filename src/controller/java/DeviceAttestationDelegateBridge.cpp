/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

CHIP_ERROR N2J_AttestationInfo(JNIEnv * env, const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
                               jobject & outAttestationInfo)
{
    CHIP_ERROR err                                        = CHIP_NO_ERROR;
    jclass infoClass                                      = nullptr;
    jmethodID constructor                                 = nullptr;
    jbyteArray javaDAC                                    = nullptr;
    jbyteArray javaPAI                                    = nullptr;
    jbyteArray javaCD                                     = nullptr;
    const ByteSpan DAC                                    = info.dacDerBuffer();
    const ByteSpan PAI                                    = info.paiDerBuffer();
    const Optional<ByteSpan> certificationDeclarationSpan = info.cdBuffer();
    uint16_t vendorId                                     = info.BasicInformationVendorId();
    uint16_t productId                                    = info.BasicInformationProductId();

    err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/AttestationInfo", infoClass);
    SuccessOrExit(err);

    env->ExceptionClear();
    constructor = env->GetMethodID(infoClass, "<init>", "([B[B[BII)V");
    VerifyOrExit(constructor != nullptr, err = CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    err = JniReferences::GetInstance().N2J_ByteArray(env, DAC.data(), static_cast<jsize>(DAC.size()), javaDAC);
    SuccessOrExit(err);
    err = JniReferences::GetInstance().N2J_ByteArray(env, PAI.data(), static_cast<jsize>(PAI.size()), javaPAI);
    SuccessOrExit(err);
    if (certificationDeclarationSpan.HasValue())
    {
        err = JniReferences::GetInstance().N2J_ByteArray(env, certificationDeclarationSpan.Value().data(),
                                                         static_cast<jsize>(certificationDeclarationSpan.Value().size()), javaCD);
        SuccessOrExit(err);
    }
    outAttestationInfo = (jobject) env->NewObject(infoClass, constructor, javaDAC, javaPAI, javaCD, static_cast<jint>(vendorId),
                                                  static_cast<jint>(productId));
    VerifyOrExit(!env->ExceptionCheck(), err = CHIP_JNI_ERROR_EXCEPTION_THROWN);
exit:
    return err;
}

void DeviceAttestationDelegateBridge::OnDeviceAttestationCompleted(
    chip::Controller::DeviceCommissioner * deviceCommissioner, chip::DeviceProxy * device,
    const chip::Credentials::DeviceAttestationVerifier::AttestationDeviceInfo & info,
    chip::Credentials::AttestationVerificationResult attestationResult)
{
    ChipLogProgress(Controller, "OnDeviceAttestationCompleted with result: %hu", static_cast<uint16_t>(attestationResult));

    mResult = attestationResult;
    if (mDeviceAttestationDelegate.HasValidObjectRef())
    {
        JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
        VerifyOrReturn(env != nullptr, ChipLogError(Controller, "Could not get JNIEnv for current thread"));
        JniLocalReferenceScope scope(env);
        jclass deviceAttestationDelegateCls = nullptr;
        JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/DeviceAttestationDelegate",
                                                      deviceAttestationDelegateCls);
        VerifyOrReturn(deviceAttestationDelegateCls != nullptr,
                       ChipLogError(Controller, "Could not find device attestation delegate class."));

        if (env->IsInstanceOf(mDeviceAttestationDelegate.ObjectRef(), deviceAttestationDelegateCls))
        {
            jmethodID onDeviceAttestationCompletedMethod;
            JniReferences::GetInstance().FindMethod(env, mDeviceAttestationDelegate.ObjectRef(), "onDeviceAttestationCompleted",
                                                    "(JLchip/devicecontroller/AttestationInfo;J)V",
                                                    &onDeviceAttestationCompletedMethod);
            VerifyOrReturn(onDeviceAttestationCompletedMethod != nullptr,
                           ChipLogError(Controller, "Could not find deviceAttestation completed method"));

            jobject javaAttestationInfo = nullptr;

            //  Don't need to pass attestationInfo for additional verification when attestation failed.
            if (attestationResult == chip::Credentials::AttestationVerificationResult::kSuccess)
            {
                CHIP_ERROR err = N2J_AttestationInfo(env, info, javaAttestationInfo);
                VerifyOrReturn(err == CHIP_NO_ERROR,
                               ChipLogError(Controller, "Failed to create AttestationInfo, error: %s", err.AsString()));
            }

            env->CallVoidMethod(mDeviceAttestationDelegate.ObjectRef(), onDeviceAttestationCompletedMethod,
                                reinterpret_cast<jlong>(device), javaAttestationInfo, static_cast<jint>(attestationResult));
        }
    }
}
