package chip.setuppayload;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/** Class to hold the data from the scanned QR code or manual entry code. */
public class SetupPayload {
  /** Version info of the SetupPayload */
  public int version;
  /** The CHIP device vendor ID */
  public int vendorId;
  /** The CHIP device product ID */
  public int productId;
  /** Commissioning flow: 0 = standard, 1 = requires user action, 2 = custom */
  public int commissioningFlow;
  /** The CHIP device supported rendezvous flags */
  public Set<DiscoveryCapability> discoveryCapabilities;
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
      int commissioningFlow,
      Set<DiscoveryCapability> discoveryCapabilities,
      int discriminator,
      long setupPinCode) {
    this.version = version;
    this.vendorId = vendorId;
    this.productId = productId;
    this.commissioningFlow = commissioningFlow;
    this.discoveryCapabilities = discoveryCapabilities;
    this.discriminator = discriminator;
    this.setupPinCode = setupPinCode;
    this.optionalQRCodeInfo = new HashMap<Integer, OptionalQRCodeInfo>();
  }

  public void addOptionalQRCodeInfo(OptionalQRCodeInfo info) {
    this.optionalQRCodeInfo.put(info.tag, info);
  }
}
