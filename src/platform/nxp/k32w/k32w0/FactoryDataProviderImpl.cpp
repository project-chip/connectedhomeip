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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <platform/nxp/k32w/k32w0/FactoryDataProviderImpl.h>

#if CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR
extern "C" {
#include "Flash_Adapter.h"
}
extern "C" WEAK CHIP_ERROR FactoryDataDefaultRestoreMechanism();
#endif

namespace chip {
namespace DeviceLayer {

FactoryDataProviderImpl::FactoryDataProviderImpl()
{
#if CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR
    RegisterRestoreMechanism(FactoryDataDefaultRestoreMechanism);
#endif
}

CHIP_ERROR FactoryDataProviderImpl::Init()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    uint32_t sum     = 0;

#if CHIP_DEVICE_CONFIG_USE_CUSTOM_PROVIDER
    ReturnErrorOnFailure(SetCustomIds());
#endif

    for (uint8_t i = 1; i < FactoryDataProvider::kNumberOfIds; i++)
    {
        sum += maxLengths[i];
    }

    if (sum > kFactoryDataSize)
    {
        ChipLogError(DeviceLayer, "Max size of factory data: %lu is bigger than reserved factory data size: %lu", sum,
                     kFactoryDataSize);
    }

#if CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR
    error = ValidateWithRestore();
#else
    error = Validate();
#endif
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Factory data init failed with: %s", ErrorStr(error));
    }

    return error;
}

CHIP_ERROR FactoryDataProviderImpl::SignWithDacKey(const ByteSpan & messageToSign, MutableByteSpan & outSignBuffer)
{
    Crypto::P256ECDSASignature signature;
    Crypto::P256Keypair keypair;
    Crypto::P256SerializedKeypair serializedKeypair;

    VerifyOrReturnError(IsSpanUsable(outSignBuffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(messageToSign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(outSignBuffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    /* Get private key of DAC certificate from reserved section */
    uint8_t keyBuf[Crypto::kP256_PrivateKey_Length];
    MutableByteSpan dacPrivateKeySpan(keyBuf);
    uint16_t keySize = 0;
    ReturnErrorOnFailure(SearchForId(FactoryDataId::kDacPrivateKeyId, dacPrivateKeySpan.data(), dacPrivateKeySpan.size(), keySize));
    dacPrivateKeySpan.reduce_size(keySize);

    /* Only the private key is used when signing */
    ReturnErrorOnFailure(serializedKeypair.SetLength(Crypto::kP256_PublicKey_Length + dacPrivateKeySpan.size()));
    memcpy(serializedKeypair.Bytes() + Crypto::kP256_PublicKey_Length, dacPrivateKeySpan.data(), dacPrivateKeySpan.size());

    ReturnErrorOnFailure(keypair.Deserialize(serializedKeypair));
    ReturnErrorOnFailure(keypair.ECDSA_sign_msg(messageToSign.data(), messageToSign.size(), signature));

    // TODO: sanitize temporary buffers used to store the private key, so it doesn't leak on the stack?

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, outSignBuffer);
}

#if CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR
extern "C" WEAK CHIP_ERROR FactoryDataDefaultRestoreMechanism()
{
    CHIP_ERROR error      = CHIP_NO_ERROR;
    uint16_t backupLength = 0;

    // Check if PDM id related to factory data backup exists.
    // If it does, it means an external event (such as a power loss)
    // interrupted the factory data update process and the section
    // from internal flash is most likely erased and should be restored.
    if (PDM_bDoesDataExist(kNvmId_FactoryDataBackup, &backupLength))
    {
        chip::Platform::ScopedMemoryBuffer<uint8_t> buffer;
        buffer.Calloc(FactoryDataProvider::kFactoryDataSize);
        ReturnErrorCodeIf(buffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);

        auto status = PDM_eReadDataFromRecord(kNvmId_FactoryDataBackup, (void *) buffer.Get(),
                                              FactoryDataProvider::kFactoryDataSize, &backupLength);
        ReturnErrorCodeIf(PDM_E_STATUS_OK != status, CHIP_FACTORY_DATA_PDM_RESTORE);

        error = FactoryDataProviderImpl::UpdateData(buffer.Get());
        if (error == CHIP_NO_ERROR)
        {
            ChipLogProgress(DeviceLayer, "Factory data was restored successfully");
        }
    }

    return error;
}

CHIP_ERROR FactoryDataProviderImpl::ValidateWithRestore()
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrReturnError(mRestoreMechanisms.size() > 0, CHIP_FACTORY_DATA_RESTORE_MECHANISM);

    for (auto & restore : mRestoreMechanisms)
    {
        error = restore();
        if (error != CHIP_NO_ERROR)
        {
            continue;
        }

        error = Validate();
        if (error != CHIP_NO_ERROR)
        {
            continue;
        }

        break;
    }

    if (error == CHIP_NO_ERROR)
    {
        PDM_vDeleteDataRecord(kNvmId_FactoryDataBackup);
    }

    return error;
}

CHIP_ERROR FactoryDataProviderImpl::UpdateData(uint8_t * pBuf)
{
    NV_Init();

    auto status = NV_FlashEraseSector(kFactoryDataStart, kFactoryDataSize);
    ReturnErrorCodeIf(status != kStatus_FLASH_Success, CHIP_FACTORY_DATA_FLASH_ERASE);

    Header * header = (Header *) pBuf;
    status          = NV_FlashProgramUnaligned(kFactoryDataStart, sizeof(Header) + header->size, pBuf);
    ReturnErrorCodeIf(status != kStatus_FLASH_Success, CHIP_FACTORY_DATA_FLASH_PROGRAM);

    return CHIP_NO_ERROR;
}

void FactoryDataProviderImpl::RegisterRestoreMechanism(RestoreMechanism restore)
{
    mRestoreMechanisms.insert(mRestoreMechanisms.end(), restore);
}
#endif // CONFIG_CHIP_K32W0_OTA_FACTORY_DATA_PROCESSOR

} // namespace DeviceLayer
} // namespace chip
