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

package com.matter.controller.commands.common;

import chip.devicecontroller.ChipDeviceController;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;
import java.util.logging.Logger;

public abstract class MatterCommand extends Command {
  private final ChipDeviceController mChipDeviceController;
  private final Optional<CredentialsIssuer> mCredIssuerCmds;
  private final StringBuffer mCommissionerName = new StringBuffer();
  private final StringBuffer mPaaTrustStorePath = new StringBuffer();
  private final StringBuffer mCDTrustStorePath = new StringBuffer();
  private final AtomicLong mCommissionerNodeId = new AtomicLong();
  private final AtomicBoolean mUseMaxSizedCerts = new AtomicBoolean();
  private final AtomicBoolean mOnlyAllowTrustedCdKeys = new AtomicBoolean();
  private FutureResult mFutureResult = new FutureResult();
  private static Logger logger = Logger.getLogger(MatterCommand.class.getName());

  public MatterCommand(
      ChipDeviceController controller, String commandName, CredentialsIssuer credIssuerCmds) {
    this(controller, commandName, credIssuerCmds, null);
  }

  public MatterCommand(
      ChipDeviceController controller,
      String commandName,
      CredentialsIssuer credIssuerCmds,
      String helpText) {
    super(commandName, helpText);
    this.mCredIssuerCmds = Optional.ofNullable(credIssuerCmds);
    this.mChipDeviceController = controller;

    addArgument(
        "paa-trust-store-path",
        mPaaTrustStorePath,
        "Path to directory holding PAA certificate information.  Can be absolute or relative to the current working "
            + "directory.",
        true);
    addArgument(
        "cd-trust-store-path",
        mCDTrustStorePath,
        "Path to directory holding CD certificate information.  Can be absolute or relative to the current working "
            + "directory.",
        true);
    addArgument(
        "commissioner-name",
        mCommissionerName,
        "Name of fabric to use. Valid values are \"alpha\", \"beta\", \"gamma\", and integers greater than or equal to "
            + "4.  The default if not specified is \"alpha\".",
        true);
    addArgument(
        "commissioner-nodeid",
        0,
        Long.MAX_VALUE,
        mCommissionerNodeId,
        "The node id to use for java-matter-controller.  If not provided, kTestControllerNodeId (112233, 0x1B669) will be used.",
        true);
    addArgument(
        "use-max-sized-certs",
        mUseMaxSizedCerts,
        "Maximize the size of operational certificates. If not provided or 0 (\"false\"), normally sized operational "
            + "certificates are generated.",
        true);
    addArgument(
        "only-allow-trusted-cd-keys",
        mOnlyAllowTrustedCdKeys,
        "Only allow trusted CD verifying keys (disallow test keys). If not provided or 0 (\"false\"), untrusted CD "
            + "verifying keys are allowed. If 1 (\"true\"), test keys are disallowed.",
        true);
  }

  // This method returns the commissioner instance to be used for running the command.
  public ChipDeviceController currentCommissioner() {
    return mChipDeviceController;
  }

  /////////// Command Interface /////////
  @Override
  public void run() throws Exception {
    runCommand();
  }

  protected abstract void runCommand();

  public void setSuccess() {
    mFutureResult.setRealResult(RealResult.Success());
  }

  public void setFailure(String error) {
    mFutureResult.setRealResult(RealResult.Error(error));
  }

  public void waitCompleteMs(long timeoutMs) {
    mFutureResult.setTimeoutMs(timeoutMs);
    mFutureResult.waitResult();
  }
}
