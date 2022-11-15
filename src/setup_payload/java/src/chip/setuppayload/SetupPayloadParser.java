package chip.setuppayload;

import java.util.logging.Level;
import java.util.logging.Logger;

/** Parser for scanned QR code or manual entry code. */
public class SetupPayloadParser {

  private static final Logger LOGGER = Logger.getLogger(SetupPayloadParser.class.getSimpleName());

  /**
   * Returns {@link SetupPayload} parsed from the QR code string. If an invalid element is included
   * in the QRCode Parse result, SetupPayloadException occurs. Refer to {@link SetupPayload} for the
   * description of the invalid element.
   */
  public SetupPayload parseQrCode(String qrCodeString)
      throws UnrecognizedQrCodeException, SetupPayloadException {
    return fetchPayloadFromQrCode(qrCodeString, false);
  }

  /**
   * Returns {@link SetupPayload} parsed from the QR code string.
   *
   * @param qrCodeString the QRCode for commissioning device.
   * @param allowInvalidPayload Allow invalid payload components. If this value is true, payload
   *     element validation is not checked. Consider saying that the payload must still parse
   *     correctly, but this skips validation of the content past parsing (i.e. it does not validate
   *     ranges for individual elements). Refer to {@link SetupPayload} for the description of the
   *     invalid element.
   */
  public SetupPayload parseQrCode(String qrCodeString, boolean allowInvalidPayload)
      throws UnrecognizedQrCodeException, SetupPayloadException {
    return fetchPayloadFromQrCode(qrCodeString, allowInvalidPayload);
  }

  /**
   * Returns {@link SetupPayload} parsed from the manual entry code string. If an SetupPINCode has
   * invalid value, SetupPayloadException occurs. Refer to {@link SetupPayload} for the description
   * of the invalid element.
   */
  public SetupPayload parseManualEntryCode(String entryCodeString)
      throws InvalidEntryCodeFormatException, SetupPayloadException {
    return fetchPayloadFromManualEntryCode(entryCodeString, false);
  }

  /**
   * Returns {@link SetupPayload} parsed from the manual entry code string.
   *
   * @param entryCodeString the manual Pairing Code for commissioning device.
   * @param allowInvalidPayload Allow invalid payload components. If this value is true, payload
   *     element validation is not checked. Consider saying that the payload must still parse
   *     correctly, but this skips validation of the content past parsing (i.e. it does not validate
   *     ranges for individual elements). Refer to {@link SetupPayload} for the description of the
   *     invalid element.
   */
  public SetupPayload parseManualEntryCode(String entryCodeString, boolean allowInvalidPayload)
      throws InvalidEntryCodeFormatException, SetupPayloadException {
    return fetchPayloadFromManualEntryCode(entryCodeString, allowInvalidPayload);
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
    try {
      System.loadLibrary("SetupPayloadParser");
    } catch (UnsatisfiedLinkError e) {
      LOGGER.log(Level.SEVERE, "Cannot load library.", e);
    }
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
