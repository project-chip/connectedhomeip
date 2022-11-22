package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;
import java.net.UnknownHostException;

public final class PairOnNetworkFabricCommand extends PairingCommand {
  public PairOnNetworkFabricCommand(ChipDeviceController controller, CredentialsIssuer credsIssue)
      throws UnknownHostException {
    super(
        controller,
        "onnetwork-fabric",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.COMPRESSED_FABRIC_ID);
  }

  @Override
  protected void runCommand() {}
}
