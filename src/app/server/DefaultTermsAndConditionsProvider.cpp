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

#define VerifyNoErrorOrReturnValue(expr, code, ...) VerifyOrReturnValue(CHIP_NO_ERROR == expr, code, ##__VA_ARGS__)
#define VerifyNoErrorOrReturnInternal(expr, ...) VerifyNoErrorOrReturnValue(expr, CHIP_ERROR_INTERNAL, ##__VA_ARGS__)
#define VerifyOrReturnInvalidArgument(expr, ...) VerifyOrReturnValue(expr, CHIP_ERROR_INVALID_ARGUMENT, ##__VA_ARGS__)
#define VerifyOrReturnUninitialized(expr, ...) VerifyOrReturnValue(expr, CHIP_ERROR_UNINITIALIZED, ##__VA_ARGS__)
#define VerifyOrReturnInternal(expr, ...) VerifyOrReturnValue(expr, CHIP_ERROR_INTERNAL, ##__VA_ARGS__)
#define VerifyNotNullOrReturnInvalidArgument(expr, ...)                                                                            \
    VerifyOrReturnValue(nullptr != expr, CHIP_ERROR_INVALID_ARGUMENT, ##__VA_ARGS__)
#define VerifyNotNullOrReturnUninitialized(expr, ...) VerifyOrReturnValue(nullptr != expr, CHIP_ERROR_UNINITIALIZED, ##__VA_ARGS__)

namespace {
constexpr chip::TLV::Tag kSerializationVersionTag            = chip::TLV::ContextTag(1);
constexpr chip::TLV::Tag kAcceptedAcknowledgementsTag        = chip::TLV::ContextTag(2);
constexpr chip::TLV::Tag kAcceptedAcknowledgementsVersionTag = chip::TLV::ContextTag(3);
constexpr uint8_t kSerializationVersion                      = 1;

constexpr size_t kEstimatedTlvBufferSize = chip::TLV::EstimateStructOverhead(sizeof(uint8_t),  // SerializationVersion
                                                                             sizeof(uint16_t), // AcceptedAcknowledgements
                                                                             sizeof(uint16_t)  // AcceptedAcknowledgementsVersion
                                                                             ) *
    4; // Extra space for rollback compatibility
} // namespace

CHIP_ERROR chip::app::DefaultTermsAndConditionsStorageDelegate::Init(PersistentStorageDelegate * const inPersistentStorageDelegate)
{
    VerifyNotNullOrReturnInvalidArgument(inPersistentStorageDelegate);

    mStorageDelegate = inPersistentStorageDelegate;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsStorageDelegate::Delete()
{
    VerifyNotNullOrReturnUninitialized(mStorageDelegate);

    const chip::StorageKeyName kStorageKey = chip::DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();
    VerifyNoErrorOrReturnInternal(mStorageDelegate->SyncDeleteKeyValue(kStorageKey.KeyName()));

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsStorageDelegate::Get(Optional<TermsAndConditions> & outTermsAndConditions)
{
    VerifyNotNullOrReturnUninitialized(mStorageDelegate);

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

    VerifyNoErrorOrReturnInternal(err);

    tlvReader.Init(buffer, bufferSize);
    VerifyNoErrorOrReturnInternal(tlvReader.Next(chip::TLV::kTLVType_Structure, chip::TLV::AnonymousTag()));
    VerifyNoErrorOrReturnInternal(tlvReader.EnterContainer(tlvContainer));
    VerifyNoErrorOrReturnInternal(tlvReader.Next(kSerializationVersionTag));
    VerifyNoErrorOrReturnInternal(tlvReader.Get(serializationVersion));
    VerifyNoErrorOrReturnInternal(tlvReader.Next(kAcceptedAcknowledgementsTag));
    VerifyNoErrorOrReturnInternal(tlvReader.Get(acknowledgements));
    VerifyNoErrorOrReturnInternal(tlvReader.Next(kAcceptedAcknowledgementsVersionTag));
    VerifyNoErrorOrReturnInternal(tlvReader.Get(acknowledgementsVersion));
    VerifyNoErrorOrReturnInternal(tlvReader.ExitContainer(tlvContainer));

    // Only v1 serialization format is supported
    VerifyOrReturnInternal(kSerializationVersion == serializationVersion);

    outTermsAndConditions = Optional<TermsAndConditions>(TermsAndConditions{
        .value   = acknowledgements,
        .version = acknowledgementsVersion,
    });

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsStorageDelegate::Set(const TermsAndConditions & inTermsAndConditions)
{
    uint8_t buffer[kEstimatedTlvBufferSize] = { 0 };
    chip::TLV::TLVWriter tlvWriter;
    chip::TLV::TLVType tlvContainer;

    VerifyNotNullOrReturnUninitialized(mStorageDelegate);

    tlvWriter.Init(buffer, sizeof(buffer));
    VerifyNoErrorOrReturnInternal(tlvWriter.StartContainer(chip::TLV::AnonymousTag(), chip::TLV::kTLVType_Structure, tlvContainer));
    VerifyNoErrorOrReturnInternal(tlvWriter.Put(kSerializationVersionTag, kSerializationVersion));
    VerifyNoErrorOrReturnInternal(tlvWriter.Put(kAcceptedAcknowledgementsTag, inTermsAndConditions.value));
    VerifyNoErrorOrReturnInternal(tlvWriter.Put(kAcceptedAcknowledgementsVersionTag, inTermsAndConditions.version));
    VerifyNoErrorOrReturnInternal(tlvWriter.EndContainer(tlvContainer));
    VerifyNoErrorOrReturnInternal(tlvWriter.Finalize());

    uint32_t lengthWritten = tlvWriter.GetLengthWritten();
    VerifyOrReturnInternal(CanCastTo<uint16_t>(lengthWritten));
    VerifyOrReturnInternal(lengthWritten <= kEstimatedTlvBufferSize);

    const chip::StorageKeyName kStorageKey = chip::DefaultStorageKeyAllocator::TermsAndConditionsAcceptance();
    VerifyNoErrorOrReturnInternal(
        mStorageDelegate->SyncSetKeyValue(kStorageKey.KeyName(), buffer, static_cast<uint16_t>(lengthWritten)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::Init(
    TermsAndConditionsStorageDelegate * const inStorageDelegate,
    const chip::Optional<chip::app::TermsAndConditions> & inRequiredTermsAndConditions)
{
    VerifyNotNullOrReturnInvalidArgument(inStorageDelegate);

    mTermsAndConditionsStorageDelegate = inStorageDelegate;
    mRequiredAcknowledgements          = inRequiredTermsAndConditions;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::CheckAcceptance(const Optional<TermsAndConditions> & inTermsAndConditions,
                                                                         TermsAndConditionsState & outState) const
{
    VerifyNotNullOrReturnInvalidArgument(mTermsAndConditionsStorageDelegate);

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
    if (requiredTermsAndConditions.version > termsAndConditionsToCheck.version)
    {
        ChipLogError(AppServer, "Minimum terms and conditions version, 0x%04x, has not been accepted",
                     requiredTermsAndConditions.version);
        outState = TermsAndConditionsState::TC_MIN_VERSION_NOT_MET;
        return CHIP_NO_ERROR;
    }

    // Validate the accepted bits second...
    if (requiredTermsAndConditions.value != (requiredTermsAndConditions.value & termsAndConditionsToCheck.value))
    {
        ChipLogError(AppServer, "Required terms and conditions, 0x%04x, have not been accepted", requiredTermsAndConditions.value);
        outState = TermsAndConditionsState::REQUIRED_TC_NOT_ACCEPTED;
        return CHIP_NO_ERROR;
    }

    // All validation check succeeded...
    ChipLogProgress(AppServer, "Required terms and conditions, 0x%04x, have been accepted", requiredTermsAndConditions.value);
    outState = TermsAndConditionsState::OK;
    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::CommitAcceptance()
{
    VerifyNotNullOrReturnUninitialized(mTermsAndConditionsStorageDelegate);

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
    VerifyNotNullOrReturnUninitialized(mTermsAndConditionsStorageDelegate);

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
    VerifyNotNullOrReturnUninitialized(mTermsAndConditionsStorageDelegate);

    outTermsAndConditions = mRequiredAcknowledgements;

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::ResetAcceptance()
{
    VerifyNotNullOrReturnUninitialized(mTermsAndConditionsStorageDelegate);

    CHIP_ERROR err = mTermsAndConditionsStorageDelegate->Delete();
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Failed storage delegate Delete(): %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_ERROR_INTERNAL;
    }

    return RevertAcceptance();
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::RevertAcceptance()
{
    VerifyNotNullOrReturnUninitialized(mTermsAndConditionsStorageDelegate);

    mTemporalAcceptance.ClearValue();

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::app::DefaultTermsAndConditionsProvider::SetAcceptance(const Optional<TermsAndConditions> & inTermsAndConditions)
{
    VerifyNotNullOrReturnUninitialized(mTermsAndConditionsStorageDelegate);

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
