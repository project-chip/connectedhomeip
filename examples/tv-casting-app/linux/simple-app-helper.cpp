/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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

#include "../tv-casting-common/core/ConnectionCallbacks.h"
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
#include <platform/TestOnlyCommissionableDataProvider.h>

// VendorId of the Endpoint on the CastingPlayer that the CastingApp desires to interact with after connection
const uint16_t kDesiredEndpointVendorId = 65521;
// EndpointId of the Endpoint on the CastingPlayer that the CastingApp desires to interact with after connection using the
// Commissioner-Generated passcode commissioning flow
const uint8_t kDesiredEndpointId = 1;
// Indicates that the Commissioner-Generated passcode commissioning flow is in progress.
bool gCommissionerGeneratedPasscodeFlowRunning = false;

DiscoveryDelegateImpl * DiscoveryDelegateImpl::_discoveryDelegateImpl = nullptr;
bool gAwaitingCommissionerPasscodeInput                               = false;
LinuxCommissionableDataProvider gSimpleAppCommissionableDataProvider;
std::shared_ptr<matter::casting::core::CastingPlayer> targetCastingPlayer;

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
    ChipLogProgress(AppServer, "DiscoveryDelegateImpl::HandleOnAdded()");
    if (commissionersCount == 0)
    {
        ChipLogProgress(AppServer, "---- Awaiting user input ----");
        ChipLogProgress(AppServer, "Select a discovered Casting Player (start index = 0) to request commissioning.");
        ChipLogProgress(
            AppServer,
            "Include the commissioner-generated-passcode flag to attempt the Commissioner-Generated passcode commissioning flow.");
        ChipLogProgress(AppServer, "Example 1 Commissionee Passcode:     cast request 0");
        ChipLogProgress(AppServer, "Example 2 Commissioner Passcode:     cast request 0 commissioner-generated-passcode");
        ChipLogProgress(AppServer, "---- Awaiting user input ----");
    }
    ChipLogProgress(AppServer, "Discovered CastingPlayer #%d", commissionersCount);
    ++commissionersCount;
    player->LogDetail();
}

void DiscoveryDelegateImpl::HandleOnUpdated(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player)
{
    ChipLogProgress(AppServer, "DiscoveryDelegateImpl::HandleOnUpdated() Updated CastingPlayer with ID: %s", player->GetId());
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

CHIP_ERROR InitCommissionableDataProvider(LinuxCommissionableDataProvider & provider, LinuxDeviceOptions & options)
{
    ChipLogProgress(Discovery, "InitCommissionableDataProvider()");
    chip::Optional<uint32_t> setupPasscode;

    if (options.payload.setUpPINCode != 0)
    {
        setupPasscode.SetValue(options.payload.setUpPINCode);
        ChipLogProgress(Discovery, "InitCommissionableDataProvider() using setupPasscode: %d", setupPasscode.Value());
    }
    else if (!options.spake2pVerifier.HasValue())
    {
        uint32_t defaultTestPasscode = 0;
        chip::DeviceLayer::TestOnlyCommissionableDataProvider TestOnlyCommissionableDataProvider;
        VerifyOrDie(TestOnlyCommissionableDataProvider.GetSetupPasscode(defaultTestPasscode) == CHIP_NO_ERROR);

        ChipLogError(Support,
                     "InitCommissionableDataProvider() *** WARNING: Using temporary passcode %u due to no neither --passcode or "
                     "--spake2p-verifier-base64 "
                     "given on command line. This is temporary and will be deprecated. Please update your scripts "
                     "to explicitly configure onboarding credentials. ***",
                     static_cast<unsigned>(defaultTestPasscode));
        setupPasscode.SetValue(defaultTestPasscode);
        options.payload.setUpPINCode = defaultTestPasscode;
    }
    else
    {
        ChipLogError(Support,
                     "InitCommissionableDataProvider() *** WARNING: Passcode is 0, so will be ignored, and verifier will take "
                     "over. Onboarding payload printed for debug will be invalid, but if the onboarding payload had been given "
                     "properly to the commissioner later, PASE will succeed. ***");
    }

    // Default to the minimum PBKDF iterations (1,000) for this example implementation. For TV devices and TV casting app production
    // implementations, you should use a higher number of PBKDF iterations to enhance security. The default minimum iterations are
    // not sufficient against brute-force and rainbow table attacks. Increasing the number of iterations will increase the
    // computational time required to derive the key. This can slow down the authentication process, especially on devices with
    // limited processing power like a Raspberry Pi 4. For a production implementation, you should measure the actual performance on
    // the target device.
    uint32_t spake2pIterationCount =
        chip::Crypto::kSpake2p_Min_PBKDF_Iterations; // 1,000 - Hypothetical key derivation time: ~20 milliseconds (ms).
    // uint32_t spake2pIterationCount = chip::Crypto::kSpake2p_Max_PBKDF_Iterations; // 100,000 - Hypothetical key derivation time:
    // ~2 seconds.
    if (options.spake2pIterations == 1000)
    {
        spake2pIterationCount = options.spake2pIterations;
        ChipLogError(Support,
                     "InitCommissionableDataProvider() *** WARNING: PASE PBKDF iterations provided are the minimum allowable: %u. "
                     "Increase for production use to enhance security. ***",
                     static_cast<unsigned>(spake2pIterationCount));
    }
    else if ((options.spake2pIterations > 1000))
    {
        spake2pIterationCount = options.spake2pIterations;
        ChipLogProgress(Support, "InitCommissionableDataProvider() PASE PBKDF iterations set to: %u.",
                        static_cast<unsigned>(spake2pIterationCount));
    }
    else
    {
        ChipLogError(Support,
                     "InitCommissionableDataProvider() *** WARNING: PASE PBKDF iterations set to the minimum allowable: %u. "
                     "Increase for production use to enhance security. ***",
                     static_cast<unsigned>(spake2pIterationCount));
    }

    return provider.Init(options.spake2pVerifier, options.spake2pSalt, spake2pIterationCount, setupPasscode,
                         options.payload.discriminator.GetLongValue());
}

void LogEndpointsDetails(const std::vector<matter::casting::memory::Strong<matter::casting::core::Endpoint>> & endpoints)
{
    ChipLogProgress(AppServer, "simple-app-helper.cpp::LogEndpointsDetails() Number of Endpoints: %d",
                    static_cast<int>(endpoints.size()));
    for (const auto & endpoint : endpoints)
    {
        endpoint->LogDetail();
    }
}

void ConnectionHandler(CHIP_ERROR err, matter::casting::core::CastingPlayer * castingPlayer)
{
    ChipLogProgress(AppServer, "simple-app-helper.cpp::ConnectionHandler()");

    // For a connection failure, called back with an error and nullptr.
    VerifyOrReturn(
        err == CHIP_NO_ERROR,
        ChipLogError(
            AppServer,
            "simple-app-helper.cpp::ConnectionHandler(): Failed to connect to CastingPlayer (ID: %s) with err %" CHIP_ERROR_FORMAT,
            targetCastingPlayer->GetId(), err.Format()));

    if (gCommissionerGeneratedPasscodeFlowRunning)
    {
        ChipLogProgress(AppServer,
                        "simple-app-helper.cpp::ConnectionHandler(): Successfully connected to CastingPlayer (ID: %s) using "
                        "Commissioner-Generated passcode",
                        castingPlayer->GetId());
        ChipLogProgress(AppServer, "simple-app-helper.cpp::ConnectionHandler(): Desired Endpoint ID for demo interactions: 1");
    }
    else
    {
        ChipLogProgress(AppServer, "simple-app-helper.cpp::ConnectionHandler(): Successfully connected to CastingPlayer (ID: %s)",
                        castingPlayer->GetId());
        ChipLogProgress(AppServer,
                        "simple-app-helper.cpp::ConnectionHandler(): Desired Endpoint Vendor ID for demo interactions: %d",
                        kDesiredEndpointVendorId);
    }

    ChipLogProgress(AppServer, "simple-app-helper.cpp::ConnectionHandler(): Getting endpoints available for demo interactions");
    std::vector<matter::casting::memory::Strong<matter::casting::core::Endpoint>> endpoints = castingPlayer->GetEndpoints();
    LogEndpointsDetails(endpoints);

    // Find the desired Endpoint and auto-trigger some Matter Casting demo interactions
    auto it = std::find_if(endpoints.begin(), endpoints.end(),
                           [](const matter::casting::memory::Strong<matter::casting::core::Endpoint> & endpoint) {
                               if (gCommissionerGeneratedPasscodeFlowRunning)
                               {
                                   // For the example Commissioner-Generated passcode commissioning flow, run demo interactions with
                                   // the Endpoint with ID 1. For this flow, we commissioned with the Target Content Application
                                   // with Vendor ID 1111. Since this target content application does not report its Endpoint's
                                   // Vendor IDs, we find the desired endpoint based on the Endpoint ID. See
                                   // connectedhomeip/examples/tv-app/tv-common/include/AppTv.h.
                                   return endpoint->GetId() == kDesiredEndpointId;
                               }
                               return endpoint->GetVendorId() == kDesiredEndpointVendorId;
                           });
    if (it != endpoints.end())
    {
        // The desired endpoint is endpoints[index]
        unsigned index = (unsigned int) std::distance(endpoints.begin(), it);

        ChipLogProgress(
            AppServer,
            "simple-app-helper.cpp::ConnectionHandler(): Triggering demo interactions with CastingPlayer (ID: %s). Endpoint ID: %d",
            castingPlayer->GetId(), endpoints[index]->GetId());

        // demonstrate invoking a command
        ChipLogProgress(AppServer, "simple-app-helper.cpp::ConnectionHandler() calling InvokeContentLauncherLaunchURL()");
        InvokeContentLauncherLaunchURL(endpoints[index]);

        // demonstrate reading an attribute
        ChipLogProgress(AppServer, "simple-app-helper.cpp::ConnectionHandler() calling ReadApplicationBasicVendorID()");
        ReadApplicationBasicVendorID(endpoints[index]);

        // demonstrate subscribing to an attribute
        ChipLogProgress(AppServer, "simple-app-helper.cpp::ConnectionHandler() calling SubscribeToMediaPlaybackCurrentState()");
        SubscribeToMediaPlaybackCurrentState(endpoints[index]);
    }
    else
    {
        ChipLogError(
            AppServer,
            "simple-app-helper.cpp::ConnectionHandler():Desired Endpoint Vendor ID not found on the CastingPlayer (ID: %s)",
            castingPlayer->GetId());
    }
}

void CommissionerDeclarationCallback(const chip::Transport::PeerAddress & source,
                                     chip::Protocols::UserDirectedCommissioning::CommissionerDeclaration cd)
{
    ChipLogProgress(AppServer,
                    "simple-app-helper.cpp::CommissionerDeclarationCallback() called with CommissionerDeclaration message:");
    cd.DebugLog();
    if (cd.GetCommissionerPasscode())
    {
        ChipLogProgress(AppServer, "---- Awaiting user input ----");
        ChipLogProgress(AppServer, "Input the Commissioner-Generated passcode displayed on the CastingPlayer UX.");
        ChipLogProgress(AppServer, "Input 12345678 to use the default passcode.");
        ChipLogProgress(AppServer, "Example:     cast setcommissionerpasscode 12345678");
        ChipLogProgress(AppServer, "---- Awaiting user input ----");
        gAwaitingCommissionerPasscodeInput = true;
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
        ChipLogProgress(AppServer, "CommandHandler() discover");

        return matter::casting::core::CastingPlayerDiscovery::GetInstance()->StartDiscovery(kTargetPlayerDeviceType);
    }
    if (strcmp(argv[0], "stop-discovery") == 0)
    {
        ChipLogProgress(AppServer, "CommandHandler() stop-discovery");
        return matter::casting::core::CastingPlayerDiscovery::GetInstance()->StopDiscovery();
    }
    if (strcmp(argv[0], "request") == 0)
    {
        ChipLogProgress(AppServer, "CommandHandler() request");
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;
        unsigned long index = static_cast<unsigned long>(strtol(argv[1], &eptr, 10));
        std::vector<matter::casting::memory::Strong<matter::casting::core::CastingPlayer>> castingPlayers =
            matter::casting::core::CastingPlayerDiscovery::GetInstance()->GetCastingPlayers();
        VerifyOrReturnValue(index < castingPlayers.size(), CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(AppServer, "Invalid casting player index provided: %lu", index));
        targetCastingPlayer = castingPlayers.at(index);

        gCommissionerGeneratedPasscodeFlowRunning = false;

        // Specify the TargetApp that the client wants to interact with after commissioning. If this value is passed in,
        // VerifyOrEstablishConnection() will force UDC, in case the desired TargetApp is not found in the on-device
        // CastingStore
        matter::casting::core::IdentificationDeclarationOptions idOptions;
        chip::Protocols::UserDirectedCommissioning::TargetAppInfo targetAppInfo;
        targetAppInfo.vendorId = kDesiredEndpointVendorId;

        if (argc == 3)
        {

            if (strcmp(argv[2], "commissioner-generated-passcode") == 0)
            {
                // Attempt Commissioner-Generated Passcode (commissioner-generated-passcode) commissioning flow only if the
                // CastingPlayer indicates support for it.
                if (targetCastingPlayer->GetSupportsCommissionerGeneratedPasscode())
                {
                    ChipLogProgress(AppServer,
                                    "CommandHandler() request %lu commissioner-generated-passcode. Attempting the "
                                    "Commissioner-Generated Passcode commissioning flow",
                                    index);
                    idOptions.mCommissionerPasscode = true;

                    // For the example Commissioner-Generated passcode commissioning flow, override the default Target Content
                    // Application Vendor ID, which is configured on the tv-app. This Target Content Application Vendor ID (1111),
                    // does not implement the AccountLogin cluster, which would otherwise auto commission using the
                    // Commissionee-Generated passcode upon recieving the IdentificationDeclaration Message. See
                    // connectedhomeip/examples/tv-app/tv-common/include/AppTv.h.
                    targetAppInfo.vendorId                    = 1111;
                    gCommissionerGeneratedPasscodeFlowRunning = true;
                }
                else
                {
                    ChipLogError(AppServer,
                                 "CommandHandler() request %lu commissioner-generated-passcode. Selected CastingPLayer does not "
                                 "support the Commissioner-Generated Passcode commissioning flow",
                                 index);
                }
            }
        }

        CHIP_ERROR result = idOptions.addTargetAppInfo(targetAppInfo);
        if (result != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "CommandHandler() request, failed to add targetAppInfo: %" CHIP_ERROR_FORMAT, result.Format());
        }

        matter::casting::core::ConnectionCallbacks connectionCallbacks;
        connectionCallbacks.mOnConnectionComplete = ConnectionHandler;
        // Provide an handler (Optional) for Commissioner's CommissionerDeclaration messages. The CommissionerDeclaration messages
        // provide information indicating the Commissioner's pre-commissioning state.
        connectionCallbacks.mCommissionerDeclarationCallback = CommissionerDeclarationCallback;

        targetCastingPlayer->VerifyOrEstablishConnection(connectionCallbacks, matter::casting::core::kCommissioningWindowTimeoutSec,
                                                         idOptions);
        ChipLogProgress(AppServer, "CommandHandler() request, VerifyOrEstablishConnection() called, calling StopDiscovery()");
        // Stop discovery since we have discovered, and are now connecting to the desired CastingPlayer.
        matter::casting::core::CastingPlayerDiscovery::GetInstance()->StopDiscovery();
        return CHIP_NO_ERROR;
    }
    if (strcmp(argv[0], "setcommissionerpasscode") == 0)
    {
        ChipLogProgress(AppServer, "CommandHandler() setcommissionerpasscode");
        if (argc < 2)
        {
            return PrintAllCommands();
        }
        char * eptr;
        uint32_t userEnteredPasscode = (uint32_t) strtol(argv[1], &eptr, 10);
        if (gAwaitingCommissionerPasscodeInput)
        {
            ChipLogProgress(AppServer, "CommandHandler() setcommissionerpasscode user-entered passcode: %d", userEnteredPasscode);
            gAwaitingCommissionerPasscodeInput = false;

            // Per connectedhomeip/examples/platform/linux/LinuxCommissionableDataProvider.h: We don't support overriding the
            // passcode post-init (it is deprecated!). Therefore we need to initiate a new provider with the user-entered
            // Commissioner-generated passcode, and then update the CastigApp's AppParameters to update the commissioning session's
            // passcode.
            LinuxDeviceOptions::GetInstance().payload.setUpPINCode = userEnteredPasscode;
            CHIP_ERROR err                                         = CHIP_NO_ERROR;
            err = InitCommissionableDataProvider(gSimpleAppCommissionableDataProvider, LinuxDeviceOptions::GetInstance());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "CommandHandler() setcommissionerpasscode InitCommissionableDataProvider() err %" CHIP_ERROR_FORMAT,
                             err.Format());
                return err;
            }
            // Update the CommissionableDataProvider stored in this CastingApp's AppParameters and the CommissionableDataProvider to
            // be used for the commissioning session.
            err = matter::casting::core::CastingApp::GetInstance()->UpdateCommissionableDataProvider(
                &gSimpleAppCommissionableDataProvider);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "CommandHandler() setcommissionerpasscode InitCommissionableDataProvider() err %" CHIP_ERROR_FORMAT,
                             err.Format());
                return err;
            }

            // Continue Connecting to the target CastingPlayer with the user entered Commissioner-generated Passcode.
            err = targetCastingPlayer->ContinueConnecting();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "CommandHandler() setcommissionerpasscode ContinueConnecting() failed due to err %" CHIP_ERROR_FORMAT,
                             err.Format());
                // Since continueConnecting() failed, Attempt to cancel the connection attempt with
                // the CastingPlayer/Commissioner by calling StopConnecting().
                err = targetCastingPlayer->StopConnecting();
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer,
                                 "CommandHandler() setcommissionerpasscode, ContinueConnecting() failed and then StopConnecting "
                                 "failed due to err %" CHIP_ERROR_FORMAT,
                                 err.Format());
                }
                return err;
            }
        }
        else
        {
            ChipLogError(
                AppServer,
                "CommandHandler() setcommissionerpasscode, no Commissioner-Generated passcode input expected at this time.");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    if (strcmp(argv[0], "stop-connecting") == 0)
    {
        ChipLogProgress(AppServer, "CommandHandler() stop-connecting");
        CHIP_ERROR err = targetCastingPlayer->StopConnecting();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "CommandHandler() stop-connecting, err %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
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
    streamer_printf(sout,
                    "  request <index>                                  Request connecting to discovered Casting Player with "
                    "[index] using the Commissionee-Generated passcode commissioning flow. Usage: cast request 0\r\n");
    streamer_printf(sout,
                    "  request <index> commissioner-generated-passcode  Request connecting to discovered Casting Player with "
                    "[index] using the Commissioner-Generated passcode commissioning flow. Usage: cast request 0 cgp\r\n");
    streamer_printf(sout,
                    "  setcommissionerpasscode <passcode>               Set the commissioning session's passcode to the "
                    "Commissioner-Generated passcode. Used for the the Commissioner-Generated passcode commissioning flow. Usage: "
                    "cast setcommissionerpasscode 12345678\r\n");
    streamer_printf(sout,
                    "  stop-connecting                                  Stop connecting to Casting Player upon "
                    "Commissioner-Generated passcode commissioning flow passcode input request. Usage: cast stop-connecting\r\n");
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
                        binding.remote, ChipLogValueMEI(binding.clusterId.value_or(0)));
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
