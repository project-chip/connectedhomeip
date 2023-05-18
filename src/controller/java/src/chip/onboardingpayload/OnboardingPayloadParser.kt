package chip.onboardingpayload

import java.util.logging.Level
import java.util.logging.Logger

/** Parser for scanned QR code or manual entry code.  */
class OnboardingPayloadParser {
  /**
   * Returns [OnboardingPayload] parsed from the QR code string. If an invalid element is included
   * in the QRCode Parse result, OnboardingPayloadException occurs. Refer to [OnboardingPayload] for the
   * description of the invalid element.
   */
  @Throws(UnrecognizedQrCodeException::class, OnboardingPayloadException::class)
  fun parseQrCode(qrCodeString: String): OnboardingPayload {
    return fetchPayloadFromQrCode(qrCodeString, false)
  }

  /**
   * Returns [OnboardingPayload] parsed from the QR code string.
   *
   * @param qrCodeString the QRCode for commissioning device.
   * @param allowInvalidPayload Allow invalid payload components. If this value is true, payload
   * element validation is not checked. Consider saying that the payload must still parse
   * correctly, but this skips validation of the content past parsing (i.e. it does not validate
   * ranges for individual elements). Refer to [OnboardingPayload] for the description of the
   * invalid element.
   */
  @Throws(UnrecognizedQrCodeException::class, OnboardingPayloadException::class)
  fun parseQrCode(qrCodeString: String, allowInvalidPayload: Boolean): OnboardingPayload {
    return fetchPayloadFromQrCode(qrCodeString, allowInvalidPayload)
  }

  /**
   * Returns [OnboardingPayload] parsed from the manual entry code string. If an SetupPINCode has
   * invalid value, OnboardingPayloadException occurs. Refer to [OnboardingPayload] for the description
   * of the invalid element.
   */
  @Throws(InvalidEntryCodeFormatException::class, OnboardingPayloadException::class)
  fun parseManualEntryCode(entryCodeString: String): OnboardingPayload {
    return fetchPayloadFromManualEntryCode(entryCodeString, false)
  }

  /**
   * Returns [OnboardingPayload] parsed from the manual entry code string.
   *
   * @param entryCodeString the manual Pairing Code for commissioning device.
   * @param allowInvalidPayload Allow invalid payload components. If this value is true, payload
   * element validation is not checked. Consider saying that the payload must still parse
   * correctly, but this skips validation of the content past parsing (i.e. it does not validate
   * ranges for individual elements). Refer to [OnboardingPayload] for the description of the
   * invalid element.
   */
  @Throws(InvalidEntryCodeFormatException::class, OnboardingPayloadException::class)
  fun parseManualEntryCode(entryCodeString: String, allowInvalidPayload: Boolean): OnboardingPayload {
    return fetchPayloadFromManualEntryCode(entryCodeString, allowInvalidPayload)
  }

  /** Get QR code string from [OnboardingPayload].  */
  @Throws(OnboardingPayloadException::class)
  external fun getQrCodeFromPayload(payload: OnboardingPayload): String?

  /** Get manual entry code string from [OnboardingPayload].  */
  @Throws(OnboardingPayloadException::class)
  external fun getManualEntryCodeFromPayload(payload: OnboardingPayload): String?

  @Throws(UnrecognizedQrCodeException::class, OnboardingPayloadException::class)
  private external fun fetchPayloadFromQrCode(
    qrCodeString: String, isAllowInvalidPayload: Boolean
  ): OnboardingPayload

  @Throws(InvalidEntryCodeFormatException::class, OnboardingPayloadException::class)
  private external fun fetchPayloadFromManualEntryCode(
    entryCodeString: String, isAllowInvalidPayload: Boolean
  ): OnboardingPayload

  class UnrecognizedQrCodeException(qrCode: String) :
    Exception(String.format("Invalid QR code string: %s", qrCode), null) {
    companion object {
      private const val serialVersionUID = 1L
    }
  }

  class InvalidEntryCodeFormatException(entryCode: String) :
    Exception(String.format("Invalid format for entry code string: %s", entryCode), null) {
    companion object {
      private const val serialVersionUID = 1L
    }
  }

  class OnboardingPayloadException(var errorCode: Int, message: String?) :
    Exception(message ?: String.format("Error Code %d", errorCode)) {
    companion object {
      private const val serialVersionUID = 1L
    }
  }

  companion object {
    private val LOGGER: Logger = Logger.getLogger(OnboardingPayloadParser::class.java.getSimpleName())

    init {
      try {
        System.loadLibrary("OnboardingPayload")
      } catch (e: UnsatisfiedLinkError) {
        LOGGER.log(Level.SEVERE, "Cannot load library.", e)
      }
    }
  }
}
