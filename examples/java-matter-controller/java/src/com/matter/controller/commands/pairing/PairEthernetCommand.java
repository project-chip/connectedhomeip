package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairEthernetCommand extends PairingCommand {
  public PairEthernetCommand(ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(
        controller, "ethernet", PairingModeType.ETHERNET, PairingNetworkType.ETHERNET, credsIssue);
  }

  @Override
  protected void runCommand() {
    currentCommissioner()
        .pairDeviceWithAddress(
            getNodeId(),
            getRemoteAddr().getHostAddress(),
            getRemotePort(),
            getDiscriminator(),
            getSetupPINCode(),
            null);
    currentCommissioner().setCompletionListener(this);
    waitCompleteMs(getTimeoutMillis());
  }
}
