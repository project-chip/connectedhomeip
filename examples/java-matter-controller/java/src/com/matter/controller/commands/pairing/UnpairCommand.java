package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class UnpairCommand extends PairingCommand {
  public UnpairCommand(CredentialsIssuer credsIssue) {
    super("unpair", PairingModeType.NONE, PairingNetworkType.NONE, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
