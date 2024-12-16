/*
 *  Copyright (c) 2024 Project CHIP Authors
 *  All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "DefaultTermsAndConditionsProvider.h"
#include "TermsAndConditionsProvider.h"

#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>
#include <lib/core/TLVTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <protocols/Protocols.h>
#include <protocols/interaction_model/StatusCode.h>

namespace {
constexpr chip::TLV::Tag kSerializationVersionTag            = chip::TLV::ContextTag(1);
constexpr chip::TLV::Tag kAcceptedAcknowledgementsTag        = chip::TLV::ContextTag(2);
constexpr chip::TLV::Tag kAcceptedAcknowledgementsVersionTag = chip::TLV::ContextTag(3);
constexpr uint8_t kSerializationSchemaMinimumVersion         = 1;
constexpr uint8_t kSerializationSchemaCurrentVersion         = 1;

constexpr size_t kEstimatedTlvBufferSize = chip::TLV::EstimateStructOverhead(sizeof(uint8_t),  // SerializationVersion
                                                                             sizeof(uint16_t), // AcceptedAcknowledgements
                                                                             sizeof(uint16_t)  // AcceptedAcknowledgementsVersion
                                                                             ) *
    4; // Extra space for rollback compatibility
} // namespace

CHIP_ERROR chip::app::DefaultTermsAndConditionsStorageDelegate::Init(PersistentStorageDelegate * inPersistentStorageDelegate)
{
    VerifyOrReturnValue(nullptr != inPersistentStorageDelegate, CHIP_ERROR_INVALID_ARGUMENT);

    mStorageDelegate = inPersistentStorageDelegate;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsStorageDelegate::Delete()
{
    VerifyOrReturnValue(nullptr != mStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    const chip::StorageKeyName kStorageKey = chip::DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();
    ReturnErrorOnFailure(mStorageDelegate->SyncDeleteKeyValue(kStorageKey.KeyName()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsStorageDelegate::Get(Optional<TermsAndConditions> & outTermsAndConditions)
{
    VerifyOrReturnValue(nullptr != mStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    uint8_t serializationVersion     = 0;
    uint16_t acknowledgements        = 0;
    uint16_t acknowledgementsVersion = 0;

    chip::TLV::TLVReader tlvReader;
    chip::TLV::TLVType tlvContainer;

    uint8_t buffer[kEstimatedTlvBufferSize] = { 0 };
    uint16_t bufferSize                     = sizeof(buffer);

    const chip::StorageKeyName kStorageKey = chip::DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();

    CHIP_ERROR err = mStorageDelegate->SyncGetKeyValue(kStorageKey.KeyName(), &buffer, bufferSize);
    VerifyOrReturnValue(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == err || CHIP_NO_ERROR == err, err);

    if (CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == err)
    {
        outTermsAndConditions.ClearValue();
        return CHIP_NO_ERROR;
    }

    tlvReader.Init(buffer, bufferSize);
    ReturnErrorOnFailure(tlvReader.Next(chip::TLV::kTLVType_Structure, chip::TLV::AnonymousTag()));
    ReturnErrorOnFailure(tlvReader.EnterContainer(tlvContainer));
    ReturnErrorOnFailure(tlvReader.Next(kSerializationVersionTag));
    ReturnErrorOnFailure(tlvReader.Get(serializationVersion));

    if (serializationVersion < kSerializationSchemaMinimumVersion)
    {
        ChipLogError(AppServer, "The terms and conditions datastore schema (%hhu) is newer than oldest compatible schema (%hhu)",
                     serializationVersion, kSerializationSchemaMinimumVersion);
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    if (serializationVersion != kSerializationSchemaCurrentVersion)
    {
        ChipLogDetail(AppServer, "The terms and conditions datastore schema (%hhu) differs from current schema (%hhu)",
                      serializationVersion, kSerializationSchemaCurrentVersion);
    }

    ReturnErrorOnFailure(tlvReader.Next(kAcceptedAcknowledgementsTag));
    ReturnErrorOnFailure(tlvReader.Get(acknowledgements));
    ReturnErrorOnFailure(tlvReader.Next(kAcceptedAcknowledgementsVersionTag));
    ReturnErrorOnFailure(tlvReader.Get(acknowledgementsVersion));
    ReturnErrorOnFailure(tlvReader.ExitContainer(tlvContainer));

    outTermsAndConditions = Optional<TermsAndConditions>(TermsAndConditions(acknowledgements, acknowledgementsVersion));

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsStorageDelegate::Set(const TermsAndConditions & inTermsAndConditions)
{
    uint8_t buffer[kEstimatedTlvBufferSize] = { 0 };
    chip::TLV::TLVWriter tlvWriter;
    chip::TLV::TLVType tlvContainer;

    VerifyOrReturnValue(nullptr != mStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    tlvWriter.Init(buffer);
    ReturnErrorOnFailure(tlvWriter.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, tlvContainer));
    ReturnErrorOnFailure(tlvWriter.Put(kSerializationVersionTag, kSerializationSchemaCurrentVersion));
    ReturnErrorOnFailure(tlvWriter.Put(kAcceptedAcknowledgementsTag, inTermsAndConditions.GetValue()));
    ReturnErrorOnFailure(tlvWriter.Put(kAcceptedAcknowledgementsVersionTag, inTermsAndConditions.GetVersion()));
    ReturnErrorOnFailure(tlvWriter.EndContainer(tlvContainer));
    ReturnErrorOnFailure(tlvWriter.Finalize());

    const chip::StorageKeyName kStorageKey = chip::DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();
    ReturnErrorOnFailure(
        mStorageDelegate->SyncSetKeyValue(kStorageKey.KeyName(), buffer, static_cast<uint16_t>(tlvWriter.GetLengthWritten())));

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::Init(
    TermsAndConditionsStorageDelegate * inStorageDelegate,
    const chip::Optional<chip::app::TermsAndConditions> & inRequiredTermsAndConditions)
{
    VerifyOrReturnValue(nullptr != inStorageDelegate, CHIP_ERROR_INVALID_ARGUMENT);

    mTermsAndConditionsStorageDelegate = inStorageDelegate;
    mRequiredAcknowledgements          = inRequiredTermsAndConditions;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::CommitAcceptance()
{
    VerifyOrReturnValue(nullptr != mTermsAndConditionsStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    // No terms and conditions to commit
    VerifyOrReturnValue(mTemporalAcceptance.HasValue(), CHIP_NO_ERROR);

    ReturnErrorOnFailure(mTermsAndConditionsStorageDelegate->Set(mTemporalAcceptance.Value()));
    ChipLogProgress(AppServer, "Terms and conditions have been committed");
    mTemporalAcceptance.ClearValue();

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::GetAcceptance(Optional<TermsAndConditions> & outTermsAndConditions) const
{
    VerifyOrReturnValue(nullptr != mTermsAndConditionsStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    // Return the in-memory acceptance state
    if (mTemporalAcceptance.HasValue())
    {
        outTermsAndConditions = mTemporalAcceptance;
        return CHIP_NO_ERROR;
    }

    // Otherwise, try to get the persisted acceptance state
    CHIP_ERROR err = mTermsAndConditionsStorageDelegate->Get(outTermsAndConditions);
    VerifyOrReturnValue(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == err || CHIP_NO_ERROR == err, err);

    if (CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == err)
    {
        ChipLogError(AppServer, "No terms and conditions have been accepted");
        outTermsAndConditions.ClearValue();
        return CHIP_NO_ERROR;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::GetAcknowledgementsRequired(bool & outAcknowledgementsRequired) const
{
    Optional<TermsAndConditions> requiredTermsAndConditionsMaybe;
    ReturnErrorOnFailure(GetRequirements(requiredTermsAndConditionsMaybe));

    if (!requiredTermsAndConditionsMaybe.HasValue())
    {
        outAcknowledgementsRequired = false;
        return CHIP_NO_ERROR;
    }

    Optional<TermsAndConditions> acceptedTermsAndConditionsMaybe;
    ReturnErrorOnFailure(GetAcceptance(acceptedTermsAndConditionsMaybe));

    if (!acceptedTermsAndConditionsMaybe.HasValue())
    {
        outAcknowledgementsRequired = true;
        return CHIP_NO_ERROR;
    }

    TermsAndConditions requiredTermsAndConditions = requiredTermsAndConditionsMaybe.Value();
    TermsAndConditions acceptedTermsAndConditions = acceptedTermsAndConditionsMaybe.Value();
    outAcknowledgementsRequired                   = requiredTermsAndConditions.Validate(acceptedTermsAndConditions);
    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::GetRequirements(Optional<TermsAndConditions> & outTermsAndConditions) const
{
    outTermsAndConditions = mRequiredAcknowledgements;
    return CHIP_NO_ERROR;
}

CHIP_ERROR
chip::app::DefaultTermsAndConditionsProvider::GetUpdateAcceptanceDeadline(Optional<uint32_t> & outUpdateAcceptanceDeadline) const
{
    // No-op stub implementation. This feature is not implemented in this default implementation.
    outUpdateAcceptanceDeadline = Optional<uint32_t>();
    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::ResetAcceptance()
{
    VerifyOrReturnValue(nullptr != mTermsAndConditionsStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    (void) mTermsAndConditionsStorageDelegate->Delete();
    ReturnErrorOnFailure(RevertAcceptance());
    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::RevertAcceptance()
{
    mTemporalAcceptance.ClearValue();
    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::SetAcceptance(const Optional<TermsAndConditions> & inTermsAndConditions)
{
    mTemporalAcceptance = inTermsAndConditions;
    return CHIP_NO_ERROR;
}
