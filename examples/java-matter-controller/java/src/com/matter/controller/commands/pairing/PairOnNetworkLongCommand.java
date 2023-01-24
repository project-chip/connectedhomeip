package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairOnNetworkLongCommand extends PairingCommand {
  private static final int MATTER_PORT = 5540;

  public PairOnNetworkLongCommand(ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(
        controller,
        "onnetwork-long",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.LONG_DISCRIMINATOR);
  }

  @Override
  protected void runCommand() {
    currentCommissioner()
        .pairDeviceWithAddress(
            getNodeId(),
            getRemoteAddr().getHostAddress(),
            MATTER_PORT,
            getDiscriminator(),
            getSetupPINCode(),
            null);
    currentCommissioner().setCompletionListener(this);
    waitCompleteMs(getTimeoutMillis());
  }
}
