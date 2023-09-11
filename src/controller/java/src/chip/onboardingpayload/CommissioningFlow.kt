/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package chip.onboardingpayload

/**
 * Enum values for possible flows for out-of-box commissioning that a Matter device manufacturer may
 * select for a given product.
 */
enum class CommissioningFlow(val value: Int) {
  STANDARD(0), // Device automatically enters pairing mode upon power-up
  USER_ACTION_REQUIRED(1), // Device requires a user interaction to enter pairing mode
  CUSTOM(2) // Commissioning steps should be retrieved from the distributed compliance ledger
}
