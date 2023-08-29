#include "sample-mei-server.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/support/logging/CHIPLogging.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::SampleMei;
using namespace chip::app::Clusters::SampleMei::Commands;
using namespace chip::app::Clusters::SampleMei::Attributes;

SampleMeiServer sampleMeiServer;

// *****************************************************************************
// Init/Shutdown Callbacks

void MatterSampleMeiPluginServerInitCallback()
{
    ChipLogProgress(Zcl,"Sample Mei Init. Ep# %d Total Ep# %lu", EMBER_AF_SAMPLE_MEI_CLUSTER_SERVER_ENDPOINT_COUNT, kNumSupportedEndpoints);
    ReturnOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(&sampleMeiServer));
    VerifyOrReturn(registerAttributeAccessOverride(&sampleMeiServer), CHIP_ERROR_INCORRECT_STATE);

    // registers the fixed endpoints
    for (uint16_t endpointIndex = 0; endpointIndex < FIXED_ENDPOINT_COUNT; endpointIndex++) {
        auto endpoint = emberAfEndpointFromIndex(endpointIndex);
        ChipLogProgress(Zcl,"Creating cluster, Ep index %d", endpoint);
        if (emberAfContainsServer(endpoint, SampleMei::Id)) {
            sampleMeiServer.RegisterEndpoint(endpoint);
        }
    }
}

void MatterSampleMeiClusterServerShutdownCallback(chip::EndpointId endpoint) {
    sampleMeiServer.Shutdown();
}

// *****************************************************************************
// SampleMeiContent

namespace chip {
namespace app {
namespace Clusters {
namespace SampleMei {

SampleMeiContent::SampleMeiContent() : SampleMeiContent(kInvalidEndpointId) {}

SampleMeiContent::SampleMeiContent(EndpointId aEndpoint) {
    endpoint = aEndpoint;

    // Attribute default values
    flipflop = false;
}

// *****************************************************************************
// SampleMeiServer

void SampleMeiServer::InvokeCommand(HandlerContext & ctxt)
{
    auto endpoint = ctxt.mRequestPath.mEndpointId;
    auto endpointIndex = EndpointIndex(endpoint);
    if (endpointIndex == std::numeric_limits<size_t>::max()) {
        ctxt.mCommandHandler.AddStatus(ctxt.mRequestPath, Protocols::InteractionModel::Status::UnsupportedEndpoint);
        return;
    }

    switch (ctxt.mRequestPath.mCommandId)
    {
    case Commands::Ping::Id:
        HandleCommand<Commands::Ping::DecodableType>(ctxt, [this, endpoint](HandlerContext & ctx, const auto & req) {
            ChipLogProgress(Zcl,"Ping Command on endpoint %d", endpoint);
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
        });
        return;
    case Commands::AddArguments::Id:
        HandleCommand<Commands::AddArguments::DecodableType>(ctxt, [this, endpoint](HandlerContext & ctx, const auto & req) {
            ChipLogProgress(Zcl,"AddArgumentsCommand on endpoint %d", endpoint);
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto endpoint = aPath.mEndpointId;
    auto endpointIndex = EndpointIndex(endpoint);
    if (endpointIndex == std::numeric_limits<size_t>::max()) {
        return CHIP_ERROR_NO_ENDPOINT;
    }

    switch (aPath.mAttributeId)
    {
        case Attributes::FlipFlop::Id:
            ChipLogProgress(Zcl,"Read Attribute flip-flop from endpoint %d index %lu value %d", endpoint, endpointIndex, content[endpointIndex].flipflop);
            err = aEncoder.Encode(content[endpointIndex].flipflop);
            break;
        default:
        break;
    }

    return err;
}

CHIP_ERROR SampleMeiServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto endpoint = aPath.mEndpointId;
    auto endpointIndex = EndpointIndex(endpoint);
    if (endpointIndex == std::numeric_limits<size_t>::max()) {
        return CHIP_ERROR_NO_ENDPOINT;
    }

    switch (aPath.mAttributeId)
    {
        case Attributes::FlipFlop::Id:
            {
                auto oldValue = content[endpointIndex].flipflop;
                ReturnErrorOnFailure(aDecoder.Decode(content[endpointIndex].flipflop));
                ChipLogProgress(Zcl,"Write Attribute flip-flop on endpoint %d index %lu newValue %d oldValue %d", endpoint, endpointIndex, content[endpointIndex].flipflop, oldValue);
            }
            break;
        default:
        break;
    }

    return err;
}

SampleMeiServer & SampleMeiServer::Instance()
{
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
    if (endpointIndex == std::numeric_limits<size_t>::max()) {
        return CHIP_ERROR_NO_ENDPOINT;
    }

    content[endpointIndex] = SampleMeiContent(endpointId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SampleMeiServer::UnregisterEndpoint(EndpointId endpointId)
{
    size_t endpointIndex = EndpointIndex(endpointId);
    if (endpointIndex == std::numeric_limits<size_t>::max()) {
        return CHIP_ERROR_NO_ENDPOINT;
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

