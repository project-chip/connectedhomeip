/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.matter.virtual.device.app;

public interface DeviceAppCallback {
  void onClusterInit(DeviceApp app, long clusterId, int endpoint);

  void onEvent(long event);
}
