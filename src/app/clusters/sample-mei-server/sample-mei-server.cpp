#include "sample-mei-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SampleMei;
using namespace chip::app::Clusters::SampleMei::Commands;
using namespace chip::app::Clusters::SampleMei::Attributes;

// *****************************************************************************
// Init/Shutdown Callbacks

void MatterSampleMeiPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Sample MEI Init. Ep %d, Total Ep %u", MATTER_DM_SAMPLE_MEI_CLUSTER_SERVER_ENDPOINT_COUNT,
                    static_cast<uint16_t>(kNumSupportedEndpoints));
    ReturnOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(&SampleMeiServer::Instance()));
    VerifyOrReturn(registerAttributeAccessOverride(&SampleMeiServer::Instance()), CHIP_ERROR_INCORRECT_STATE);
}

void emberAfSampleMeiClusterServerInitCallback(chip::EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Creating Sample MEI cluster, Ep %d", endpoint);
    SampleMeiServer::Instance().RegisterEndpoint(endpoint);
}

void MatterSampleMeiClusterServerShutdownCallback(chip::EndpointId endpoint)
{
    // There's currently no whole-cluster shutdown callback. That would trigger
    // call to `Shutdown`. Thus ep-based shutdown calls `UnregisterEndpoint`
    ChipLogProgress(Zcl, "Shutting down Sample MEI cluster, Ep %d", endpoint);
    SampleMeiServer::Instance().UnregisterEndpoint(endpoint);
}

// *****************************************************************************
// SampleMeiContent

namespace chip {
namespace app {
namespace Clusters {
namespace SampleMei {

SampleMeiContent::SampleMeiContent() : SampleMeiContent(kInvalidEndpointId) {}

SampleMeiContent::SampleMeiContent(EndpointId aEndpoint)
{
    endpoint  = aEndpoint;
    pingCount = 10000;

    // Attribute default values
    flipflop = false;
}

// *****************************************************************************
// SampleMeiServer

void SampleMeiServer::InvokeCommand(HandlerContext & ctxt)
{
    auto endpoint      = ctxt.mRequestPath.mEndpointId;
    auto fabricIndex   = ctxt.mCommandHandler.GetAccessingFabricIndex();
    auto endpointIndex = EndpointIndex(endpoint);
    if (endpointIndex == std::numeric_limits<size_t>::max())
    {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedEndpoint);
        return;
    }

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::Ping::Id:
        HandleCommand<Commands::Ping::DecodableType>(
            ctxt, [this, endpoint, fabricIndex, endpointIndex](HandlerContext & ctx, const auto & req) {
                ChipLogProgress(Zcl, "Ping Command on Ep %d", endpoint);
                Events::PingCountEvent::Type event{ .count = content[endpointIndex].pingCount++, .fabricIndex = fabricIndex };
                chip::EventNumber placeholderEventNumber;
                CHIP_ERROR err = LogEvent(event, endpoint, placeholderEventNumber);
                if (CHIP_NO_ERROR != err)
                {
                    ChipLogError(Zcl, "Failed to record event on endpoint %d: %" CHIP_ERROR_FORMAT, static_cast<int>(endpoint),
                                 err.Format());
                }
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
            });
        return;
    case Commands::AddArguments::Id:
        HandleCommand<Commands::AddArguments::DecodableType>(ctxt, [endpoint](HandlerContext & ctx, const auto & req) {
            ChipLogProgress(Zcl, "AddArgumentsCommand on Ep %d", endpoint);
            if (req.arg1 > UINT8_MAX - req.arg2)
            {
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidCommand);
                return;
            }

            AddArgumentsResponse::Type response;
            response.returnValue = static_cast<uint8_t>(req.arg1 + req.arg2);
            ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
        });
        return;
    }
}

CHIP_ERROR SampleMeiServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    auto endpoint      = aPath.mEndpointId;
    auto endpointIndex = EndpointIndex(endpoint);
    if (endpointIndex == std::numeric_limits<size_t>::max())
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
    }

    switch (aPath.mAttributeId)
    {
    case Attributes::FlipFlop::Id:
        ChipLogProgress(Zcl, "Read Attribute flip-flop from Ep %d index %u value %d", endpoint,
                        static_cast<uint16_t>(endpointIndex), content[endpointIndex].flipflop);
        err = aEncoder.Encode(content[endpointIndex].flipflop);
        break;
    default:
        break;
    }

    return err;
}

CHIP_ERROR SampleMeiServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    auto endpoint      = aPath.mEndpointId;
    auto endpointIndex = EndpointIndex(endpoint);
    if (endpointIndex == std::numeric_limits<size_t>::max())
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
    }

    switch (aPath.mAttributeId)
    {
    case Attributes::FlipFlop::Id: {
        auto oldValue = content[endpointIndex].flipflop;
        ReturnErrorOnFailure(aDecoder.Decode(content[endpointIndex].flipflop));
        ChipLogProgress(Zcl, "Write Attribute flip-flop on Ep %d index %u newValue %d oldValue %d", endpoint,
                        static_cast<uint16_t>(endpointIndex), content[endpointIndex].flipflop, oldValue);
        break;
    }
    default:
        break;
    }

    return err;
}

SampleMeiServer & SampleMeiServer::Instance()
{
    static SampleMeiServer sampleMeiServer;
    return sampleMeiServer;
}

void SampleMeiServer::Shutdown()
{
    for (size_t i = 0; i < kNumSupportedEndpoints; ++i)
    {
        content[i].endpoint = kInvalidEndpointId;
    }
}

size_t SampleMeiServer::GetNumSupportedEndpoints() const
{
    return kNumSupportedEndpoints;
}

CHIP_ERROR SampleMeiServer::RegisterEndpoint(EndpointId endpointId)
{
    size_t endpointIndex = NextEmptyIndex();
    if (endpointIndex == std::numeric_limits<size_t>::max())
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    content[endpointIndex] = SampleMeiContent(endpointId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SampleMeiServer::UnregisterEndpoint(EndpointId endpointId)
{
    size_t endpointIndex = EndpointIndex(endpointId);
    if (endpointIndex == std::numeric_limits<size_t>::max())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    content[endpointIndex].endpoint = kInvalidEndpointId;
    return CHIP_NO_ERROR;
}

size_t SampleMeiServer::EndpointIndex(EndpointId endpointId) const
{
    for (size_t i = 0; i < kNumSupportedEndpoints; ++i)
    {
        if (content[i].endpoint == endpointId)
        {
            return i;
        }
    }
    return std::numeric_limits<size_t>::max();
}

size_t SampleMeiServer::NextEmptyIndex() const
{
    for (size_t i = 0; i < kNumSupportedEndpoints; ++i)
    {
        if (content[i].endpoint == kInvalidEndpointId)
        {
            return i;
        }
    }
    return std::numeric_limits<size_t>::max();
}

} // namespace SampleMei
} // namespace Clusters
} // namespace app
} // namespace chip
