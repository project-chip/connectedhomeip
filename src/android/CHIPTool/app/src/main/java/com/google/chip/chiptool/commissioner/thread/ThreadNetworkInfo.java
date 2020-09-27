/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package com.google.chip.chiptool.commissioner.thread;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;

public class ThreadNetworkInfo implements Parcelable {

  @NonNull
  public String networkName;

  @NonNull
  public byte[] extendedPanId;

  public ThreadNetworkInfo(@NonNull String networkName, @NonNull byte[] extendedPanId) {
    this.networkName = networkName;
    this.extendedPanId = extendedPanId;
  }

  protected ThreadNetworkInfo(Parcel in) {
    networkName = in.readString();
    extendedPanId = in.createByteArray();
  }

  public static final Creator<ThreadNetworkInfo> CREATOR =
    new Creator<ThreadNetworkInfo>() {
      @Override
      public ThreadNetworkInfo createFromParcel(Parcel in) {
        return new ThreadNetworkInfo(in);
      }

      @Override
      public ThreadNetworkInfo[] newArray(int size) {
        return new ThreadNetworkInfo[size];
      }
    };

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel parcel, int flags) {
    parcel.writeString(networkName);
    parcel.writeByteArray(extendedPanId);
  }

  @Override
  public String toString() {
    return String.format("{name=%s, extendedPanId=%s}", networkName, CommissionerUtils.getHexString(extendedPanId));
  }
}
