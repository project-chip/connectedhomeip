/**
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "wifi-network-management-server.h"

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <app/util/config.h>
#include <lib/core/Global.h>
#include <lib/support/CodeUtils.h>

#include <functional>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WiFiNetworkManagement::Attributes;
using namespace std::placeholders;

namespace chip {
namespace app {
namespace Clusters {
namespace WiFiNetworkManagement {

Global<Server> gServerInstance;

Server & Server::Instance()
{
    return gServerInstance.get();
}

Server::Server() : AttributeAccessInterface(NullOptional, Id), CommandHandlerInterface(NullOptional, Id) {}

CHIP_ERROR Server::Init(EndpointId endpoint)
{
    VerifyOrReturnError(endpoint != kInvalidEndpointId, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);

    mEndpointId = endpoint;
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR Server::ClearNetworkCredentials()
{
    VerifyOrReturnError(!SsidSpan().empty() || !PassphraseSpan().empty(), CHIP_NO_ERROR);

    mSsidLen = 0;
    mPassphrase.SetLength(0);
    MatterReportingAttributeChangeCallback(mEndpointId, Id, Ssid::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Server::SetNetworkCredentials(ByteSpan ssid, ByteSpan passphrase)
{
    VerifyOrReturnError(1 <= ssid.size() && ssid.size() <= sizeof(mSsid), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(1 <= passphrase.size() && passphrase.size() <= mPassphrase.Capacity(), CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(!SsidSpan().data_equal(ssid) || !PassphraseSpan().data_equal(passphrase), CHIP_NO_ERROR);

    memcpy(mSsid, ssid.data(), ssid.size());
    mSsidLen = static_cast<decltype(mSsidLen)>(ssid.size());

    ReturnErrorOnFailure(mPassphrase.SetLength(passphrase.size()));
    memcpy(mPassphrase.Bytes(), passphrase.data(), passphrase.size());

    MatterReportingAttributeChangeCallback(mEndpointId, Id, Ssid::Id); // report SSID change even if only passphrase changed
    return CHIP_NO_ERROR;
}

CHIP_ERROR Server::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Ssid::Id: {
        auto ssid = SsidSpan();
        return (ssid.empty()) ? aEncoder.EncodeNull() : aEncoder.Encode(ssid);
    }
    }
    return CHIP_NO_ERROR;
}

void Server::InvokeCommand(HandlerContext & ctx)
{
    switch (ctx.mRequestPath.mCommandId)
    {
    case Commands::NetworkPassphraseRequest::Id:
        HandleCommand<Commands::NetworkPassphraseRequest::DecodableType>(
            ctx, std::bind(&Server::HandleNetworkPassphraseRequest, this, _1, _2));
        return;
    }
}

void Server::HandleNetworkPassphraseRequest(HandlerContext & ctx, const Commands::NetworkPassphraseRequest::DecodableType & req)
{
    if (mPassphrase.Length() > 0)
    {
        Commands::NetworkPassphraseResponse::Type response;
        response.passphrase = mPassphrase.Span();
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::InvalidInState);
    }
}

} // namespace WiFiNetworkManagement
} // namespace Clusters
} // namespace app
} // namespace chip

#if defined(MATTER_DM_WIFI_NETWORK_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT) &&                                                    \
    MATTER_DM_WIFI_NETWORK_MANAGEMENT_CLUSTER_SERVER_ENDPOINT_COUNT > 1
#error Only a single Wi-Fi Network Management Cluster instance is supported.
#endif

void MatterWiFiNetworkManagementPluginServerInitCallback() {}

void emberAfWiFiNetworkManagementClusterServerInitCallback(EndpointId endpoint)
{
    // We could delay constructing the instance until this point; however it's not
    // clear if this is inconvenient in terms of forcing the application to initialize
    // the network credentials later than it otherwise would.
    LogErrorOnFailure(chip::app::Clusters::WiFiNetworkManagement::Server::Instance().Init(endpoint));
}
