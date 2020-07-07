package chip.setuppayload;

/** Parser for scanned QR code or manual entry code. */
public class SetupPayloadParser {

  /** Returns {@link SetupPayload} parsed from the QR code string. */
  public SetupPayload parseQrCode(String qrCodeString) {
    return fetchPayloadFromQrCode(qrCodeString);
  }

  /** Returns {@link SetupPayload} parsed from the manual entry code string. */
  public SetupPayload parseManualEntryCode(String entryCodeString) {
    return fetchPayloadFromManualEntryCode(entryCodeString);
  }

  private native SetupPayload fetchPayloadFromQrCode(String qrCodeString);

  private native SetupPayload fetchPayloadFromManualEntryCode(String entryCodeString);

  static {
    System.loadLibrary("SetupPayloadParser");
  }
}
