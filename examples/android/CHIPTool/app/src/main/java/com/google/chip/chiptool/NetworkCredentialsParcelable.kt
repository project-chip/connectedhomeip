/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.google.chip.chiptool

import android.os.Parcel
import android.os.Parcelable
import android.os.Parcelable.Creator

/** Class for holding WiFi or Thread credentials, but not both. */
class NetworkCredentialsParcelable : Parcelable {
  var wiFiCredentials: WiFiCredentials?
    private set

  var threadCredentials: ThreadCredentials?
    private set

  private constructor(wifiCredentials: WiFiCredentials?, threadCredentials: ThreadCredentials?) {
    this.wiFiCredentials = wifiCredentials
    this.threadCredentials = threadCredentials
  }

  // Begin Parcelable implementation
  private constructor(parcel: Parcel) {
    wiFiCredentials = parcel.readParcelable(WiFiCredentials::class.java.classLoader)
    threadCredentials = parcel.readParcelable(ThreadCredentials::class.java.classLoader)
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
    private constructor(parcel: Parcel) {
      ssid = parcel.readString()
      password = parcel.readString()
    }

    override fun describeContents(): Int {
      return 0
    }

    override fun writeToParcel(out: Parcel, flags: Int) {
      out.writeString(ssid)
      out.writeString(password)
    }

    companion object CREATOR : Parcelable.Creator<WiFiCredentials?> {
      override fun createFromParcel(parcel: Parcel): WiFiCredentials? {
        return WiFiCredentials(parcel)
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
    private constructor(parcel: Parcel) {
      operationalDataset = ByteArray(parcel.readInt())
      parcel.readByteArray(operationalDataset)
    }

    override fun describeContents(): Int {
      return 0
    }

    override fun writeToParcel(out: Parcel, flags: Int) {
      out.writeInt(operationalDataset.size)
      out.writeByteArray(operationalDataset)
    }

    companion object CREATOR : Parcelable.Creator<ThreadCredentials?> {
      override fun createFromParcel(parcel: Parcel): ThreadCredentials? {
        return ThreadCredentials(parcel)
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
    val CREATOR =
      object : Parcelable.Creator<NetworkCredentialsParcelable?> {
        override fun createFromParcel(parcel: Parcel): NetworkCredentialsParcelable? {
          return NetworkCredentialsParcelable(parcel)
        }

        override fun newArray(size: Int): Array<NetworkCredentialsParcelable?> {
          return arrayOfNulls(size)
        }
      }
  }
}
