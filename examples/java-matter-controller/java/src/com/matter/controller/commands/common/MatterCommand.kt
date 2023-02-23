/*
 *   Copyright (c) 2022 Project CHIP Authors
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
import java.util.Optional
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicLong
import java.util.logging.Logger

abstract class MatterCommand(
  controller: ChipDeviceController,
  commandName: String?,
  credIssuerCmds: CredentialsIssuer?,
  helpText: String?
) : Command(commandName!!, helpText) {
  private val logger = Logger.getLogger(MatterCommand::class.java.name)
  private val mChipDeviceController: ChipDeviceController
  private val mCredIssuerCmds: Optional<CredentialsIssuer>
  private val mCommissionerName = StringBuffer()
  private val mPaaTrustStorePath = StringBuffer()
  private val mCDTrustStorePath = StringBuffer()
  private val mCommissionerNodeId: AtomicLong = AtomicLong()
  private val mUseMaxSizedCerts: AtomicBoolean = AtomicBoolean()
  private val mOnlyAllowTrustedCdKeys: AtomicBoolean = AtomicBoolean()
  private val mFutureResult = FutureResult()

  constructor(
    controller: ChipDeviceController, commandName: String?, credIssuerCmds: CredentialsIssuer?
  ) : this(controller, commandName, credIssuerCmds, null)

  init {
    mCredIssuerCmds = Optional.ofNullable(credIssuerCmds)
    mChipDeviceController = controller
    addArgument(
      "paa-trust-store-path",
      mPaaTrustStorePath,
      "Path to directory holding PAA certificate information.  Can be absolute or relative to the current working "
              + "directory.",
      true
    )
    addArgument(
      "cd-trust-store-path",
      mCDTrustStorePath,
      "Path to directory holding CD certificate information.  Can be absolute or relative to the current working "
              + "directory.",
      true
    )
    addArgument(
      "commissioner-name",
      mCommissionerName,
      "Name of fabric to use. Valid values are \"alpha\", \"beta\", \"gamma\", and integers greater than or equal to "
              + "4.  The default if not specified is \"alpha\".",
      true
    )
    addArgument(
      "commissioner-nodeid",
      0, Long.MAX_VALUE,
      mCommissionerNodeId,
      "The node id to use for java-matter-controller.  If not provided, kTestControllerNodeId (112233, 0x1B669) will be used.",
      true
    )
    addArgument(
      "use-max-sized-certs",
      mUseMaxSizedCerts,
      "Maximize the size of operational certificates. If not provided or 0 (\"false\"), normally sized operational "
              + "certificates are generated.",
      true
    )
    addArgument(
      "only-allow-trusted-cd-keys",
      mOnlyAllowTrustedCdKeys,
      "Only allow trusted CD verifying keys (disallow test keys). If not provided or 0 (\"false\"), untrusted CD "
              + "verifying keys are allowed. If 1 (\"true\"), test keys are disallowed.",
      true
    )
  }

  // This method returns the commissioner instance to be used for running the command.
  fun currentCommissioner(): ChipDeviceController {
    return mChipDeviceController
  }

  /////////// Command Interface /////////
  @Throws(Exception::class)
  override fun run() {
    runCommand()
  }

  protected abstract fun runCommand()
  fun setSuccess() {
    mFutureResult.setRealResult(RealResult.Success())
  }

  fun setFailure(error: String?) {
    mFutureResult.setRealResult(RealResult.Error(error))
  }

  fun waitCompleteMs(timeoutMs: Long) {
    mFutureResult.setTimeoutMs(timeoutMs)
    mFutureResult.waitResult()
  }
}