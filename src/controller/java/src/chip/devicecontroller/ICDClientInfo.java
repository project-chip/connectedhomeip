/*
 *   Copyright (c) 2020-2023 Project CHIP Authors
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

/** Class for holding ICD Client information. */
public class ICDClientInfo {
  private final long peerNodeId;
  private final long startCounter;
  private final long offset;
  private final long monitoredSubject;
  private final byte[] icdAesKey;
  private final byte[] icdHmacKey;

  public ICDClientInfo(
      long peerNodeId,
      long startCounter,
      long offset,
      long monitoredSubject,
      byte[] icdAesKey,
      byte[] icdHmacKey) {
    this.peerNodeId = peerNodeId;
    this.startCounter = startCounter;
    this.offset = offset;
    this.monitoredSubject = monitoredSubject;
    this.icdAesKey = icdAesKey;
    this.icdHmacKey = icdHmacKey;
  }

  /** Returns the check in peer node ID. */
  public long getPeerNodeId() {
    return peerNodeId;
  }

  /** Returns the Start Counter. */
  public long getStartCounter() {
    return startCounter;
  }

  /** Returns the Counter offset. */
  public long getOffset() {
    return offset;
  }

  /** Returns the monitored subject of the ICD. */
  public long getMonitoredSubject() {
    return monitoredSubject;
  }

  /** Returns the 16 bytes ICD AES key. */
  public byte[] getIcdAesKey() {
    return icdAesKey;
  }

  /** Returns the 16 bytes ICD HMAC key. */
  public byte[] getIcdHmacKey() {
    return icdHmacKey;
  }

  @Override
  public String toString() {
    return "ICDClientInfo{"
        + "\n\tpeerNodeId="
        + peerNodeId
        + "\n\t, startCounter="
        + startCounter
        + "\n\t, offset="
        + offset
        + "\n\t, monitoredSubject="
        + monitoredSubject
        + "\n\t, icdAesKey='"
        + bytesToHex(icdAesKey)
        + '\''
        + "\n\t, icdHmacKey='"
        + bytesToHex(icdHmacKey)
        + '\''
        + "\n}";
  }

  private static String bytesToHex(byte[] bytes) {
    StringBuilder hexString = new StringBuilder();
    for (byte b : bytes) {
      String hex = Integer.toHexString(0xFF & b);
      if (hex.length() == 1) {
        hexString.append('0');
      }
      hexString.append(hex);
    }
    return hexString.toString();
  }
}
