package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairCodeThreadCommand extends PairingCommand {
  public PairCodeThreadCommand(ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(controller, "code-thread", PairingModeType.CODE, PairingNetworkType.THREAD, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
