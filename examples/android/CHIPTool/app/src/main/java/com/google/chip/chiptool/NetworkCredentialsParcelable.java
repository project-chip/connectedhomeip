/*
 *   Copyright (c) 2020-2022 Project CHIP Authors
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

package com.google.chip.chiptool;

import android.os.Parcel;
import android.os.Parcelable;
import androidx.annotation.Nullable;

/** Class for holding WiFi or Thread credentials, but not both. */
public class NetworkCredentialsParcelable implements Parcelable {
  @Nullable private WiFiCredentials wifiCredentials;
  @Nullable private ThreadCredentials threadCredentials;

  private NetworkCredentialsParcelable(
      @Nullable WiFiCredentials wifiCredentials, @Nullable ThreadCredentials threadCredentials) {
    this.wifiCredentials = wifiCredentials;
    this.threadCredentials = threadCredentials;
  }

  /**
   * Return a NetworkCredentialsParcelable object with the given WiFiCredentials and null
   * ThreadCredentials.
   */
  public static NetworkCredentialsParcelable forWiFi(WiFiCredentials wifiCredentials) {
    return new NetworkCredentialsParcelable(wifiCredentials, null);
  }

  /**
   * Return a NetworkCredentialsParcelable object with the given ThreadCredentials and null
   * WiFiCredentials.
   */
  public static NetworkCredentialsParcelable forThread(ThreadCredentials threadCredentials) {
    return new NetworkCredentialsParcelable(null, threadCredentials);
  }

  public WiFiCredentials getWiFiCredentials() {
    return wifiCredentials;
  }

  public ThreadCredentials getThreadCredentials() {
    return threadCredentials;
  }

  // Begin Parcelable implementation

  private NetworkCredentialsParcelable(Parcel in) {
    wifiCredentials = in.readParcelable(WiFiCredentials.class.getClassLoader());
    threadCredentials = in.readParcelable(ThreadCredentials.class.getClassLoader());
  }

  public int describeContents() {
    return 0;
  }

  public void writeToParcel(Parcel out, int flags) {
    out.writeParcelable(wifiCredentials, 0);
    out.writeParcelable(threadCredentials, 0);
  }

  public static final Parcelable.Creator<NetworkCredentialsParcelable> CREATOR =
      new Parcelable.Creator<NetworkCredentialsParcelable>() {
        public NetworkCredentialsParcelable createFromParcel(Parcel in) {
          return new NetworkCredentialsParcelable(in);
        }

        public NetworkCredentialsParcelable[] newArray(int size) {
          return new NetworkCredentialsParcelable[size];
        }
      };

  public static class WiFiCredentials implements Parcelable {
    private final String ssid;
    private final String password;

    public WiFiCredentials(String ssid, String password) {
      this.ssid = ssid;
      this.password = password;
    }

    public String getSsid() {
      return ssid;
    }

    public String getPassword() {
      return password;
    }

    // Begin Parcelable implementation

    private WiFiCredentials(Parcel in) {
      ssid = in.readString();
      password = in.readString();
    }

    public int describeContents() {
      return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
      out.writeString(ssid);
      out.writeString(password);
    }

    public static final Parcelable.Creator<WiFiCredentials> CREATOR =
        new Parcelable.Creator<WiFiCredentials>() {
          public WiFiCredentials createFromParcel(Parcel in) {
            return new WiFiCredentials(in);
          }

          public WiFiCredentials[] newArray(int size) {
            return new WiFiCredentials[size];
          }
        };
  }

  public static class ThreadCredentials implements Parcelable {
    private final byte[] operationalDataset;

    public ThreadCredentials(byte[] operationalDataset) {
      this.operationalDataset = operationalDataset;
    }

    public byte[] getOperationalDataset() {
      return operationalDataset;
    }

    // Begin Parcelable implementation

    private ThreadCredentials(Parcel in) {
      operationalDataset = new byte[in.readInt()];
      in.readByteArray(operationalDataset);
    }

    public int describeContents() {
      return 0;
    }

    public void writeToParcel(Parcel out, int flags) {
      out.writeInt(operationalDataset.length);
      out.writeByteArray(operationalDataset);
    }

    public static final Parcelable.Creator<ThreadCredentials> CREATOR =
        new Parcelable.Creator<ThreadCredentials>() {
          public ThreadCredentials createFromParcel(Parcel in) {
            return new ThreadCredentials(in);
          }

          public ThreadCredentials[] newArray(int size) {
            return new ThreadCredentials[size];
          }
        };
  }
}
