/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <ble/BleConnectionDelegate.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class BleConnectionDelegateImpl : public Ble::BleConnectionDelegate
{
public:
    void StartScan(BleScannerDelegate * delegate = nullptr);
    void StopScan();
    virtual void NewConnection(Ble::BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator);
    virtual void NewConnection(Ble::BleLayer * bleLayer, void * appState, BLE_CONNECTION_OBJECT connObj);
    virtual CHIP_ERROR CancelConnection();
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
