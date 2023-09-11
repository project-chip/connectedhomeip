/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * This file includes a specific configured concrete implementation of System::Layer.
 */

#pragma once

#include <system/SystemLayer.h>

#ifdef CHIP_SYSTEM_LAYER_IMPL_CONFIG_FILE
#include CHIP_SYSTEM_LAYER_IMPL_CONFIG_FILE
#else // CHIP_SYSTEM_LAYER_IMPL_CONFIG_FILE
#include <system/SystemLayerImplSelect.h>
#endif // CHIP_SYSTEM_LAYER_IMPL_CONFIG_FILE
#undef INCLUDING_CHIP_SYSTEM_LAYER_IMPL_CONFIG_FILE
