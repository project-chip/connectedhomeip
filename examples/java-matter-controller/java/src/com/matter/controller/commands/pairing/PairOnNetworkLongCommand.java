package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkLongCommand extends PairingCommand {
  public PairOnNetworkLongCommand(CredentialsIssuer credsIssue) {
    super(
        "onnetwork-long",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.LONG_DISCRIMINATOR);
  }

  @Override
  protected void runCommand() {}
}
