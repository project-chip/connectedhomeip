package chip.setuppayload;

import java.util.HashMap;
import java.util.Map;

/** Class to hold the data from the scanned QR code or manual entry code. */
public class SetupPayload {
  /** Version info of the SetupPayload */
  public int version;
  /** The CHIP device vendor ID */
  public int vendorId;
  /** The CHIP device product ID */
  public int productId;
  /** Boolean indicating if the CHIP device needs custom flow */
  public boolean requiresCustomFlow;
  /** The CHIP device supported rendezvous flags */
  public int rendezvousInformation;
  /** The CHIP device discriminator */
  public int discriminator;
  /** The CHIP device manual setup code */
  public long setupPinCode;
  /** The CHIP device optional QR code info map */
  public Map<Integer, OptionalQRCodeInfo> optionalQRCodeInfo;

  public SetupPayload() {
    this.optionalQRCodeInfo = new HashMap<Integer, OptionalQRCodeInfo>();
  }

  public SetupPayload(
      int version,
      int vendorId,
      int productId,
      boolean requiresCustomFlow,
      int rendezvousInfo,
      int discriminator,
      long setupPinCode) {
    this.version = version;
    this.vendorId = vendorId;
    this.productId = productId;
    this.requiresCustomFlow = requiresCustomFlow;
    this.rendezvousInformation = rendezvousInfo;
    this.discriminator = discriminator;
    this.setupPinCode = setupPinCode;
    this.optionalQRCodeInfo = new HashMap<Integer, OptionalQRCodeInfo>();
  }

  public void addOptionalQRCodeInfo(OptionalQRCodeInfo info) {
    this.optionalQRCodeInfo.put(info.tag, info);
  }
}
