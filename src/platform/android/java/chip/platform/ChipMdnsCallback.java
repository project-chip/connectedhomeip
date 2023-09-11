/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

import java.util.Map;

/** Interface for communicating with the CHIP mDNS stack. */
public interface ChipMdnsCallback {
  void handleServiceResolve(
      String instanceName,
      String serviceType,
      String hostName,
      String address,
      int port,
      Map<String, byte[]> textEntries,
      long callbackHandle,
      long contextHandle);

  void handleServiceBrowse(
      String[] instanceName, String serviceType, long callbackHandle, long contextHandle);
}
