package chip.devicecontroller;

/** Represents information relating to product attestation. */
public final class AttestationInfo {

  private byte[] challenge;
  private byte[] nonce;
  private byte[] elements;
  private byte[] elementsSignature;
  private byte[] dac;
  private byte[] pai;
  private byte[] certificationDeclaration;
  private byte[] firmwareInfo;
  private int vendorId;
  private int productId;

  public AttestationInfo(byte[] dac, byte[] pai, byte[] certificationDeclaration) {
    this.dac = dac;
    this.pai = pai;
    this.certificationDeclaration = certificationDeclaration;
  }

  public AttestationInfo(
      byte[] dac, byte[] pai, byte[] certificationDeclaration, int vendorId, int productId) {
    this.dac = dac;
    this.pai = pai;
    this.certificationDeclaration = certificationDeclaration;
    this.vendorId = vendorId;
    this.productId = productId;
  }

  public AttestationInfo(
      byte[] challenge,
      byte[] nonce,
      byte[] elements,
      byte[] elementsSignature,
      byte[] dac,
      byte[] pai,
      byte[] certificationDeclaration,
      byte[] firmwareInfo) {
    this.challenge = challenge;
    this.nonce = nonce;
    this.elements = elements;
    this.elementsSignature = elementsSignature;
    this.dac = dac;
    this.pai = pai;
    this.certificationDeclaration = certificationDeclaration;
    this.firmwareInfo = firmwareInfo;
  }

  public AttestationInfo(
      byte[] challenge,
      byte[] nonce,
      byte[] elements,
      byte[] elementsSignature,
      byte[] dac,
      byte[] pai,
      byte[] certificationDeclaration,
      byte[] firmwareInfo,
      int vendorId,
      int productId) {
    this.challenge = challenge;
    this.nonce = nonce;
    this.elements = elements;
    this.elementsSignature = elementsSignature;
    this.dac = dac;
    this.pai = pai;
    this.certificationDeclaration = certificationDeclaration;
    this.firmwareInfo = firmwareInfo;
    this.vendorId = vendorId;
    this.productId = productId;
  }

  public byte[] getChallenge() {
    return challenge;
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

  public byte[] getDAC() {
    return dac;
  }

  public byte[] getPAI() {
    return pai;
  }

  public byte[] getCertificationDeclaration() {
    return certificationDeclaration;
  }

  public byte[] getFirmwareInfo() {
    return firmwareInfo;
  }

  public int getVendorId() {
    return vendorId;
  }

  public int getProductId() {
    return productId;
  }
}
