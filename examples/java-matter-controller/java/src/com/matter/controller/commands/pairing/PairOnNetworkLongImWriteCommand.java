package com.matter.controller.commands.pairing;

import chip.devicecontroller.ChipDeviceController;
import chip.devicecontroller.GetConnectedDeviceCallbackJni.GetConnectedDeviceCallback;
import chip.devicecontroller.WriteAttributesCallback;
import chip.devicecontroller.model.AttributeWriteRequest;
import chip.devicecontroller.model.ChipAttributePath;
import com.matter.controller.commands.common.CredentialsIssuer;
import java.util.ArrayList;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.annotation.Nullable;

public final class PairOnNetworkLongImWriteCommand extends PairingCommand {
  private static final int MATTER_PORT = 5540;
  private long devicePointer;
  private static final int CLUSTER_ID_BASIC = 0x0028;
  private static final int ATTR_ID_LOCAL_CONFIG_DISABLED = 16;
  private static Logger logger = Logger.getLogger(PairOnNetworkLongImWriteCommand.class.getName());

  private void setDevicePointer(long devicePointer) {
    this.devicePointer = devicePointer;
  }

  private class InternalWriteAttributesCallback implements WriteAttributesCallback {
    @Override
    public void onError(@Nullable ChipAttributePath attributePath, Exception e) {
      logger.log(Level.INFO, "Write receive onError on ");
      if (attributePath != null) {
        logger.log(Level.INFO, attributePath.toString());
      }

      setFailure("write failure");
    }

    @Override
    public void onResponse(ChipAttributePath attributePath) {
      logger.log(Level.INFO, "Write receve OnResponse on ");
      if (attributePath != null) {
        logger.log(Level.INFO, attributePath.toString());
      }
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
    AttributeWriteRequest attribute =
        AttributeWriteRequest.newInstance(
            /* endpointId= */ 0, CLUSTER_ID_BASIC, ATTR_ID_LOCAL_CONFIG_DISABLED, booleanTLV);
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
    currentCommissioner()
        .getConnectedDevicePointer(getNodeId(), new InternalGetConnectedDeviceCallback());
    clear();

    currentCommissioner()
        .write(new InternalWriteAttributesCallback(), devicePointer, attributeList, 0, 0);

    waitCompleteMs(getTimeoutMillis());
  }
}
