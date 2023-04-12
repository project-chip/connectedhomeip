package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkCommissionerCommand extends PairingCommand {
  public PairOnNetworkCommissionerCommand(
      ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(
        controller,
        "onnetwork-commissioner",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.COMMISSIONER);
  }

  @Override
  protected void runCommand() {}
}
