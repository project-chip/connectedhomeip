package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairEthernetCommand extends PairingCommand {
  public PairEthernetCommand(CredentialsIssuer credsIssue) {
    super("ethernet", PairingModeType.ETHERNET, PairingNetworkType.ETHERNET, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
