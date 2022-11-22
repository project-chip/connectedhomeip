package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class UnpairCommand extends PairingCommand {
  public UnpairCommand(ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(controller, "unpair", PairingModeType.NONE, PairingNetworkType.NONE, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
