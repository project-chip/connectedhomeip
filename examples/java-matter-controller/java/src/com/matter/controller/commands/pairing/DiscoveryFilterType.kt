/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.pairing

enum class DiscoveryFilterType {
  NONE,
  SHORT_DISCRIMINATOR,
  LONG_DISCRIMINATOR,
  VENDOR_ID,
  DEVICE_TYPE,
  COMMISSIONING_MODE,
  INSTANCE_NAME,
  COMMISSIONER,
  COMPRESSED_FABRIC_ID
}
