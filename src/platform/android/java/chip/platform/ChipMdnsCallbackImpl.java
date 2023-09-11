/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

import java.util.Map;

public class ChipMdnsCallbackImpl implements ChipMdnsCallback {
  public native void handleServiceResolve(
      String instanceName,
      String serviceType,
      String hostName,
      String address,
      int port,
      Map<String, byte[]> textEntries,
      long callbackHandle,
      long contextHandle);

  public native void handleServiceBrowse(
      String[] instanceName, String serviceType, long callbackHandle, long contextHandle);

  public String[] getTextEntryKeys(Map<String, byte[]> textEntries) {
    return textEntries.keySet().toArray(new String[textEntries.size()]);
  }

  public byte[] getTextEntryData(Map<String, byte[]> textEntries, String key) {
    return textEntries.get(key);
  }
}
