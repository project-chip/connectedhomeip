package chip.setuppayload;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/** Class to hold the data from the scanned QR code or manual entry code. */
public class SetupPayload {
  /** Version info of the SetupPayload: version SHALL be 0 */
  public int version;
  /** The CHIP device vendor ID: Vendor ID SHALL be between 1 and 0xFFF4. */
  public int vendorId;
  /** The CHIP device product ID: Product ID SHALL BE greater than 0. */
  public int productId;
  /** Commissioning flow: 0 = standard, 1 = requires user action, 2 = custom */
  public int commissioningFlow;
  /**
   * The CHIP device supported rendezvous flags: At least one DiscoveryCapability must be included.
   */
  public Set<DiscoveryCapability> discoveryCapabilities;
  /** The CHIP device discriminator: */
  public int discriminator;
  /**
   * The CHIP device setup PIN code: setupPINCode SHALL be greater than 0. Also invalid setupPINCode
   * is {000000000, 11111111, 22222222, 33333333, 44444444, 55555555, 66666666, 77777777, 88888888,
   * 99999999, 12345678, 87654321}.
   */
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
