#include "messages-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>

#include <app/util/af.h>
#include <app/util/common.h>
#include <app/util/error-mapping.h>
#include <array>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::Messages;
using chip::Protocols::InteractionModel::Status;

bool emberAfMessagesClusterPresentMessagesRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Messages::Commands::PresentMessagesRequest::DecodableType & commandData)
{
    return true;
}
/**
 * @brief Messages Cluster CancelMessagesRequest Command callback (from client)
 */

bool emberAfMessagesClusterCancelMessagesRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::Messages::Commands::CancelMessagesRequest::DecodableType & commandData)
{
    return true;
}

void MatterMessagesPluginServerInitCallback() {}
