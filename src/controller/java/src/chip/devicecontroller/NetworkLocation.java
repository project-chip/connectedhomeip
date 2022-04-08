package chip.devicecontroller;

import java.util.Locale;

/** Represents a location on an operational network. */
public final class NetworkLocation {
  private final String ipAddress;
  private final int port;

  public NetworkLocation(String ipAddress, int port) {
    this.ipAddress = ipAddress;
    this.port = port;
  }

  /** Returns the IP address (e.g. fe80::3e61:5ff:fe0c:89f8). */
  public String getIpAddress() {
    return ipAddress;
  }

  public int getPort() {
    return port;
  }

  @Override
  public String toString() {
    return String.format(Locale.ROOT, "%s[%d]", ipAddress, port);
  }
}
