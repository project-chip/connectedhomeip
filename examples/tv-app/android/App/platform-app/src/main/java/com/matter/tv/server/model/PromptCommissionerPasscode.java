package com.matter.tv.server.model;

import android.os.Parcel;
import android.os.Parcelable;

public class PromptCommissionerPasscode implements Parcelable {
  private int vendorId;
  private int productId;
  private String commissioneeName;
  private long passcode;
  private int pairingHint;
  private String pairingInstruction;

  public PromptCommissionerPasscode(
      int vendorId,
      int productId,
      String commissioneeName,
      long passcode,
      int pairingHint,
      String pairingInstruction) {
    this.vendorId = vendorId;
    this.productId = productId;
    this.commissioneeName = commissioneeName;
    this.passcode = passcode;
    this.pairingHint = pairingHint;
    this.pairingInstruction = pairingInstruction;
  }

  protected PromptCommissionerPasscode(Parcel in) {
    vendorId = in.readInt();
    productId = in.readInt();
    commissioneeName = in.readString();
    passcode = in.readLong();
    pairingHint = in.readInt();
    pairingInstruction = in.readString();
  }

  public static final Creator<PromptCommissionerPasscode> CREATOR =
      new Creator<PromptCommissionerPasscode>() {
        @Override
        public PromptCommissionerPasscode createFromParcel(Parcel in) {
          return new PromptCommissionerPasscode(in);
        }

        @Override
        public PromptCommissionerPasscode[] newArray(int size) {
          return new PromptCommissionerPasscode[size];
        }
      };

  public int getVendorId() {
    return vendorId;
  }

  public void setVendorId(int vendorId) {
    this.vendorId = vendorId;
  }

  public int getProductId() {
    return productId;
  }

  public void setProductId(int productId) {
    this.productId = productId;
  }

  public String getCommissioneeName() {
    return commissioneeName;
  }

  public void setCommissioneeName(String commissioneeName) {
    this.commissioneeName = commissioneeName;
  }

  public long getPasscode() {
    return passcode;
  }

  public void setPasscode(long passcode) {
    this.passcode = passcode;
  }

  public int getPairingHint() {
    return pairingHint;
  }

  public void setPairingHint(int pairingHint) {
    this.pairingHint = pairingHint;
  }

  public String getPairingInstruction() {
    return pairingInstruction;
  }

  public void setPairingInstruction(String pairingInstruction) {
    this.pairingInstruction = pairingInstruction;
  }

  @Override
  public String toString() {
    return "PromptCommissionerPasscode{"
        + "vendorId="
        + vendorId
        + ", productId="
        + productId
        + ", commissioneeName='"
        + commissioneeName
        + '\''
        + ", passcode="
        + passcode
        + ", pairingHint="
        + pairingHint
        + ", pairingInstruction='"
        + pairingInstruction
        + '\''
        + '}';
  }

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel parcel, int i) {
    parcel.writeInt(vendorId);
    parcel.writeInt(productId);
    parcel.writeString(commissioneeName);
    parcel.writeLong(passcode);
    parcel.writeInt(pairingHint);
    parcel.writeString(pairingInstruction);
  }
}
