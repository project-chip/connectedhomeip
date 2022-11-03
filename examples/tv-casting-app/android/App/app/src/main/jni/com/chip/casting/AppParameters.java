/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.chip.casting;

public class AppParameters {
  public static final int MIN_ROTATING_DEVICE_ID_UNIQUE_ID_LENGTH = 16;

  private byte[] rotatingDeviceIdUniqueId;

  public void setRotatingDeviceIdUniqueId(byte[] rotatingDeviceIdUniqueId) {
    this.rotatingDeviceIdUniqueId = rotatingDeviceIdUniqueId;
  }

  public byte[] getRotatingDeviceIdUniqueId() {
    return rotatingDeviceIdUniqueId;
  }
}
