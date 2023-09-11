/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file declares test entry point for CHIP over BLE code unit tests in Linux
 *
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
int TestCHIPoBLEStackManager();
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
