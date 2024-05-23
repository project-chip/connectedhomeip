/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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

#pragma once

#include "core/CastingPlayer.h"
#include "core/CastingPlayerDiscovery.h"
#include "core/IdentificationDeclarationOptions.h"
#include "core/Types.h"
#include <LinuxCommissionableDataProvider.h>
#include <Options.h>
#include <platform/CHIPDeviceLayer.h>

/**
 * @brief Matter deviceType that the Linux tv-casting-app will discover over Commissionable Node discovery (DNS-SD)
 *
 * 35 represents device type of Matter Video Player
 */
const uint64_t kTargetPlayerDeviceType = 35;

/**
 * @brief Test values used for demo command and attribute read/subscribe calls
 */
const char kContentURL[]                          = "https://www.test.com/videoid";
const char kContentDisplayStr[]                   = "Test video";
const unsigned short kTimedInvokeCommandTimeoutMs = 5 * 1000;
const uint16_t kMinIntervalFloorSeconds           = 0;
const uint16_t kMaxIntervalCeilingSeconds         = 1;

/**
 * @brief Singleton that reacts to CastingPlayer discovery results
 */
class DiscoveryDelegateImpl : public matter::casting::core::DiscoveryDelegate
{
private:
    DiscoveryDelegateImpl(){};
    static DiscoveryDelegateImpl * _discoveryDelegateImpl;
    int commissionersCount = 0;

public:
    static DiscoveryDelegateImpl * GetInstance();

    /**
     * @brief Called when a new CastingPlayer is discovered
     *
     * @param player the discovered CastingPlayer
     */
    void HandleOnAdded(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player) override;

    /**
     * @brief Called when there are updates to the Attributes of a previously discovered CastingPlayer
     *
     * @param player the updated CastingPlayer
     */
    void HandleOnUpdated(matter::casting::memory::Strong<matter::casting::core::CastingPlayer> player) override;
};

/**
 * @brief Initializes a LinuxCommissionableDataProvider using configuration options provided via a LinuxDeviceOptions. It first
 * checks if a setup PIN code is specified; if not, it attempts to use a temporary default passcode.
 */
CHIP_ERROR InitCommissionableDataProvider(LinuxCommissionableDataProvider & provider, LinuxDeviceOptions & options);

/**
 * @brief Linux tv-casting-app's onCompleted handler for CastingPlayer.VerifyOrEstablishConnection API
 */
void ConnectionHandler(CHIP_ERROR err, matter::casting::core::CastingPlayer * castingPlayer);

#if defined(ENABLE_CHIP_SHELL)
/**
 * @brief Register CHIP Shell commands for the Linux tv-casting-app CLI
 */
void RegisterCommands();

/**
 * @brief Top level handler that parses text entered on the Linux tv-casting-app CLI and invokes the appropriate handler
 */
CHIP_ERROR CommandHandler(int argc, char ** argv);

/**
 * @brief Prints all the commands available on the Linux tv-casting-app CLI
 */
CHIP_ERROR PrintAllCommands();

/**
 * @brief Implements the "print-bindings" command
 */
void PrintBindings();

/**
 * @brief Implements the "print-fabrics" command
 */
void PrintFabrics();
#endif
