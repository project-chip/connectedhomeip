/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThreadNetworkDiagnosticsClusterNeighborTableStruct(
  val extAddress: ULong,
  val age: ULong,
  val rloc16: UInt,
  val linkFrameCounter: ULong,
  val mleFrameCounter: ULong,
  val lqi: UInt,
  val averageRssi: Int?,
  val lastRssi: Int?,
  val frameErrorRate: UInt,
  val messageErrorRate: UInt,
  val rxOnWhenIdle: Boolean,
  val fullThreadDevice: Boolean,
  val fullNetworkData: Boolean,
  val isChild: Boolean,
) {
  override fun toString(): String = buildString {
    append("ThreadNetworkDiagnosticsClusterNeighborTableStruct {\n")
    append("\textAddress : $extAddress\n")
    append("\tage : $age\n")
    append("\trloc16 : $rloc16\n")
    append("\tlinkFrameCounter : $linkFrameCounter\n")
    append("\tmleFrameCounter : $mleFrameCounter\n")
    append("\tlqi : $lqi\n")
    append("\taverageRssi : $averageRssi\n")
    append("\tlastRssi : $lastRssi\n")
    append("\tframeErrorRate : $frameErrorRate\n")
    append("\tmessageErrorRate : $messageErrorRate\n")
    append("\trxOnWhenIdle : $rxOnWhenIdle\n")
    append("\tfullThreadDevice : $fullThreadDevice\n")
    append("\tfullNetworkData : $fullNetworkData\n")
    append("\tisChild : $isChild\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_EXT_ADDRESS), extAddress)
      put(ContextSpecificTag(TAG_AGE), age)
      put(ContextSpecificTag(TAG_RLOC16), rloc16)
      put(ContextSpecificTag(TAG_LINK_FRAME_COUNTER), linkFrameCounter)
      put(ContextSpecificTag(TAG_MLE_FRAME_COUNTER), mleFrameCounter)
      put(ContextSpecificTag(TAG_LQI), lqi)
      if (averageRssi != null) {
        put(ContextSpecificTag(TAG_AVERAGE_RSSI), averageRssi)
      } else {
        putNull(ContextSpecificTag(TAG_AVERAGE_RSSI))
      }
      if (lastRssi != null) {
        put(ContextSpecificTag(TAG_LAST_RSSI), lastRssi)
      } else {
        putNull(ContextSpecificTag(TAG_LAST_RSSI))
      }
      put(ContextSpecificTag(TAG_FRAME_ERROR_RATE), frameErrorRate)
      put(ContextSpecificTag(TAG_MESSAGE_ERROR_RATE), messageErrorRate)
      put(ContextSpecificTag(TAG_RX_ON_WHEN_IDLE), rxOnWhenIdle)
      put(ContextSpecificTag(TAG_FULL_THREAD_DEVICE), fullThreadDevice)
      put(ContextSpecificTag(TAG_FULL_NETWORK_DATA), fullNetworkData)
      put(ContextSpecificTag(TAG_IS_CHILD), isChild)
      endStructure()
    }
  }

  companion object {
    private const val TAG_EXT_ADDRESS = 0
    private const val TAG_AGE = 1
    private const val TAG_RLOC16 = 2
    private const val TAG_LINK_FRAME_COUNTER = 3
    private const val TAG_MLE_FRAME_COUNTER = 4
    private const val TAG_LQI = 5
    private const val TAG_AVERAGE_RSSI = 6
    private const val TAG_LAST_RSSI = 7
    private const val TAG_FRAME_ERROR_RATE = 8
    private const val TAG_MESSAGE_ERROR_RATE = 9
    private const val TAG_RX_ON_WHEN_IDLE = 10
    private const val TAG_FULL_THREAD_DEVICE = 11
    private const val TAG_FULL_NETWORK_DATA = 12
    private const val TAG_IS_CHILD = 13

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ThreadNetworkDiagnosticsClusterNeighborTableStruct {
      tlvReader.enterStructure(tlvTag)
      val extAddress = tlvReader.getULong(ContextSpecificTag(TAG_EXT_ADDRESS))
      val age = tlvReader.getULong(ContextSpecificTag(TAG_AGE))
      val rloc16 = tlvReader.getUInt(ContextSpecificTag(TAG_RLOC16))
      val linkFrameCounter = tlvReader.getULong(ContextSpecificTag(TAG_LINK_FRAME_COUNTER))
      val mleFrameCounter = tlvReader.getULong(ContextSpecificTag(TAG_MLE_FRAME_COUNTER))
      val lqi = tlvReader.getUInt(ContextSpecificTag(TAG_LQI))
      val averageRssi =
        if (!tlvReader.isNull()) {
          tlvReader.getInt(ContextSpecificTag(TAG_AVERAGE_RSSI))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_AVERAGE_RSSI))
          null
        }
      val lastRssi =
        if (!tlvReader.isNull()) {
          tlvReader.getInt(ContextSpecificTag(TAG_LAST_RSSI))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LAST_RSSI))
          null
        }
      val frameErrorRate = tlvReader.getUInt(ContextSpecificTag(TAG_FRAME_ERROR_RATE))
      val messageErrorRate = tlvReader.getUInt(ContextSpecificTag(TAG_MESSAGE_ERROR_RATE))
      val rxOnWhenIdle = tlvReader.getBoolean(ContextSpecificTag(TAG_RX_ON_WHEN_IDLE))
      val fullThreadDevice = tlvReader.getBoolean(ContextSpecificTag(TAG_FULL_THREAD_DEVICE))
      val fullNetworkData = tlvReader.getBoolean(ContextSpecificTag(TAG_FULL_NETWORK_DATA))
      val isChild = tlvReader.getBoolean(ContextSpecificTag(TAG_IS_CHILD))

      tlvReader.exitContainer()

      return ThreadNetworkDiagnosticsClusterNeighborTableStruct(
        extAddress,
        age,
        rloc16,
        linkFrameCounter,
        mleFrameCounter,
        lqi,
        averageRssi,
        lastRssi,
        frameErrorRate,
        messageErrorRate,
        rxOnWhenIdle,
        fullThreadDevice,
        fullNetworkData,
        isChild,
      )
    }
  }
}
