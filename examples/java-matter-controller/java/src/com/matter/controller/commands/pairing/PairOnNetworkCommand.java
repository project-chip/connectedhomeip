package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkCommand extends PairingCommand {
  public PairOnNetworkCommand(ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(controller, "onnetwork", PairingModeType.ON_NETWORK, PairingNetworkType.NONE, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
