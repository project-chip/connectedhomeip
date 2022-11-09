package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkCommand extends PairingCommand {
  public PairOnNetworkCommand(CredentialsIssuer credsIssue) {
    super("onnetwork", PairingModeType.ON_NETWORK, PairingNetworkType.NONE, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
