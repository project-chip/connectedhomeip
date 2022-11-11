package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkVendorCommand extends PairingCommand {
  public PairOnNetworkVendorCommand(CredentialsIssuer credsIssue) {
    super(
        "onnetwork-vendor",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.VENDOR_ID);
  }

  @Override
  protected void runCommand() {}
}
