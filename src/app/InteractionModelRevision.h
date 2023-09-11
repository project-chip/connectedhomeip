/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
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
 *
 * See section 8.1.1. "Revision History" in the "Interaction Model
 * Specification" chapter of the core Matter specification.
 */
#ifndef CHIP_DEVICE_INTERACTION_MODEL_REVISION
#define CHIP_DEVICE_INTERACTION_MODEL_REVISION 10
#endif

constexpr uint8_t kInteractionModelRevisionTag = 0xFF;
