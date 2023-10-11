/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.matter.controller

import com.matter.controller.commands.common.Command
import com.matter.controller.commands.common.CommandManager
import com.matter.controller.commands.common.CredentialsIssuer
import com.matter.controller.commands.pairing.PairOnNetworkLongCommand
import com.matter.controller.commands.pairing.PairOnNetworkLongImInvokeCommand
import com.matter.controller.commands.pairing.PairOnNetworkLongImReadCommand
import com.matter.controller.commands.pairing.PairOnNetworkLongImSubscribeCommand
import com.matter.controller.commands.pairing.PairOnNetworkLongImWriteCommand
import matter.controller.ControllerParams
import matter.controller.MatterController
import matter.controller.MatterControllerImpl

private fun getPairingCommands(
  controller: MatterController,
  credentialsIssuer: CredentialsIssuer
): List<Command> {
  return listOf(
    PairOnNetworkLongCommand(controller, credentialsIssuer),
  )
}

private fun getImCommands(
  controller: MatterController,
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
  val controller: MatterController = MatterControllerImpl(ControllerParams(countryCode = "US"))
  val credentialsIssuer = CredentialsIssuer()
  val commandManager = CommandManager()

  commandManager.register("pairing", getPairingCommands(controller, credentialsIssuer))
  commandManager.register("im", getImCommands(controller, credentialsIssuer))

  try {
    commandManager.run(args)
  } catch (e: Exception) {
    println("Run command failed with exception: " + e.message)
    controller.close()
    System.exit(1)
  }

  controller.close()
}
