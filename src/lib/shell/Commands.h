/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

namespace chip {
namespace Shell {

/**
 * This function registers the base64 encode/decode commands.
 *
 */
void RegisterBase64Commands();

/**
 * This function registers the BLE commands.
 *
 */
void RegisterBLECommands();

/**
 * This function registers the common commands.
 *
 */
void RegisterMetaCommands();

/**
 * This function registers the device configuration commands.
 *
 */
void RegisterConfigCommands();

/**
 * This function registers the device management commands.
 *
 */
void RegisterDeviceCommands();

/**
 * This function registers the OTA (Over-the-Air) software update commands.
 *
 */
void RegisterOtaCommands();

/**
 * This function registers the resource usage statistics commands.
 *
 */
void RegisterStatCommands();

/**
 * This function registers the device onboarding codes commands.
 *
 */
void RegisterOnboardingCodesCommands();

/**
 * This function registers the wifi commands.
 *
 */
void RegisterWiFiCommands();

/**
 * This function registers the commissioning commands.
 *
 */
void RegisterCommissioningCommands();

/**
 * This function registers the NFC commands.
 *
 */
void RegisterNFCCommands();

/**
 * This function registers the DNS client commands.
 *
 */
void RegisterDnsCommands();

} // namespace Shell
} // namespace chip
