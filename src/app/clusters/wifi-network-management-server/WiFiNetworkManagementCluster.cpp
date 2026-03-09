/**
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include "WiFiNetworkManagementCluster.h"

#include <access/SubjectDescriptor.h>
#include <app/CommandHandler.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/WiFiNetworkManagement/Attributes.h>
#include <clusters/WiFiNetworkManagement/CommandIds.h>
#include <clusters/WiFiNetworkManagement/Commands.h>
#include <clusters/WiFiNetworkManagement/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemClock.h>

#include <algorithm>
#include <cctype>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WiFiNetworkManagement;
using namespace chip::app::Clusters::WiFiNetworkManagement::Attributes;
using namespace chip::app::Clusters::WiFiNetworkManagement::Commands;

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

namespace chip::app::Clusters {

CHIP_ERROR WiFiNetworkManagementCluster::ClearNetworkCredentials()
{
    VerifyOrReturnError(HasNetworkCredentials(), CHIP_NO_ERROR);

    mSsid.clear();
    RETURN_SAFELY_IGNORED mPassphrase.SetLength(0);

    NotifyAttributeChanged(Ssid::Id);
    NotifyAttributeChanged(PassphraseSurrogate::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR WiFiNetworkManagementCluster::SetNetworkCredentials(ByteSpan ssid, ByteSpan passphrase)
{
    VerifyOrReturnError(1 <= ssid.size() && ssid.size() <= mSsid.capacity(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsValidWpaPersonalCredential(passphrase), CHIP_ERROR_INVALID_ARGUMENT);

    bool ssidChanged       = !Ssid().data_equal(ssid);
    bool passphraseChanged = !Passphrase().data_equal(passphrase);
    VerifyOrReturnError(ssidChanged || passphraseChanged, CHIP_NO_ERROR);

    mSsid.assign(ssid);

    RETURN_SAFELY_IGNORED mPassphrase.SetLength(passphrase.size());
    memcpy(mPassphrase.Bytes(), passphrase.data(), passphrase.size());

    if (ssidChanged)
    {
        NotifyAttributeChanged(Ssid::Id);
    }
    if (passphraseChanged)
    {
        mPassphraseSurrogate++;
        System::Clock::Milliseconds64 realtime;
        if (System::SystemClock().GetClock_RealTimeMS(realtime) == CHIP_NO_ERROR)
        {
            mPassphraseSurrogate = std::max(mPassphraseSurrogate, realtime.count());
        }
        NotifyAttributeChanged(PassphraseSurrogate::Id);
    }
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus WiFiNetworkManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                          AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode<uint32_t>(0);
    case ClusterRevision::Id:
        return encoder.Encode(WiFiNetworkManagement::kRevision);
    case Ssid::Id:
        return HasNetworkCredentials() ? encoder.Encode(Ssid()) : encoder.EncodeNull();
    case PassphraseSurrogate::Id:
        return HasNetworkCredentials() ? encoder.Encode(mPassphraseSurrogate) : encoder.EncodeNull();
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

std::optional<DataModel::ActionReturnStatus> WiFiNetworkManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                         TLV::TLVReader & input_arguments,
                                                                                         CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case NetworkPassphraseRequest::Id: {
        VerifyOrReturnValue(request.subjectDescriptor->authMode == Access::AuthMode::kCase,
                            Protocols::InteractionModel::Status::UnsupportedAccess);
        VerifyOrReturnValue(HasNetworkCredentials(), Protocols::InteractionModel::Status::InvalidInState);

        NetworkPassphraseResponse::Type response;
        response.passphrase = mPassphrase.Span();
        handler->AddResponse(request.path, response);
        return std::nullopt;
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedCommand;
    }
}

CHIP_ERROR WiFiNetworkManagementCluster::Attributes(const ConcreteClusterPath & path,
                                                    ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span<const DataModel::AttributeEntry>(WiFiNetworkManagement::Attributes::kMandatoryMetadata),
                              Span<const AttributeListBuilder::OptionalAttributeEntry>());
}

CHIP_ERROR WiFiNetworkManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                          ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
        WiFiNetworkManagement::Commands::NetworkPassphraseRequest::kMetadataEntry,
    };
    return builder.AppendElements(kAcceptedCommands);
}

CHIP_ERROR WiFiNetworkManagementCluster::GeneratedCommands(const ConcreteClusterPath & path,
                                                           ReadOnlyBufferBuilder<CommandId> & builder)
{
    static constexpr CommandId kGeneratedCommands[] = {
        WiFiNetworkManagement::Commands::NetworkPassphraseResponse::Id,
    };
    return builder.AppendElements(kGeneratedCommands);
}

} // namespace chip::app::Clusters
