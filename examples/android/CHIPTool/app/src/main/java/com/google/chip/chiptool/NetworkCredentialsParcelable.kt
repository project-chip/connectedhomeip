/*
 *   Copyright (c) 2023 Project CHIP Authors
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
package com.google.chip.chiptool

import android.os.Parcelable
import android.os.Parcel
import android.os.Parcelable.Creator

/** Class for holding WiFi or Thread credentials, but not both.  */
class NetworkCredentialsParcelable : Parcelable {
  var wiFiCredentials: WiFiCredentials?
    private set
  var threadCredentials: ThreadCredentials?
    private set

  private constructor(
    wifiCredentials: WiFiCredentials?, threadCredentials: ThreadCredentials?
  ) {
    wiFiCredentials = wifiCredentials
    this.threadCredentials = threadCredentials
  }

  // Begin Parcelable implementation
  private constructor(`in`: Parcel) {
    wiFiCredentials = `in`.readParcelable(WiFiCredentials::class.java.classLoader)
    threadCredentials = `in`.readParcelable(ThreadCredentials::class.java.classLoader)
  }

  override fun describeContents(): Int {
    return 0
  }

  override fun writeToParcel(out: Parcel, flags: Int) {
    out.writeParcelable(wiFiCredentials, 0)
    out.writeParcelable(threadCredentials, 0)
  }

  class WiFiCredentials : Parcelable {
    val ssid: String?
    val password: String?

    constructor(ssid: String?, password: String?) {
      this.ssid = ssid
      this.password = password
    }

    // Begin Parcelable implementation
    private constructor(`in`: Parcel) {
      ssid = `in`.readString()
      password = `in`.readString()
    }

    override fun describeContents(): Int {
      return 0
    }

    override fun writeToParcel(out: Parcel, flags: Int) {
      out.writeString(ssid)
      out.writeString(password)
    }

    companion object CREATOR : Parcelable.Creator<WiFiCredentials?> {
      override fun createFromParcel(`in`: Parcel): WiFiCredentials? {
        return WiFiCredentials(`in`)
      }

      override fun newArray(size: Int): Array<WiFiCredentials?> {
        return arrayOfNulls(size)
      }
    }
  }

  class ThreadCredentials : Parcelable {
    val operationalDataset: ByteArray

    constructor(operationalDataset: ByteArray) {
      this.operationalDataset = operationalDataset
    }

    // Begin Parcelable implementation
    private constructor(`in`: Parcel) {
      operationalDataset = ByteArray(`in`.readInt())
      `in`.readByteArray(operationalDataset)
    }

    override fun describeContents(): Int {
      return 0
    }

    override fun writeToParcel(out: Parcel, flags: Int) {
      out.writeInt(operationalDataset.size)
      out.writeByteArray(operationalDataset)
    }

    companion object CREATOR : Parcelable.Creator<ThreadCredentials?> {
      override fun createFromParcel(`in`: Parcel): ThreadCredentials? {
        return ThreadCredentials(`in`)
      }

      override fun newArray(size: Int): Array<ThreadCredentials?> {
        return arrayOfNulls(size)
      }
    }
  }

  companion object {
    /**
     * Return a NetworkCredentialsParcelable object with the given WiFiCredentials and null
     * ThreadCredentials.
     */
    fun forWiFi(wifiCredentials: WiFiCredentials?): NetworkCredentialsParcelable {
      return NetworkCredentialsParcelable(wifiCredentials, null)
    }

    /**
     * Return a NetworkCredentialsParcelable object with the given ThreadCredentials and null
     * WiFiCredentials.
     */
    fun forThread(threadCredentials: ThreadCredentials?): NetworkCredentialsParcelable {
      return NetworkCredentialsParcelable(null, threadCredentials)
    }

    @JvmField
    val CREATOR = object : Parcelable.Creator<NetworkCredentialsParcelable?> {
      override fun createFromParcel(`in`: Parcel): NetworkCredentialsParcelable? {
        return NetworkCredentialsParcelable(`in`)
      }

      override fun newArray(size: Int): Array<NetworkCredentialsParcelable?> {
        return arrayOfNulls(size)
      }
    }
  }
}