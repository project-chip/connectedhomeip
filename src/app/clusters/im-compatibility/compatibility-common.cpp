#include "compatibility-common.h"

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <util/util.h>

namespace chip {
namespace app {
namespace Compatibility {
namespace {
EmberAfClusterCommand imCompatibilityEmberAfCluster;
EmberApsFrame imCompatibilityEmberApsFrame;
EmberAfInterpanHeader imCompatibilityInterpanHeader;
Command * currentCommandObject;
} // namespace

void SetupEmberAfObjects(Command * command, ClusterId clusterId, CommandId commandId, EndpointId endpointId)
{
    imCompatibilityEmberApsFrame.clusterId           = clusterId;
    imCompatibilityEmberApsFrame.destinationEndpoint = endpointId;
    imCompatibilityEmberApsFrame.sourceEndpoint      = endpointId;
    imCompatibilityEmberAfCluster.commandId          = commandId;
    imCompatibilityEmberAfCluster.apsFrame           = &imCompatibilityEmberApsFrame;
    imCompatibilityEmberAfCluster.interPanHeader     = &imCompatibilityInterpanHeader;
    emAfCurrentCommand                               = &imCompatibilityEmberAfCluster;
    currentCommandObject                             = command;
}

bool IMEmberAfSendImmediateDefaultResponseHandle(EmberAfStatus status)
{
    if (currentCommandObject == nullptr)
    {
        // If this command is not handled by IM, then let ember send response.
        return false;
    }
    CHIP_ERROR err = CHIP_NO_ERROR;
    (void) status;

    // TODO: handle the response according to status value
    chip::app::Command::CommandParams returnCommandParam = { imCompatibilityEmberApsFrame.sourceEndpoint,
                                                             0, // GroupId
                                                             imCompatibilityEmberApsFrame.clusterId,
                                                             imCompatibilityEmberAfCluster.commandId,
                                                             (chip::app::Command::kCommandPathFlag_EndpointIdValid) };

    chip::TLV::TLVType dummyType = chip::TLV::kTLVType_NotSpecified;
    chip::TLV::TLVWriter writer  = currentCommandObject->CreateCommandDataElementTLVWriter();

    SuccessOrExit(err = writer.StartContainer(chip::TLV::AnonymousTag, chip::TLV::kTLVType_Structure, dummyType));
    SuccessOrExit(err = writer.EndContainer(dummyType));
    SuccessOrExit(err = writer.Finalize());
    SuccessOrExit(err = currentCommandObject->AddCommand(returnCommandParam));
exit:
    return true;
}

void ResetEmberAfObjects()
{
    emAfCurrentCommand   = nullptr;
    currentCommandObject = nullptr;
}

} // namespace Compatibility
} // namespace app
} // namespace chip
