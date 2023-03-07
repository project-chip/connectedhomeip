package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback;
import chip.devicecontroller.InvokeCallback;
import chip.devicecontroller.model.InvokeElement;
import com.matter.controller.commands.common.CredentialsIssuer;
import java.util.logging.Level;
import java.util.logging.Logger;

public final class PairOnNetworkLongImInvokeCommand extends PairingCommand {
  private static final Logger logger =
      Logger.getLogger(PairOnNetworkLongImInvokeCommand.class.getName());
  private static final int MATTER_PORT = 5540;
  private static final int CLUSTER_ID_IDENTIFY = 0x0003;
  private static final int IDENTIFY_COMMAND = 0;
  private long devicePointer;

  private void setDevicePointer(long devicePointer) {
    this.devicePointer = devicePointer;
  }

  private class InternalInvokeCallback implements InvokeCallback {
    @Override
    public void onError(Exception e) {
      logger.log(Level.INFO, "Invoke receive onError" + e.getMessage());
      setFailure("write failure");
    }

    @Override
    public void onResponse(InvokeElement element, long successCode) {
      logger.log(Level.INFO, "Invoke receive OnResponse on ");
      if (element != null) {
        logger.log(Level.INFO, element.toString());
      }
      logger.log(Level.INFO, "success code is" + String.valueOf(successCode));
      setSuccess();
    }
  }

  private class InternalGetConnectedDeviceCallback implements GetConnectedDeviceCallback {
    @Override
    public void onDeviceConnected(long devicePointer) {
      setDevicePointer(devicePointer);
      logger.log(Level.INFO, "onDeviceConnected");
    }

    @Override
    public void onConnectionFailure(long nodeId, Exception error) {
      logger.log(Level.INFO, "onConnectionFailure");
    }
  }

  public PairOnNetworkLongImInvokeCommand(
      ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(
        controller,
        "onnetwork-long-im-invoke",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.LONG_DISCRIMINATOR);
  }

  @Override
  protected void runCommand() {
    // tlv structure with tag 0, unsigned integer 1 inside, {0: 1}
    byte[] intTLV = {0x15, 0x24, 0x00, 0x01, 0x18};
    InvokeElement element =
        InvokeElement.newInstance(
            /* endpointId= */ 0, CLUSTER_ID_IDENTIFY, IDENTIFY_COMMAND, intTLV, null);

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
    currentCommissioner()
        .getConnectedDevicePointer(getNodeId(), new InternalGetConnectedDeviceCallback());
    clear();

    currentCommissioner().invoke(new InternalInvokeCallback(), devicePointer, element, 0, 0);

    waitCompleteMs(getTimeoutMillis());
  }
}
