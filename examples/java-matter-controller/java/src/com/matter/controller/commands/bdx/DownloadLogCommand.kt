/*
 *   Copyright (c) 2024 Project CHIP Authors
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
package com.matter.controller.commands.bdx

import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.DiagnosticLogType
import chip.devicecontroller.DownloadLogCallback
import com.matter.controller.commands.common.CredentialsIssuer
import com.matter.controller.commands.common.MatterCommand
import java.util.concurrent.TimeUnit
import java.util.concurrent.atomic.AtomicInteger
import java.util.concurrent.atomic.AtomicLong

class DownloadLogCommand(controller: ChipDeviceController, credsIssuer: CredentialsIssuer?) :
  MatterCommand(controller, credsIssuer, "downloadLog") {
  private val nodeId: AtomicLong = AtomicLong()
  private val logType: StringBuffer = StringBuffer()
  private val timeout: AtomicInteger = AtomicInteger()
  private val buffer: StringBuffer = StringBuffer()

  init {
    addArgument("nodeid", 0, Long.MAX_VALUE, nodeId, null, false)
    addArgument("logType", logType, null, false)
    addArgument("timeout", 0, Int.MAX_VALUE, timeout, null, false)
  }

  override fun runCommand() {
    currentCommissioner()
      .downloadLogFromNode(
        nodeId.toLong(),
        DiagnosticLogType.value(logType.toString()),
        timeout.toLong(),
        object : DownloadLogCallback {
          override fun onError(fabricIndex: Int, nodeId: Long, errorCode: Long) {
            println("error :")
            println("FabricIndex : $fabricIndex, NodeID : $nodeId, errorCode : $errorCode")
          }

          override fun onSuccess(fabricIndex: Int, nodeId: Long) {
            println()
            println("FabricIndex : $fabricIndex, NodeID : $nodeId")
            println(buffer.toString())
            println("Log Download Finish!")
          }

          override fun onTransferData(fabricIndex: Int, nodeId: Long, data: ByteArray): Boolean {
            buffer.append(String(data))
            return true
          }
        }
      )
    try {
      TimeUnit.SECONDS.sleep(timeout.toLong())
    } catch (e: InterruptedException) {
      throw RuntimeException(e)
    }
  }
}
