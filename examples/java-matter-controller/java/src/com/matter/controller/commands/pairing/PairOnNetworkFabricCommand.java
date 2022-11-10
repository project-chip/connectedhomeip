package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;
import java.net.UnknownHostException;

public final class PairOnNetworkFabricCommand extends PairingCommand {
  public PairOnNetworkFabricCommand(CredentialsIssuer credsIssue) throws UnknownHostException {
    super(
        "onnetwork-fabric",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.COMPRESSED_FABRIC_ID);
  }

  @Override
  protected void runCommand() {}
}
