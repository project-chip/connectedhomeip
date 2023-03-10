#include "chef-test-cluster-server.h"

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
using namespace chip::app::Clusters::ChefTestCluster;
using namespace chip::app::Clusters::ChefTestCluster::Commands;
using namespace chip::app::Clusters::ChefTestCluster::Attributes;


namespace chip {
namespace app {
namespace Clusters {
namespace ChefTestCluster {

CHIP_ERROR ChefTestClusterServer::Init()
{
    ChipLogProgress(Zcl,">>>>> ChefTest Init");
    ReturnErrorOnFailure(chip::app::InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);
    attribute1 = true;
    return CHIP_NO_ERROR;
}

void ChefTestClusterServer::InvokeCommand(HandlerContext & ctxt)
{
    ChipLogProgress(Zcl,">>>>> Invoke");
    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::Test::Id:
        HandleCommand<Commands::Test::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            ChipLogProgress(Zcl,">>>>> TestCommand");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
        });
        return;
    case Commands::TestAddArguments::Id:
        HandleCommand<Commands::TestAddArguments::DecodableType>(ctxt, [this](HandlerContext & ctx, const auto & req) {
            ChipLogProgress(Zcl,">>>>> TestAddArgumentsCommand");
            if (req.arg1 > UINT8_MAX - req.arg2)
            {
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
                return;
            }

            TestAddArgumentsResponse::Type responseData;
            responseData.returnValue = static_cast<uint8_t>(req.arg1 + req.arg2);
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, responseData);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
        });
        return;
    }
}

CHIP_ERROR ChefTestClusterServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::Attribute1::Id:
        return aEncoder.Encode(attribute1);

    default:
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR ChefTestClusterServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::Attribute1::Id:
        bool value;
        ReturnErrorOnFailure(aDecoder.Decode(value));
        attribute1 = value;
        return CHIP_NO_ERROR;
    default:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
}

} // namespace ChefTestCluster
} // namespace Clusters
} // namespace app
} // namespace chip

/**********************************************************
 * Callbacks Implementation
 *********************************************************/

/**
 * @brief Chef Test Cluster Cluster Test Command callback (from client)
 */
bool emberAfChefTestClusterClusterTestCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                              const Clusters::ChefTestCluster::Commands::Test::DecodableType & commandData)
{
    return true;
}

/**
 * @brief Chef Test Cluster Cluster TestAddArgumentsResponse Command callback (from server)
 */
bool emberAfChefTestClusterClusterTestAddArgumentsResponseCallback(EndpointId endpoint, CommandSender * commandObj,
                                                                  uint8_t returnValue)
{
    return true;
}

/**
 * @brief Chef Test Cluster Cluster TestAddArguments Command callback (from client)
 */
bool emberAfChefTestClusterClusterTestAddArgumentsCallback(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                          const Commands::TestAddArguments::DecodableType & commandData)
{
    return true;
}

void MatterChefTestClusterPluginServerInitCallback() {}
