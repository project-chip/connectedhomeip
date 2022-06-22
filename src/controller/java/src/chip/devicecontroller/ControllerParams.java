package chip.devicecontroller;

import androidx.annotation.Nullable;

/** Parameters representing initialization arguments for {@link ChipDeviceController}. */
public final class ControllerParams {

  private final int udpListenPort;
  @Nullable private final KeypairDelegate keypairDelegate;
  @Nullable private final byte[] rootCertificate;
  @Nullable private final byte[] intermediateCertificate;
  @Nullable private final byte[] operationalCertificate;
  @Nullable private final byte[] ipk;

  private static final int LEGACY_GLOBAL_CHIP_PORT = 5540;

  /** @param udpListenPort the UDP listening port, or 0 to pick any available port. */
  private ControllerParams(Builder builder) {
    this.udpListenPort = builder.udpListenPort;
    this.keypairDelegate = builder.keypairDelegate;
    this.rootCertificate = builder.rootCertificate;
    this.intermediateCertificate = builder.intermediateCertificate;
    this.operationalCertificate = builder.operationalCertificate;
    this.ipk = builder.ipk;
  }

  /** Gets the UDP listening port; 0 indicates "any available port" */
  public int getUdpListenPort() {
    return udpListenPort;
  }

  public KeypairDelegate getKeypairDelegate() {
    return keypairDelegate;
  }

  public byte[] getRootCertificate() {
    return rootCertificate;
  }

  public byte[] getIntermediateCertificate() {
    return intermediateCertificate;
  }

  public byte[] getOperationalCertificate() {
    return operationalCertificate;
  }

  public byte[] getIpk() {
    return ipk;
  }

  /** Returns parameters with ephemerally generated operational credentials */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Returns parameters which uses the provided {@code operationalKeyConfig} as its operating
   * credentials.
   */
  public static Builder newBuilder(OperationalKeyConfig operationalKeyConfig) {
    return newBuilder()
        .setKeypairDelegate(operationalKeyConfig.getKeypairDelegate())
        .setRootCertificate(operationalKeyConfig.getTrustedRootCertificate())
        .setIntermediateCertificate(operationalKeyConfig.getIntermediateCertificate())
        .setOperationalCertificate(operationalKeyConfig.getNodeOperationalCertificate())
        .setIpk(operationalKeyConfig.getIpkEpochKey());
  }

  /** Builder for {@link ControllerParams}. */
  public static class Builder {
    private int udpListenPort = LEGACY_GLOBAL_CHIP_PORT + 1;
    @Nullable private KeypairDelegate keypairDelegate = null;
    @Nullable private byte[] rootCertificate = null;
    @Nullable private byte[] intermediateCertificate = null;
    @Nullable private byte[] operationalCertificate = null;
    @Nullable private byte[] ipk = null;

    private Builder() {}

    public Builder setUdpListenPort(int udpListenPort) {
      if (udpListenPort < 0) {
        throw new IllegalArgumentException("udpListenPort must be >= 0");
      }
      this.udpListenPort = udpListenPort;
      return this;
    }

    public Builder setKeypairDelegate(KeypairDelegate keypairDelegate) {
      this.keypairDelegate = keypairDelegate;
      return this;
    }

    public Builder setRootCertificate(byte[] rootCertificate) {
      this.rootCertificate = rootCertificate;
      return this;
    }

    public Builder setIntermediateCertificate(byte[] intermediateCertificate) {
      this.intermediateCertificate = intermediateCertificate;
      return this;
    }

    public Builder setOperationalCertificate(byte[] operationalCertificate) {
      this.operationalCertificate = operationalCertificate;
      return this;
    }

    public Builder setIpk(byte[] ipk) {
      this.ipk = ipk;
      return this;
    }

    public ControllerParams build() {
      return new ControllerParams(this);
    }
  }
}
