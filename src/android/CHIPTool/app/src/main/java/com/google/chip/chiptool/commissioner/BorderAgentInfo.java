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
import androidx.annotation.NonNull;
import java.net.InetAddress;
import java.net.UnknownHostException;

public class BorderAgentInfo implements Parcelable {
  public String discriminator;
  public String networkName;
  public byte[] extendedPanId;
  public InetAddress host;
  public int port;
  public byte[] pskc;

  public BorderAgentInfo(
      @NonNull String discriminator,
      @NonNull String networkName,
      @NonNull byte[] extendedPanId,
      @NonNull InetAddress host,
      @NonNull int port,
      @NonNull byte[] pskc) {
    this.discriminator = discriminator;
    this.networkName = networkName;
    this.extendedPanId = extendedPanId;
    this.host = host;
    this.port = port;
    this.pskc = pskc;
  }

  protected BorderAgentInfo(Parcel in) {
    discriminator = in.readString();
    networkName = in.readString();
    extendedPanId = in.createByteArray();
    try {
      host = InetAddress.getByAddress(in.createByteArray());
    } catch (UnknownHostException e) {
    }
    port = in.readInt();
    pskc = in.createByteArray();
  }

  @Override
  public void writeToParcel(Parcel dest, int flags) {
    dest.writeString(discriminator);
    dest.writeString(networkName);
    dest.writeByteArray(extendedPanId);
    dest.writeByteArray(host.getAddress());
    dest.writeInt(port);
    dest.writeByteArray(pskc);
  }

  @Override
  public int describeContents() {
    return 0;
  }

  public static final Creator<BorderAgentInfo> CREATOR =
      new Creator<BorderAgentInfo>() {
        @Override
        public BorderAgentInfo createFromParcel(Parcel in) {
          return new BorderAgentInfo(in);
        }

        @Override
        public BorderAgentInfo[] newArray(int size) {
          return new BorderAgentInfo[size];
        }
      };
}
