/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <lib/core/TLV.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/nxp/k32w/common/CHIPDevicePlatformRamStorageConfig.h>
#include <platform/nxp/k32w/k32w0/CHIPDevicePlatformConfig.h>
#include <platform/nxp/k32w/k32w0/OTAFactoryDataProcessor.h>

#include "PDM.h"
#include "fsl_flash.h"

namespace chip {

CHIP_ERROR OTAFactoryDataProcessor::Init()
{
    mAccumulator.Init(mLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFactoryDataProcessor::Clear()
{
    OTATlvProcessor::ClearInternal();
    mAccumulator.Clear();
    mPayload.Clear();
    ClearBuffer();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFactoryDataProcessor::ProcessInternal(ByteSpan & block)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    ReturnErrorOnFailure(mAccumulator.Accumulate(block));
#if OTA_ENCRYPTION_ENABLE
    MutableByteSpan mBlock = MutableByteSpan(mAccumulator.data(), mAccumulator.GetThreshold());
    OTATlvProcessor::vOtaProcessInternalEncryption(mBlock);
#endif
    error = DecodeTlv();

    if (error != CHIP_NO_ERROR)
    {
        // The factory data payload can contain a variable number of fields
        // to be updated. CHIP_END_OF_TLV is returned if no more fields are
        // found.
        if (error == CHIP_END_OF_TLV)
        {
            return CHIP_NO_ERROR;
        }

        Clear();
    }

    return error;
}

CHIP_ERROR OTAFactoryDataProcessor::ApplyAction()
{
    CHIP_ERROR error           = CHIP_NO_ERROR;
    FactoryProvider * provider = nullptr;

    ReturnErrorOnFailure(Read());
    ReturnErrorOnFailure(Backup());

    SuccessOrExit(error = Update((uint8_t) Tags::kDacPrivateKeyId, mPayload.mCertDacKey));
    SuccessOrExit(error = Update((uint8_t) Tags::kDacCertificateId, mPayload.mCertDac));
    SuccessOrExit(error = Update((uint8_t) Tags::kPaiCertificateId, mPayload.mCertPai));
    SuccessOrExit(error = Update((uint8_t) Tags::kCertDeclarationId, mPayload.mCertDeclaration));

    SuccessOrExit(error = FactoryProviderImpl::UpdateData(mFactoryData));

    /* Check integrity of freshly copied data. If validation fails, OTA will be aborted
     * and factory data will be restored to the previous version. Use device instance info
     * provider getter to access the factory data provider instance. The instance is created
     * by the application, so it's easier to access it this way.*/
    provider = static_cast<FactoryProvider *>(DeviceLayer::GetDeviceInstanceInfoProvider());
    SuccessOrExit(error = ((provider != nullptr) ? CHIP_NO_ERROR : CHIP_ERROR_INVALID_ADDRESS));

    error = provider->Validate();

exit:
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to update factory data. Error: %s", ErrorStr(error));
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Factory data update finished.");
    }

    return error;
}

CHIP_ERROR OTAFactoryDataProcessor::AbortAction()
{
    ReturnErrorOnFailure(Restore());
    ReturnErrorOnFailure(FactoryProviderImpl::UpdateData(mFactoryData));

    PDM_vDeleteDataRecord(kNvmId_FactoryDataBackup);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFactoryDataProcessor::DecodeTlv()
{
    TLV::TLVReader tlvReader;
    tlvReader.Init(mAccumulator.data(), mLength);
    ReturnErrorOnFailure(tlvReader.Next(TLV::TLVType::kTLVType_Structure, TLV::AnonymousTag()));

    TLV::TLVType outerType;
    ReturnErrorOnFailure(tlvReader.EnterContainer(outerType));
    ReturnErrorOnFailure(tlvReader.Next());

    if (tlvReader.GetTag() == TLV::ContextTag((uint8_t) Tags::kDacPrivateKeyId))
    {
        ReturnErrorOnFailure(tlvReader.Get(mPayload.mCertDacKey.Emplace()));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    if (tlvReader.GetTag() == TLV::ContextTag((uint8_t) Tags::kDacCertificateId))
    {
        ReturnErrorOnFailure(tlvReader.Get(mPayload.mCertDac.Emplace()));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    if (tlvReader.GetTag() == TLV::ContextTag((uint8_t) Tags::kPaiCertificateId))
    {
        ReturnErrorOnFailure(tlvReader.Get(mPayload.mCertPai.Emplace()));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    if (tlvReader.GetTag() == TLV::ContextTag((uint8_t) Tags::kCertDeclarationId))
    {
        ReturnErrorOnFailure(tlvReader.Get(mPayload.mCertDeclaration.Emplace()));
    }

    ReturnErrorOnFailure(tlvReader.ExitContainer(outerType));

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFactoryDataProcessor::Update(uint8_t tag, Optional<ByteSpan> & optional)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    if (optional.HasValue())
    {
        error = UpdateValue(tag, optional.Value());
    }

    return error;
}

CHIP_ERROR OTAFactoryDataProcessor::Read()
{
    FactoryProvider::Header header;
    memcpy(&header, (void *) FactoryProvider::kFactoryDataStart, sizeof(FactoryProvider::Header));

    mFactoryData = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(FactoryProvider::kFactoryDataSize));
    ReturnErrorCodeIf(mFactoryData == nullptr, CHIP_FACTORY_DATA_NULL);
    memset(mFactoryData, 0, FactoryProvider::kFactoryDataSize);
    memcpy(mFactoryData, (void *) FactoryProvider::kFactoryDataStart, sizeof(FactoryProvider::Header) + header.size);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFactoryDataProcessor::Backup()
{
    ReturnErrorCodeIf(mFactoryData == nullptr, CHIP_FACTORY_DATA_NULL);

    auto status = PDM_eSaveRecordData(kNvmId_FactoryDataBackup, (void *) mFactoryData, FactoryProvider::kFactoryDataSize);
    ReturnErrorCodeIf(status != PDM_E_STATUS_OK, CHIP_FACTORY_DATA_PDM_SAVE_RECORD);
    // PDM save will do an encryption in place, so a restore is neeeded in order
    // to have the decrypted data back in the mFactoryData buffer.
    ReturnErrorOnFailure(Restore());

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFactoryDataProcessor::Restore()
{
    ReturnErrorCodeIf(mFactoryData == nullptr, CHIP_FACTORY_DATA_NULL);

    uint16_t bytesRead = 0;

    auto status =
        PDM_eReadDataFromRecord(kNvmId_FactoryDataBackup, (void *) mFactoryData, FactoryProvider::kFactoryDataSize, &bytesRead);
    ReturnErrorCodeIf(status != PDM_E_STATUS_OK, CHIP_FACTORY_DATA_PDM_READ_RECORD);

    return CHIP_NO_ERROR;
}

void OTAFactoryDataProcessor::ClearBuffer()
{
    if (mFactoryData)
    {
        memset(mFactoryData, 0, FactoryProvider::kFactoryDataSize);
        chip::Platform::MemoryFree(mFactoryData);
        mFactoryData = nullptr;
    }
}

CHIP_ERROR OTAFactoryDataProcessor::UpdateValue(uint8_t tag, ByteSpan & newValue)
{
    uint16_t oldLength               = 0;
    uint16_t newLength               = newValue.size();
    uint32_t offset                  = 0;
    FactoryProvider::Header * header = (FactoryProvider::Header *) mFactoryData;
    uint8_t * data                   = mFactoryData + sizeof(FactoryProvider::Header);

    while (offset < header->size)
    {
        memcpy(&oldLength, &data[offset + FactoryProvider::kLengthOffset], sizeof(oldLength));

        if (tag != data[offset])
        {
            offset += FactoryProvider::kValueOffset + oldLength;
            continue;
        }

        if (oldLength == newLength)
        {
            memcpy(&data[offset + FactoryProvider::kValueOffset], newValue.data(), newLength);
        }
        else
        {
            uint32_t oldEndOffset = offset + FactoryProvider::kValueOffset + oldLength;

            memcpy(&data[offset + FactoryProvider::kLengthOffset], &newLength, sizeof(newLength));
            memmove(&data[offset + FactoryProvider::kValueOffset + newLength], &data[oldEndOffset], header->size - oldEndOffset);
            memcpy(&data[offset + FactoryProvider::kValueOffset], newValue.data(), newLength);
        }

        header->size = header->size - oldLength + newLength;

        uint8_t sha256Output[SHA256_HASH_SIZE] = { 0 };
        SHA256_Hash(data, header->size, sha256Output);
        memcpy(header->hash, sha256Output, sizeof(header->hash));

        ChipLogProgress(DeviceLayer, "Value at tag %d updated successfully.", tag);
        return CHIP_NO_ERROR;
    }

    ChipLogError(DeviceLayer, "Failed to find tag %d.", tag);
    return CHIP_ERROR_NOT_FOUND;
}

} // namespace chip
