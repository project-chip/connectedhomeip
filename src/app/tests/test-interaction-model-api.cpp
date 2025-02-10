/*
 *    Copyright (c) 2024 Project CHIP Authors
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
#include "access/SubjectDescriptor.h"
#include <app/tests/test-interaction-model-api.h>

#include <app/InteractionModelEngine.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/util/basic-types.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/DataModelTypes.h>
#include <messaging/ReliableMessageContext.h>

using namespace chip::app::DataModel;

namespace chip {
uint8_t Test::attributeDataTLV[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
size_t Test::attributeDataTLVLen = 0;

namespace app {

class TestOnlyAttributeValueEncoderAccessor
{
public:
    TestOnlyAttributeValueEncoderAccessor(AttributeValueEncoder & encoder) : mEncoder(encoder) {}

    AttributeReportIBs::Builder & Builder() { return mEncoder.mAttributeReportIBsBuilder; }

    void SetState(const AttributeEncodeState & state) { mEncoder.mEncodeState = state; }

private:
    AttributeValueEncoder & mEncoder;
};

class TestOnlyAttributeValueDecoderAccessor
{
public:
    TestOnlyAttributeValueDecoderAccessor(AttributeValueDecoder & decoder) : mDecoder(decoder) {}

    TLV::TLVReader & GetTlvReader() { return mDecoder.mReader; }
    void SetTriedDecode(bool triedDecode) { mDecoder.mTriedDecode = triedDecode; }

private:
    AttributeValueDecoder & mDecoder;
};

// strong defintion in TestCommandInteraction.cpp
__attribute__((weak)) void DispatchSingleClusterCommand(const ConcreteCommandPath & aRequestCommandPath,
                                                        chip::TLV::TLVReader & aReader, CommandHandler * apCommandObj)
{}

// Used by the code in TestReadInteraction.cpp (and generally tests that interact with the Reporting Engine may need this).
static CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                        const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                        AttributeEncodeState * apEncoderState)
{
    if (aPath.mClusterId >= Test::kMockEndpointMin)
    {
        return Test::ReadSingleMockClusterData(aSubjectDescriptor.fabricIndex, aPath, aAttributeReports, apEncoderState);
    }

    if (!(aPath.mClusterId == Test::kTestClusterId && aPath.mEndpointId == Test::kTestEndpointId))
    {
        AttributeReportIB::Builder & attributeReport = aAttributeReports.CreateAttributeReport();
        ReturnErrorOnFailure(aAttributeReports.GetError());
        ChipLogDetail(DataManagement, "TEST Cluster %" PRIx32 ", Field %" PRIx32 " is dirty", aPath.mClusterId, aPath.mAttributeId);

        AttributeStatusIB::Builder & attributeStatus = attributeReport.CreateAttributeStatus();
        ReturnErrorOnFailure(attributeReport.GetError());
        AttributePathIB::Builder & attributePath = attributeStatus.CreatePath();
        ReturnErrorOnFailure(attributeStatus.GetError());

        attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePath.GetError());
        StatusIB::Builder & errorStatus = attributeStatus.CreateErrorStatus();
        ReturnErrorOnFailure(attributeStatus.GetError());
        errorStatus.EncodeStatusIB(StatusIB(Protocols::InteractionModel::Status::UnsupportedAttribute));
        ReturnErrorOnFailure(errorStatus.GetError());
        ReturnErrorOnFailure(attributeStatus.EndOfAttributeStatusIB());
        return attributeReport.EndOfAttributeReportIB();
    }

    return AttributeValueEncoder(aAttributeReports, aSubjectDescriptor, aPath, 0 /* dataVersion */).Encode(Test::kTestFieldValue1);
}

TestImCustomDataModel & TestImCustomDataModel::Instance()
{
    static TestImCustomDataModel model;
    return model;
}

ActionReturnStatus TestImCustomDataModel::ReadAttribute(const ReadAttributeRequest & request, AttributeValueEncoder & encoder)
{
    AttributeEncodeState mutableState(&encoder.GetState()); // provide a state copy to start.

    Access::SubjectDescriptor subjectDescriptor;
    if (request.subjectDescriptor != nullptr)
    {
        subjectDescriptor = *request.subjectDescriptor;
    }

    CHIP_ERROR err = ReadSingleClusterData(subjectDescriptor, request.readFlags.Has(ReadFlags::kFabricFiltered), request.path,
                                           TestOnlyAttributeValueEncoderAccessor(encoder).Builder(), &mutableState);

    // state must survive CHIP_ERRORs as it is used for chunking
    TestOnlyAttributeValueEncoderAccessor(encoder).SetState(mutableState);

    return err;
}

ActionReturnStatus TestImCustomDataModel::WriteAttribute(const WriteAttributeRequest & request, AttributeValueDecoder & decoder)
{
    if (request.path.mDataVersion.HasValue() && request.path.mDataVersion.Value() == Test::kRejectedDataVersion)
    {
        return CHIP_IM_GLOBAL_STATUS(DataVersionMismatch);
    }

    TestOnlyAttributeValueDecoderAccessor decodeAccess(decoder);

    decodeAccess.SetTriedDecode(true);

    TLV::TLVWriter writer;
    writer.Init(chip::Test::attributeDataTLV);
    writer.CopyElement(TLV::AnonymousTag(), decodeAccess.GetTlvReader());
    chip::Test::attributeDataTLVLen = writer.GetLengthWritten();

    return CHIP_NO_ERROR;
}

std::optional<ActionReturnStatus> TestImCustomDataModel::Invoke(const InvokeRequest & request,
                                                                chip::TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    return std::make_optional<ActionReturnStatus>(CHIP_ERROR_NOT_IMPLEMENTED);
}

} // namespace app
} // namespace chip
