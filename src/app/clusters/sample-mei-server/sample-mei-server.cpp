#include "sample-mei-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>

#include <algorithm>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SampleMei;
using namespace chip::app::Clusters::SampleMei::Commands;
using namespace chip::app::Clusters::SampleMei::Attributes;


namespace chip {
namespace app {
namespace Clusters {
namespace SampleMei {

CHIP_ERROR SampleMeiServer::Init()
{
    ChipLogProgress(Zcl,">>>>> Sample Mei Test Init");
    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    attribute1 = true;
    return CHIP_NO_ERROR;
}

void SampleMeiServer::InvokeCommand(HandlerContext & ctxt)
{
    ChipLogProgress(Zcl,">>>>> Invoke");
    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::Ping::Id:
        HandleCommand<Commands::Ping::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            ChipLogProgress(Zcl,">>>>> Ping Command");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
        });
        return;
    case Commands::AddArguments::Id:
        HandleCommand<Commands::AddArguments::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            ChipLogProgress(Zcl,">>>>> AddArgumentsCommand");
            if (req.arg1 > UINT8_MAX - req.arg2)
            {
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
                return;
            }

            AddArgumentsResponse::Type responseData;
            responseData.returnValue = static_cast<uint8_t>(req.arg1 + req.arg2);
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, responseData);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
        });
        return;
    }
}

CHIP_ERROR SampleMeiServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::FlipFlop::Id:
        return aEncoder.Encode(attribute1);

    default:
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR SampleMeiServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::FlipFlop::Id:
        bool value;
        ReturnErrorOnFailure(aDecoder.Decode(value));
        attribute1 = value;
        return CHIP_NO_ERROR;
    default:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
}

} // namespace SampleMei
} // namespace Clusters
} // namespace app
} // namespace chip

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

/**
 * @brief Sample Mei Cluster Ping Command callback (from client)
 */
bool emberAfSampleMeiClusterPingCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                              const Clusters::SampleMei::Commands::Ping::DecodableType & commandData)
{
    return true;
}

/**
 * @brief Sample Mei Cluster AddArgumentsResponse Command callback (from server)
 */
bool emberAfSampleMeiClusterAddArgumentsResponseCallback(EndpointId endpoint, CommandSender * commandObj,
                                                                  uint8_t returnValue)
{
    return true;
}

/**
 * @brief Sample Mei Cluster AddArguments Command callback (from client)
 */
bool emberAfSampleMeiClusterAddArgumentsCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                          const Commands::AddArguments::DecodableType & commandData)
{
    return true;
}

void MatterSampleMeiPluginServerInitCallback() {}
