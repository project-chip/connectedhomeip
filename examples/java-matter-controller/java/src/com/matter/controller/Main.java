/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package com.matter.controller;

import chip.devicecontroller.ChipDeviceController;
import chip.devicecontroller.ControllerParams;

public class Main {
  public static void main(String[] args) {
    ChipDeviceController controller =
        new ChipDeviceController(
            ControllerParams.newBuilder()
                .setUdpListenPort(0)
                .setControllerVendorId(0xFFF1)
                .build());
    System.out.println("Hello Matter Controller!");

    for (String s : args) {
      System.out.println(s);
    }
  }
}
