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

#include "NrfWiFiDriver.h"

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer::Internal;
namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

namespace {
constexpr char kWiFiSSIDKeyName[]        = "wifi-ssid";
constexpr char kWiFiCredentialsKeyName[] = "wifi-pass";
static uint8_t WiFiSSIDStr[DeviceLayer::Internal::kMaxWiFiSSIDLength];
} // namespace

CHIP_ERROR NrfWiFiDriver::Init(NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR NrfWiFiDriver::Shutdown()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR NrfWiFiDriver::CommitConfiguration()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR NrfWiFiDriver::RevertConfiguration()
{
    return CHIP_NO_ERROR;
}

Status NrfWiFiDriver::AddOrUpdateNetwork(ByteSpan ssid, ByteSpan credentials, MutableCharSpan & outDebugText,
                                         uint8_t & outNetworkIndex)
{
    return Status::kSuccess;
}

Status NrfWiFiDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    return Status::kSuccess;
}

Status NrfWiFiDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    // Only one network is supported now
    return Status::kSuccess;
}

void NrfWiFiDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback) {}

CHIP_ERROR GetConfiguredNetwork(Network & network)
{
    return CHIP_NO_ERROR;
}

void NrfWiFiDriver::ScanNetworks(ByteSpan ssid, WiFiDriver::ScanCallback * callback) {}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
