/*
 *   Copyright (c) 2020-2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package chip.devicecontroller;

import javax.annotation.Nullable;

/** Class for holding WiFi or Thread credentials, but not both. */
public class NetworkCredentials {
  @Nullable private WiFiCredentials wifiCredentials;
  @Nullable private ThreadCredentials threadCredentials;

  public NetworkCredentials() {}

  private NetworkCredentials(
      @Nullable WiFiCredentials wifiCredentials, @Nullable ThreadCredentials threadCredentials) {
    this.wifiCredentials = wifiCredentials;
    this.threadCredentials = threadCredentials;
  }

  /**
   * Return a NetworkCredentials object with the given WiFiCredentials and null ThreadCredentials.
   */
  public static NetworkCredentials forWiFi(WiFiCredentials wifiCredentials) {
    return new NetworkCredentials(wifiCredentials, null);
  }

  /**
   * Return a NetworkCredentials object with the given ThreadCredentials and null WiFiCredentials.
   */
  public static NetworkCredentials forThread(ThreadCredentials threadCredentials) {
    return new NetworkCredentials(null, threadCredentials);
  }

  public WiFiCredentials getWiFiCredentials() {
    return wifiCredentials;
  }

  public ThreadCredentials getThreadCredentials() {
    return threadCredentials;
  }

  public void setWiFiCredentials(String ssid, String password) {
    wifiCredentials.set(ssid, password);
  }

  public void setThreadCredentials(byte[] operationalDataset) {
    threadCredentials.set(operationalDataset);
  }

  public static class WiFiCredentials {
    private String ssid;
    private String password;

    public WiFiCredentials(String ssid, String password) {
      set(ssid, password);
    }

    public void set(String ssid, String password) {
      this.ssid = ssid;
      this.password = password;
    }

    public String getSsid() {
      return ssid;
    }

    public String getPassword() {
      return password;
    }
  }

  public static class ThreadCredentials {
    private byte[] operationalDataset;

    public ThreadCredentials(byte[] operationalDataset) {
      set(operationalDataset);
    }

    public void set(byte[] operationalDataset) {
      this.operationalDataset = operationalDataset;
    }

    public byte[] getOperationalDataset() {
      return operationalDataset;
    }
  }
}
