/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

public interface UnpairDeviceCallback {
  public void onError(int status, long remoteDeviceId);

  public void onSuccess(long remoteDeviceId);
}
