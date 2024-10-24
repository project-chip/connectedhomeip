/*
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
#include <app/reporting/Read-Checked.h>

#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/reporting/Read-DataModel.h>
#include <app/reporting/Read-Ember.h>
#include <app/util/MatterCallbacks.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/StringBuilder.h>

namespace chip {
namespace app {
namespace reporting {
namespace CheckedImpl {
namespace {

using DataModel::ActionReturnStatus;

/// Checkpoints and saves the state (including error state) for a
/// AttributeReportIBs::Builder
class ScopedAttributeReportIBsBuilderState
{
public:
    ScopedAttributeReportIBsBuilderState(AttributeReportIBs::Builder & builder) : mBuilder(builder), mError(mBuilder.GetError())
    {
        mBuilder.Checkpoint(mCheckpoint);
    }

    ~ScopedAttributeReportIBsBuilderState()
    {
        mBuilder.Rollback(mCheckpoint);
        mBuilder.ResetError(mError);
    }

private:
    AttributeReportIBs::Builder & mBuilder;
    chip::TLV::TLVWriter mCheckpoint;
    CHIP_ERROR mError;
};

} // namespace

ActionReturnStatus RetrieveClusterData(DataModel::Provider * dataModel, const Access::SubjectDescriptor & subjectDescriptor,
                                       bool isFabricFiltered, AttributeReportIBs::Builder & reportBuilder,
                                       const ConcreteReadAttributePath & path, AttributeEncodeState * encoderState)
{
    ChipLogDetail(DataManagement, "<RE:Run> Cluster %" PRIx32 ", Attribute %" PRIx32 " is dirty", path.mClusterId,
                  path.mAttributeId);
    DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Read,
                                                          DataModelCallbacks::OperationOrder::Pre, path);

    ActionReturnStatus statusEmber(CHIP_NO_ERROR);
    uint32_t lengthWrittenEmber = 0;

    // a copy for DM logic only. Ember changes state directly
    // IMPORTANT: the copy MUST be taken BEFORE ember processes/changes encoderState inline.
    AttributeEncodeState stateDm(encoderState);

    {
        ScopedAttributeReportIBsBuilderState builderState(reportBuilder); // temporary only
        statusEmber =
            EmberImpl::RetrieveClusterData(dataModel, subjectDescriptor, isFabricFiltered, reportBuilder, path, encoderState);
        lengthWrittenEmber = reportBuilder.GetWriter()->GetLengthWritten();
    }

    ActionReturnStatus statusDm = DataModelImpl::RetrieveClusterData(dataModel, subjectDescriptor, isFabricFiltered, reportBuilder,
                                                                     path, encoderState != nullptr ? &stateDm : nullptr);

    if (statusEmber != statusDm)
    {
        ActionReturnStatus::StringStorage buffer;

        // Note log + chipDie instead of VerifyOrDie so that breakpoints (and usage of rr)
        // is easier to debug.
        ChipLogError(Test, "Different return codes between ember and DM");
        ChipLogError(Test, "  Ember status: %s", statusEmber.c_str(buffer));
        ChipLogError(Test, "  DM status:    %s", statusDm.c_str(buffer));

        // For time-dependent data, we may have size differences here: one data fitting in buffer
        // while another not, resulting in different errors (success vs out of space).
        //
        // Make unit tests strict; otherwise allow it with potentially odd mismatch errors
        // (in which case logs will be odd, however we also expect Checked versions to only
        // run for a short period until we switch over to either ember or DM completely).
#if CHIP_CONFIG_DATA_MODEL_CHECK_DIE_ON_FAILURE
        chipDie();
#endif
    }

    // data should be identical for most cases EXCEPT that for time-deltas (e.g. seconds since boot or similar)
    // it may actually differ. As a result, the amount of data written in bytes MUST be the same, however if the rest of the
    // data is not the same, we just print it out as a warning for manual inspection
    //
    // We have no direct access to TLV buffer data (especially given backing store splits)
    // so for now we check that data length was identical.
    //
    // NOTE: RetrieveClusterData is responsible for encoding StatusIB errors in case of failures
    //       so we validate length written requirements for BOTH success and failure.
    //
    // NOTE: data length is NOT reliable if the data content differs in encoding length. E.g. numbers changing
    //       from 0xFF to 0x100 or similar will use up more space.
    //       For unit tests we make the validation strict, however for runtime we just report an
    //       error for different sizes.
    if (lengthWrittenEmber != reportBuilder.GetWriter()->GetLengthWritten())
    {
        ChipLogError(Test, "Different written length: %" PRIu32 " (Ember) vs %" PRIu32 " (DataModel)", lengthWrittenEmber,
                     reportBuilder.GetWriter()->GetLengthWritten());
#if CHIP_CONFIG_DATA_MODEL_CHECK_DIE_ON_FAILURE
        chipDie();
#endif
    }

    // For chunked reads, the encoder state MUST be identical (since this is what controls
    // where chunking resumes).
    if (statusEmber.IsOutOfSpaceEncodingResponse())
    {
        // Encoder state MUST match on partial reads (used by chunking)
        // specifically ReadViaAccessInterface in ember-compatibility-functions only
        // sets the encoder state in case an error occurs.
        if (encoderState != nullptr)
        {
            if (encoderState->AllowPartialData() != stateDm.AllowPartialData())
            {
                ChipLogError(Test, "Different partial data");
                // NOTE: die on unit tests only, since partial data size may differ across
                //       time-dependent data (very rarely because fast code, but still possible)
#if CHIP_CONFIG_DATA_MODEL_CHECK_DIE_ON_FAILURE
                chipDie();
#endif
            }
            if (encoderState->CurrentEncodingListIndex() != stateDm.CurrentEncodingListIndex())
            {
                ChipLogError(Test, "Different partial data");
                // NOTE: die on unit tests only, since partial data size may differ across
                //       time-dependent data (very rarely because fast code, but still possible)
#if CHIP_CONFIG_DATA_MODEL_CHECK_DIE_ON_FAILURE
                chipDie();
#endif
            }
        }
    }

    DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Read,
                                                          DataModelCallbacks::OperationOrder::Post, path);

    return statusDm;
}

bool IsClusterDataVersionEqualTo(DataModel::Provider * dataModel, const ConcreteClusterPath & path, DataVersion dataVersion)
{
    bool emberResult = EmberImpl::IsClusterDataVersionEqualTo(dataModel, path, dataVersion);
    bool dmResult    = DataModelImpl::IsClusterDataVersionEqualTo(dataModel, path, dataVersion);

    if (emberResult != dmResult)
    {
        ChipLogError(Test, "Different data model check result between ember (%s) and data model provider(%s)",
                     emberResult ? "TRUE" : "FALSE", dmResult ? "TRUE" : "FALSE");
#if CHIP_CONFIG_DATA_MODEL_CHECK_DIE_ON_FAILURE
        chipDie();
#endif
    }

    return dmResult;
}

} // namespace CheckedImpl
} // namespace reporting
} // namespace app
} // namespace chip
