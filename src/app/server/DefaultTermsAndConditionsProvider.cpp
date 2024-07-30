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
#include <platform/KeyValueStoreManager.h>

namespace {
static constexpr chip::TLV::Tag kAcceptedAcknowledgementsTag        = chip::TLV::ContextTag(1);
static constexpr chip::TLV::Tag kAcceptedAcknowledgementsVersionTag = chip::TLV::ContextTag(2);
static constexpr size_t kEstimatedTlvBufferSize = chip::TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(uint16_t));
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

    tlvReader.Init(buffer);
    ReturnErrorOnFailure(tlvReader.Next(chip::TLV::kTLVType_Structure, chip::TLV::AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(tlvContainer));
    ReturnErrorOnFailure(tlvReader.Next());
    ReturnErrorOnFailure(tlvReader.Expect(kAcceptedAcknowledgementsTag));
    ReturnErrorOnFailure(tlvReader.Get(acknowledgements));
    ReturnErrorOnFailure(tlvReader.Next());
    ReturnErrorOnFailure(tlvReader.Expect(kAcceptedAcknowledgementsVersionTag));
    ReturnErrorOnFailure(tlvReader.Get(acknowledgementsVersion));
    ReturnErrorOnFailure(tlvReader.ExitContainer(tlvContainer));

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
    ReturnErrorOnFailure(tlvWriter.Put(kAcceptedAcknowledgementsTag, inAcceptedAcknowledgementsValue));
    ReturnErrorOnFailure(tlvWriter.Put(kAcceptedAcknowledgementsVersionTag, inAcceptedAcknowledgementsVersionValue));
    ReturnErrorOnFailure(tlvWriter.EndContainer(tlvContainer));
    ReturnErrorOnFailure(tlvWriter.Finalize());

    const chip::StorageKeyName storageKey = DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();
    ReturnErrorOnFailure(mPersistentStorageDelegate->SyncSetKeyValue(storageKey.KeyName(), buffer, sizeof(buffer)));

    return CHIP_NO_ERROR;
}
