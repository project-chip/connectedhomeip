package chip.devicecontroller;

import java.util.Arrays;
import java.util.Objects;

/** Parameters representing a generated PASE verifier for a setup PIN code. */
public final class PaseVerifierParams {

  private final long setupPincode;
  private final byte[] pakeVerifier;

  /**
   * Constructor
   *
   * @param setupPincode the PIN code associated with this verifier
   * @param pakeVerifier the encoded verifier (concatenation of w0 and L)
   */
  public PaseVerifierParams(long setupPincode, byte[] pakeVerifier) {
    this.setupPincode = setupPincode;
    this.pakeVerifier = pakeVerifier.clone();
  }

  /** Returns the PIN code associated with this verifier. */
  public long getSetupPincode() {
    return setupPincode;
  }

  /**
   * Returns the encoded PAKE verifier (the concatenation of w0 and L, as described in section 3.10
   * (PAKE) of the Matter specification).
   */
  public byte[] getPakeVerifier() {
    return pakeVerifier.clone();
  }

  @Override
  public boolean equals(Object other) {
    if (this == other) {
      return true;
    } else if (!(other instanceof PaseVerifierParams)) {
      return false;
    } else {
      PaseVerifierParams that = (PaseVerifierParams) other;
      return setupPincode == that.setupPincode && Arrays.equals(pakeVerifier, that.pakeVerifier);
    }
  }

  @Override
  public int hashCode() {
    int result = Objects.hash(setupPincode);
    result = 31 * result + Arrays.hashCode(pakeVerifier);
    return result;
  }
}
