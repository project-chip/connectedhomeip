/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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
 *      This file is an umbrella header for the Ble library, a
 *      portable Bluetooth Low Energy (BLE), also known as Bluetooth
 *      Smart, layer for transporting chip over a BLE connection.
 *
 */

#pragma once

#include <ble/BleConfig.h>

#include <ble/BLEEndPoint.h>
#include <ble/BleApplicationDelegate.h>
#include <ble/BleConnectionDelegate.h>
#include <ble/BleError.h>
#include <ble/BleLayer.h>
#include <ble/BlePlatformDelegate.h>
#include <ble/BleUUID.h>
#include <ble/BtpEngine.h>

/**
 *   @namespace Ble
 *
 *   @brief
 *     This namespace includes all interfaces within chip for
 *     Bluetooth Low Energy (BLE), also known as Bluetooth Smart.
 */
