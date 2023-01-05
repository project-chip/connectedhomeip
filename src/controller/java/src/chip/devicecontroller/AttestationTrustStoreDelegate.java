package chip.devicecontroller;

/**
 * Delegate for attestation trust store for device attestation verifiers.
 *
 * <p>API is synchronous.
 */
public interface AttestationTrustStoreDelegate {
  /**
   * Look-up a product attestation authority (PAA) cert by subject key identifier (SKID).
   *
   * <p>The implementations of this interface must have access to a set of PAAs to trust.
   *
   * <p>Interface is synchronous, and therefore this should not be used unless to expose a PAA store
   * that is both fully local and quick to access.
   *
   * @param skid Buffer containing the subject key identifier (SKID) of the PAA to look-up
   * @return If found, the result should return paa cert in x.509 format, if not found, return null.
   */
  byte[] getProductAttestationAuthorityCert(byte[] skid);
}
