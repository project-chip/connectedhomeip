/**
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

#include "CHIPP256KeypairBridge.h"
#include "lib/core/CHIPError.h"
#include "lib/support/CHIPJNIError.h"
#include "lib/support/JniReferences.h"
#include "lib/support/JniTypeWrappers.h"
#include "lib/support/SafeInt.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <jni.h>
#include <string.h>
#include <type_traits>

using namespace chip;
using namespace chip::Crypto;

CHIPP256KeypairBridge::~CHIPP256KeypairBridge() {}

CHIP_ERROR CHIPP256KeypairBridge::SetDelegate(jobject delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrExit(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);
    ReturnErrorOnFailure(mDelegate.Init(delegate));
    jclass keypairDelegateClass;
    err = JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/KeypairDelegate", keypairDelegateClass);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Failed to find class for KeypairDelegate."));
    SuccessOrExit(err = mKeypairDelegateClass.Init(static_cast<jobject>(keypairDelegateClass)));
    err = JniReferences::GetInstance().FindMethod(env, delegate, "createCertificateSigningRequest", "()[B",
                                                  &mCreateCertificateSigningRequestMethod);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(Controller, "Failed to find KeypairDelegate.createCertificateSigningRequest() method."));

    err = JniReferences::GetInstance().FindMethod(env, delegate, "getPublicKey", "()[B", &mGetPublicKeyMethod);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Failed to find KeypairDelegate.getPublicKey() method."));

    err = JniReferences::GetInstance().FindMethod(env, delegate, "ecdsaSignMessage", "([B)[B", &mEcdsaSignMessageMethod);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Failed to find KeypairDelegate.ecdsaSignMessage() method."));

exit:
    return err;
}

CHIP_ERROR CHIPP256KeypairBridge::Initialize(ECPKeyTarget key_target)
{
    if (HasKeypair())
    {
        SetPubkey();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CHIPP256KeypairBridge::Serialize(P256SerializedKeypair & output) const
{
    if (!HasKeypair())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR CHIPP256KeypairBridge::Deserialize(P256SerializedKeypair & input)
{
    if (!HasKeypair())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR CHIPP256KeypairBridge::NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const
{
    if (!HasKeypair())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Not supported for use from within the CHIP SDK. We provide our own
    // implementation that is JVM-specific.
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR CHIPP256KeypairBridge::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length, P256ECDSASignature & out_signature) const
{
    if (!HasKeypair())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    VerifyOrReturnError(CanCastTo<jsize>(msg_length), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = CHIP_NO_ERROR;
    jbyteArray jniMsg;
    jobject signedResult = nullptr;
    JNIEnv * env         = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, err = CHIP_JNI_ERROR_NO_ENV);
    err = JniReferences::GetInstance().N2J_ByteArray(env, msg, static_cast<jsize>(msg_length), jniMsg);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);
    VerifyOrReturnError(jniMsg != nullptr, err);
    VerifyOrReturnError(mDelegate.HasValidObjectRef(), CHIP_ERROR_INCORRECT_STATE);
    signedResult = env->CallObjectMethod(mDelegate.ObjectRef(), mEcdsaSignMessageMethod, jniMsg);

    if (env->ExceptionCheck())
    {
        ChipLogError(Controller, "Java exception in KeypairDelegate.ecdsaSignMessage()");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    JniByteArray jniSignature(env, static_cast<jbyteArray>(signedResult));
    MutableByteSpan signatureSpan(out_signature.Bytes(), out_signature.Capacity());
    ReturnErrorOnFailure(EcdsaAsn1SignatureToRaw(CHIP_CRYPTO_GROUP_SIZE_BYTES, jniSignature.byteSpan(), signatureSpan));
    ReturnErrorOnFailure(out_signature.SetLength(signatureSpan.size()));

    return err;
}

CHIP_ERROR CHIPP256KeypairBridge::ECDH_derive_secret(const P256PublicKey & remote_public_key,
                                                     P256ECDHDerivedSecret & out_secret) const
{
    if (!HasKeypair())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Not required for Java SDK.
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR CHIPP256KeypairBridge::SetPubkey()
{
    jobject publicKey = nullptr;
    JNIEnv * env      = nullptr;

    VerifyOrReturnError(HasKeypair(), CHIP_ERROR_INCORRECT_STATE);

    env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_JNI_ERROR_NO_ENV);
    VerifyOrReturnError(mDelegate.HasValidObjectRef(), CHIP_ERROR_INCORRECT_STATE);
    publicKey = env->CallObjectMethod(mDelegate.ObjectRef(), mGetPublicKeyMethod);
    if (env->ExceptionCheck())
    {
        ChipLogError(Controller, "Java exception in KeypairDelegate.getPublicKey()");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CHIP_JNI_ERROR_EXCEPTION_THROWN;
    }

    VerifyOrReturnError(publicKey != nullptr, CHIP_JNI_ERROR_NULL_OBJECT);
    JniByteArray jniPublicKey(env, static_cast<jbyteArray>(publicKey));
    FixedByteSpan<Crypto::kP256_PublicKey_Length> publicKeySpan =
        FixedByteSpan<kP256_PublicKey_Length>(reinterpret_cast<const uint8_t *>(jniPublicKey.data()));
    mPublicKey = P256PublicKey(publicKeySpan);
    return CHIP_NO_ERROR;
}
