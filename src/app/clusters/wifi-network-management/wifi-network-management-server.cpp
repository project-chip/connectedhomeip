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
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/reporting/reporting.h>
#include <lib/support/CodeUtils.h>
#include <system/SystemClock.h>

#include <algorithm>
#include <cctype>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WiFiNetworkManagement::Attributes;
using namespace chip::app::Clusters::WiFiNetworkManagement::Commands;
using IMStatus = chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

namespace {

// TODO: Move this into lib/support somewhere and also use it network-commissioning.cpp
bool IsValidWpaPersonalCredential(ByteSpan credential)
{
    // As per spec section 11.9.7.3. AddOrUpdateWiFiNetwork Command
    if (8 <= credential.size() && credential.size() <= 63) // passphrase
    {
        return true;
    }
    if (credential.size() == 64) // raw hex psk
    {
        return std::all_of(credential.begin(), credential.end(), [](auto c) { return std::isxdigit(c); });
    }
    return false;
}

} // namespace

WiFiNetworkManagementServer::WiFiNetworkManagementServer(EndpointId endpoint) :
    AttributeAccessInterface(MakeOptional(endpoint), WiFiNetworkManagement::Id),
    CommandHandlerInterface(MakeOptional(endpoint), WiFiNetworkManagement::Id)
{}

WiFiNetworkManagementServer::~WiFiNetworkManagementServer()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
}

CHIP_ERROR WiFiNetworkManagementServer::Init()
{
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiNetworkManagementServer::ClearNetworkCredentials()
{
    VerifyOrReturnError(HaveNetworkCredentials(), CHIP_NO_ERROR);

    mSsidLen = 0;
    mPassphrase.SetLength(0);
    MatterReportingAttributeChangeCallback(GetEndpointId(), WiFiNetworkManagement::Id, Ssid::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiNetworkManagementServer::SetNetworkCredentials(ByteSpan ssid, ByteSpan passphrase)
{
    VerifyOrReturnError(1 <= ssid.size() && ssid.size() <= sizeof(mSsid), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsValidWpaPersonalCredential(passphrase), CHIP_ERROR_INVALID_ARGUMENT);

    bool ssidChanged       = !SsidSpan().data_equal(ssid);
    bool passphraseChanged = !PassphraseSpan().data_equal(passphrase);
    VerifyOrReturnError(ssidChanged || passphraseChanged, CHIP_NO_ERROR);

    memcpy(mSsid, ssid.data(), ssid.size());
    mSsidLen = static_cast<decltype(mSsidLen)>(ssid.size());

    VerifyOrDie(mPassphrase.SetLength(passphrase.size()) == CHIP_NO_ERROR);
    memcpy(mPassphrase.Bytes(), passphrase.data(), passphrase.size());

    if (ssidChanged)
    {
        MatterReportingAttributeChangeCallback(GetEndpointId(), WiFiNetworkManagement::Id, Ssid::Id);
    }
    if (passphraseChanged)
    {
        mPassphraseSurrogate++;
        System::Clock::Milliseconds64 realtime;
        if (System::SystemClock().GetClock_RealTimeMS(realtime) == CHIP_NO_ERROR)
        {
            mPassphraseSurrogate = std::max(mPassphraseSurrogate, realtime.count());
        }
        MatterReportingAttributeChangeCallback(GetEndpointId(), WiFiNetworkManagement::Id, PassphraseSurrogate::Id);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiNetworkManagementServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Ssid::Id:
        return HaveNetworkCredentials() ? aEncoder.Encode(SsidSpan()) : aEncoder.EncodeNull();
    case PassphraseSurrogate::Id:
        return HaveNetworkCredentials() ? aEncoder.Encode(mPassphraseSurrogate) : aEncoder.EncodeNull();
    }
    return CHIP_NO_ERROR;
}

void WiFiNetworkManagementServer::InvokeCommand(HandlerContext & ctx)
{
    switch (ctx.mRequestPath.mCommandId)
    {
    case NetworkPassphraseRequest::Id:
        HandleCommand<NetworkPassphraseRequest::DecodableType>(
            ctx, [this](HandlerContext & aCtx, const auto & req) { HandleNetworkPassphraseRequest(aCtx, req); });
        return;
    }
}

void WiFiNetworkManagementServer::HandleNetworkPassphraseRequest(HandlerContext & ctx,
                                                                 const NetworkPassphraseRequest::DecodableType & req)
{
    if (ctx.mCommandHandler.GetSubjectDescriptor().authMode != Access::AuthMode::kCase)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, IMStatus::UnsupportedAccess);
        return;
    }

    if (HaveNetworkCredentials())
    {
        NetworkPassphraseResponse::Type response;
        response.passphrase = mPassphrase.Span();
        ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
    }
    else
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, IMStatus::InvalidInState);
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip

void MatterWiFiNetworkManagementPluginServerInitCallback() {}
