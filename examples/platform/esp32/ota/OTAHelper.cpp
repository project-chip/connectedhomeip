/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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
#include <app/clusters/ota-requestor/CodegenIntegration.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/clusters/ota-requestor/ExtendedOTARequestorDriver.h>
#ifdef CONFIG_ENABLE_MULTI_IMAGE_OTA
#include <platform/ESP32/multi_ota/AppImageProcessor.h>
#include <platform/ESP32/multi_ota/MultiImageOTAProcessorImpl.h>
#else
#include <platform/ESP32/OTAImageProcessorImpl.h>
#endif // CONFIG_ENABLE_MULTI_IMAGE_OTA
#include <system/SystemEvent.h>

#include <app/clusters/ota-requestor/DefaultOTARequestorUserConsent.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/logging/CHIPLogging.h>

#if defined(CONFIG_AUTO_UPDATE_RCP) && defined(CONFIG_OPENTHREAD_BORDER_ROUTER) && !defined(CONFIG_ENABLE_MULTI_IMAGE_OTA)
#include "esp_check.h"
#include "esp_rcp_ota.h"
#include "esp_rcp_update.h"
#endif // CONFIG_AUTO_UPDATE_RCP && CONFIG_OPENTHREAD_BORDER_ROUTER && !CONFIG_ENABLE_MULTI_IMAGE_OTA

#if defined(CONFIG_ENABLE_MULTI_IMAGE_OTA) && defined(CONFIG_AUTO_UPDATE_RCP) && defined(CONFIG_OPENTHREAD_BORDER_ROUTER)
#include "OTARcpImageProcessor.h"
#endif // CONFIG_ENABLE_MULTI_IMAGE_OTA && CONFIG_AUTO_UPDATE_RCP && CONFIG_OPENTHREAD_BORDER_ROUTER

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
#ifdef CONFIG_ENABLE_MULTI_IMAGE_OTA
MultiImageOTAProcessorImpl gImageProcessor;
AppImageProcessor gAppImageProcessor;
ImageProcessorEntry gAppImageEntry{ kAppImageProcessorTag, &gAppImageProcessor };
#if defined(CONFIG_AUTO_UPDATE_RCP) && defined(CONFIG_OPENTHREAD_BORDER_ROUTER)
OTARcpImageProcessor gRcpImageProcessor;
ImageProcessorEntry gRcpImageEntry{ kRcpImageProcessorTag, &gRcpImageProcessor };
#endif // CONFIG_AUTO_UPDATE_RCP && CONFIG_OPENTHREAD_BORDER_ROUTER
#else
OTAImageProcessorImpl gImageProcessor;
#endif // CONFIG_ENABLE_MULTI_IMAGE_OTA
chip::Optional<bool> gRequestorCanConsent;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kUnknown;
chip::ota::DefaultOTARequestorUserConsent gUserConsentProvider;
#if defined(CONFIG_AUTO_UPDATE_RCP) && defined(CONFIG_OPENTHREAD_BORDER_ROUTER) && !defined(CONFIG_ENABLE_MULTI_IMAGE_OTA)
class OTARcpProcessorImpl : public OTAImageProcessorImpl::OTARcpProcessorDelegate
{
public:
    esp_err_t OnOtaRcpPrepareDownload() override;
    esp_err_t OnOtaRcpProcessBlock(const uint8_t * buffer, size_t bufLen, size_t & rcpOtaReceivedLen) override;
    esp_err_t OnOtaRcpFinalize() override;
    esp_err_t OnOtaRcpAbort() override;

private:
    void ResetRcpOtaState()
    {
        mRcpOtaHandle          = 0;
        mBrFirmwareSize        = 0;
        mRcpFirmwareDownloaded = false;
    }
    esp_rcp_ota_handle_t mRcpOtaHandle;
    bool mRcpFirmwareDownloaded;
    uint32_t mBrFirmwareSize;
};

esp_err_t OTARcpProcessorImpl::OnOtaRcpPrepareDownload()
{
    ResetRcpOtaState();
    return esp_rcp_ota_begin(&mRcpOtaHandle);
}

esp_err_t OTARcpProcessorImpl::OnOtaRcpProcessBlock(const uint8_t * buffer, size_t bufLen, size_t & rcpOtaReceivedLen)
{
    esp_err_t err = ESP_OK;

    if (!mRcpFirmwareDownloaded)
    {
        err = esp_rcp_ota_receive(mRcpOtaHandle, buffer, bufLen, &rcpOtaReceivedLen);

        if (esp_rcp_ota_get_state(mRcpOtaHandle) == ESP_RCP_OTA_STATE_FINISHED)
        {
            mBrFirmwareSize        = esp_rcp_ota_get_subfile_size(mRcpOtaHandle, FILETAG_HOST_FIRMWARE);
            mRcpFirmwareDownloaded = true;
        }
    }
    else if (mBrFirmwareSize > 0)
    {
        rcpOtaReceivedLen = 0;
    }
    else
    {
        err = ESP_FAIL;
    }

    return err;
}

esp_err_t OTARcpProcessorImpl::OnOtaRcpFinalize()
{
    esp_err_t err = esp_rcp_ota_end(mRcpOtaHandle);
    ResetRcpOtaState();
    return err;
}

esp_err_t OTARcpProcessorImpl::OnOtaRcpAbort()
{
    esp_err_t err = esp_rcp_ota_abort(mRcpOtaHandle);
    ResetRcpOtaState();
    return err;
}

OTARcpProcessorImpl gOtaRcpDelegate;
#endif // CONFIG_AUTO_UPDATE_RCP && CONFIG_OPENTHREAD_BORDER_ROUTER && !CONFIG_ENABLE_MULTI_IMAGE_OTA

// WARNING: This is just an example for using key for decrypting the encrypted OTA image
// Please do not use it as is for production use cases
#ifdef CONFIG_ENABLE_ENCRYPTED_OTA
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
#ifdef CONFIG_ENABLE_MULTI_IMAGE_OTA
        // Register the application-firmware sub-processor before the requestor is initialised.
        // The app image is registered first so it is applied last (it triggers the reboot).
        LogErrorOnFailure(gImageProcessor.RegisterProcessor(gAppImageEntry));
#if defined(CONFIG_AUTO_UPDATE_RCP) && defined(CONFIG_OPENTHREAD_BORDER_ROUTER)
        LogErrorOnFailure(gImageProcessor.RegisterProcessor(gRcpImageEntry));
#endif // CONFIG_AUTO_UPDATE_RCP && CONFIG_OPENTHREAD_BORDER_ROUTER
#endif // CONFIG_ENABLE_MULTI_IMAGE_OTA
        SetRequestorInstance(&gRequestorCore);
        gRequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
        TEMPORARY_RETURN_IGNORED gRequestorCore.Init(Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader,
                                                     GetOTARequestorAttributes(), GetDefaultOTARequestorEventGenerator());
        gImageProcessor.SetOTADownloader(&gDownloader);
#if defined(CONFIG_AUTO_UPDATE_RCP) && defined(CONFIG_OPENTHREAD_BORDER_ROUTER) && !defined(CONFIG_ENABLE_MULTI_IMAGE_OTA)
        gImageProcessor.SetOtaRcpDelegate(&gOtaRcpDelegate);
#endif // CONFIG_AUTO_UPDATE_RCP && CONFIG_OPENTHREAD_BORDER_ROUTER && !CONFIG_ENABLE_MULTI_IMAGE_OTA
        gDownloader.SetImageProcessorDelegate(&gImageProcessor);
        gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

#ifdef CONFIG_ENABLE_ENCRYPTED_OTA
#ifdef CONFIG_ENABLE_MULTI_IMAGE_OTA
        LogErrorOnFailure(gAppImageProcessor.InitEncryptedOTA(sOTADecryptionKey));
#else
        gImageProcessor.InitEncryptedOTA(sOTADecryptionKey);
#endif // CONFIG_ENABLE_MULTI_IMAGE_OTA
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

        if (gUserConsentState != chip::ota::UserConsentState::kUnknown)
        {
            gUserConsentProvider.SetUserConsentState(gUserConsentState);
            gRequestorUser.SetUserConsentDelegate(&gUserConsentProvider);
        }
    }
}

#ifdef CONFIG_ENABLE_MULTI_IMAGE_OTA
CHIP_ERROR OTAHelpers::RegisterSubImageProcessor(chip::ImageProcessorEntry & entry)
{
    return gImageProcessor.RegisterProcessor(entry);
}
#endif // CONFIG_ENABLE_MULTI_IMAGE_OTA

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
