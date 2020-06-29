package chip.setuppayload;

/** Parser for scanned QR code or manual entry code. */
public class SetupPayloadParser {

    /** Returns {@link SetupPayload} parsed from the QR code string. */
    public SetupPayload parseQrCode(String qrCodeString) {
        return fetchPayloadFromQrCode(qrCodeString);
    }

    private native SetupPayload fetchPayloadFromQrCode(String qrCodeString);

    static {
        System.loadLibrary("SetupPayloadParser");
    }
}
