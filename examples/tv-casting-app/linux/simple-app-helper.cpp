/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "simple-app-helper.h"

#include "clusters/Clusters.h"

#include "app/clusters/bindings/BindingManager.h"
#include <inttypes.h>
#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

// VendorId of the Endpoint on the CastingPlayer that the CastingApp desires to interact with after connection
const uint16_t kDesiredEndpointVendorId = 65521;

DiscoveryDelegateImpl * DiscoveryDelegateImpl::_discoveryDelegateImpl = nullptr;

DiscoveryDelegateImpl * DiscoveryDelegateImpl::GetInstance()
{
    if (_discoveryDelegateImpl == nullptr)
    {
        _discoveryDelegateImpl = new DiscoveryDelegateImpl();
    }
    return _discoveryDelegateImpl;
}

void DiscoveryDelegateImpl::HandleOnAdded(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player)
{
    if (commissionersCount == 0)
    {
        ChipLogProgress(AppServer, "Select discovered Casting Player (start index = 0) to request commissioning");

        ChipLogProgress(AppServer, "Example: cast request 0");
    }
    ChipLogProgress(AppServer, "Discovered CastingPlayer #%d", commissionersCount);
    ++commissionersCount;
    player->LogDetail();
}

void DiscoveryDelegateImpl::HandleOnUpdated(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player)
{
    ChipLogProgress(AppServer, "Updated CastingPlayer with ID: %s", player->GetId());
}

void InvokeContentLauncherLaunchURL(matter::casting::memory::Strong<matter::casting::core::Endpoint> endpoint)
{
    // get contentLauncherCluster from the endpoint
    matter::casting::memory::Strong<matter::casting::clusters::content_launcher::ContentLauncherCluster> contentLauncherCluster =
        endpoint->GetCluster<matter::casting::clusters::content_launcher::ContentLauncherCluster>();
    VerifyOrReturn(contentLauncherCluster != nullptr);

    // get the launchURLCommand from the contentLauncherCluster
    matter::casting::core::Command<chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type> * launchURLCommand =
        static_cast<matter::casting::core::Command<chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type> *>(
            contentLauncherCluster->GetCommand(chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Id));
    VerifyOrReturn(launchURLCommand != nullptr, ChipLogError(AppServer, "LaunchURL command not found on ContentLauncherCluster"));

    // create the LaunchURL request
    chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type request;
    request.contentURL    = chip::CharSpan::fromCharString(kContentURL);
    request.displayString = chip::Optional<chip::CharSpan>(chip::CharSpan::fromCharString(kContentDisplayStr));
    request.brandingInformation =
        chip::MakeOptional(chip::app::Clusters::ContentLauncher::Structs::BrandingInformationStruct::Type());

    // call Invoke on launchURLCommand while passing in success/failure callbacks
    launchURLCommand->Invoke(
        request, nullptr,
        [](void * context, const chip::app::Clusters::ContentLauncher::Commands::LaunchURL::Type::ResponseType & response) {
            ChipLogProgress(AppServer, "LaunchURL Success with response.data: %.*s", static_cast<int>(response.data.Value().size()),
                            response.data.Value().data());
        },
        [](void * context, CHIP_ERROR error) {
            ChipLogError(AppServer, "LaunchURL Failure with err %" CHIP_ERROR_FORMAT, error.Format());
        },
        chip::MakeOptional(kTimedInvokeCommandTimeoutMs)); // time out after kTimedInvokeCommandTimeoutMs
}

void ReadApplicationBasicVendorID(matter::casting::memory::Strong<matter::casting::core::Endpoint> endpoint)
{
    // get applicationBasicCluster from the endpoint
    matter::casting::memory::Strong<matter::casting::clusters::application_basic::ApplicationBasicCluster> applicationBasicCluster =
        endpoint->GetCluster<matter::casting::clusters::application_basic::ApplicationBasicCluster>();
    VerifyOrReturn(applicationBasicCluster != nullptr);

    // get the vendorIDAttribute from the applicationBasicCluster
    matter::casting::core::Attribute<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo> * vendorIDAttribute =
        static_cast<matter::casting::core::Attribute<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo> *>(
            applicationBasicCluster->GetAttribute(chip::app::Clusters::ApplicationBasic::Attributes::VendorID::Id));
    VerifyOrReturn(vendorIDAttribute != nullptr,
                   ChipLogError(AppServer, "VendorID attribute not found on ApplicationBasicCluster"));

    // call Read on vendorIDAttribute while passing in success/failure callbacks
    vendorIDAttribute->Read(
        nullptr,
        [](void * context,
           chip::Optional<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType> before,
           chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType after) {
            if (before.HasValue())
            {
                ChipLogProgress(AppServer, "Read VendorID value: %d [Before reading value: %d]", after, before.Value());
            }
            else
            {
                ChipLogProgress(AppServer, "Read VendorID value: %d", after);
            }
        },
        [](void * context, CHIP_ERROR error) {
            ChipLogError(AppServer, "VendorID Read failure with err %" CHIP_ERROR_FORMAT, error.Format());
        });
}

void SubscribeToMediaPlaybackCurrentState(matter::casting::memory::Strong<matter::casting::core::Endpoint> endpoint)
{
    // get mediaPlaybackCluster from the endpoint
    matter::casting::memory::Strong<matter::casting::clusters::media_playback::MediaPlaybackCluster> mediaPlaybackCluster =
        endpoint->GetCluster<matter::casting::clusters::media_playback::MediaPlaybackCluster>();
    VerifyOrReturn(mediaPlaybackCluster != nullptr);

    // get the currentStateAttribute from the applicationBasicCluster
    matter::casting::core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo> *
        currentStateAttribute =
            static_cast<matter::casting::core::Attribute<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo> *>(
                mediaPlaybackCluster->GetAttribute(chip::app::Clusters::MediaPlayback::Attributes::CurrentState::Id));
    VerifyOrReturn(currentStateAttribute != nullptr,
                   ChipLogError(AppServer, "CurrentState attribute not found on MediaPlaybackCluster"));

    // call Subscribe on currentStateAttribute while passing in success/failure callbacks
    currentStateAttribute->Subscribe(
        nullptr,
        [](void * context,
           chip::Optional<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType> before,
           chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType after) {
            if (before.HasValue())
            {
                ChipLogProgress(AppServer, "Read CurrentState value: %d [Before reading value: %d]", static_cast<int>(after),
                                static_cast<int>(before.Value()));
            }
            else
            {
                ChipLogProgress(AppServer, "Read CurrentState value: %d", static_cast<int>(after));
            }
        },
        [](void * context, CHIP_ERROR error) {
            ChipLogError(AppServer, "CurrentState Read failure with err %" CHIP_ERROR_FORMAT, error.Format());
        },
        kMinIntervalFloorSeconds, kMaxIntervalCeilingSeconds);
}

void ConnectionHandler(CHIP_ERROR err, matter::casting::core::CastingPlayer * castingPlayer)
{
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogProgress(AppServer,
                                   "ConnectionHandler: Failed to connect to CastingPlayer(ID: %s) with err %" CHIP_ERROR_FORMAT,
                                   castingPlayer->GetId(), err.Format()));

    ChipLogProgress(AppServer, "ConnectionHandler: Successfully connected to CastingPlayer(ID: %s)", castingPlayer->GetId());

    std::vector<matter::casting::memory::Strong<matter::casting::core::Endpoint>> endpoints = castingPlayer->GetEndpoints();
    // Find the desired Endpoint and auto-trigger some Matter Casting demo interactions
    auto it = std::find_if(endpoints.begin(), endpoints.end(),
                           [](const matter::casting::memory::Strong<matter::casting::core::Endpoint> & endpoint) {
                               return endpoint->GetVendorId() == 65521;
                           });
    if (it != endpoints.end())
    {
        // The desired endpoint is endpoints[index]
        unsigned index = (unsigned int) std::distance(endpoints.begin(), it);

        // demonstrate invoking a command
        InvokeContentLauncherLaunchURL(endpoints[index]);

        // demonstrate reading an attribute
        ReadApplicationBasicVendorID(endpoints[index]);

        // demonstrate subscribing to an attribute
        SubscribeToMediaPlaybackCurrentState(endpoints[index]);
    }
    else
    {
        ChipLogError(AppServer, "Desired Endpoint not found on the CastingPlayer(ID: %s)", castingPlayer->GetId());
    }
}

#if defined(ENABLE_CHIP_SHELL)
void RegisterCommands()
{
    static const chip::Shell::shell_command_t sDeviceComand = { &CommandHandler, "cast",
                                                                "Casting commands. Usage: cast [command_name]" };

    // Register the root `device` command with the top-level shell.
    chip::Shell::Engine::Root().RegisterCommands(&sDeviceComand, 1);
}

CHIP_ERROR CommandHandler(int argc, char ** argv)
{
    if (argc == 0 || strcmp(argv[0], "help") == 0)
    {
        return PrintAllCommands();
    }
    if (strcmp(argv[0], "discover") == 0)
    {
        ChipLogProgress(AppServer, "discover");

        return matter::casting::core::CastingPlayerDiscovery::GetInstance()->StartDiscovery(kTargetPlayerDeviceType);
    }
    if (strcmp(argv[0], "stop-discovery") == 0)
    {
        ChipLogProgress(AppServer, "stop-discovery");
        return matter::casting::core::CastingPlayerDiscovery::GetInstance()->StopDiscovery();
    }
    if (strcmp(argv[0], "request") == 0)
    {
        ChipLogProgress(AppServer, "request");
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;
        unsigned long index = static_cast<unsigned long>(strtol(argv[1], &eptr, 10));
        std::vector<matter::casting::memory::Strong<matter::casting::core::CastingPlayer>> castingPlayers =
            matter::casting::core::CastingPlayerDiscovery::GetInstance()->GetCastingPlayers();
        VerifyOrReturnValue(0 <= index && index < castingPlayers.size(), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(AppServer, "Invalid casting player index provided: %lu", index));
        std::shared_ptr<matter::casting::core::CastingPlayer> targetCastingPlayer = castingPlayers.at(index);

        matter::casting::core::EndpointFilter desiredEndpointFilter;
        desiredEndpointFilter.vendorId = kDesiredEndpointVendorId;
        targetCastingPlayer->VerifyOrEstablishConnection(ConnectionHandler, matter::casting::core::kCommissioningWindowTimeoutSec,
                                                         desiredEndpointFilter);
        return CHIP_NO_ERROR;
    }
    if (strcmp(argv[0], "print-bindings") == 0)
    {
        PrintBindings();
        return CHIP_NO_ERROR;
    }
    if (strcmp(argv[0], "print-fabrics") == 0)
    {
        PrintFabrics();
        return CHIP_NO_ERROR;
    }
    if (strcmp(argv[0], "delete-fabric") == 0)
    {
        char * eptr;
        chip::FabricIndex fabricIndex = (chip::FabricIndex) strtol(argv[1], &eptr, 10);
        chip::Server::GetInstance().GetFabricTable().Delete(fabricIndex);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
}

CHIP_ERROR PrintAllCommands()
{
    chip::Shell::streamer_t * sout = chip::Shell::streamer_get();
    streamer_printf(sout, "  help                 Usage: cast <subcommand>\r\n");
    streamer_printf(sout, "  print-bindings       Usage: cast print-bindings\r\n");
    streamer_printf(sout, "  print-fabrics        Usage: cast print-fabrics\r\n");
    streamer_printf(
        sout,
        "  delete-fabric <index>     Delete a fabric from the casting client's fabric store. Usage: cast delete-fabric 1\r\n");
    streamer_printf(sout, "  discover             Discover Casting Players. Usage: cast discover\r\n");
    streamer_printf(sout, "  stop-discovery       Stop Discovery of Casting Players. Usage: cast stop-discovery\r\n");
    streamer_printf(
        sout, "  request <index>      Request connecting to discovered Casting Player with [index]. Usage: cast request 0\r\n");
    streamer_printf(sout, "\r\n");

    return CHIP_NO_ERROR;
}

void PrintBindings()
{
    for (const auto & binding : chip::BindingTable::GetInstance())
    {
        ChipLogProgress(AppServer,
                        "Binding type=%d fab=%d nodeId=0x" ChipLogFormatX64
                        " groupId=%d local endpoint=%d remote endpoint=%d cluster=" ChipLogFormatMEI,
                        binding.type, binding.fabricIndex, ChipLogValueX64(binding.nodeId), binding.groupId, binding.local,
                        binding.remote, ChipLogValueMEI(binding.clusterId.ValueOr(0)));
    }
}

void PrintFabrics()
{
    // set fabric to be the first in the list
    for (const auto & fb : chip::Server::GetInstance().GetFabricTable())
    {
        chip::FabricIndex fabricIndex = fb.GetFabricIndex();
        ChipLogError(AppServer, "Next Fabric index=%d", fabricIndex);
        if (!fb.IsInitialized())
        {
            ChipLogError(AppServer, " -- Not initialized");
            continue;
        }
        chip::NodeId myNodeId = fb.GetNodeId();
        ChipLogProgress(AppServer,
                        "---- Current Fabric nodeId=0x" ChipLogFormatX64 " fabricId=0x" ChipLogFormatX64 " fabricIndex=%d",
                        ChipLogValueX64(myNodeId), ChipLogValueX64(fb.GetFabricId()), fabricIndex);
    }
}

#endif // ENABLE_CHIP_SHELL
