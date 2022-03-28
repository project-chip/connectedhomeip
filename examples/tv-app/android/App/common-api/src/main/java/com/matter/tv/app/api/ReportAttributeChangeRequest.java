package com.matter.tv.app.api;

import android.os.Parcel;
import android.os.Parcelable;

public class ReportAttributeChangeRequest implements Parcelable {
  protected ReportAttributeChangeRequest(Parcel in) {}

  public static final Creator<ReportAttributeChangeRequest> CREATOR =
      new Creator<ReportAttributeChangeRequest>() {
        @Override
        public ReportAttributeChangeRequest createFromParcel(Parcel in) {
          return new ReportAttributeChangeRequest(in);
        }

        @Override
        public ReportAttributeChangeRequest[] newArray(int size) {
          return new ReportAttributeChangeRequest[size];
        }
      };

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel parcel, int i) {}
}
