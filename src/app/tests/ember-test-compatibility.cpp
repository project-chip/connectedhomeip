#include "lib/support/CHIPMem.h"
#include <app/tests/ember-test-compatibility.h>
#include <app/util/basic-types.h>

#include <app/InteractionModelEngine.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/util/basic-types.h>
#include <app/util/mock/Constants.h>
#include <app/util/mock/Functions.h>
#include <lib/core/CHIPCore.h>
#include <messaging/ReliableMessageContext.h>

namespace chip {
uint8_t chip::Test::attributeDataTLV[CHIP_CONFIG_DEFAULT_UDP_MTU_SIZE];
size_t chip::Test::attributeDataTLVLen = 0;
namespace app {

// was previously in TestWriteInteraction.cpp
const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aConcreteClusterPath)
{
    // Note: This test does not make use of the real attribute metadata.
    static EmberAfAttributeMetadata stub = { .defaultValue = EmberAfDefaultOrMinMaxAttributeValue(uint32_t(0)) };
    return &stub;
}

// was previously in TestWriteInteraction.cpp
CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, const ConcreteDataAttributePath & aPath,
                                  TLV::TLVReader & aReader, WriteHandler * aWriteHandler)
{
    if (aPath.mDataVersion.HasValue() && aPath.mDataVersion.Value() == Test::kRejectedDataVersion)
    {
        return aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::DataVersionMismatch);
    }

    TLV::TLVWriter writer;
    writer.Init(chip::Test::attributeDataTLV);
    writer.CopyElement(TLV::AnonymousTag(), aReader);
    chip::Test::attributeDataTLVLen = writer.GetLengthWritten();
    return aWriteHandler->AddStatus(aPath, Protocols::InteractionModel::Status::Success);
}

// was previously in TestAclAttribute.cpp
bool ConcreteAttributePathExists(const ConcreteAttributePath & aPath)
{
    return aPath.mClusterId != Test::kTestDeniedClusterId1;
}

// was previously in TestAclAttribute.cpp
Protocols::InteractionModel::Status CheckEventSupportStatus(const ConcreteEventPath & aPath)
{
    if (aPath.mClusterId == Test::kTestDeniedClusterId1)
    {
        return Protocols::InteractionModel::Status::UnsupportedCluster;
    }

    return Protocols::InteractionModel::Status::Success;
}

// strong defintion in TestCommandInteraction.cpp
__attribute__((weak)) Protocols::InteractionModel::Status
ServerClusterCommandExists(const ConcreteCommandPath & aRequestCommandPath)
{
    // Mock cluster catalog, only support commands on one cluster on one endpoint.
    using Protocols::InteractionModel::Status;

    return Status::Success;
}

// strong defintion in TestCommandInteraction.cpp
__attribute__((weak)) void DispatchSingleClusterCommand(const ConcreteCommandPath & aRequestCommandPath,
                                                        chip::TLV::TLVReader & aReader, CommandHandler * apCommandObj)
{}

// was previously in TestReadInteraction.cpp
bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion)
{
    if (Test::kTestDataVersion1 == aRequiredVersion)
    {
        return true;
    }

    return false;
}

// was previously in TestReadInteraction.cpp
bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint)
{
    return false;
}

// was previously in TestReadInteraction.cpp
CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
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

} // namespace app

} // namespace chip
