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

#include "DataModelFixtures.h"

#include <access/SubjectDescriptor.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeValueDecoder.h>
#include <app/AttributeValueEncoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteClusterPath.h>
#include <app/InteractionModelEngine.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/logging/TextOnlyLogging.h>
#include <optional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModelTests;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::UnitTesting;
using namespace chip::Protocols;

// Mock function for linking
void InitDataModelHandler() {}

namespace chip {
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

private:
    AttributeValueDecoder & mDecoder;
};

namespace DataModelTests {

ScopedChangeOnly<ReadResponseDirective> gReadResponseDirective(ReadResponseDirective::kSendDataResponse);
ScopedChangeOnly<WriteResponseDirective> gWriteResponseDirective(WriteResponseDirective::kSendAttributeSuccess);
ScopedChangeOnly<CommandResponseDirective> gCommandResponseDirective(CommandResponseDirective::kSendSuccessStatusCode);

ScopedChangeOnly<bool> gIsLitIcd(false);

// TODO: usage of a global value that changes as a READ sideffect is problematic for
//       dual-read use cases (i.e. during checked ember/datamodel tests)
//
//       For now see the hack "change undo" in CustomDataModel::ReadAttribute, however
//       overall this is problematic.
uint16_t gInt16uTotalReadCount = 0;
CommandHandler::Handle gAsyncCommandHandle;

} // namespace DataModelTests

static CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                        const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                        AttributeEncodeState * apEncoderState)
{
    if (aPath.mEndpointId >= chip::Test::kMockEndpointMin)
    {
        return chip::Test::ReadSingleMockClusterData(aSubjectDescriptor.fabricIndex, aPath, aAttributeReports, apEncoderState);
    }

    if (gReadResponseDirective == ReadResponseDirective::kSendManyDataResponses ||
        gReadResponseDirective == ReadResponseDirective::kSendManyDataResponsesWrongPath)
    {
        if (aPath.mClusterId != Clusters::UnitTesting::Id || aPath.mAttributeId != Clusters::UnitTesting::Attributes::Boolean::Id)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }

        for (size_t i = 0; i < 4; ++i)
        {
            ConcreteAttributePath path(aPath);
            // Use an incorrect attribute id for some of the responses.
            path.mAttributeId = static_cast<AttributeId>(
                path.mAttributeId + (i / 2) + (gReadResponseDirective == ReadResponseDirective::kSendManyDataResponsesWrongPath));
            AttributeEncodeState state(apEncoderState);
            AttributeValueEncoder valueEncoder(aAttributeReports, aSubjectDescriptor, path, kDataVersion, aIsFabricFiltered, state);
            ReturnErrorOnFailure(valueEncoder.Encode(true));
        }

        return CHIP_NO_ERROR;
    }

    if (gReadResponseDirective == ReadResponseDirective::kSendDataResponse)
    {
        if (aPath.mClusterId == app::Clusters::UnitTesting::Id &&
            aPath.mAttributeId == app::Clusters::UnitTesting::Attributes::ListFabricScoped::Id)
        {
            AttributeEncodeState state(apEncoderState);
            AttributeValueEncoder valueEncoder(aAttributeReports, aSubjectDescriptor, aPath, kDataVersion, aIsFabricFiltered,
                                               state);

            return valueEncoder.EncodeList([aSubjectDescriptor](const auto & encoder) -> CHIP_ERROR {
                app::Clusters::UnitTesting::Structs::TestFabricScoped::Type val;
                val.fabricIndex = aSubjectDescriptor.fabricIndex;
                ReturnErrorOnFailure(encoder.Encode(val));
                val.fabricIndex = (val.fabricIndex == 1) ? 2 : 1;
                ReturnErrorOnFailure(encoder.Encode(val));
                return CHIP_NO_ERROR;
            });
        }
        if (aPath.mClusterId == app::Clusters::UnitTesting::Id &&
            aPath.mAttributeId == app::Clusters::UnitTesting::Attributes::Int16u::Id)
        {
            AttributeEncodeState state(apEncoderState);
            AttributeValueEncoder valueEncoder(aAttributeReports, aSubjectDescriptor, aPath, kDataVersion, aIsFabricFiltered,
                                               state);

            return valueEncoder.Encode(++gInt16uTotalReadCount);
        }
        if (aPath.mClusterId == kPerpetualClusterId ||
            (aPath.mClusterId == app::Clusters::UnitTesting::Id && aPath.mAttributeId == kPerpetualAttributeid))
        {
            AttributeEncodeState state;
            AttributeValueEncoder valueEncoder(aAttributeReports, aSubjectDescriptor, aPath, kDataVersion, aIsFabricFiltered,
                                               state);

            CHIP_ERROR err = valueEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
                encoder.Encode(static_cast<uint8_t>(1));
                return CHIP_ERROR_NO_MEMORY;
            });

            if (err != CHIP_NO_ERROR)
            {
                // If the err is not CHIP_NO_ERROR, means the encoding was aborted, then the valueEncoder may save its state.
                // The state is used by list chunking feature for now.
                if (apEncoderState != nullptr)
                {
                    *apEncoderState = valueEncoder.GetState();
                }
                return err;
            }
        }
        if (aPath.mClusterId == app::Clusters::IcdManagement::Id &&
            aPath.mAttributeId == app::Clusters::IcdManagement::Attributes::OperatingMode::Id)
        {
            AttributeEncodeState state(apEncoderState);
            AttributeValueEncoder valueEncoder(aAttributeReports, aSubjectDescriptor, aPath, kDataVersion, aIsFabricFiltered,
                                               state);

            return valueEncoder.Encode(gIsLitIcd ? Clusters::IcdManagement::OperatingModeEnum::kLit
                                                 : Clusters::IcdManagement::OperatingModeEnum::kSit);
        }

        AttributeReportIB::Builder & attributeReport = aAttributeReports.CreateAttributeReport();
        ReturnErrorOnFailure(aAttributeReports.GetError());
        AttributeDataIB::Builder & attributeData = attributeReport.CreateAttributeData();
        ReturnErrorOnFailure(attributeReport.GetError());
        Clusters::UnitTesting::Attributes::ListStructOctetString::TypeInfo::Type value;
        Clusters::UnitTesting::Structs::TestListStructOctet::Type valueBuf[4];

        value = valueBuf;

        uint8_t i = 0;
        for (auto & item : valueBuf)
        {
            item.member1 = i;
            i++;
        }

        attributeData.DataVersion(kDataVersion);
        ReturnErrorOnFailure(attributeData.GetError());
        AttributePathIB::Builder & attributePath = attributeData.CreatePath();
        attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePath.GetError());

        ReturnErrorOnFailure(DataModel::Encode(*(attributeData.GetWriter()), TLV::ContextTag(AttributeDataIB::Tag::kData), value));
        ReturnErrorOnFailure(attributeData.EndOfAttributeDataIB());
        return attributeReport.EndOfAttributeReportIB();
    }

    for (size_t i = 0; i < (gReadResponseDirective == ReadResponseDirective::kSendTwoDataErrors ? 2 : 1); ++i)
    {
        AttributeReportIB::Builder & attributeReport = aAttributeReports.CreateAttributeReport();
        ReturnErrorOnFailure(aAttributeReports.GetError());
        AttributeStatusIB::Builder & attributeStatus = attributeReport.CreateAttributeStatus();
        AttributePathIB::Builder & attributePath     = attributeStatus.CreatePath();
        attributePath.Endpoint(aPath.mEndpointId).Cluster(aPath.mClusterId).Attribute(aPath.mAttributeId).EndOfAttributePathIB();
        ReturnErrorOnFailure(attributePath.GetError());

        StatusIB::Builder & errorStatus = attributeStatus.CreateErrorStatus();
        ReturnErrorOnFailure(attributeStatus.GetError());
        errorStatus.EncodeStatusIB(StatusIB(Protocols::InteractionModel::Status::Busy));
        attributeStatus.EndOfAttributeStatusIB();
        ReturnErrorOnFailure(attributeStatus.GetError());
        ReturnErrorOnFailure(attributeReport.EndOfAttributeReportIB());
    }

    return CHIP_NO_ERROR;
}

void DispatchSingleClusterCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj)
{
    ChipLogDetail(Controller, "Received Cluster Command: Endpoint=%x Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                  aCommandPath.mEndpointId, ChipLogValueMEI(aCommandPath.mClusterId), ChipLogValueMEI(aCommandPath.mCommandId));

    if (aCommandPath.mClusterId == Clusters::UnitTesting::Id &&
        aCommandPath.mCommandId == Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::Type::GetCommandId())
    {
        Clusters::UnitTesting::Commands::TestSimpleArgumentRequest::DecodableType dataRequest;

        if (DataModel::Decode(aReader, dataRequest) != CHIP_NO_ERROR)
        {
            apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::Failure, "Unable to decode the request");
            return;
        }

        if (gCommandResponseDirective == CommandResponseDirective::kSendDataResponse)
        {
            Clusters::UnitTesting::Commands::TestStructArrayArgumentResponse::Type dataResponse;
            Clusters::UnitTesting::Structs::NestedStructList::Type nestedStructList[4];

            uint8_t i = 0;
            for (auto & item : nestedStructList)
            {
                item.a   = i;
                item.b   = false;
                item.c.a = i;
                item.c.b = true;
                i++;
            }

            dataResponse.arg1 = nestedStructList;
            dataResponse.arg6 = true;

            apCommandObj->AddResponse(aCommandPath, dataResponse);
        }
        else if (gCommandResponseDirective == CommandResponseDirective::kSendSuccessStatusCode)
        {
            apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::Success);
        }
        else if (gCommandResponseDirective == CommandResponseDirective::kSendMultipleSuccessStatusCodes)
        {
            apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::Success,
                                    "No error but testing status success case");

            // TODO: Right now all but the first AddStatus call fail, so this
            // test is not really testing what it should.
            for (size_t i = 0; i < 3; ++i)
            {
                (void) apCommandObj->FallibleAddStatus(aCommandPath, Protocols::InteractionModel::Status::Success,
                                                       "No error but testing status success case");
            }
            // And one failure on the end.
            (void) apCommandObj->FallibleAddStatus(aCommandPath, Protocols::InteractionModel::Status::Failure);
        }
        else if (gCommandResponseDirective == CommandResponseDirective::kSendError)
        {
            apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::Failure);
        }
        else if (gCommandResponseDirective == CommandResponseDirective::kSendMultipleErrors)
        {
            apCommandObj->AddStatus(aCommandPath, Protocols::InteractionModel::Status::Failure);

            // TODO: Right now all but the first AddStatus call fail, so this
            // test is not really testing what it should.
            for (size_t i = 0; i < 3; ++i)
            {
                (void) apCommandObj->FallibleAddStatus(aCommandPath, Protocols::InteractionModel::Status::Failure);
            }
        }
        else if (gCommandResponseDirective == CommandResponseDirective::kSendSuccessStatusCodeWithClusterStatus)
        {
            apCommandObj->AddStatus(
                aCommandPath, Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificSuccess(kTestSuccessClusterStatus));
        }
        else if (gCommandResponseDirective == CommandResponseDirective::kSendErrorWithClusterStatus)
        {
            apCommandObj->AddStatus(
                aCommandPath, Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(kTestFailureClusterStatus));
        }
        else if (gCommandResponseDirective == CommandResponseDirective::kAsync)
        {
            gAsyncCommandHandle = apCommandObj;
        }
    }
}

CustomDataModel & CustomDataModel::Instance()
{
    static CustomDataModel model;
    return model;
}

ActionReturnStatus CustomDataModel::ReadAttribute(const ReadAttributeRequest & request, AttributeValueEncoder & encoder)
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

ActionReturnStatus CustomDataModel::WriteAttribute(const WriteAttributeRequest & request, AttributeValueDecoder & decoder)
{
    static ListIndex listStructOctetStringElementCount = 0;

    if (request.path.mDataVersion.HasValue() && request.path.mDataVersion.Value() == kRejectedDataVersion)
    {
        return InteractionModel::Status::DataVersionMismatch;
    }

    if (request.path.mClusterId == Clusters::UnitTesting::Id &&
        request.path.mAttributeId == Attributes::ListStructOctetString::TypeInfo::GetAttributeId())
    {
        if (gWriteResponseDirective == WriteResponseDirective::kSendAttributeSuccess)
        {
            if (!request.path.IsListOperation() || request.path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
            {

                Attributes::ListStructOctetString::TypeInfo::DecodableType value;

                ReturnErrorOnFailure(decoder.Decode(value));

                auto iter                         = value.begin();
                listStructOctetStringElementCount = 0;
                while (iter.Next())
                {
                    auto & item = iter.GetValue();

                    VerifyOrReturnError(item.member1 == listStructOctetStringElementCount, CHIP_ERROR_INVALID_ARGUMENT);
                    listStructOctetStringElementCount++;
                }
                return CHIP_NO_ERROR;
            }

            if (request.path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
            {
                Structs::TestListStructOctet::DecodableType item;
                ReturnErrorOnFailure(decoder.Decode(item));
                VerifyOrReturnError(item.member1 == listStructOctetStringElementCount, CHIP_ERROR_INVALID_ARGUMENT);
                listStructOctetStringElementCount++;

                return CHIP_NO_ERROR;
            }

            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }

        return CHIP_IM_GLOBAL_STATUS(Failure);
    }
    if (request.path.mClusterId == Clusters::UnitTesting::Id && request.path.mAttributeId == Attributes::ListFabricScoped::Id)
    {
        // TODO(backwards compatibility): unit tests here undoes the subject descriptor usage
        //   - original tests were completely bypassing the passed in subject descriptor for this test
        //     and overriding it with a invalid subject descriptor
        //   - we do the same here, however this seems somewhat off: decoder.Decode() will fail for list
        //     items so we could just return the error directly without this extra step

        // Mock an invalid Subject Descriptor
        AttributeValueDecoder invalidSubjectDescriptorDecoder(TestOnlyAttributeValueDecoderAccessor(decoder).GetTlvReader(),
                                                              Access::SubjectDescriptor());
        if (!request.path.IsListOperation() || request.path.mListOp == ConcreteDataAttributePath::ListOperation::ReplaceAll)
        {
            Attributes::ListFabricScoped::TypeInfo::DecodableType value;

            ReturnErrorOnFailure(invalidSubjectDescriptorDecoder.Decode(value));

            auto iter = value.begin();
            while (iter.Next())
            {
                auto & item = iter.GetValue();
                (void) item;
            }
        }
        else if (request.path.mListOp == ConcreteDataAttributePath::ListOperation::AppendItem)
        {
            Structs::TestFabricScoped::DecodableType item;
            ReturnErrorOnFailure(invalidSubjectDescriptorDecoder.Decode(item));
        }
        else
        {
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
        }
        return CHIP_NO_ERROR;
    }

    // Boolean attribute of unit testing cluster triggers "multiple errors" case.
    if (request.path.mClusterId == Clusters::UnitTesting::Id &&
        request.path.mAttributeId == Attributes::Boolean::TypeInfo::GetAttributeId())
    {
        // TODO(IMDM): this used to send 4 responses (hence the multiple status)
        //
        //    for (size_t i = 0; i < 4; ++i)
        //    {
        //        aWriteHandler->AddStatus(request.path, status);
        //    }
        //
        // which are NOT encodable by a simple response. It is unclear how this is
        // convertible (if at all): we write path by path only. Having multiple
        // responses for the same path within the write code makes no sense
        //
        // This should NOT be possible anymore when one can only return a single
        // status (nobody has access to multiple path status updates at this level)
        switch (gWriteResponseDirective)
        {
        case WriteResponseDirective::kSendMultipleSuccess:
            return InteractionModel::Status::Success;
        case WriteResponseDirective::kSendMultipleErrors:
            return InteractionModel::Status::Failure;
        default:
            chipDie();
        }
    }

    if (request.path.mClusterId == Clusters::UnitTesting::Id &&
        request.path.mAttributeId == Attributes::Int8u::TypeInfo::GetAttributeId())
    {
        switch (gWriteResponseDirective)
        {
        case WriteResponseDirective::kSendClusterSpecificSuccess:
            return InteractionModel::ClusterStatusCode::ClusterSpecificSuccess(kExampleClusterSpecificSuccess);
        case WriteResponseDirective::kSendClusterSpecificFailure:
            return InteractionModel::ClusterStatusCode::ClusterSpecificFailure(kExampleClusterSpecificFailure);
        default:
            // this should not be reached, our tests only set up these for this test case
            chipDie();
        }
    }

    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

std::optional<ActionReturnStatus> CustomDataModel::InvokeCommand(const InvokeRequest & request,
                                                                 chip::TLV::TLVReader & input_arguments, CommandHandler * handler)
{
    DispatchSingleClusterCommand(request.path, input_arguments, handler);
    return std::nullopt; // handler status is set by the dispatch
}

} // namespace app
} // namespace chip
