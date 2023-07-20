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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ThreadNetworkDiagnosticsClusterNeighborTableStruct (
    val extAddress: Long,
    val age: Long,
    val rloc16: Int,
    val linkFrameCounter: Long,
    val mleFrameCounter: Long,
    val lqi: Int,
    val averageRssi: Int?,
    val lastRssi: Int?,
    val frameErrorRate: Int,
    val messageErrorRate: Int,
    val rxOnWhenIdle: Boolean,
    val fullThreadDevice: Boolean,
    val fullNetworkData: Boolean,
    val isChild: Boolean) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ThreadNetworkDiagnosticsClusterNeighborTableStruct {\n")
    builder.append("\textAddress : $extAddress\n")
    builder.append("\tage : $age\n")
    builder.append("\trloc16 : $rloc16\n")
    builder.append("\tlinkFrameCounter : $linkFrameCounter\n")
    builder.append("\tmleFrameCounter : $mleFrameCounter\n")
    builder.append("\tlqi : $lqi\n")
    builder.append("\taverageRssi : $averageRssi\n")
    builder.append("\tlastRssi : $lastRssi\n")
    builder.append("\tframeErrorRate : $frameErrorRate\n")
    builder.append("\tmessageErrorRate : $messageErrorRate\n")
    builder.append("\trxOnWhenIdle : $rxOnWhenIdle\n")
    builder.append("\tfullThreadDevice : $fullThreadDevice\n")
    builder.append("\tfullNetworkData : $fullNetworkData\n")
    builder.append("\tisChild : $isChild\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_EXT_ADDRESS), extAddress)
    tlvWriter.put(ContextSpecificTag(TAG_AGE), age)
    tlvWriter.put(ContextSpecificTag(TAG_RLOC16), rloc16)
    tlvWriter.put(ContextSpecificTag(TAG_LINK_FRAME_COUNTER), linkFrameCounter)
    tlvWriter.put(ContextSpecificTag(TAG_MLE_FRAME_COUNTER), mleFrameCounter)
    tlvWriter.put(ContextSpecificTag(TAG_LQI), lqi)
    if (averageRssi == null) { tlvWriter.putNull(ContextSpecificTag(TAG_AVERAGE_RSSI)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_AVERAGE_RSSI), averageRssi)
    }
    if (lastRssi == null) { tlvWriter.putNull(ContextSpecificTag(TAG_LAST_RSSI)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_LAST_RSSI), lastRssi)
    }
    tlvWriter.put(ContextSpecificTag(TAG_FRAME_ERROR_RATE), frameErrorRate)
    tlvWriter.put(ContextSpecificTag(TAG_MESSAGE_ERROR_RATE), messageErrorRate)
    tlvWriter.put(ContextSpecificTag(TAG_RX_ON_WHEN_IDLE), rxOnWhenIdle)
    tlvWriter.put(ContextSpecificTag(TAG_FULL_THREAD_DEVICE), fullThreadDevice)
    tlvWriter.put(ContextSpecificTag(TAG_FULL_NETWORK_DATA), fullNetworkData)
    tlvWriter.put(ContextSpecificTag(TAG_IS_CHILD), isChild)
    tlvWriter.endStructure()
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

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterNeighborTableStruct {
      tlvReader.enterStructure(tag)
      val extAddress: Long = tlvReader.getLong(ContextSpecificTag(TAG_EXT_ADDRESS))
      val age: Long = tlvReader.getLong(ContextSpecificTag(TAG_AGE))
      val rloc16: Int = tlvReader.getInt(ContextSpecificTag(TAG_RLOC16))
      val linkFrameCounter: Long = tlvReader.getLong(ContextSpecificTag(TAG_LINK_FRAME_COUNTER))
      val mleFrameCounter: Long = tlvReader.getLong(ContextSpecificTag(TAG_MLE_FRAME_COUNTER))
      val lqi: Int = tlvReader.getInt(ContextSpecificTag(TAG_LQI))
      val averageRssi: Int? = try {
      tlvReader.getInt(ContextSpecificTag(TAG_AVERAGE_RSSI))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_AVERAGE_RSSI))
      null
    }
      val lastRssi: Int? = try {
      tlvReader.getInt(ContextSpecificTag(TAG_LAST_RSSI))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_LAST_RSSI))
      null
    }
      val frameErrorRate: Int = tlvReader.getInt(ContextSpecificTag(TAG_FRAME_ERROR_RATE))
      val messageErrorRate: Int = tlvReader.getInt(ContextSpecificTag(TAG_MESSAGE_ERROR_RATE))
      val rxOnWhenIdle: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_RX_ON_WHEN_IDLE))
      val fullThreadDevice: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_FULL_THREAD_DEVICE))
      val fullNetworkData: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_FULL_NETWORK_DATA))
      val isChild: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_IS_CHILD))
      
      tlvReader.exitContainer()

      return ThreadNetworkDiagnosticsClusterNeighborTableStruct(extAddress, age, rloc16, linkFrameCounter, mleFrameCounter, lqi, averageRssi, lastRssi, frameErrorRate, messageErrorRate, rxOnWhenIdle, fullThreadDevice, fullNetworkData, isChild)
    }
  }
}
