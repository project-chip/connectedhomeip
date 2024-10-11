package chip.devicecontroller;

import java.util.Optional;
import javax.annotation.Nullable;

/** Parameters representing initialization arguments for {@link ChipDeviceController}. */
public final class ControllerParams {

  private final long fabricId;
  private final int udpListenPort;
  private final int controllerVendorId;
  private final int failsafeTimerSeconds;
  private final int caseFailsafeTimerSeconds;
  private final boolean attemptNetworkScanWiFi;
  private final boolean attemptNetworkScanThread;
  private final boolean skipCommissioningComplete;
  private final boolean skipAttestationCertificateValidation;
  private final Optional<String> countryCode;
  private final Optional<Integer> regulatoryLocationType;
  @Nullable private final KeypairDelegate keypairDelegate;
  @Nullable private final byte[] rootCertificate;
  @Nullable private final byte[] intermediateCertificate;
  @Nullable private final byte[] operationalCertificate;
  @Nullable private final byte[] ipk;
  private final long adminSubject;
  private final boolean enableServerInteractions;

  /** @param udpListenPort the UDP listening port, or 0 to pick any available port. */
  private ControllerParams(Builder builder) {
    this.fabricId = builder.fabricId;
    this.udpListenPort = builder.udpListenPort;
    this.controllerVendorId = builder.controllerVendorId;
    this.failsafeTimerSeconds = builder.failsafeTimerSeconds;
    this.caseFailsafeTimerSeconds = builder.caseFailsafeTimerSeconds;
    this.attemptNetworkScanWiFi = builder.attemptNetworkScanWiFi;
    this.attemptNetworkScanThread = builder.attemptNetworkScanThread;
    this.skipCommissioningComplete = builder.skipCommissioningComplete;
    this.skipAttestationCertificateValidation = builder.skipAttestationCertificateValidation;
    this.countryCode = builder.countryCode;
    this.regulatoryLocationType = builder.regulatoryLocationType;
    this.keypairDelegate = builder.keypairDelegate;
    this.rootCertificate = builder.rootCertificate;
    this.intermediateCertificate = builder.intermediateCertificate;
    this.operationalCertificate = builder.operationalCertificate;
    this.ipk = builder.ipk;
    this.adminSubject = builder.adminSubject;
    this.enableServerInteractions = builder.enableServerInteractions;
  }

  public long getFabricId() {
    return fabricId;
  }

  /** Gets the UDP listening port; 0 indicates "any available port" */
  public int getUdpListenPort() {
    return udpListenPort;
  }

  public int getControllerVendorId() {
    return controllerVendorId;
  }

  public int getFailsafeTimerSeconds() {
    return failsafeTimerSeconds;
  }

  public int getCASEFailsafeTimerSeconds() {
    return caseFailsafeTimerSeconds;
  }

  public boolean getAttemptNetworkScanWiFi() {
    return attemptNetworkScanWiFi;
  }

  public boolean getAttemptNetworkScanThread() {
    return attemptNetworkScanThread;
  }

  public boolean getSkipCommissioningComplete() {
    return skipCommissioningComplete;
  }

  public boolean getSkipAttestationCertificateValidation() {
    return skipAttestationCertificateValidation;
  }

  public Optional<String> getCountryCode() {
    return countryCode;
  }

  public Optional<Integer> getRegulatoryLocation() {
    return regulatoryLocationType;
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

  public long getAdminSubject() {
    return adminSubject;
  }

  public boolean getEnableServerInteractions() {
    return enableServerInteractions;
  }

  /** Returns parameters with ephemerally generated operational credentials */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Returns parameters which uses the provided {@code operationalKeyConfig} as its operating
   * credentials. You must set a vendor ID, 0xFFF4 is a test vendor ID
   * ControllerParams.newBuilder().setControllerVendorId(0xFFF4).build()
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
    private long fabricId = 1;
    private int udpListenPort = 0;
    private int controllerVendorId = 0xFFFF;
    private int failsafeTimerSeconds = 30;
    private int caseFailsafeTimerSeconds = 0;
    private boolean attemptNetworkScanWiFi = false;
    private boolean attemptNetworkScanThread = false;
    private boolean skipCommissioningComplete = false;
    private boolean skipAttestationCertificateValidation = false;
    private Optional<String> countryCode = Optional.empty();
    private Optional<Integer> regulatoryLocationType = Optional.empty();
    @Nullable private KeypairDelegate keypairDelegate = null;
    @Nullable private byte[] rootCertificate = null;
    @Nullable private byte[] intermediateCertificate = null;
    @Nullable private byte[] operationalCertificate = null;
    @Nullable private byte[] ipk = null;
    private long adminSubject = 0;
    private boolean enableServerInteractions = false;

    private Builder() {}

    public Builder setFabricId(long fabricId) {
      this.fabricId = fabricId;
      return this;
    }

    public Builder setUdpListenPort(int udpListenPort) {
      if (udpListenPort < 0) {
        throw new IllegalArgumentException("udpListenPort must be >= 0");
      }
      this.udpListenPort = udpListenPort;
      return this;
    }

    public Builder setControllerVendorId(int controllerVendorId) {
      this.controllerVendorId = controllerVendorId;
      return this;
    }

    /**
     * Sets the FailsafeTimer duration passed to ChipDeviceCommissioner's CommissioningParameters.
     * Increasing this value from its default will allow more time for network scans, cloud op cert
     * signing calls, and user interaction.
     *
     * <p>Note: It is also possible for internal logic (within Autocommissioner, etc) to re-call
     * ArmFailSafe to account for network config delays.
     *
     * @param failsafeTimerSeconds
     * @return
     */
    public Builder setFailsafeTimerSeconds(int failsafeTimerSeconds) {
      if (failsafeTimerSeconds < 1 || failsafeTimerSeconds > 900) {
        throw new IllegalArgumentException("failsafeTimerSeconds must be between 0 and 900");
      }
      this.failsafeTimerSeconds = failsafeTimerSeconds;
      return this;
    }

    /**
     * Sets the CASEFailsafeExpirySeconds duration passed to ChipDeviceCommissioner's
     * CommissioningParameters. After PASE session has finished, the failsafe is rearmed with the
     * specified expiry before continuing commissioning.
     *
     * <p>Note: If CASEFailsafeExpirySeconds is not set (or is 0), the failsafe will not be rearmed.
     *
     * @param caseFailsafeExpirySeconds
     * @return
     */
    public Builder setCASEFailsafeTimerSeconds(int failsafeTimerSeconds) {
      if (failsafeTimerSeconds < 1 || failsafeTimerSeconds > 900) {
        throw new IllegalArgumentException("failsafeTimerSeconds must be between 0 and 900");
      }
      this.caseFailsafeTimerSeconds = failsafeTimerSeconds;
      return this;
    }

    /**
     * Enable/disable wifi network scan during commissioning in the the default
     * CommissioningDelegate used by the ChipDeviceCommissioner.
     *
     * <p>Specifically, this sets AttemptWiFiNetworkScan in the CommissioningParameters passed to
     * the CommissioningDelegate.
     *
     * <p>When a WiFi scan is attempted, the result will be propagated to the ScanNetworksListener
     * assigned to the ChipDeviceController.
     *
     * @param attemptNetworkScanWiFi
     * @return
     */
    public Builder setAttemptNetworkScanWiFi(boolean attemptNetworkScanWiFi) {
      this.attemptNetworkScanWiFi = attemptNetworkScanWiFi;
      return this;
    }

    /**
     * Enable/disable Thread network scan during commissioning in the the default
     * CommissioningDelegate used by the ChipDeviceCommissioner.
     *
     * <p>Specifically, this sets AttemptThreadNetworkScan in the CommissioningParameters passed to
     * the CommissioningDelegate.
     *
     * <p>When a Thread scan is attempted, the result will be propagated to the ScanNetworksListener
     * assigned to the ChipDeviceController.
     *
     * @param attemptNetworkScanWiFi
     * @return
     */
    public Builder setAttemptNetworkScanThread(boolean attemptNetworkScanThread) {
      this.attemptNetworkScanThread = attemptNetworkScanThread;
      return this;
    }

    /**
     * Disable the CASE phase of commissioning when the CommissioningComplete command is sent by
     * this ChipDeviceCommissioner.
     *
     * <p>Specifically, this sets SkipCommissioningComplete in the CommissioningParameters passed to
     * the CommissioningDelegate.
     *
     * <p>A controller will set this to true when the CASE phase of commissioning is done by a
     * separate process, for example, by a Hub on the network.
     *
     * @param skipCommissioningComplete
     * @return
     */
    public Builder setSkipCommissioningComplete(boolean skipCommissioningComplete) {
      this.skipCommissioningComplete = skipCommissioningComplete;
      return this;
    }

    /**
     * Used when the Commissioner disables Attestation Certificate Validation.
     *
     * <p>Specifically, this sets SkipAttestationCertificateValidation in the
     * CommissioningParameters passed to the CommissioningDelegate.
     *
     * @param skipAttestationCertificateValidation
     * @return
     */
    public Builder setSkipAttestationCertificateValidation(
        boolean skipAttestationCertificateValidation) {
      this.skipAttestationCertificateValidation = skipAttestationCertificateValidation;
      return this;
    }

    /**
     * Sets the Regulatory Location country code passed to ChipDeviceCommissioner's
     * CommissioningParameters.
     *
     * <p>Setting the country code will set the CountryCode when the SetRegulatoryConfig command is
     * sent by this ChipDeviceCommissioner.
     *
     * @param countryCode an ISO 3166-1 alpha-2 code to represent the country, dependent territory,
     *     or special area of geographic interest
     * @return
     */
    public Builder setCountryCode(final String countryCode) {
      if (countryCode == null || countryCode.length() != 2) {
        throw new IllegalArgumentException("countryCode must be 2 characters");
      }
      this.countryCode = Optional.of(countryCode);
      return this;
    }

    /**
     * Sets the Regulatory Location capability passed to ChipDeviceCommissioner's
     * CommissioningParameters.
     *
     * <p>Setting the regulatory location type will set the NewRegulatoryConfig when the
     * SetRegulatoryConfig command is sent by this ChipDeviceCommissioner.
     *
     * @param regulatoryLocation an app::Clusters::GeneralCommissioning::RegulatoryLocationTypeEnum
     *     enum value
     * @return
     */
    public Builder setRegulatoryLocation(int regulatoryLocation) {
      if ((regulatoryLocation < 0) || (regulatoryLocation > 2)) {
        throw new IllegalArgumentException(
            "regulatoryLocation value must be between RegulatoryLocationTypeEnum::kIndoor and "
                + "RegulatoryLocationTypeEnum::kIndoorOutdoor");
      }
      this.regulatoryLocationType = Optional.of(regulatoryLocation);
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

    /**
     * Sets the AdminSubject value passed to ChipDeviceCommissioner's CommissioningParameters. This
     * value is passed in the AddNoc command sent to the commissionee and represents the subject of
     * the default ACL created by that call.
     *
     * @param adminSubject
     * @return
     */
    public Builder setAdminSubject(long adminSubject) {
      this.adminSubject = adminSubject;
      return this;
    }

    /**
     * Controls enabling server interactions on a controller. For ICD check-in message, this feature
     * has to be enabled.
     *
     * @param enableServerInteractions indicates whether to enable server interactions.
     */
    public Builder setEnableServerInteractions(boolean enableServerInteractions) {
      this.enableServerInteractions = enableServerInteractions;
      return this;
    }

    public ControllerParams build() {
      return new ControllerParams(this);
    }
  }
}
