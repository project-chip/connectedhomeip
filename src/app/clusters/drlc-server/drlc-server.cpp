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
using namespace chip::app::Clusters::DemandReponseLoadControl;
using chip::Protocols::InteractionModel::Status;

bool emberAfDemandReponseLoadControlClusterRegisterLoadControlProgramRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DemandReponseLoadControl::Commands::RegisterLoadControlProgramRequest::DecodableType & commandData)
{
    return true;
}

bool emberAfDemandReponseLoadControlClusterUnregisterLoadControlProgramRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DemandReponseLoadControl::Commands::UnregisterLoadControlProgramRequest::DecodableType &
        commandData)
{
    return true;
}

bool emberAfDemandReponseLoadControlClusterAddLoadControlEventRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DemandReponseLoadControl::Commands::AddLoadControlEventRequest::DecodableType & commandData)
{
    return true;
}

bool emberAfDemandReponseLoadControlClusterRemoveLoadControlEventRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DemandReponseLoadControl::Commands::RemoveLoadControlEventRequest::DecodableType & commandData)
{
    return true;
}

bool emberAfDemandReponseLoadControlClusterClearLoadControlEventsRequestCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DemandReponseLoadControl::Commands::ClearLoadControlEventsRequest::DecodableType & commandData)
{
    return true;
}


void MatterDemandReponseLoadControlPluginServerInitCallback() {}