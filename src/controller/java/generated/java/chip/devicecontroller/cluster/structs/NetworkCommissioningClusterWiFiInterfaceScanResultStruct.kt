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

class NetworkCommissioningClusterWiFiInterfaceScanResultStruct (
    val security: Int,
    val ssid: ByteArray,
    val bssid: ByteArray,
    val channel: Int,
    val wiFiBand: Int,
    val rssi: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("NetworkCommissioningClusterWiFiInterfaceScanResultStruct {\n")
    builder.append("\tsecurity : $security\n")
    builder.append("\tssid : $ssid\n")
    builder.append("\tbssid : $bssid\n")
    builder.append("\tchannel : $channel\n")
    builder.append("\twiFiBand : $wiFiBand\n")
    builder.append("\trssi : $rssi\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), security)
    tlvWriter.put(ContextSpecificTag(1), ssid)
    tlvWriter.put(ContextSpecificTag(2), bssid)
    tlvWriter.put(ContextSpecificTag(3), channel)
    tlvWriter.put(ContextSpecificTag(4), wiFiBand)
    tlvWriter.put(ContextSpecificTag(5), rssi)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : NetworkCommissioningClusterWiFiInterfaceScanResultStruct {
      tlvReader.enterStructure(tag)
      val security: Int = tlvReader.getInt(ContextSpecificTag(0))
      val ssid: ByteArray = tlvReader.getByteArray(ContextSpecificTag(1))
      val bssid: ByteArray = tlvReader.getByteArray(ContextSpecificTag(2))
      val channel: Int = tlvReader.getInt(ContextSpecificTag(3))
      val wiFiBand: Int = tlvReader.getInt(ContextSpecificTag(4))
      val rssi: Int = tlvReader.getInt(ContextSpecificTag(5))
      
      tlvReader.exitContainer()

      return NetworkCommissioningClusterWiFiInterfaceScanResultStruct(security, ssid, bssid, channel, wiFiBand, rssi)
    }
  }
}
