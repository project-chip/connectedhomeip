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
namespace app {

constexpr chip::ClusterId kTestClusterId   = 6;
constexpr uint8_t kTestFieldValue1         = 1;
constexpr chip::EndpointId kTestEndpointId = 1;

// strong defintion in TestWriteInteraction.cpp
__attribute__((weak)) CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor,
                                                        const ConcreteDataAttributePath & aPath, TLV::TLVReader & aReader,
                                                        WriteHandler * aWriteHandler)
{

    return CHIP_NO_ERROR;
}

// strong defintion in TestWriteInteraction.cpp
__attribute__((weak)) const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aConcreteClusterPath)
{
    // Note: This test does not make use of the real attribute metadata.
    static EmberAfAttributeMetadata stub = { .defaultValue = EmberAfDefaultOrMinMaxAttributeValue(uint32_t(0)) };
    return &stub;
}

// strong defintion in TestAclAttribute.cpp
__attribute__((weak)) bool ConcreteAttributePathExists(const ConcreteAttributePath & aPath)
{
    return true;
}

// strong defintion in TestAclAttribute.cpp
__attribute__((weak)) Protocols::InteractionModel::Status CheckEventSupportStatus(const ConcreteEventPath & aPath)
{
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
{
    return;
}

// strong defintion in TestReadInteraction.cpp
__attribute__((weak)) bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint)
{
    return false;
}

// strong defintion in TestReadInteraction.cpp
__attribute__((weak)) bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion)
{
    return false;
}

// strong defintion in TestReadInteraction.cpp
__attribute__((weak)) CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                                       const ConcreteReadAttributePath & aPath,
                                                       AttributeReportIBs::Builder & aAttributeReports,
                                                       AttributeEncodeState * apEncoderState)
{
    if (aPath.mClusterId >= Test::kMockEndpointMin)
    {
        return Test::ReadSingleMockClusterData(aSubjectDescriptor.fabricIndex, aPath, aAttributeReports, apEncoderState);
    }

    if (!(aPath.mClusterId == kTestClusterId && aPath.mEndpointId == kTestEndpointId))
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

    return AttributeValueEncoder(aAttributeReports, aSubjectDescriptor, aPath, 0 /* dataVersion */).Encode(kTestFieldValue1);
}

} // namespace app

} // namespace chip
