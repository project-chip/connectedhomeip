package chip.devicecontroller;

import java.util.Locale;

/** Represents a location on an operational network. */
public final class NetworkLocation {
  private final String ipAddress;
  private final int port;
  private final int interfaceIndex;

  public NetworkLocation(String ipAddress, int port, int interfaceIndex) {
    this.ipAddress = ipAddress;
    this.port = port;
    this.interfaceIndex = interfaceIndex;
  }

  /** Returns the IP address (e.g. fe80::3e61:5ff:fe0c:89f8). */
  public String getIpAddress() {
    return ipAddress;
  }

  public int getPort() {
    return port;
  }

  /** Returns the index of the network interface to which this address belongs, or zero. */
  public int getInterfaceIndex() {
    return interfaceIndex;
  }

  @Override
  public String toString() {
    return String.format(
        Locale.ROOT,
        "%s%s[%d]",
        ipAddress,
        (interfaceIndex == 0 ? "" : "%" + interfaceIndex),
        port);
  }
}
