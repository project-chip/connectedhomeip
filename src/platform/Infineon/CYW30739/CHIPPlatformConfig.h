/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *    Platform-specific configuration overrides for CHIP.
 */

#pragma once

#define CHIP_CONFIG_MAX_FABRICS 5
#define CHIP_CONFIG_UNAUTHENTICATED_CONNECTION_POOL_SIZE 10
#define CHIP_DEVICE_CONFIG_ENABLE_JUST_IN_TIME_PROVISIONING 1
#define CHIP_DEVICE_CONFIG_ENABLE_THREAD_COMMISSIONABLE_DISCOVERY 1
