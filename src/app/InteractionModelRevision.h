/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <inttypes.h>
#include <stddef.h>

/**
 * CHIP_DEVICE_INTERACTION_MODEL_REVISION
 *
 * A monothonic number identifying the interaction model revision.
 */
#ifndef CHIP_DEVICE_INTERACTION_MODEL_REVISION
#define CHIP_DEVICE_INTERACTION_MODEL_REVISION 1
#endif

constexpr uint8_t kInteractionModelRevisionTag = 0xFF;
