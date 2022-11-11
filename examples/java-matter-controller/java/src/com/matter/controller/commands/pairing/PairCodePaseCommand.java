package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairCodePaseCommand extends PairingCommand {
  public PairCodePaseCommand(CredentialsIssuer credsIssue) {
    super("code-paseonly", PairingModeType.CODE_PASE_ONLY, PairingNetworkType.NONE, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
