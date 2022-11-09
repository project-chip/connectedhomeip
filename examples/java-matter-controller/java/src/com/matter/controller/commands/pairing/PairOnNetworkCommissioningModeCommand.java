package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkCommissioningModeCommand extends PairingCommand {
  public PairOnNetworkCommissioningModeCommand(CredentialsIssuer credsIssue) {
    super(
        "onnetwork-commissioning-mode",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.COMMISSIONING_MODE);
  }

  @Override
  protected void runCommand() {}
}
