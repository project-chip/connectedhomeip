package com.google.chip.chiptool.commissioner.thread;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;
import com.google.chip.chiptool.commissioner.NetworkCredential;

public class ThreadNetworkCredential implements NetworkCredential, Parcelable {

  private byte[] activeOperationalDataset;

  public ThreadNetworkCredential(@NonNull byte[] activeOperationalDataset) {
    this.activeOperationalDataset = activeOperationalDataset;
  }

  public byte[] getActiveOperationalDataset() {
    return activeOperationalDataset;
  }

  @Override
  public byte[] getEncoded() {
    return activeOperationalDataset;
  }

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel parcel, int i) {
    parcel.writeByteArray(activeOperationalDataset);
  }
}
