package chip.devicecontroller;

import androidx.annotation.Nullable;

/** Represents a set of operating credentials for a ChipDeviceController. */
public final class OperationalKeyConfig {
  private final byte[] trustedRootCertificate;
  @Nullable private final byte[] intermediateCertificate;
  private final byte[] nodeOperationalCertificate;
  private final byte[] ipkEpochKey;
  private final KeypairDelegate keypairDelegate;

  /**
   * @param keypairDelegate a delegate for signing operations
   * @param trustedRootCertificate the trusted root X.509 certificate in DER-encoded form
   * @param intermediateCertificate the intermediate X.509 certificate in DER-encoded form
   * @param nodeOperationalCertificate the node operational X.509 certificate in DER-encoded form
   * @param ipkEpochKey the IPK epoch key to use (assuming a single one)
   */
  public OperationalKeyConfig(
      KeypairDelegate keypairDelegate,
      byte[] trustedRootCertificate,
      @Nullable byte[] intermediateCertificate,
      byte[] nodeOperationalCertificate,
      byte[] ipkEpochKey) {
    this.keypairDelegate = keypairDelegate;
    this.trustedRootCertificate = trustedRootCertificate.clone();
    this.intermediateCertificate =
        intermediateCertificate == null ? null : intermediateCertificate.clone();
    this.nodeOperationalCertificate = nodeOperationalCertificate.clone();
    this.ipkEpochKey = ipkEpochKey.clone();
  }

  /** Returns the trusted root X.509 certificate in DER-encoded form. */
  public byte[] getTrustedRootCertificate() {
    return trustedRootCertificate.clone();
  }

  /** Returns the intermediate X.509 certificate in DER-encoded form, if provided. */
  @Nullable
  public byte[] getIntermediateCertificate() {
    return intermediateCertificate == null ? null : intermediateCertificate.clone();
  }

  /** Returns the node operational X.509 certificate in DER-encoded form. */
  public byte[] getNodeOperationalCertificate() {
    return nodeOperationalCertificate.clone();
  }

  /** Returns the IPK epoch key. */
  public byte[] getIpkEpochKey() {
    return ipkEpochKey.clone();
  }

  /** Returns the delegate for signing operations. */
  public KeypairDelegate getKeypairDelegate() {
    return keypairDelegate;
  }
}
