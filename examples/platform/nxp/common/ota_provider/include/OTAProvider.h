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

#pragma once

#include "fwk_filesystem.h"
#include <RTBdxOtaSender.h>
#include <RTOTAProviderExample.h>
#include <app/clusters/ota-provider/DefaultOTAProviderUserConsent.h>
#include <app/clusters/ota-provider/ota-provider.h>
#include <app/server/Server.h>

#define OTA_FILE_NAME "NEW_FW_FILE"
#define CONFIG_NXP_OTA_IMAGE_NAME "RT1060_OTA"

using chip::BitFlags;
using chip::app::Clusters::OTAProviderDelegate;
using chip::bdx::TransferControlFlags;
using chip::Messaging::ExchangeManager;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;

namespace chip {
namespace NXP {
namespace App {
// TODO: this should probably be done dynamically
constexpr chip::EndpointId kOtaProviderEndpoint = 0;

static RTOTAProviderExample gOtaProvider;

// Global variables used for passing the CLI arguments to the OTAProviderExample object
static OTAQueryStatus gQueryImageStatus              = OTAQueryStatus::kUpdateAvailable;
static OTAApplyUpdateAction gOptionUpdateAction      = OTAApplyUpdateAction::kProceed;
static uint32_t gDelayedQueryActionTimeSec           = 0;
static uint32_t gDelayedApplyActionTimeSec           = 0;
static const char * gOtaFilepath                     = OTA_FILE_NAME;
static chip::ota::UserConsentState gUserConsentState = chip::ota::UserConsentState::kUnknown;
static bool gUserConsentNeeded                       = false;
static uint32_t gIgnoreQueryImageCount               = 0;
static uint32_t gIgnoreApplyUpdateCount              = 0;
static uint32_t gPollInterval                        = 0;
static chip::Optional<uint32_t> gSoftwareVersion;
static const char * gSoftwareVersionString = nullptr;

extern "C" void InitOTAServer();
} // namespace App
} // namespace NXP
} // namespace chip
