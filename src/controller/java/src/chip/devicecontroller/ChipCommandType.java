/*
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

public enum ChipCommandType {
  OFF(0),
  ON(1),
  TOGGLE(2),
  LEVEL(3);

  private int value;

  private ChipCommandType(int value) {
    this.value = value;
  }

  public int getValue() {
    return value;
  }
}
