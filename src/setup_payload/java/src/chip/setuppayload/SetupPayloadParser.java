package chip.setuppayload;

/** Parser for scanned QR code or manual entry code. */
public class SetupPayloadParser {

  /** Returns {@link SetupPayload} parsed from the QR code string. */
  public SetupPayload parseQrCode(String qrCodeString) throws UnrecognizedQrCodeException {
    return fetchPayloadFromQrCode(qrCodeString);
  }

  /** Returns {@link SetupPayload} parsed from the manual entry code string. */
  public SetupPayload parseManualEntryCode(String entryCodeString)
      throws InvalidEntryCodeFormatException {
    return fetchPayloadFromManualEntryCode(entryCodeString);
  }

  private native SetupPayload fetchPayloadFromQrCode(String qrCodeString)
      throws UnrecognizedQrCodeException;

  private native SetupPayload fetchPayloadFromManualEntryCode(String entryCodeString)
      throws InvalidEntryCodeFormatException;

  static {
    System.loadLibrary("SetupPayloadParser");
  }

  public static class UnrecognizedQrCodeException extends Exception {
    private static final long serialVersionUID = 1L;

    public UnrecognizedQrCodeException(String qrCode) {
      super(String.format("Invalid QR code string: %s", qrCode), null);
    }
  }

  public static class InvalidEntryCodeFormatException extends Exception {
    private static final long serialVersionUID = 1L;

    public InvalidEntryCodeFormatException(String entryCode) {
      super(String.format("Invalid format for entry code string: %s", entryCode), null);
    }
  }
}
