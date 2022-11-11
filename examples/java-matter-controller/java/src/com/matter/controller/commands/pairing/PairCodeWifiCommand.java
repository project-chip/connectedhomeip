package com.matter.controller.commands.pairing;

import com.matter.controller.commands.common.CredentialsIssuer;

public final class PairCodeWifiCommand extends PairingCommand {
  public PairCodeWifiCommand(CredentialsIssuer credsIssue) {
    super("code-wifi", PairingModeType.CODE, PairingNetworkType.WIFI, credsIssue);
  }

  @Override
  protected void runCommand() {}
}
