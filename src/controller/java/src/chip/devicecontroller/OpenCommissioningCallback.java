/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

public interface OpenCommissioningCallback {
  public void onError(int status, long deviceId);

  public void onSuccess(long deviceId, String manualPairingCode, String qrCode);
}
