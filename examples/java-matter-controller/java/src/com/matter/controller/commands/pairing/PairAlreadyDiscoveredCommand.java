package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairAlreadyDiscoveredCommand extends PairingCommand {
  public PairAlreadyDiscoveredCommand(
      ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(
        controller,
        "already-discovered",
        PairingModeType.ALREADY_DISCOVERED,
        PairingNetworkType.NONE,
        credsIssue);
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
