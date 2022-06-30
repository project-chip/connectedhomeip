package chip.devicecontroller;

/** Delegate for a P256Keypair for use within the Java environment. */
public interface KeypairDelegate {
  /**
   * Ensure that a private key is generated when this method returns.
   *
   * @throws KeypairException if a private key could not be generated or resolved
   */
  void generatePrivateKey() throws KeypairException;

  /**
   * Returns an operational PKCS#10 CSR in DER-encoded form, signed by the underlying private key.
   *
   * @throws KeypairException if the CSR could not be generated
   */
  byte[] createCertificateSigningRequest() throws KeypairException;

  /**
   * Returns the DER-encoded X.509 public key, generating a new private key if one has not already
   * been created.
   *
   * @throws KeypairException if a private key could not be resolved
   */
  byte[] getPublicKey() throws KeypairException;

  /**
   * Signs the given message with the private key (generating one if it has not yet been created)
   * using ECDSA and returns a DER-encoded signature.
   *
   * @throws KeypairException if a private key could not be resolved, or the message could not be
   *     signed
   */
  byte[] ecdsaSignMessage(byte[] message) throws KeypairException;

  /** Encompassing exception to encapsulate errors thrown during operations. */
  final class KeypairException extends Exception {
    private static final long serialVersionUID = 2646523289554350914L;

    /** Constructs an exception with the specified {@code msg} as the message. */
    public KeypairException(String msg) {
      super(msg);
    }
    /**
     * Constructs an exception with the specified {@code msg} as the message and the provided {@code
     * cause}.
     */
    public KeypairException(String msg, Throwable cause) {
      super(msg, cause);
    }
  }
}
