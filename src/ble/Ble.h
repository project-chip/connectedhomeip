/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2015-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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
