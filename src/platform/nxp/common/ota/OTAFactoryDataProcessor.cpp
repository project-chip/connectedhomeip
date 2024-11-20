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

#include <app/server/Server.h>
#include <lib/core/TLV.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/nxp/common/ota/OTAFactoryDataProcessor.h>

namespace chip {

CHIP_ERROR OTAFactoryDataProcessor::Init()
{
    mAccumulator.Init(mLength);
    mFactoryDataDriver = &chip::DeviceLayer::FactoryDataDrv();

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFactoryDataProcessor::Clear()
{
    OTATlvProcessor::ClearInternal();
    mAccumulator.Clear();
    mPayload.Clear();
    mFactoryDataDriver->ClearRamBackup();

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
    CHIP_ERROR error = CHIP_NO_ERROR;
    FactoryProvider * provider;

    ReturnErrorOnFailure(mFactoryDataDriver->InitRamBackup());
    ReturnErrorOnFailure(mFactoryDataDriver->BackupFactoryData());

    SuccessOrExit(error = Update((uint8_t) Tags::kDacPrivateKeyId, mPayload.mCertDacKey));
    SuccessOrExit(error = Update((uint8_t) Tags::kDacCertificateId, mPayload.mCertDac));
    SuccessOrExit(error = Update((uint8_t) Tags::kPaiCertificateId, mPayload.mCertPai));
    SuccessOrExit(error = Update((uint8_t) Tags::kCertDeclarationId, mPayload.mCertDeclaration));

    SuccessOrExit(error = mFactoryDataDriver->UpdateFactoryData());

    provider = &chip::DeviceLayer::FactoryDataPrvd();
    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_INTERNAL);

    SuccessOrExit(error = provider->PreResetCheck());

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
    CHIP_ERROR error = CHIP_NO_ERROR;
    ReturnErrorOnFailure(mFactoryDataDriver->ReadBackupInRam());
    ReturnErrorOnFailure(mFactoryDataDriver->UpdateFactoryData());

    error = mFactoryDataDriver->DeleteBackup();
    ReturnErrorOnFailure(error);

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
        error = mFactoryDataDriver->UpdateValueInRam(tag, optional.Value());
    }

    return error;
}

} // namespace chip
