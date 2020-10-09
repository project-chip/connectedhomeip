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

package com.google.chip.chiptool.commissioner.thread.internal;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.NonNull;
import com.google.chip.chiptool.commissioner.thread.BorderAgentInfo;
import com.google.chip.chiptool.commissioner.thread.ThreadNetworkInfo;
import java.util.ArrayList;

class ThreadNetworkInfoHolder implements Parcelable {
  @NonNull
  private final ThreadNetworkInfo networkInfo;

  @NonNull
  private final ArrayList<BorderAgentInfo> borderAgents;

  public ThreadNetworkInfoHolder(BorderAgentInfo borderAgent) {
    networkInfo = new ThreadNetworkInfo(borderAgent.networkName, borderAgent.extendedPanId);
    borderAgents = new ArrayList<>();
    borderAgents.add(borderAgent);
  }

  public ThreadNetworkInfo getNetworkInfo() {
    return networkInfo;
  }

  public ArrayList<BorderAgentInfo> getBorderAgents() {
    return borderAgents;
  }

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel dest, int flags) {
    dest.writeParcelable(networkInfo, flags);
    dest.writeParcelableArray((BorderAgentInfo[]) borderAgents.toArray(), flags);
  }
}
