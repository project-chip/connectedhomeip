/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.discover

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer
import com.matter.controller.commands.common.MatterCommand
import java.util.concurrent.atomic.AtomicLong

class DiscoverCommand(controller: ChipDeviceController, credsIssuer: CredentialsIssuer?) :
  MatterCommand(controller, credsIssuer, "resolve") {
  private val nodeId: AtomicLong = AtomicLong()
  private val fabricId: AtomicLong = AtomicLong()

  init {
    addArgument("nodeid", 0, Long.MAX_VALUE, nodeId, null, false)
    addArgument("fabricid", 0, Long.MAX_VALUE, fabricId, null, false)
  }

  override fun runCommand() {}
}
