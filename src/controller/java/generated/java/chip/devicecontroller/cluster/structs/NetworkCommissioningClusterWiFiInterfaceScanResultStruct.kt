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
    tlvWriter.put(ContextSpecificTag(TAG_SECURITY), security)
    tlvWriter.put(ContextSpecificTag(TAG_SSID), ssid)
    tlvWriter.put(ContextSpecificTag(TAG_BSSID), bssid)
    tlvWriter.put(ContextSpecificTag(TAG_CHANNEL), channel)
    tlvWriter.put(ContextSpecificTag(TAG_WI_FI_BAND), wiFiBand)
    tlvWriter.put(ContextSpecificTag(TAG_RSSI), rssi)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_SECURITY = 0
    private const val TAG_SSID = 1
    private const val TAG_BSSID = 2
    private const val TAG_CHANNEL = 3
    private const val TAG_WI_FI_BAND = 4
    private const val TAG_RSSI = 5

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : NetworkCommissioningClusterWiFiInterfaceScanResultStruct {
      tlvReader.enterStructure(tag)
      val security: Int = tlvReader.getInt(ContextSpecificTag(TAG_SECURITY))
      val ssid: ByteArray = tlvReader.getByteArray(ContextSpecificTag(TAG_SSID))
      val bssid: ByteArray = tlvReader.getByteArray(ContextSpecificTag(TAG_BSSID))
      val channel: Int = tlvReader.getInt(ContextSpecificTag(TAG_CHANNEL))
      val wiFiBand: Int = tlvReader.getInt(ContextSpecificTag(TAG_WI_FI_BAND))
      val rssi: Int = tlvReader.getInt(ContextSpecificTag(TAG_RSSI))
      
      tlvReader.exitContainer()

      return NetworkCommissioningClusterWiFiInterfaceScanResultStruct(security, ssid, bssid, channel, wiFiBand, rssi)
    }
  }
}
