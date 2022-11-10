package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkCommissionerCommand extends PairingCommand {
  public PairOnNetworkCommissionerCommand(CredentialsIssuer credsIssue) {
    super(
        "onnetwork-commissioner",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.COMMISSIONER);
  }

  @Override
  protected void runCommand() {}
}
