/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "BluetoothWidget.h"
#include "LEDWidget.h"
#include "WiFiWidget.h"
#include "transport/raw/MessageHeader.h"

extern LEDWidget statusLED1;
extern LEDWidget statusLED2;
extern BluetoothWidget bluetoothLED;
extern WiFiWidget wifiLED;
extern const chip::NodeId kLocalNodeId;
extern WiFiWidget pairingWindowLED;
