/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Overrides to default OpenThread configuration.
 *
 */

#pragma once

#ifndef OPENTHREAD_CONFIG_LOG_LEVEL
#define OPENTHREAD_CONFIG_LOG_LEVEL OT_LOG_LEVEL_CRIT
#endif

#define OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE 1
#define OPENTHREAD_CONFIG_PLATFORM_ASSERT_MANAGEMENT 1
#define OPENTHREAD_CONFIG_SRP_CLIENT_BUFFERS_MAX_HOST_ADDRESSES 4
#define OPENTHREAD_CONFIG_TCP_ENABLE 0

#include <openthread-core-cyw30739-config.h>
