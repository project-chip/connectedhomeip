/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

public interface ResubscriptionAttemptCallback {
  void onResubscriptionAttempt(long terminationCause, long nextResubscribeIntervalMsec);
}
