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

public abstract class CHIPCommand extends Command {

  public static final short kMaxGroupsPerFabric = 5;
  public static final short kMaxGroupKeysPerFabric = 8;

  protected PersistentStorage mDefaultStorage = new PersistentStorage();
  protected PersistentStorage mCommissionerStorage = new PersistentStorage();
  protected PersistentStorageOperationalKeystore mOperationalKeystore =
      new PersistentStorageOperationalKeystore();
  protected PersistentStorageOpCertStore mOpCertStore = new PersistentStorageOpCertStore();

  protected CredentialIssuerCommands mCredIssuerCmds = null;
  protected String mCommissionerName = null;
  protected String mPaaTrustStorePath = null;
  protected String mCDTrustStorePath = null;
  protected Optional<Long> mCommissionerNodeId;
  protected Optional<Short> mBleAdapterId;
  protected Optional<Boolean> mUseMaxSizedCerts;
  protected Optional<Boolean> mOnlyAllowTrustedCdKeys;

  public CHIPCommand(String commandName, CredentialIssuerCommands credIssuerCmds) {
    this(commandName, credIssuerCmds, null);
  }

  public CHIPCommand(String commandName, CredentialIssuerCommands credIssuerCmds, String helpText) {
    super(commandName, helpText);
    this.mCredIssuerCmds = credIssuerCmds;

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
  public int run() {
    if (maybeSetUpStack() != 0) {
      System.out.println("Failed to setup stack");
      return -1;
    }

    int err = runCommand();

    shutdown();

    maybeTearDownStack();

    return err;
  }

  // Will be called in a setting in which it's safe to touch the CHIP
  // stack. The rules for Run() are as follows:
  //
  // 1) If error is returned, Run() must not call SetCommandExitStatus.
  // 2) If success is returned Run() must either have called
  //    SetCommandExitStatus() or scheduled async work that will do that.
  protected abstract int runCommand();

  // Shut down the command.  After a Shutdown call the command object is ready
  // to be used for another command invocation.
  protected void shutdown() {
    resetArguments();
  }

  private int maybeSetUpStack() {
    int err;

    err = mDefaultStorage.init();
    if (err != 0) {
      System.out.println("Failed to init mDefaultStorage");
      return -1;
    }

    err = mOperationalKeystore.init(mDefaultStorage);
    if (err != 0) {
      System.out.println("Failed to init mOperationalKeystore");
      return -1;
    }

    err = mOpCertStore.init(mDefaultStorage);
    if (err != 0) {
      System.out.println("Failed to init mOpCertStore");
      return -1;
    }

    return 0;
  }

  private void maybeTearDownStack() {}
}
