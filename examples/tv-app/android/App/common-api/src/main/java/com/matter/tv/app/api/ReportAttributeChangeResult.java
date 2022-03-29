package com.matter.tv.app.api;

import android.os.Parcel;
import android.os.Parcelable;

public class ReportAttributeChangeResult implements Parcelable {
  protected ReportAttributeChangeResult(Parcel in) {}

  public static final Creator<ReportAttributeChangeResult> CREATOR =
      new Creator<ReportAttributeChangeResult>() {
        @Override
        public ReportAttributeChangeResult createFromParcel(Parcel in) {
          return new ReportAttributeChangeResult(in);
        }

        @Override
        public ReportAttributeChangeResult[] newArray(int size) {
          return new ReportAttributeChangeResult[size];
        }
      };

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel parcel, int i) {}
}
