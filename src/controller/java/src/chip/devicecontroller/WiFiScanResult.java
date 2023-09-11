/*
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

import java.util.Arrays;

public class WiFiScanResult {
  public Integer security;
  public byte[] ssid;
  public byte[] bssid;
  public Integer channel;
  public Integer wiFiBand;
  public Integer rssi;

  public WiFiScanResult(
      Integer security,
      byte[] ssid,
      byte[] bssid,
      Integer channel,
      Integer wiFiBand,
      Integer rssi) {
    this.security = security;
    this.ssid = ssid;
    this.bssid = bssid;
    this.channel = channel;
    this.wiFiBand = wiFiBand;
    this.rssi = rssi;
  }

  @Override
  public String toString() {
    StringBuilder output = new StringBuilder();
    output.append("WiFiScanResult {\n");
    output.append("\tsecurity: ");
    output.append(security);
    output.append("\n");
    output.append("\tssid: ");
    output.append(Arrays.toString(ssid));
    output.append("\n");
    output.append("\tbssid: ");
    output.append(Arrays.toString(bssid));
    output.append("\n");
    output.append("\tchannel: ");
    output.append(channel);
    output.append("\n");
    output.append("\twiFiBand: ");
    output.append(wiFiBand);
    output.append("\n");
    output.append("\trssi: ");
    output.append(rssi);
    output.append("\n");
    output.append("}\n");
    return output.toString();
  }
}
