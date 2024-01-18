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
package com.matter.controller.commands.icd

import chip.devicecontroller.ChipDeviceController
import com.matter.controller.commands.common.CredentialsIssuer
import com.matter.controller.commands.common.MatterCommand

class ICDListCommand(val controller: ChipDeviceController, credsIssuer: CredentialsIssuer?) :
  MatterCommand(controller, credsIssuer, "list") {

  override fun runCommand() {
    val info = controller.icdClientInfo
    println("  +-----------------------------------------------------------------------------+")
    System.out.format(
      "  | %-52s %10s : %8d |\n",
      "Known ICDs",
      "FabricIndex",
      controller.fabricIndex
    )
    println("  +-----------------------------------------------------------------------------+")
    System.out.format(
      "  | %20s | %15s | %15s | %16s |\n",
      "Node ID",
      "Start Counter",
      "Counter Offset",
      "MonitoredSubject"
    )

    for (i in info) {
      println("  +-----------------------------------------------------------------------------+")
      System.out.format(
        "  | %20d | %15d | %15d | %16d |\n",
        i.peerNodeId,
        i.startCounter,
        i.offset,
        i.monitoredSubject
      )
      System.out.format("  | %-10s : %62s |\n", "aes key", i.icdAesKey.toHex())
      System.out.format("  | %-10s : %62s |\n", "hamc key", i.icdHmacKey.toHex())
    }
    println("  +-----------------------------------------------------------------------------+")
  }

  private fun ByteArray.toHex(): String =
    joinToString(separator = "") { eachByte -> "%02x".format(eachByte) }
}
