package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairCodeCommand extends PairingCommand {
  public PairCodeCommand(ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(controller, "code", PairingModeType.CODE, PairingNetworkType.NONE, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
