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

public abstract class MatterCommand extends Command {

  public static final short kMaxGroupsPerFabric = 5;
  public static final short kMaxGroupKeysPerFabric = 8;

  protected PersistentStorage mDefaultStorage = new PersistentStorage();
  protected PersistentStorage mCommissionerStorage = new PersistentStorage();
  protected PersistentStorageOperationalKeystore mOperationalKeystore =
      new PersistentStorageOperationalKeystore();
  protected PersistentStorageOpCertStore mOpCertStore = new PersistentStorageOpCertStore();

  protected Optional<CredentialsIssuer> mCredIssuerCmds;
  protected StringBuffer mCommissionerName = new StringBuffer();
  protected StringBuffer mPaaTrustStorePath = new StringBuffer();
  protected StringBuffer mCDTrustStorePath = new StringBuffer();
  protected Optional<Long> mCommissionerNodeId;
  protected Optional<Short> mBleAdapterId;
  protected Optional<Boolean> mUseMaxSizedCerts;
  protected Optional<Boolean> mOnlyAllowTrustedCdKeys;

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
    addArgument("ble-adapter", (short) 0, Short.MAX_VALUE, mBleAdapterId, null);
  }

  /////////// Command Interface /////////
  @Override
  public void run() throws Exception {
    maybeSetUpStack();
    runCommand();
    shutdown();
    maybeTearDownStack();
  }

  protected abstract void runCommand();

  // Shut down the command.  After a Shutdown call the command object is ready
  // to be used for another command invocation.
  protected void shutdown() {
    resetArguments();
  }

  private void maybeSetUpStack() throws Exception {
    mDefaultStorage.init();
    mOperationalKeystore.init(mDefaultStorage);
    mOpCertStore.init(mDefaultStorage);
  }

  private void maybeTearDownStack() {}
}
