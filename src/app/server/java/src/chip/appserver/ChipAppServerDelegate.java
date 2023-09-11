/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.appserver;

public interface ChipAppServerDelegate {
  void onCommissioningSessionEstablishmentStarted();

  void onCommissioningSessionStarted();

  void onCommissioningSessionEstablishmentError(int errorCode);

  void onCommissioningSessionStopped();

  void onCommissioningWindowOpened();

  void onCommissioningWindowClosed();
}
