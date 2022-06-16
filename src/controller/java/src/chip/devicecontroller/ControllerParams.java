package chip.devicecontroller;

/** Parameters representing initialization arguments for {@link ChipDeviceController}. */
public final class ControllerParams {
  private final int udpListenPort;

  private static final int LEGACY_GLOBAL_CHIP_PORT = 5540;

  /** Creates a {@link ControllerParams} with default values. */
  public ControllerParams() {
    this(LEGACY_GLOBAL_CHIP_PORT + 1 /* To match old behavior of CHIP_PORT + 1 */);
  }

  /** @param udpListenPort the UDP listening port, or 0 to pick any available port. */
  public ControllerParams(int udpListenPort) {
    if (udpListenPort < 0) {
      throw new IllegalArgumentException("udpListenPort must be >= 0");
    }
    this.udpListenPort = udpListenPort;
  }

  /** Gets the UDP listening port; 0 indicates "any available port" */
  public int getUdpListenPort() {
    return udpListenPort;
  }
}
