/*
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
                   ChipLogError(Zcl, "Failed to Init mJNIDACProviderObject"));

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

    mGetDeviceAttestationCertPrivateKeyMethod = env->GetMethodID(JNIDACProviderClass, "GetDeviceAttestationCertPrivateKey", "()[B");
    if (mGetDeviceAttestationCertPrivateKeyMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIDACProvider 'GetDeviceAttestationCertPrivateKey' method");
        env->ExceptionClear();
    }

    mGetDeviceAttestationCertPublicKeyKeyMethod =
        env->GetMethodID(JNIDACProviderClass, "GetDeviceAttestationCertPublicKeyKey", "()[B");
    if (mGetDeviceAttestationCertPublicKeyKeyMethod == nullptr)
    {
        ChipLogError(Zcl, "Failed to access JNIDACProvider 'GetDeviceAttestationCertPublicKeyKey' method");
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

// TODO: This should be moved to a method of P256Keypair
CHIP_ERROR LoadKeypairFromRaw(ByteSpan private_key, ByteSpan public_key, Crypto::P256Keypair & keypair)
{
    Crypto::P256SerializedKeypair serialized_keypair;
    ReturnErrorOnFailure(serialized_keypair.SetLength(private_key.size() + public_key.size()));
    memcpy(serialized_keypair.Bytes(), public_key.data(), public_key.size());
    memcpy(serialized_keypair.Bytes() + public_key.size(), private_key.data(), private_key.size());
    return keypair.Deserialize(serialized_keypair);
}

CHIP_ERROR JNIDACProvider::SignWithDeviceAttestationKey(const ByteSpan & digest_to_sign, MutableByteSpan & out_signature_buffer)
{
    ChipLogProgress(Zcl, "Received SignWithDeviceAttestationKey");
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;

    VerifyOrReturnError(!out_signature_buffer.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!digest_to_sign.empty(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_signature_buffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    uint8_t privateKeyBuf[Crypto::kP256_PrivateKey_Length];
    MutableByteSpan privateKeyBufSpan(privateKeyBuf);
    ReturnErrorOnFailure(GetJavaByteByMethod(mGetDeviceAttestationCertPrivateKeyMethod, privateKeyBufSpan));

    uint8_t publicKeyBuf[Crypto::kP256_PublicKey_Length];
    MutableByteSpan publicKeyBufSpan(publicKeyBuf);
    ReturnErrorOnFailure(GetJavaByteByMethod(mGetDeviceAttestationCertPublicKeyKeyMethod, publicKeyBufSpan));

    // In a non-exemplary implementation, the public key is not needed here. It is used here merely because
    // Crypto::P256Keypair is only (currently) constructable from raw keys if both private/public keys are present.
    ReturnErrorOnFailure(LoadKeypairFromRaw(privateKeyBufSpan, publicKeyBufSpan, keypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(digest_to_sign.data(), digest_to_sign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, out_signature_buffer);
}
