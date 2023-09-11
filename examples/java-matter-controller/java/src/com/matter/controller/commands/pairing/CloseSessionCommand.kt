/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.pairing

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer
import com.matter.controller.commands.common.MatterCommand
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicLong

class CloseSessionCommand(controller: ChipDeviceController, credsIssuer: CredentialsIssuer?) :
  MatterCommand(controller, credsIssuer, "close-session") {
  private val destinationId: AtomicLong = AtomicLong()
  private val timeoutSecs: AtomicInteger = AtomicInteger()

  init {
    addArgument("destination-id", 0, Long.MAX_VALUE, destinationId, null, false)
    addArgument(
      "timeout",
      0.toShort(),
      Short.MAX_VALUE,
      timeoutSecs,
      "Time, in seconds, before this command is considered to have timed out.",
      false
    )
  }

  override fun runCommand() {}
}
