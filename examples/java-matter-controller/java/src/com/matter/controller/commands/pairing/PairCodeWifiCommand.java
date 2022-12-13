package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairCodeWifiCommand extends PairingCommand {
  public PairCodeWifiCommand(ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(controller, "code-wifi", PairingModeType.CODE, PairingNetworkType.WIFI, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
