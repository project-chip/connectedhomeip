/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ControllerParams
import com.matter.controller.commands.common.*
import com.matter.controller.commands.discover.*
import com.matter.controller.commands.pairing.*

private fun getDiscoveryCommands(
  controller: ChipDeviceController,
  credentialsIssuer: CredentialsIssuer
): List<Command> {
  return listOf(
    DiscoverCommand(controller, credentialsIssuer),
    DiscoverCommissionablesCommand(controller, credentialsIssuer),
    DiscoverCommissionersCommand(controller, credentialsIssuer),
  )
}

private fun getPairingCommands(
  controller: ChipDeviceController,
  credentialsIssuer: CredentialsIssuer
): List<Command> {
  return listOf(
    UnpairCommand(controller, credentialsIssuer),
    PairCodeCommand(controller, credentialsIssuer),
    PairCodePaseCommand(controller, credentialsIssuer),
    PairCodeWifiCommand(controller, credentialsIssuer),
    PairCodeThreadCommand(controller, credentialsIssuer),
    PairAddressPaseCommand(controller, credentialsIssuer),
    PairAlreadyDiscoveredCommand(controller, credentialsIssuer),
    PairOnNetworkCommand(controller, credentialsIssuer),
    PairOnNetworkShortCommand(controller, credentialsIssuer),
    PairOnNetworkLongCommand(controller, credentialsIssuer),
    PairOnNetworkVendorCommand(controller, credentialsIssuer),
    PairOnNetworkCommissioningModeCommand(controller, credentialsIssuer),
    PairOnNetworkCommissionerCommand(controller, credentialsIssuer),
    PairOnNetworkDeviceTypeCommand(controller, credentialsIssuer),
    PairOnNetworkInstanceNameCommand(controller, credentialsIssuer),
  )
}

private fun getImCommands(
  controller: ChipDeviceController,
  credentialsIssuer: CredentialsIssuer
): List<Command> {
  return listOf(
    PairOnNetworkLongImReadCommand(controller, credentialsIssuer),
    PairOnNetworkLongImSubscribeCommand(controller, credentialsIssuer),
    PairOnNetworkLongImWriteCommand(controller, credentialsIssuer),
    PairOnNetworkLongImInvokeCommand(controller, credentialsIssuer),
  )
}

fun main(args: Array<String>) {
  val controller =
    ChipDeviceController(
      ControllerParams.newBuilder()
        .setUdpListenPort(0)
        .setControllerVendorId(0xFFF1)
        .setCountryCode("US")
        .build()
    )
  val credentialsIssuer = CredentialsIssuer()
  val commandManager = CommandManager()

  commandManager.register("discover", getDiscoveryCommands(controller, credentialsIssuer))
  commandManager.register("pairing", getPairingCommands(controller, credentialsIssuer))
  commandManager.register("im", getImCommands(controller, credentialsIssuer))

  try {
    commandManager.run(args)
  } catch (e: Exception) {
    println("Run command failed with exception: " + e.message)
    System.exit(1)
  }

  controller.shutdownCommissioning()
}
