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
    tlvWriter.put(ContextSpecificTag(0), extAddress)
    tlvWriter.put(ContextSpecificTag(1), age)
    tlvWriter.put(ContextSpecificTag(2), rloc16)
    tlvWriter.put(ContextSpecificTag(3), linkFrameCounter)
    tlvWriter.put(ContextSpecificTag(4), mleFrameCounter)
    tlvWriter.put(ContextSpecificTag(5), lqi)
    if (averageRssi == null) { tlvWriter.putNull(ContextSpecificTag(6)) }
    else {
      tlvWriter.put(ContextSpecificTag(6), averageRssi)
    }
    if (lastRssi == null) { tlvWriter.putNull(ContextSpecificTag(7)) }
    else {
      tlvWriter.put(ContextSpecificTag(7), lastRssi)
    }
    tlvWriter.put(ContextSpecificTag(8), frameErrorRate)
    tlvWriter.put(ContextSpecificTag(9), messageErrorRate)
    tlvWriter.put(ContextSpecificTag(10), rxOnWhenIdle)
    tlvWriter.put(ContextSpecificTag(11), fullThreadDevice)
    tlvWriter.put(ContextSpecificTag(12), fullNetworkData)
    tlvWriter.put(ContextSpecificTag(13), isChild)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ThreadNetworkDiagnosticsClusterNeighborTableStruct {
      tlvReader.enterStructure(tag)
      val extAddress: Long = tlvReader.getLong(ContextSpecificTag(0))
      val age: Long = tlvReader.getLong(ContextSpecificTag(1))
      val rloc16: Int = tlvReader.getInt(ContextSpecificTag(2))
      val linkFrameCounter: Long = tlvReader.getLong(ContextSpecificTag(3))
      val mleFrameCounter: Long = tlvReader.getLong(ContextSpecificTag(4))
      val lqi: Int = tlvReader.getInt(ContextSpecificTag(5))
      val averageRssi: Int? = try {
      tlvReader.getInt(ContextSpecificTag(6))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(6))
      null
    }
      val lastRssi: Int? = try {
      tlvReader.getInt(ContextSpecificTag(7))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(7))
      null
    }
      val frameErrorRate: Int = tlvReader.getInt(ContextSpecificTag(8))
      val messageErrorRate: Int = tlvReader.getInt(ContextSpecificTag(9))
      val rxOnWhenIdle: Boolean = tlvReader.getBoolean(ContextSpecificTag(10))
      val fullThreadDevice: Boolean = tlvReader.getBoolean(ContextSpecificTag(11))
      val fullNetworkData: Boolean = tlvReader.getBoolean(ContextSpecificTag(12))
      val isChild: Boolean = tlvReader.getBoolean(ContextSpecificTag(13))
      
      tlvReader.exitContainer()

      return ThreadNetworkDiagnosticsClusterNeighborTableStruct(extAddress, age, rloc16, linkFrameCounter, mleFrameCounter, lqi, averageRssi, lastRssi, frameErrorRate, messageErrorRate, rxOnWhenIdle, fullThreadDevice, fullNetworkData, isChild)
    }
  }
}
