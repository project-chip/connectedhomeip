package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkDeviceTypeCommand extends PairingCommand {
  public PairOnNetworkDeviceTypeCommand(CredentialsIssuer credsIssue) {
    super(
        "onnetwork-device-type",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.DEVICE_TYPE);
  }

  @Override
  protected void runCommand() {}
}
