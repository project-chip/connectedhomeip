/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.platform;

public interface BleCallback {
  void onCloseBleComplete(int connId);

  void onNotifyChipConnectionClosed(int connId);
}
