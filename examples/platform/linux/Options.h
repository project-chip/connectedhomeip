/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *      Support functions for parsing command-line arguments.
 *
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <access/AccessConfig.h>
#include <app/AppConfig.h>
#include <inet/InetInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/support/CHIPArgParser.hpp>
#include <platform/CHIPDeviceConfig.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/SetupPayload.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <testing/CustomCSRResponse.h>

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
#include <access/AccessRestrictionProvider.h>
#endif

struct LinuxDeviceOptions
{
    chip::PayloadContents payload;
    chip::Optional<uint16_t> discriminator;
    chip::Optional<std::vector<uint8_t>> spake2pVerifier;
    chip::Optional<std::vector<uint8_t>> spake2pSalt;
    chip::Optional<std::string> dacProviderFile;
    uint32_t spake2pIterations = 0; // When not provided (0), will default elsewhere
    uint32_t mBleDevice        = 0;
    bool wifiSupports5g        = false;
    bool mWiFi                 = false;
#if CHIP_ENABLE_OPENTHREAD
#if CHIP_SYSTEM_CONFIG_USE_OPENTHREAD_ENDPOINT
    uint16_t mThreadNodeId = 0;
#else
    bool mThread = false;
#endif
#endif
    bool cameraDeferredOffer = false;
    bool cameraTestVideosrc  = false;
    bool cameraTestAudiosrc  = false;
    bool cameraAudioPlayback = false;
    chip::Optional<std::string> cameraVideoDevice;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    bool mWiFiPAF                = false;
    const char * mWiFiPAFExtCmds = nullptr;
    uint32_t mPublishId          = 0;
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE || CHIP_DEVICE_ENABLE_PORT_PARAMS
    uint16_t securedDevicePort         = CHIP_PORT;
    uint16_t unsecuredCommissionerPort = CHIP_UDC_PORT;
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE || CHIP_DEVICE_ENABLE_PORT_PARAMS
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    uint16_t securedCommissionerPort    = CHIP_PORT + 12; // TODO: why + 12?
    chip::FabricId commissionerFabricId = chip::kUndefinedFabricId;
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    const char * command = nullptr;
    const char * PICS    = nullptr;
    const char * KVS     = nullptr;
    chip::Optional<std::string> KVSDataDirectory;
    chip::Optional<std::string> KVSFactoryDirectory;
    chip::Optional<std::string> KVSConfigDirectory;
    chip::Optional<std::string> KVSCountersDirectory;
    const char * app_pipe               = "";
    const char * app_pipe_out           = "";
    chip::Inet::InterfaceId interfaceId = chip::Inet::InterfaceId::Null();
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    bool traceStreamDecodeEnabled = false;
    bool traceStreamToLogEnabled  = false;
    chip::Optional<std::string> traceStreamFilename;
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::Credentials::DeviceAttestationCredentialsProvider * dacProvider = nullptr;
    chip::CSRResponseOptions mCSRResponseOptions;
    uint8_t testEventTriggerEnableKey[16] = { 0 };
    std::vector<std::string> traceTo;
    bool mSimulateNoInternalTime = false;
#if defined(PW_RPC_ENABLED)
    uint16_t rpcServerPort = 33000;
#endif
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    int32_t subscriptionCapacity                   = CHIP_IM_MAX_NUM_SUBSCRIPTIONS;
    int32_t subscriptionResumptionRetryIntervalSec = -1;
#endif
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    chip::Optional<std::vector<chip::Access::AccessRestrictionProvider::Entry>> commissioningArlEntries;
    chip::Optional<std::vector<chip::Access::AccessRestrictionProvider::Entry>> arlEntries;
#endif
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    chip::Optional<uint16_t> tcVersion;
    chip::Optional<uint16_t> tcRequired;
#endif
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    chip::Optional<chip::System::Clock::Milliseconds32> icdActiveModeDurationMs;
    chip::Optional<chip::System::Clock::Milliseconds32> icdIdleModeDurationMs;
    std::optional<chip::System::Clock::Seconds32> shortIdleModeDurationS;
#endif
    chip::Optional<std::string> vendorName;
    chip::Optional<std::string> productName;
    chip::Optional<std::string> hardwareVersionString;
    chip::Optional<std::string> serialNumber;
    static LinuxDeviceOptions & GetInstance();
};

CHIP_ERROR ParseArguments(int argc, char * const argv[], chip::ArgParser::OptionSet * customOptions = nullptr);
