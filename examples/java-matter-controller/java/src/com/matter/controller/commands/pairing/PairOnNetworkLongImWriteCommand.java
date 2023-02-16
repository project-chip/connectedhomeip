package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback;
import chip.devicecontroller.WriteAttributesCallback;
import chip.devicecontroller.model.AttributeWriteRequest;
import chip.devicecontroller.model.ChipAttributePath;
import com.matter.controller.commands.common.CredentialsIssuer;
import java.util.ArrayList;
import java.util.logging.Level;

public final class PairOnNetworkLongImWriteCommand extends PairingCommand
    implements WriteAttributesCallback, GetConnectedDeviceCallback {
  private static final int MATTER_PORT = 5540;
  private long devicePointer;

  public PairOnNetworkLongImWriteCommand(
      ChipDeviceController controller, CredentialsIssuer credsIssue) {
    super(
        controller,
        "onnetwork-long-im-write",
        PairingModeType.ON_NETWORK,
        PairingNetworkType.NONE,
        credsIssue,
        DiscoveryFilterType.LONG_DISCRIMINATOR);
  }

  @Override
  protected void runCommand() {
    // boolean true for tlv
    byte[] booleanTLV = {0x09};
    AttributeWriteRequest attribute = AttributeWriteRequest.newInstance(0, 0x0028, 16, booleanTLV);
    ArrayList<AttributeWriteRequest> attributeList = new ArrayList<>();
    attributeList.add(attribute);

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
    currentCommissioner().getConnectedDevicePointer(getNodeId(), this);
    clear();

    currentCommissioner().write(this, devicePointer, attributeList, 0, 0);

    waitCompleteMs(getTimeoutMillis());
  }

  public void onError(ChipAttributePath attributePath, Exception e) {
    logger.log(Level.INFO, "Write receive onError on ");
    if (attributePath != null) {
      logger.log(Level.INFO, attributePath.toString());
    }

    setFailure("write failure");
  }

  public void OnResponse(ChipAttributePath attributePath) {
    logger.log(Level.INFO, "Write receve OnResponse on ");
    if (attributePath != null) {
      logger.log(Level.INFO, attributePath.toString());
    }
    setSuccess();
  }

  public void onDeviceConnected(long devicePointer) {
    this.devicePointer = devicePointer;
    logger.log(Level.INFO, "onDeviceConnected");
  }

  public void onConnectionFailure(long nodeId, Exception error) {
    logger.log(Level.INFO, "onConnectionFailure");
  }
}
