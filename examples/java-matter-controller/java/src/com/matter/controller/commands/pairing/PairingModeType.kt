/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.pairing

enum class PairingModeType {
  NONE,
  CODE,
  CODE_PASE_ONLY,
  ADDRESS_PASE_ONLY,
  BLE,
  SOFT_AP,
  ALREADY_DISCOVERED,
  ON_NETWORK
}
