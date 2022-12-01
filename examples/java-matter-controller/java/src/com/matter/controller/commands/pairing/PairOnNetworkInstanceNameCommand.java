package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkInstanceNameCommand extends PairingCommand {
  public PairOnNetworkInstanceNameCommand(
      ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(
        controller,
        "onnetwork-instance-name",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.INSTANCE_NAME);
  }

  @Override
  protected void runCommand() {}
}
