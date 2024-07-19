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

#include <app/reporting/Read-DataModel.h>
#include <app/reporting/Read-Ember.h>
#include <app/util/MatterCallbacks.h>

namespace chip {
namespace app {
namespace reporting {
namespace CheckedImpl {
namespace {

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

CHIP_ERROR RetrieveClusterData(InteractionModel::DataModel * dataModel, const Access::SubjectDescriptor & subjectDescriptor,
                               bool isFabricFiltered, AttributeReportIBs::Builder & reportBuilder,
                               const ConcreteReadAttributePath & path, AttributeEncodeState * encoderState)
{
    ChipLogDetail(DataManagement, "<RE:Run> Cluster %" PRIx32 ", Attribute %" PRIx32 " is dirty", path.mClusterId,
                  path.mAttributeId);
    DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Read,
                                                          DataModelCallbacks::OperationOrder::Pre, path);

    AttributeEncodeState stateDm(encoderState); // a copy for DM logic only. Ember changes state directly

    CHIP_ERROR errEmber         = CHIP_NO_ERROR;
    uint32_t lengthWrittenEmber = 0;

    {
        ScopedAttributeReportIBsBuilderState builderState(reportBuilder); // temporary only
        errEmber =
            EmberImpl::RetrieveClusterData(dataModel, subjectDescriptor, isFabricFiltered, reportBuilder, path, encoderState);
        lengthWrittenEmber = reportBuilder.GetWriter()->GetLengthWritten();
    }

    CHIP_ERROR errDM =
        DataModelImpl::RetrieveClusterData(dataModel, subjectDescriptor, isFabricFiltered, reportBuilder, path, &stateDm);

    if (errEmber != errDM)
    {
        // Note log + chipDie instead of VerifyOrDie so that breakpoints (and usage of rr)
        // is easier to debug.
        ChipLogError(Test, "Different return codes between ember and DM");
        ChipLogError(Test, "  Ember error: %" CHIP_ERROR_FORMAT, errEmber.Format());
        ChipLogError(Test, "  DM error: %" CHIP_ERROR_FORMAT, errDM.Format());
        chipDie();
    }

    // data should be identical for most cases EXCEPT that for time-deltas (e.g. seconds since boot or similar)
    // it may actually differ. As a result, the amount of data written in bytes MUST be the same, however if the rest of the
    // data is not the same, we just print it out as a warning for manual inspection
    //
    // We have no direct access to TLV buffer data (especially given backing store splits)
    // so for now we check that data length was identical
    //
    // NOTE: RetrieveClusterData is responsible for encoding StatusIB errors in case of failures
    //       so we validate length written requirements for BOTH success and failure.
    if (lengthWrittenEmber != reportBuilder.GetWriter()->GetLengthWritten())
    {
        ChipLogError(Test, "Different written length");
        chipDie();
    }

    // For write resumes, the encoder state MUST be idential
    if ((errEmber == CHIP_ERROR_NO_MEMORY) || (errEmber == CHIP_ERROR_BUFFER_TOO_SMALL))
    {
        // Encoder state MUST match on partial reads (used by chunking)
        // specifically ReadViaAccessInterface in ember-compatibility-functions only
        // sets the encoder state in case an error occurs.
        if (encoderState != nullptr)
        {
            if (encoderState->AllowPartialData() != stateDm.AllowPartialData())
            {
                ChipLogError(Test, "Different partial data");
                chipDie();
            }
            if (encoderState->CurrentEncodingListIndex() != stateDm.CurrentEncodingListIndex())
            {
                ChipLogError(Test, "Different partial data");
                chipDie();
            }
        }
    }

    DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Read,
                                                          DataModelCallbacks::OperationOrder::Post, path);

    return errDM;
}

} // namespace CheckedImpl
} // namespace reporting
} // namespace app
} // namespace chip
