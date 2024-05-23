/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "JNIDACProvider.h"
#include "lib/support/logging/CHIPLogging.h"
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <cstdlib>
#include <jni.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/Span.h>

using namespace chip;

JNIDACProvider::JNIDACProvider(jobject provider)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturn(env != nullptr, ChipLogError(Zcl, "Failed to GetEnvForCurrentThread for JNIDACProvider"));

    VerifyOrReturn(mJNIDACProviderObject.Init(provider) == CHIP_NO_ERROR,
                   ChipLogError(Zcl, "Failed to init mJNIDACProviderObject"));

    jclass JNIDACProviderClass = env->GetObjectClass(provider);
    VerifyOrReturn(JNIDACProviderClass != nullptr, ChipLogError(Zcl, "Failed to get JNIDACProvider Java class"));

    mGetCertificationDeclarationMethod = env->GetMethodID(JNIDACProviderClass, "GetCertificationDeclaration", "()[B");
    if (mGetCertificationDeclarationMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIDACProvider 'GetCertificationDeclaration' method");
        env->ExceptionClear();
    }

    mGetFirmwareInformationMethod = env->GetMethodID(JNIDACProviderClass, "GetFirmwareInformation", "()[B");
    if (mGetFirmwareInformationMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIDACProvider 'GetFirmwareInformation' method");
        env->ExceptionClear();
    }

    mGetDeviceAttestationCertMethod = env->GetMethodID(JNIDACProviderClass, "GetDeviceAttestationCert", "()[B");
    if (mGetDeviceAttestationCertMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIDACProvider 'GetDeviceAttestationCert' method");
        env->ExceptionClear();
    }

    mGetProductAttestationIntermediateCertMethod =
        env->GetMethodID(JNIDACProviderClass, "GetProductAttestationIntermediateCert", "()[B");
    if (mGetProductAttestationIntermediateCertMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIDACProvider 'GetProductAttestationIntermediateCert' method");
        env->ExceptionClear();
    }

    mSignWithDeviceAttestationKeyMethod = env->GetMethodID(JNIDACProviderClass, "SignWithDeviceAttestationKey", "([B)[B");
    if (mSignWithDeviceAttestationKeyMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIDACProvider 'SignWithDeviceAttestationKey' method");
        env->ExceptionClear();
    }
}

CHIP_ERROR JNIDACProvider::GetJavaByteByMethod(jmethodID method, MutableByteSpan & out_buffer)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(mJNIDACProviderObject.HasValidObjectRef(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(method != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(env != nullptr, CHIP_JNI_ERROR_NO_ENV);

    jbyteArray outArray = (jbyteArray) env->CallObjectMethod(mJNIDACProviderObject.ObjectRef(), method);
    if (env->ExceptionCheck())
    {
        ChipLogError(Zcl, "Java exception in get Method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (outArray == nullptr || env->GetArrayLength(outArray) <= 0)
    {
        out_buffer.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    JniByteArray JniOutArray(env, outArray);
    return CopySpanToMutableSpan(JniOutArray.byteSpan(), out_buffer);
}

CHIP_ERROR JNIDACProvider::GetJavaByteByMethod(jmethodID method, const ByteSpan & in_buffer, MutableByteSpan & out_buffer)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(mJNIDACProviderObject.HasValidObjectRef(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(method != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnLogError(env != nullptr, CHIP_JNI_ERROR_NO_ENV);

    jbyteArray in_buffer_jbyteArray = env->NewByteArray((jsize) (in_buffer.size()));
    env->SetByteArrayRegion(in_buffer_jbyteArray, 0, (int) in_buffer.size(), reinterpret_cast<const jbyte *>(in_buffer.data()));

    jbyteArray outArray = (jbyteArray) env->CallObjectMethod(mJNIDACProviderObject.ObjectRef(), method, in_buffer_jbyteArray);
    if (env->ExceptionCheck())
    {
        ChipLogError(Zcl, "Java exception in get Method");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_ERROR_INCORRECT_STATE;
    }

    env->DeleteLocalRef(in_buffer_jbyteArray);

    if (outArray == nullptr || env->GetArrayLength(outArray) <= 0)
    {
        out_buffer.reduce_size(0);
        return CHIP_NO_ERROR;
    }

    JniByteArray JniOutArray(env, outArray);
    return CopySpanToMutableSpan(JniOutArray.byteSpan(), out_buffer);
}

CHIP_ERROR JNIDACProvider::GetCertificationDeclaration(MutableByteSpan & out_cd_buffer)
{
    ChipLogProgress(Zcl, "Received GetCertificationDeclaration");
    return GetJavaByteByMethod(mGetCertificationDeclarationMethod, out_cd_buffer);
}

CHIP_ERROR JNIDACProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    ChipLogProgress(Zcl, "Received GetFirmwareInformation");
    return GetJavaByteByMethod(mGetFirmwareInformationMethod, out_firmware_info_buffer);
}

CHIP_ERROR JNIDACProvider::GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer)
{
    ChipLogProgress(Zcl, "Received GetDeviceAttestationCert");
    return GetJavaByteByMethod(mGetDeviceAttestationCertMethod, out_dac_buffer);
}

CHIP_ERROR JNIDACProvider::GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer)
{
    ChipLogProgress(Zcl, "Received GetProductAttestationIntermediateCert");
    return GetJavaByteByMethod(mGetProductAttestationIntermediateCertMethod, out_pai_buffer);
}

CHIP_ERROR JNIDACProvider::SignWithDeviceAttestationKey(const ByteSpan & message_to_sign, MutableByteSpan & out_signature_buffer)
{
    ChipLogProgress(Zcl, "Received SignWithDeviceAttestationKey");
    uint8_t mAsn1SignatureBytes[73];

    MutableByteSpan asn1_signature_buffer(mAsn1SignatureBytes, sizeof(mAsn1SignatureBytes));

    CHIP_ERROR error = GetJavaByteByMethod(mSignWithDeviceAttestationKeyMethod, message_to_sign, asn1_signature_buffer);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "SignWithDeviceAttestationKey failed");
        return error;
    }

    return chip::Crypto::EcdsaAsn1SignatureToRaw(32, ByteSpan(asn1_signature_buffer.data(), asn1_signature_buffer.size()),
                                                 out_signature_buffer);
}
