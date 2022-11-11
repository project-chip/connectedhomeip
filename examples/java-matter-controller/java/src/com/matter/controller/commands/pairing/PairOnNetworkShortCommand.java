package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkShortCommand extends PairingCommand {
  public PairOnNetworkShortCommand(CredentialsIssuer credsIssue) {
    super(
        "onnetwork-short",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.SHORT_DISCRIMINATOR);
  }

  @Override
  protected void runCommand() {}
}
