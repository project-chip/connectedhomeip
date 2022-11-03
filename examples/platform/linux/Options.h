/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
