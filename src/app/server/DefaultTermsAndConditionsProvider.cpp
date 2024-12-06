/*
 *
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
 *  distributed under the License is distributed on an "AS IS" BASIS, *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing permissions and limitations under the License.
 */

#include "DefaultTermsAndConditionsProvider.h"
#include "TermsAndConditionsProvider.h"

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
constexpr uint8_t kSerializationSchemaMinimumVersion         = 1;
constexpr uint8_t kSerializationSchemaCurrentVersion         = 1;

constexpr size_t kEstimatedTlvBufferSize = chip::TLV::EstimateStructOverhead(sizeof(uint8_t),  // SerializationVersion
                                                                             sizeof(uint16_t), // AcceptedAcknowledgements
                                                                             sizeof(uint16_t)  // AcceptedAcknowledgementsVersion
                                                                             ) *
    4; // Extra space for rollback compatibility
} // namespace

CHIP_ERROR chip::app::DefaultTermsAndConditionsStorageDelegate::Init(PersistentStorageDelegate * const inPersistentStorageDelegate)
{
    VerifyOrReturnValue(nullptr != inPersistentStorageDelegate, CHIP_ERROR_INVALID_ARGUMENT);

    mStorageDelegate = inPersistentStorageDelegate;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsStorageDelegate::Delete()
{
    VerifyOrReturnValue(nullptr != mStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    const chip::StorageKeyName kStorageKey = chip::DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();
    VerifyOrReturnValue(CHIP_NO_ERROR == mStorageDelegate->SyncDeleteKeyValue(kStorageKey.KeyName()), CHIP_ERROR_INTERNAL);

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
    CHIP_ERROR err                         = mStorageDelegate->SyncGetKeyValue(kStorageKey.KeyName(), &buffer, bufferSize);
    if (CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == err)
    {
        outTermsAndConditions.ClearValue();
        return CHIP_NO_ERROR;
    }

    VerifyOrReturnValue(CHIP_NO_ERROR == err, CHIP_ERROR_INTERNAL);

    tlvReader.Init(buffer, bufferSize);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvReader.Next(chip::TLV::kTLVType_Structure, chip::TLV::AnonymousTag()),
                        CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvReader.EnterContainer(tlvContainer), CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvReader.Next(kSerializationVersionTag), CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvReader.Get(serializationVersion), CHIP_ERROR_INTERNAL);

    if (serializationVersion < kSerializationSchemaMinimumVersion)
    {
        ChipLogError(AppServer, "The terms and conditions datastore schema (%hhu) is newer than oldest compatible schema (%hhu)",
                     serializationVersion, kSerializationSchemaMinimumVersion);
        return CHIP_ERROR_INTERNAL;
    }

    if (serializationVersion != kSerializationSchemaCurrentVersion)
    {
        ChipLogDetail(AppServer, "The terms and conditions datastore schema (%hhu) differs from current schema (%hhu)",
                      serializationVersion, kSerializationSchemaCurrentVersion);
    }

    VerifyOrReturnValue(CHIP_NO_ERROR == tlvReader.Next(kAcceptedAcknowledgementsTag), CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvReader.Get(acknowledgements), CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvReader.Next(kAcceptedAcknowledgementsVersionTag), CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvReader.Get(acknowledgementsVersion), CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvReader.ExitContainer(tlvContainer), CHIP_ERROR_INTERNAL);

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
    VerifyOrReturnValue(CHIP_NO_ERROR ==
                            tlvWriter.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, tlvContainer),
                        CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvWriter.Put(kSerializationVersionTag, kSerializationSchemaCurrentVersion),
                        CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvWriter.Put(kAcceptedAcknowledgementsTag, inTermsAndConditions.GetValue()),
                        CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvWriter.Put(kAcceptedAcknowledgementsVersionTag, inTermsAndConditions.GetVersion()),
                        CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvWriter.EndContainer(tlvContainer), CHIP_ERROR_INTERNAL);
    VerifyOrReturnValue(CHIP_NO_ERROR == tlvWriter.Finalize(), CHIP_ERROR_INTERNAL);

    const chip::StorageKeyName kStorageKey = chip::DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();
    VerifyOrReturnValue(
        CHIP_NO_ERROR ==
            mStorageDelegate->SyncSetKeyValue(kStorageKey.KeyName(), buffer, static_cast<uint16_t>(tlvWriter.GetLengthWritten())),
        CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::Init(
    TermsAndConditionsStorageDelegate * const inStorageDelegate,
    const chip::Optional<chip::app::TermsAndConditions> & inRequiredTermsAndConditions)
{
    VerifyOrReturnValue(nullptr != inStorageDelegate, CHIP_ERROR_INVALID_ARGUMENT);

    mTermsAndConditionsStorageDelegate = inStorageDelegate;
    mRequiredAcknowledgements          = inRequiredTermsAndConditions;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::CheckAcceptance(const Optional<TermsAndConditions> & inTermsAndConditions,
                                                                         TermsAndConditionsState & outState) const
{
    VerifyOrReturnValue(nullptr != mTermsAndConditionsStorageDelegate, CHIP_ERROR_INVALID_ARGUMENT);

    // No validation checks required if no required terms and conditions
    if (!mRequiredAcknowledgements.HasValue())
    {
        ChipLogProgress(AppServer, "No terms and conditions required");
        outState = TermsAndConditionsState::OK;
        return CHIP_NO_ERROR;
    }

    // Validate if we have received any terms and conditions acceptance
    if (!inTermsAndConditions.HasValue())
    {
        ChipLogError(AppServer, "No terms and conditions have been accepted");
        outState = TermsAndConditionsState::TC_ACKNOWLEDGEMENTS_NOT_RECEIVED;
        return CHIP_NO_ERROR;
    }

    const TermsAndConditions requiredTermsAndConditions = mRequiredAcknowledgements.Value();
    const TermsAndConditions termsAndConditionsToCheck  = inTermsAndConditions.Value();

    // Validate the accepted version first...
    if (requiredTermsAndConditions.GetVersion() > termsAndConditionsToCheck.GetVersion())
    {
        ChipLogError(AppServer, "Minimum terms and conditions version, 0x%04x, has not been accepted",
                     requiredTermsAndConditions.GetVersion());
        outState = TermsAndConditionsState::TC_MIN_VERSION_NOT_MET;
        return CHIP_NO_ERROR;
    }

    // Validate the accepted bits second...
    if (requiredTermsAndConditions.GetValue() != (requiredTermsAndConditions.GetValue() & termsAndConditionsToCheck.GetValue()))
    {
        ChipLogError(AppServer, "Required terms and conditions, 0x%04x, have not been accepted",
                     requiredTermsAndConditions.GetValue());
        outState = TermsAndConditionsState::REQUIRED_TC_NOT_ACCEPTED;
        return CHIP_NO_ERROR;
    }

    // All validation check succeeded...
    ChipLogProgress(AppServer, "Required terms and conditions, 0x%04x, have been accepted", requiredTermsAndConditions.GetValue());
    outState = TermsAndConditionsState::OK;
    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::CommitAcceptance()
{
    VerifyOrReturnValue(nullptr != mTermsAndConditionsStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    if (!mTemporalAcceptance.HasValue())
    {
        ChipLogError(AppServer, "No terms and conditions to commit");
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR err = mTermsAndConditionsStorageDelegate->Set(mTemporalAcceptance.Value());
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Failed storage delegate Set(): %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_ERROR_INTERNAL;
    }

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
    if (CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == err)
    {
        ChipLogError(AppServer, "No terms and conditions have been accepted");
        return CHIP_NO_ERROR;
    }

    // If the storage delegate returns an error, other than "value not found" then there was an unexpected datastore failure
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Failed storage delegate Get(): %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::GetRequirements(Optional<TermsAndConditions> & outTermsAndConditions) const
{
    VerifyOrReturnValue(nullptr != mTermsAndConditionsStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    outTermsAndConditions = mRequiredAcknowledgements;

    return CHIP_NO_ERROR;
}

CHIP_ERROR
chip::app::DefaultTermsAndConditionsProvider::GetUpdateAcceptanceDeadline(Optional<uint32_t> & outUpdateAcceptanceDeadline) const
{
    VerifyOrReturnValue(nullptr != mTermsAndConditionsStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    outUpdateAcceptanceDeadline = Optional<uint32_t>();

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::ResetAcceptance()
{
    VerifyOrReturnValue(nullptr != mTermsAndConditionsStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    CHIP_ERROR err = mTermsAndConditionsStorageDelegate->Delete();
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Failed storage delegate Delete(): %" CHIP_ERROR_FORMAT, err.Format());
    }

    return RevertAcceptance();
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::RevertAcceptance()
{
    VerifyOrReturnValue(nullptr != mTermsAndConditionsStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    mTemporalAcceptance.ClearValue();

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::SetAcceptance(const Optional<TermsAndConditions> & inTermsAndConditions)
{
    VerifyOrReturnValue(nullptr != mTermsAndConditionsStorageDelegate, CHIP_ERROR_UNINITIALIZED);

    TermsAndConditionsState termsAndConditionsState = TermsAndConditionsState::OK;

    CHIP_ERROR err = CheckAcceptance(inTermsAndConditions, termsAndConditionsState);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Failed to check acceptance state: %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    mTemporalAcceptance = inTermsAndConditions;
    return CHIP_NO_ERROR;
}
