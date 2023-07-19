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

class NetworkCommissioningClusterThreadInterfaceScanResultStruct (
    val panId: Int,
    val extendedPanId: Long,
    val networkName: String,
    val channel: Int,
    val version: Int,
    val extendedAddress: ByteArray,
    val rssi: Int,
    val lqi: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("NetworkCommissioningClusterThreadInterfaceScanResultStruct {\n")
    builder.append("\tpanId : $panId\n")
    builder.append("\textendedPanId : $extendedPanId\n")
    builder.append("\tnetworkName : $networkName\n")
    builder.append("\tchannel : $channel\n")
    builder.append("\tversion : $version\n")
    builder.append("\textendedAddress : $extendedAddress\n")
    builder.append("\trssi : $rssi\n")
    builder.append("\tlqi : $lqi\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), panId)
    tlvWriter.put(ContextSpecificTag(1), extendedPanId)
    tlvWriter.put(ContextSpecificTag(2), networkName)
    tlvWriter.put(ContextSpecificTag(3), channel)
    tlvWriter.put(ContextSpecificTag(4), version)
    tlvWriter.put(ContextSpecificTag(5), extendedAddress)
    tlvWriter.put(ContextSpecificTag(6), rssi)
    tlvWriter.put(ContextSpecificTag(7), lqi)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : NetworkCommissioningClusterThreadInterfaceScanResultStruct {
      tlvReader.enterStructure(tag)
      val panId: Int = tlvReader.getInt(ContextSpecificTag(0))
      val extendedPanId: Long = tlvReader.getLong(ContextSpecificTag(1))
      val networkName: String = tlvReader.getString(ContextSpecificTag(2))
      val channel: Int = tlvReader.getInt(ContextSpecificTag(3))
      val version: Int = tlvReader.getInt(ContextSpecificTag(4))
      val extendedAddress: ByteArray = tlvReader.getByteArray(ContextSpecificTag(5))
      val rssi: Int = tlvReader.getInt(ContextSpecificTag(6))
      val lqi: Int = tlvReader.getInt(ContextSpecificTag(7))
      
      tlvReader.exitContainer()

      return NetworkCommissioningClusterThreadInterfaceScanResultStruct(panId, extendedPanId, networkName, channel, version, extendedAddress, rssi, lqi)
    }
  }
}
