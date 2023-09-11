/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

public interface DiagnosticDataProvider {
  int getRebootCount();

  NetworkInterface[] getNetworkInterfaces();
}
