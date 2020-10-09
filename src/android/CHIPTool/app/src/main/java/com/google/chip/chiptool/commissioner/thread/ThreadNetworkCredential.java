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
import com.google.chip.chiptool.commissioner.NetworkCredential;

public class ThreadNetworkCredential implements NetworkCredential, Parcelable {

  @NonNull private final byte[] activeOperationalDataset;

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
