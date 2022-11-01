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

import com.matter.controller.config.PersistentStorage;
import com.matter.controller.config.PersistentStorageOpCertStore;
import com.matter.controller.config.PersistentStorageOperationalKeystore;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;

public abstract class MatterCommand extends Command {
  private final PersistentStorage mDefaultStorage = new PersistentStorage();
  private final PersistentStorage mCommissionerStorage = new PersistentStorage();
  private final PersistentStorageOperationalKeystore mOperationalKeystore =
      new PersistentStorageOperationalKeystore();
  private final PersistentStorageOpCertStore mOpCertStore = new PersistentStorageOpCertStore();

  private final Optional<CredentialsIssuer> mCredIssuerCmds;
  private final StringBuffer mCommissionerName = new StringBuffer();
  private final StringBuffer mPaaTrustStorePath = new StringBuffer();
  private final StringBuffer mCDTrustStorePath = new StringBuffer();
  private final AtomicLong mCommissionerNodeId = new AtomicLong();
  private final AtomicBoolean mUseMaxSizedCerts = new AtomicBoolean();;
  private final AtomicBoolean mOnlyAllowTrustedCdKeys = new AtomicBoolean();;

  public MatterCommand(String commandName, CredentialsIssuer credIssuerCmds) {
    this(commandName, credIssuerCmds, null);
  }

  public MatterCommand(String commandName, CredentialsIssuer credIssuerCmds, String helpText) {
    super(commandName, helpText);
    this.mCredIssuerCmds = Optional.ofNullable(credIssuerCmds);

    addArgument(
        "paa-trust-store-path",
        mPaaTrustStorePath,
        "Path to directory holding PAA certificate information.  Can be absolute or relative to the current working "
            + "directory.");
    addArgument(
        "cd-trust-store-path",
        mCDTrustStorePath,
        "Path to directory holding CD certificate information.  Can be absolute or relative to the current working "
            + "directory.");
    addArgument(
        "commissioner-name",
        mCommissionerName,
        "Name of fabric to use. Valid values are \"alpha\", \"beta\", \"gamma\", and integers greater than or equal to "
            + "4.  The default if not specified is \"alpha\".");
    addArgument(
        "commissioner-nodeid",
        0,
        Long.MAX_VALUE,
        mCommissionerNodeId,
        "The node id to use for chip-tool.  If not provided, kTestControllerNodeId (112233, 0x1B669) will be used.");
    addArgument(
        "use-max-sized-certs",
        mUseMaxSizedCerts,
        "Maximize the size of operational certificates. If not provided or 0 (\"false\"), normally sized operational "
            + "certificates are generated.");
    addArgument(
        "only-allow-trusted-cd-keys",
        mOnlyAllowTrustedCdKeys,
        "Only allow trusted CD verifying keys (disallow test keys). If not provided or 0 (\"false\"), untrusted CD "
            + "verifying keys are allowed. If 1 (\"true\"), test keys are disallowed.");
  }

  /////////// Command Interface /////////
  @Override
  public void run() throws Exception {
    maybeSetUpStack();
    runCommand();
    maybeTearDownStack();
  }

  protected abstract void runCommand();

  private void maybeSetUpStack() throws Exception {
    mDefaultStorage.init();
    mOperationalKeystore.init(mDefaultStorage);
    mOpCertStore.init(mDefaultStorage);
  }

  private void maybeTearDownStack() {
    // ToDo:We need to call DeviceController::Shutdown()
  }
}
