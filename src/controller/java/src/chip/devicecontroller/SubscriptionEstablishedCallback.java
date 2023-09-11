/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package chip.devicecontroller;

public interface SubscriptionEstablishedCallback {
  void onSubscriptionEstablished(long subscriptionId);
}
