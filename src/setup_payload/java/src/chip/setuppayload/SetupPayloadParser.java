package chip.setuppayload;

/** Parser for scanned QR code or manual entry code. */
public class SetupPayloadParser {

  /** Returns {@link SetupPayload} parsed from the QR code string. */
  public SetupPayload parseQrCode(String qrCodeString)
      throws UnrecognizedQrCodeException, SetupPayloadException {
    return fetchPayloadFromQrCode(qrCodeString, false);
  }

  public SetupPayload parseQrCode(String qrCodeString, boolean isAllowInvalidPayload)
      throws UnrecognizedQrCodeException, SetupPayloadException {
    return fetchPayloadFromQrCode(qrCodeString, isAllowInvalidPayload);
  }

  /** Returns {@link SetupPayload} parsed from the manual entry code string. */
  public SetupPayload parseManualEntryCode(String entryCodeString)
      throws InvalidEntryCodeFormatException, SetupPayloadException {
    return fetchPayloadFromManualEntryCode(entryCodeString, false);
  }

  public SetupPayload parseManualEntryCode(String entryCodeString, boolean isAllowInvalidPayload)
      throws InvalidEntryCodeFormatException, SetupPayloadException {
    return fetchPayloadFromManualEntryCode(entryCodeString, isAllowInvalidPayload);
  }

  /** Get QR code string from {@link SetupPayload}. */
  public native String getQrCodeFromPayload(SetupPayload payload) throws SetupPayloadException;

  /** Get manual entry code string from {@link SetupPayload}. */
  public native String getManualEntryCodeFromPayload(SetupPayload payload)
      throws SetupPayloadException;

  private native SetupPayload fetchPayloadFromQrCode(
      String qrCodeString, boolean isAllowInvalidPayload)
      throws UnrecognizedQrCodeException, SetupPayloadException;

  private native SetupPayload fetchPayloadFromManualEntryCode(
      String entryCodeString, boolean isAllowInvalidPayload)
      throws InvalidEntryCodeFormatException, SetupPayloadException;

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

  public static class SetupPayloadException extends Exception {
    private static final long serialVersionUID = 1L;
    public int errorCode;

    public SetupPayloadException(int errorCode, String message) {
      super(message != null ? message : String.format("Error Code %d", errorCode));
      this.errorCode = errorCode;
    }
  }
}
