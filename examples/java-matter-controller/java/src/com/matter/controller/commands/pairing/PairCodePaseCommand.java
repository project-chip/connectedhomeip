package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairCodePaseCommand extends PairingCommand {
  public PairCodePaseCommand(ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(
        controller,
        "code-paseonly",
        PairingModeType.CODE_PASE_ONLY,
        PairingNetworkType.NONE,
        credsIssue);
  }

  @Override
  protected void runCommand() {}
}
