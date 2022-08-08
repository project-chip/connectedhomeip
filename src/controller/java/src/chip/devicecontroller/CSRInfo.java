package chip.devicecontroller;

/** Represents information relating to NOC CSR. */
public final class CSRInfo {

  private byte[] nonce;
  private byte[] elements;
  private byte[] elementsSignature;
  private byte[] csr;

  public CSRInfo(byte[] nonce, byte[] elements, byte[] elementsSignature, byte[] csr) {
    this.nonce = nonce;
    this.elements = elements;
    this.elementsSignature = elementsSignature;
    this.csr = csr;
  }

  public byte[] getNonce() {
    return nonce;
  }

  public byte[] getElements() {
    return elements;
  }

  public byte[] getElementsSignature() {
    return elementsSignature;
  }

  public byte[] getCSR() {
    return csr;
  }
}
