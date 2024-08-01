/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "DefaultTermsAndConditionsProvider.h"

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>

namespace {
constexpr chip::TLV::Tag kSerializationVersionTag            = chip::TLV::ContextTag(1);
constexpr chip::TLV::Tag kAcceptedAcknowledgementsTag        = chip::TLV::ContextTag(2);
constexpr chip::TLV::Tag kAcceptedAcknowledgementsVersionTag = chip::TLV::ContextTag(3);
constexpr uint8_t kSerializationVersion                      = 1;
constexpr size_t kEstimatedTlvBufferSize = chip::TLV::EstimateStructOverhead(sizeof(uint8_t), sizeof(uint16_t), sizeof(uint16_t));
}; // namespace

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::Init(chip::PersistentStorageDelegate * const inPersistentStorageDelegate,
                                                              uint16_t inRequiredAcknowledgementsValue,
                                                              uint16_t inRequiredAcknowledgementsVersionValue)
{
    VerifyOrReturnError(nullptr != inPersistentStorageDelegate, CHIP_ERROR_INVALID_ARGUMENT);

    mPersistentStorageDelegate            = inPersistentStorageDelegate;
    mRequiredAcknowledgementsValue        = inRequiredAcknowledgementsValue;
    mRequiredAcknowledgementsVersionValue = inRequiredAcknowledgementsVersionValue;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::ClearAcceptance()
{
    VerifyOrReturnError(nullptr != mPersistentStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    const chip::StorageKeyName storageKey = DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();
    ReturnErrorOnFailure(mPersistentStorageDelegate->SyncDeleteKeyValue(storageKey.KeyName()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::GetAcceptance(uint16_t & outAcknowledgementsValue,
                                                                       uint16_t & outAcknowledgementsVersionValue) const
{
    uint8_t serializationVersion     = 0;
    uint16_t acknowledgements        = 0;
    uint16_t acknowledgementsVersion = 0;

    chip::TLV::TLVReader tlvReader;
    chip::TLV::TLVType tlvContainer;

    uint8_t buffer[kEstimatedTlvBufferSize] = { 0 };
    uint16_t bufferSize                     = sizeof(buffer);

    VerifyOrReturnError(nullptr != mPersistentStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    const chip::StorageKeyName storageKey = DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();
    CHIP_ERROR err                        = mPersistentStorageDelegate->SyncGetKeyValue(storageKey.KeyName(), &buffer, bufferSize);
    if (CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == err)
    {
        outAcknowledgementsValue        = acknowledgements;
        outAcknowledgementsVersionValue = acknowledgementsVersion;

        return CHIP_NO_ERROR;
    }

    VerifyOrReturnError(CHIP_NO_ERROR == err, err);

    tlvReader.Init(buffer, bufferSize);
    ReturnErrorOnFailure(tlvReader.Next(chip::TLV::kTLVType_Structure, chip::TLV::AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(tlvContainer));
    ReturnErrorOnFailure(tlvReader.Next(kSerializationVersionTag));
    ReturnErrorOnFailure(tlvReader.Get(serializationVersion));
    ReturnErrorOnFailure(tlvReader.Next(kAcceptedAcknowledgementsTag));
    ReturnErrorOnFailure(tlvReader.Get(acknowledgements));
    ReturnErrorOnFailure(tlvReader.Next(kAcceptedAcknowledgementsVersionTag));
    ReturnErrorOnFailure(tlvReader.Get(acknowledgementsVersion));
    ReturnErrorOnFailure(tlvReader.ExitContainer(tlvContainer));

    if (kSerializationVersion != serializationVersion)
    {
        return CHIP_ERROR_VERSION_MISMATCH;
    }

    outAcknowledgementsValue        = acknowledgements;
    outAcknowledgementsVersionValue = acknowledgementsVersion;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::GetRequirements(uint16_t & outAcknowledgementsValue,
                                                                         uint16_t & outAcknowledgementsVersionValue) const
{
    outAcknowledgementsValue        = mRequiredAcknowledgementsValue;
    outAcknowledgementsVersionValue = mRequiredAcknowledgementsVersionValue;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::SetAcceptance(uint16_t inAcceptedAcknowledgementsValue,
                                                                       uint16_t inAcceptedAcknowledgementsVersionValue)
{
    uint8_t buffer[kEstimatedTlvBufferSize] = { 0 };
    chip::TLV::TLVWriter tlvWriter;
    chip::TLV::TLVType tlvContainer;

    VerifyOrReturnError(nullptr != mPersistentStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    tlvWriter.Init(buffer, sizeof(buffer));
    ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, tlvContainer));
    ReturnErrorOnFailure(tlvWriter.Put(kSerializationVersionTag, kSerializationVersion));
    ReturnErrorOnFailure(tlvWriter.Put(kAcceptedAcknowledgementsTag, inAcceptedAcknowledgementsValue));
    ReturnErrorOnFailure(tlvWriter.Put(kAcceptedAcknowledgementsVersionTag, inAcceptedAcknowledgementsVersionValue));
    ReturnErrorOnFailure(tlvWriter.EndContainer(tlvContainer));
    ReturnErrorOnFailure(tlvWriter.Finalize());
    uint32_t lengthWritten = tlvWriter.GetLengthWritten();
    VerifyOrReturnError(CanCastTo<uint16_t>(lengthWritten), CHIP_ERROR_BUFFER_TOO_SMALL);

    const chip::StorageKeyName storageKey = DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();
    ReturnErrorOnFailure(
        mPersistentStorageDelegate->SyncSetKeyValue(storageKey.KeyName(), buffer, static_cast<uint16_t>(lengthWritten)));

    return CHIP_NO_ERROR;
}
