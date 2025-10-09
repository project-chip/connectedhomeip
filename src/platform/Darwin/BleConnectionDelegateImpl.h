/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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

#include <ble/Ble.h>
#include <lib/support/Span.h>
#include <platform/Darwin/BleScannerDelegate.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BleConnectionDelegateImpl : public Ble::BleConnectionDelegate
{
public:
    void StartScan(BleScannerDelegate * delegate, BleScanMode mode = BleScanMode::kDefault);
    void StopScan();

    void NewConnection(Ble::BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator) override;
    void NewConnection(Ble::BleLayer * bleLayer, void * appState, BLE_CONNECTION_OBJECT connObj) override;
    CHIP_ERROR NewConnection(Ble::BleLayer * bleLayer, void * appState, const Span<const SetupDiscriminator> & discriminators,
                             OnConnectionByDiscriminatorsCompleteFunct onConnectionComplete,
                             OnConnectionErrorFunct onConnectionError) override;
    CHIP_ERROR CancelConnection() override;

private:
    CHIP_ERROR DoCancel();

    // Caller should pass in only one of onConnectionComplete and onConnectionCompleteWithDiscriminator
    void
    NewConnectionImpl(Ble::BleLayer * bleLayer, void * appState, const chip::Span<const SetupDiscriminator> & desiredDiscriminators,
                      Ble::BleConnectionDelegate::OnConnectionCompleteFunct onConnectionComplete,
                      Ble::BleConnectionDelegate::OnConnectionByDiscriminatorsCompleteFunct onConnectionCompleteWithDiscriminator,
                      Ble::BleConnectionDelegate::OnConnectionErrorFunct onConnectionError);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
