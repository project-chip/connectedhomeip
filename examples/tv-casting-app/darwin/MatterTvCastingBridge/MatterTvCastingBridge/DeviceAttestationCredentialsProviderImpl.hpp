/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#pragma once

#include <credentials/DeviceAttestationCredsProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/logging/CHIPLogging.h>

class DeviceAttestationCredentialsProviderImpl : public chip::Credentials::DeviceAttestationCredentialsProvider
{
public:
    DeviceAttestationCredentialsProviderImpl(chip::MutableByteSpan * certificationDeclaration,
                                             chip::MutableByteSpan * firmwareInformation,
                                             chip::MutableByteSpan * deviceAttestationCert,
                                             chip::MutableByteSpan * productAttestationIntermediateCert,
                                             chip::MutableByteSpan * deviceAttestationCertPrivateKey,
                                             chip::MutableByteSpan * deviceAttestationCertPublicKeyKey);

    CHIP_ERROR GetCertificationDeclaration(chip::MutableByteSpan & outCertificationDeclaration) override;
    CHIP_ERROR GetFirmwareInformation(chip::MutableByteSpan & outFirmwareInformation) override;
    CHIP_ERROR GetDeviceAttestationCert(chip::MutableByteSpan & outDeviceAttestationCert) override;
    CHIP_ERROR GetProductAttestationIntermediateCert(chip::MutableByteSpan & outProductAttestationIntermediateCert) override;
    CHIP_ERROR SignWithDeviceAttestationKey(const chip::ByteSpan & messageToSign,
                                            chip::MutableByteSpan & outSignatureBuffer) override;

private:
    chip::MutableByteSpan mCertificationDeclaration;
    chip::MutableByteSpan mFirmwareInformation;
    chip::MutableByteSpan mDeviceAttestationCert;
    chip::MutableByteSpan mProductAttestationIntermediateCert;
    chip::MutableByteSpan mDeviceAttestationCertPrivateKey;
    chip::MutableByteSpan mDeviceAttestationCertPublicKeyKey;

    CHIP_ERROR LoadKeypairFromRaw(chip::ByteSpan privateKey, chip::ByteSpan publicKey, chip::Crypto::P256Keypair & keypair)
    {
        chip::Crypto::P256SerializedKeypair serialized_keypair;
        ReturnErrorOnFailure(serialized_keypair.SetLength(privateKey.size() + publicKey.size()));
        memcpy(serialized_keypair.Bytes(), publicKey.data(), publicKey.size());
        memcpy(serialized_keypair.Bytes() + publicKey.size(), privateKey.data(), privateKey.size());
        return keypair.Deserialize(serialized_keypair);
    }
};
