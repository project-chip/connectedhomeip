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
#include <platform/silabs/multi-ota/OTAFactoryDataProcessor.h>

namespace chip {

using namespace ::chip::DeviceLayer::Silabs;

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

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAFactoryDataProcessor::ProcessInternal(ByteSpan & block)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    ReturnErrorOnFailure(mAccumulator.Accumulate(block));
#ifdef SL_MATTER_ENABLE_OTA_ENCRYPTION
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

    SuccessOrExit(error = Update((uint8_t) FactoryTags::kDacKey, mPayload.mCertDacKey));
    SuccessOrExit(error = Update((uint8_t) FactoryTags::kDacCert, mPayload.mCertDac));
    SuccessOrExit(error = Update((uint8_t) FactoryTags::kPaiCert, mPayload.mCertPai));
    SuccessOrExit(error = Update((uint8_t) FactoryTags::kCdCert, mPayload.mCertDeclaration));

exit:
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Failed to update factory data. Error: %s", ErrorStr(error));
    }
    else
    {
        ChipLogProgress(SoftwareUpdate, "Factory data update finished.");
    }

    return error;
}

CHIP_ERROR OTAFactoryDataProcessor::FinalizeAction()
{
    ChipLogProgress(SoftwareUpdate, "Finalize Action\n");
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

    if (tlvReader.GetTag() == TLV::ContextTag((uint8_t) FactoryTags::kDacKey))
    {
        ReturnErrorOnFailure(tlvReader.Get(mPayload.mCertDacKey.Emplace()));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    if (tlvReader.GetTag() == TLV::ContextTag((uint8_t) FactoryTags::kDacCert))
    {
        ReturnErrorOnFailure(tlvReader.Get(mPayload.mCertDac.Emplace()));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    if (tlvReader.GetTag() == TLV::ContextTag((uint8_t) FactoryTags::kPaiCert))
    {
        ReturnErrorOnFailure(tlvReader.Get(mPayload.mCertPai.Emplace()));
        ReturnErrorOnFailure(tlvReader.Next());
    }

    if (tlvReader.GetTag() == TLV::ContextTag((uint8_t) FactoryTags::kCdCert))
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

CHIP_ERROR OTAFactoryDataProcessor::UpdateValue(uint8_t tag, ByteSpan & newValue)
{
    switch (tag)
    {
    case (int) FactoryTags::kDacKey:
        ChipLogProgress(SoftwareUpdate, "Set Device Attestation Key");
        return Provision::Manager::GetInstance().GetStorage().SetDeviceAttestationKey(newValue);
    case (int) FactoryTags::kDacCert:
        ChipLogProgress(SoftwareUpdate, "Set Device Attestation Cert");
        return Provision::Manager::GetInstance().GetStorage().SetDeviceAttestationCert(newValue);
    case (int) FactoryTags::kPaiCert:
        ChipLogProgress(SoftwareUpdate, "Set Product Attestionation Intermediate Cert");
        return Provision::Manager::GetInstance().GetStorage().SetProductAttestationIntermediateCert(newValue);
    case (int) FactoryTags::kCdCert:
        ChipLogProgress(SoftwareUpdate, "Set Certification Declaration");
        return Provision::Manager::GetInstance().GetStorage().SetCertificationDeclaration(newValue);
    }

    ChipLogError(DeviceLayer, "Failed to find tag %d.", tag);
    return CHIP_ERROR_NOT_FOUND;
}

} // namespace chip
