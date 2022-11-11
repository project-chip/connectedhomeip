package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairCodeCommand extends PairingCommand {
  public PairCodeCommand(CredentialsIssuer credsIssue) {
    super("code", PairingModeType.CODE, PairingNetworkType.NONE, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
