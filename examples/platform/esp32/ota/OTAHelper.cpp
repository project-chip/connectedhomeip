/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "OTAHelper.h"

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/ExtendedOTARequestorDriver.h>
#include <platform/ESP32/OTAImageProcessorImpl.h>
#include <system/SystemEvent.h>

#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::DeviceLayer;
using namespace chip;

class CustomOTARequestorDriver : public DeviceLayer::ExtendedOTARequestorDriver
{
public:
    bool CanConsent() override;
};

namespace {
DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
CustomOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
chip::Optional<bool> gRequestorCanConsent;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kUnknown;
chip::ota::DefaultOTARequestorUserConsent gUserConsentProvider;

// WARNING: This is just an example for using key for decrypting the encrypted OTA image
// Please do not use it as is for production use cases
#if CONFIG_ENABLE_ENCRYPTED_OTA
extern const char sOTADecryptionKeyStart[] asm("_binary_esp_image_encryption_key_pem_start");
extern const char sOTADecryptionKeyEnd[] asm("_binary_esp_image_encryption_key_pem_end");

CharSpan sOTADecryptionKey(sOTADecryptionKeyStart, sOTADecryptionKeyEnd - sOTADecryptionKeyStart);
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

} // namespace

bool CustomOTARequestorDriver::CanConsent()
{
    return gRequestorCanConsent.ValueOr(DeviceLayer::ExtendedOTARequestorDriver::CanConsent());
}

void OTAHelpers::InitOTARequestor()
{
    if (!GetRequestorInstance())
    {
        SetRequestorInstance(&gRequestorCore);
        gRequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
        gRequestorCore.Init(Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);
        gImageProcessor.SetOTADownloader(&gDownloader);
        gDownloader.SetImageProcessorDelegate(&gImageProcessor);
        gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

#if CONFIG_ENABLE_ENCRYPTED_OTA
        gImageProcessor.InitEncryptedOTA(sOTADecryptionKey);
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

        if (gUserConsentState != chip::ota::UserConsentState::kUnknown)
        {
            gUserConsentProvider.SetUserConsentState(gUserConsentState);
            gRequestorUser.SetUserConsentDelegate(&gUserConsentProvider);
        }
    }
}

namespace chip {
namespace Shell {
namespace {

Shell::Engine sSubShell;

CHIP_ERROR UserConsentStateHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 1, CHIP_ERROR_INVALID_ARGUMENT);

    if (strcmp(argv[0], "granted") == 0)
    {
        gUserConsentState = chip::ota::UserConsentState::kGranted;
    }
    else if (strcmp(argv[0], "denied") == 0)
    {
        gUserConsentState = chip::ota::UserConsentState::kDenied;
    }
    else if (strcmp(argv[0], "deferred") == 0)
    {
        gUserConsentState = chip::ota::UserConsentState::kObtaining;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR RequestorCanConsentHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 1, CHIP_ERROR_INVALID_ARGUMENT);

    if (strcmp(argv[0], "true") == 0)
    {
        gRequestorCanConsent.SetValue(true);
    }
    else if (strcmp(argv[0], "false") == 0)
    {
        gRequestorCanConsent.SetValue(false);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR SetPeriodicQueryTimeoutHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 1, CHIP_ERROR_INVALID_ARGUMENT);
    gRequestorUser.SetPeriodicQueryTimeout(strtoul(argv[0], NULL, 0));
    gRequestorUser.RekickPeriodicQueryTimer();
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestorHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        sSubShell.ForEachCommand(PrintCommandHelp, nullptr);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR error = sSubShell.ExecCommand(argc, argv);

    if (error != CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", error.Format());
    }

    return error;
}
} // namespace

void OTARequestorCommands::Register()
{
    // Register subcommands of the `OTARequestor` commands.
    static const shell_command_t subCommands[] = {
        { &UserConsentStateHandler, "userConsentState",
          "Set UserConsentState for QueryImageCommand\n"
          "Usage: OTARequestor userConsentState <granted/denied/deferred>" },
        { &RequestorCanConsentHandler, "requestorCanConsent",
          "Set requestorCanConsent for QueryImageCommand\n"
          "Usage: OTARequestor requestorCanConsent <true/false>" },
        { &SetPeriodicQueryTimeoutHandler, "PeriodicQueryTimeout",
          "Set timeout for querying the OTA provider for an update\n"
          "Usage: OTARequestor PeriodicQueryTimeout <seconds>" },

    };

    sSubShell.RegisterCommands(subCommands, MATTER_ARRAY_SIZE(subCommands));

    // Register the root `OTA Requestor` command in the top-level shell.
    static const shell_command_t otaRequestorCommand = { &OTARequestorHandler, "OTARequestor", "OTA Requestor commands" };

    Engine::Root().RegisterCommands(&otaRequestorCommand, 1);
}

} // namespace Shell
} // namespace chip
