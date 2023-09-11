/*
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

import java.util.Arrays;

public class ThreadScanResult {
  public Integer panId;
  public Long extendedPanId;
  public String networkName;
  public Integer channel;
  public Integer version;
  public byte[] extendedAddress;
  public Integer rssi;
  public Integer lqi;

  public ThreadScanResult(
      Integer panId,
      Long extendedPanId,
      String networkName,
      Integer channel,
      Integer version,
      byte[] extendedAddress,
      Integer rssi,
      Integer lqi) {
    this.panId = panId;
    this.extendedPanId = extendedPanId;
    this.networkName = networkName;
    this.channel = channel;
    this.version = version;
    this.extendedAddress = extendedAddress;
    this.rssi = rssi;
    this.lqi = lqi;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("ThreadScanResult {\n");
    output.append("\tpanId: ");
    output.append(panId);
    output.append("\n");
    output.append("\textendedPanId: ");
    output.append(extendedPanId);
    output.append("\n");
    output.append("\tnetworkName: ");
    output.append(networkName);
    output.append("\n");
    output.append("\tchannel: ");
    output.append(channel);
    output.append("\n");
    output.append("\tversion: ");
    output.append(version);
    output.append("\n");
    output.append("\textendedAddress: ");
    output.append(Arrays.toString(extendedAddress));
    output.append("\n");
    output.append("\trssi: ");
    output.append(rssi);
    output.append("\n");
    output.append("\tlqi: ");
    output.append(lqi);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
