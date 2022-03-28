package com.matter.tv.app.api;

import android.os.Parcel;
import android.os.Parcelable;

public class SetClustersRequest implements Parcelable {

  protected SetClustersRequest(Parcel in) {}

  public static final Creator<SetClustersRequest> CREATOR =
      new Creator<SetClustersRequest>() {
        @Override
        public SetClustersRequest createFromParcel(Parcel in) {
          return new SetClustersRequest(in);
        }

        @Override
        public SetClustersRequest[] newArray(int size) {
          return new SetClustersRequest[size];
        }
      };

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel parcel, int i) {}
}
