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
package com.matter.controller.commands.common

import chip.devicecontroller.ChipDeviceController
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicLong

abstract class MatterCommand(
  private val chipDeviceController: ChipDeviceController,
  private val credIssuerCmds: CredentialsIssuer?,
  commandName: String,
  helpText: String? = null
) : Command(commandName, helpText) {
  private val commissionerName = StringBuffer()
  private val paaTrustStorePath = StringBuffer()
  private val cdTrustStorePath = StringBuffer()
  private val commissionerNodeId: AtomicLong = AtomicLong()
  private val useMaxSizedCerts: AtomicBoolean = AtomicBoolean()
  private val onlyAllowTrustedCdKeys: AtomicBoolean = AtomicBoolean()
  private val futureResult = FutureResult()

  init {
    addArgument(
      "paa-trust-store-path",
      paaTrustStorePath,
      "Path to directory holding PAA certificate information.  Can be absolute or relative to the current working " +
        "directory.",
      true
    )
    addArgument(
      "cd-trust-store-path",
      cdTrustStorePath,
      "Path to directory holding CD certificate information.  Can be absolute or relative to the current working " +
        "directory.",
      true
    )
    addArgument(
      "commissioner-name",
      commissionerName,
      "Name of fabric to use. Valid values are \"alpha\", \"beta\", \"gamma\", and integers greater than or equal to " +
        "4.  The default if not specified is \"alpha\".",
      true
    )
    addArgument(
      "commissioner-nodeid",
      0,
      Long.MAX_VALUE,
      commissionerNodeId,
      "The node id to use for java-matter-controller.  If not provided, kTestControllerNodeId (112233, 0x1B669) will be used.",
      true
    )
    addArgument(
      "use-max-sized-certs",
      useMaxSizedCerts,
      "Maximize the size of operational certificates. If not provided or 0 (\"false\"), normally sized operational " +
        "certificates are generated.",
      true
    )
    addArgument(
      "only-allow-trusted-cd-keys",
      onlyAllowTrustedCdKeys,
      "Only allow trusted CD verifying keys (disallow test keys). If not provided or 0 (\"false\"), untrusted CD " +
        "verifying keys are allowed. If 1 (\"true\"), test keys are disallowed.",
      true
    )
  }

  // This method returns the commissioner instance to be used for running the command.
  fun currentCommissioner(): ChipDeviceController {
    return chipDeviceController
  }

  /////////// Command Interface /////////
  @Throws(Exception::class)
  override fun run() {
    runCommand()
  }

  protected abstract fun runCommand()

  fun setSuccess() {
    futureResult.setRealResult(RealResult.success())
  }

  fun setFailure(error: String?) {
    futureResult.setRealResult(RealResult.error(error))
  }

  fun waitCompleteMs(timeoutMs: Long) {
    futureResult.timeoutMs = timeoutMs
    futureResult.waitResult()
  }

  fun clear() {
    futureResult.clear()
  }
}
