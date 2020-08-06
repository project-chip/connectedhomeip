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

package com.google.chip.chiptool.commissioner;

import android.os.Parcel;
import android.os.Parcelable;
import java.net.InetAddress;
import java.util.ArrayList;

public class NetworkInfo implements Parcelable {

  private ArrayList<BorderAgentInfo> borderAgents;

  public NetworkInfo(BorderAgentInfo borderAgent) {
    borderAgents = new ArrayList<>();
    borderAgents.add(borderAgent);
  }

  public InetAddress getHost() { return borderAgents.get(0).host; }

  public int getPort() { return borderAgents.get(0).port; }

  public String getNetworkName() {
    return borderAgents.get(0).networkName;
  }

  public byte[] getExtendedPanId() {
    return borderAgents.get(0).extendedPanId;
  }

  public void merge(NetworkInfo networkInfo) {
    borderAgents.addAll(networkInfo.borderAgents);
  }

  public void addBorderAgent(BorderAgentInfo borderAgent) {
    // TODO(wgtdkp): verify that the network name and extended PAN ID match.
    borderAgents.add(borderAgent);
  }

  protected NetworkInfo(Parcel in) {
    borderAgents = in.readArrayList(BorderAgentInfo.class.getClassLoader());
  }

  public static final Creator<NetworkInfo> CREATOR = new Creator<NetworkInfo>() {
    @Override
    public NetworkInfo createFromParcel(Parcel in) {
      return new NetworkInfo(in);
    }

    @Override
    public NetworkInfo[] newArray(int size) {
      return new NetworkInfo[size];
    }
  };

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel parcel, int flags) {
    parcel.writeList(borderAgents);
  }
}
