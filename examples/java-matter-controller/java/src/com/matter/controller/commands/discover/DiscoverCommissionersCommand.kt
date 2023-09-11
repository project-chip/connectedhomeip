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

class DiscoverCommissionersCommand(
  controller: ChipDeviceController,
  credsIssuer: CredentialsIssuer?
) : MatterCommand(controller, credsIssuer, "commissioners") {
  override fun runCommand() {}
}
