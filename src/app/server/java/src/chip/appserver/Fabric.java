/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.appserver;

public class Fabric {

  public int vendorId;
  public long nodeId;
  public short fabricIndex;
  public String label;

  @Override
  public String toString() {
    return "Fabric [fabricIndex="
        + fabricIndex
        + ", label="
        + label
        + ", nodeId="
        + nodeId
        + ", vendorId="
        + vendorId
        + "]";
  }
}
