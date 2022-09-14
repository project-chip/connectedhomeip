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
#include <string>
#include <vector>

#include <inet/InetInterface.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/support/CHIPArgParser.hpp>
#include <setup_payload/SetupPayload.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <testing/CustomCSRResponse.h>

struct LinuxDeviceOptions
{
    chip::PayloadContents payload;
    chip::Optional<uint16_t> discriminator;
    chip::Optional<std::vector<uint8_t>> spake2pVerifier;
    chip::Optional<std::vector<uint8_t>> spake2pSalt;
    uint32_t spake2pIterations          = 0; // When not provided (0), will default elsewhere
    uint32_t mBleDevice                 = 0;
    bool mWiFi                          = false;
    bool mThread                        = false;
    uint32_t securedDevicePort          = CHIP_PORT;
    uint32_t securedCommissionerPort    = CHIP_PORT + 2;
    uint32_t unsecuredCommissionerPort  = CHIP_UDC_PORT;
    const char * command                = nullptr;
    const char * PICS                   = nullptr;
    const char * KVS                    = nullptr;
    chip::Inet::InterfaceId interfaceId = chip::Inet::InterfaceId::Null();
    bool traceStreamDecodeEnabled       = false;
    bool traceStreamToLogEnabled        = false;
    chip::Optional<std::string> traceStreamFilename;
    chip::Credentials::DeviceAttestationCredentialsProvider * dacProvider = nullptr;
    chip::CSRResponseOptions mCSRResponseOptions;
    uint8_t testEventTriggerEnableKey[16] = { 0 };
    chip::FabricId commissionerFabricId   = chip::kUndefinedFabricId;

    static LinuxDeviceOptions & GetInstance();
};

CHIP_ERROR ParseArguments(int argc, char * const argv[], chip::ArgParser::OptionSet * customOptions = nullptr);
