/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package chip.onboardingpayload

/** Enum values for possible bits in the onboarding paylod's discovery capabilities bitmask. */
enum class DiscoveryCapability {
  SOFT_AP,
  BLE,
  ON_NETWORK
}
