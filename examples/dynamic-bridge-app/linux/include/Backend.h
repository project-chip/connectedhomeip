/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "DynamicDevice.h"

void StartUserInput();

extern std::vector<std::unique_ptr<Device>> g_devices;
extern std::vector<std::unique_ptr<DynamicDevice>> g_device_impls;

bool RemoveDeviceAt(uint32_t index);
int AddDevice(std::unique_ptr<DynamicDevice> device);
