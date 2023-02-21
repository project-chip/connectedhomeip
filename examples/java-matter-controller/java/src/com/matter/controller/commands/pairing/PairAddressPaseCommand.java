package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import com.matter.controller.commands.common.CredentialsIssuer;
import java.util.logging.Level;
import java.util.logging.Logger;

public final class PairAddressPaseCommand extends PairingCommand {
  private static Logger logger = Logger.getLogger(PairAddressPaseCommand.class.getName());

  public PairAddressPaseCommand(ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(
        controller,
        "address-paseonly",
        PairingModeType.ADDRESS_PASE_ONLY,
        PairingNetworkType.NONE,
        credsIssue);
  }

  @Override
  public void onPairingComplete(int errorCode) {
    logger.log(Level.INFO, "onPairingComplete with error code: " + errorCode);
    if (errorCode == 0) {
      setSuccess();
    } else {
      setFailure("onPairingComplete failure");
    }
  }

  @Override
  protected void runCommand() {
    currentCommissioner()
        .establishPaseConnection(
            getNodeId(), getRemoteAddr().getHostAddress(), getRemotePort(), getSetupPINCode());

    currentCommissioner().setCompletionListener(this);
    waitCompleteMs(getTimeoutMillis());
  }
}
