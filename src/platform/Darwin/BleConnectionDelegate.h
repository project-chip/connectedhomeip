/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
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
    virtual void NewConnection(Ble::BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator);
    virtual CHIP_ERROR CancelConnection();
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
