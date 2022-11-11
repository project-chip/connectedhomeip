package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairCodeThreadCommand extends PairingCommand {
  public PairCodeThreadCommand(CredentialsIssuer credsIssue) {
    super("code-thread", PairingModeType.CODE, PairingNetworkType.THREAD, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
