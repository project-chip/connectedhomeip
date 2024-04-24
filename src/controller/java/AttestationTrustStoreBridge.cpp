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

#include "AttestationTrustStoreBridge.h"
#include <credentials/CHIPCert.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

CHIP_ERROR AttestationTrustStoreBridge::GetProductAttestationAuthorityCert(const chip::ByteSpan & skid,
                                                                           chip::MutableByteSpan & outPaaDerBuffer) const
{
    VerifyOrReturnError(skid.size() == chip::Crypto::kSubjectKeyIdentifierLength, CHIP_ERROR_INVALID_ARGUMENT);

    constexpr size_t paaCertAllocatedLen = chip::Credentials::kMaxDERCertLength;
    Platform::ScopedMemoryBuffer<uint8_t> paaCert;
    VerifyOrReturnError(paaCert.Alloc(paaCertAllocatedLen), CHIP_ERROR_NO_MEMORY);

    MutableByteSpan paaDerBuffer{ paaCert.Get(), paaCertAllocatedLen };
    ReturnErrorOnFailure(GetPaaCertFromJava(skid, paaDerBuffer));

    uint8_t skidBuf[chip::Crypto::kSubjectKeyIdentifierLength] = { 0 };
    chip::MutableByteSpan candidateSkidSpan{ skidBuf };
    VerifyOrReturnError(CHIP_NO_ERROR == chip::Crypto::ExtractSKIDFromX509Cert(paaDerBuffer, candidateSkidSpan),
                        CHIP_ERROR_INTERNAL);

    // Make sure the skid of the paa cert is match.
    if (skid.data_equal(candidateSkidSpan))
    {
        // Found a match
        return CopySpanToMutableSpan(paaDerBuffer, outPaaDerBuffer);
    }
    return CHIP_ERROR_CA_CERT_NOT_FOUND;
}

CHIP_ERROR AttestationTrustStoreBridge::GetPaaCertFromJava(const chip::ByteSpan & skid,
                                                           chip::MutableByteSpan & outPaaDerBuffer) const
{
    jclass attestationTrustStoreDelegateCls            = nullptr;
    jbyteArray javaSkid                                = nullptr;
    jmethodID getProductAttestationAuthorityCertMethod = nullptr;

    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnError(env != nullptr, CHIP_ERROR_INCORRECT_STATE);
    JniLocalReferenceScope scope(env);

    VerifyOrReturnError(mAttestationTrustStoreDelegate.HasValidObjectRef(), CHIP_ERROR_INCORRECT_STATE);
    JniReferences::GetInstance().GetLocalClassRef(env, "chip/devicecontroller/AttestationTrustStoreDelegate",
                                                  attestationTrustStoreDelegateCls);
    VerifyOrReturnError(attestationTrustStoreDelegateCls != nullptr, CHIP_JNI_ERROR_TYPE_NOT_FOUND);

    JniReferences::GetInstance().FindMethod(env, mAttestationTrustStoreDelegate.ObjectRef(), "getProductAttestationAuthorityCert",
                                            "([B)[B", &getProductAttestationAuthorityCertMethod);
    VerifyOrReturnError(getProductAttestationAuthorityCertMethod != nullptr, CHIP_JNI_ERROR_METHOD_NOT_FOUND);

    JniReferences::GetInstance().N2J_ByteArray(env, skid.data(), static_cast<jsize>(skid.size()), javaSkid);
    VerifyOrReturnError(javaSkid != nullptr, CHIP_ERROR_NO_MEMORY);

    jbyteArray javaPaaCert = (jbyteArray) env->CallObjectMethod(mAttestationTrustStoreDelegate.ObjectRef(),
                                                                getProductAttestationAuthorityCertMethod, javaSkid);
    VerifyOrReturnError(javaPaaCert != nullptr, CHIP_ERROR_CA_CERT_NOT_FOUND);

    JniByteArray paaCertBytes(env, javaPaaCert);
    CopySpanToMutableSpan(paaCertBytes.byteSpan(), outPaaDerBuffer);

    return CHIP_NO_ERROR;
}
