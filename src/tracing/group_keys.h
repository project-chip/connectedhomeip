/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <matter/tracing/build_config.h>

namespace chip {
namespace Tracing {

/**
 * Defines the key type used to identify a specific group
 */
typedef const char * GroupKey;

/**
 * List of supported group keys
 */
constexpr GroupKey kAdministratorCommissioning = "AdministratorCommissioning";

constexpr GroupKey kBasicInfo = "BasicInfo";

constexpr GroupKey kBridgeBasicInfo = "BridgeBasicInfo";

constexpr GroupKey kCASEServer = "CASEServer";

constexpr GroupKey kCASESession = "CASESession";

constexpr GroupKey kColorControl = "ColorControl";

constexpr GroupKey kDeviceCommissioner = "DeviceCommissioner";

constexpr GroupKey kFabric = "Fabric";

constexpr GroupKey kG = "G";

constexpr GroupKey kGeneralCommissioning = "GeneralCommissioning";

constexpr GroupKey kGroup = "Group";

constexpr GroupKey kGroups = "Groups";

constexpr GroupKey kIdentify = "Identify";

constexpr GroupKey kJointFabricAdministrator = "JointFabricAdministrator";

constexpr GroupKey kLevelControl = "LevelControl";

constexpr GroupKey kLowPower = "LowPower";

constexpr GroupKey kMinMdnsResolver = "MinMdnsResolver";

constexpr GroupKey kModeBase = "ModeBase";

constexpr GroupKey kModeSelect = "ModeSelect";

constexpr GroupKey kNetworkCommissioning = "NetworkCommissioning";

constexpr GroupKey kOnOff = "OnOff";

constexpr GroupKey kOperationalCredentials = "OperationalCredentials";

constexpr GroupKey kPASESession = "PASESession";

constexpr GroupKey kPacketParser = "PacketParser";

constexpr GroupKey kResolver = "Resolver";

constexpr GroupKey kScenes = "Scenes";

constexpr GroupKey kSessionManager = "SessionManager";

constexpr GroupKey kWiFiDiagnosticsDelegate = "WiFiDiagnosticsDelegate";

/**
 * Array of all group keys
 */
constexpr GroupKey kAllGroupKeys[] = {
    kAdministratorCommissioning,
    kBasicInfo,
    kBridgeBasicInfo,
    kCASEServer,
    kCASESession,
    kColorControl,
    kDeviceCommissioner,
    kFabric,
    kG,
    kGeneralCommissioning,
    kGroup,
    kGroups,
    kIdentify,
    kJointFabricAdministrator,
    kLevelControl,
    kLowPower,
    kMinMdnsResolver,
    kModeBase,
    kModeSelect,
    kNetworkCommissioning,
    kOnOff,
    kOperationalCredentials,
    kPASESession,
    kPacketParser,
    kResolver,
    kScenes,
    kSessionManager,
    kWiFiDiagnosticsDelegate,
}; // kAllGroupKeys

} // namespace Tracing
} // namespace chip
