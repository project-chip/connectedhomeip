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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class NetworkCommissioningClusterWiFiInterfaceScanResultStruct (
    val security: UInt,
    val ssid: ByteArray,
    val bssid: ByteArray,
    val channel: UInt,
    val wiFiBand: UInt,
    val rssi: Int) {
  override fun toString(): String  = buildString {
    append("NetworkCommissioningClusterWiFiInterfaceScanResultStruct {\n")
    append("\tsecurity : $security\n")
    append("\tssid : $ssid\n")
    append("\tbssid : $bssid\n")
    append("\tchannel : $channel\n")
    append("\twiFiBand : $wiFiBand\n")
    append("\trssi : $rssi\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SECURITY), security)
      put(ContextSpecificTag(TAG_SSID), ssid)
      put(ContextSpecificTag(TAG_BSSID), bssid)
      put(ContextSpecificTag(TAG_CHANNEL), channel)
      put(ContextSpecificTag(TAG_WI_FI_BAND), wiFiBand)
      put(ContextSpecificTag(TAG_RSSI), rssi)
      endStructure()
    }
  }

  companion object {
    private const val TAG_SECURITY = 0
    private const val TAG_SSID = 1
    private const val TAG_BSSID = 2
    private const val TAG_CHANNEL = 3
    private const val TAG_WI_FI_BAND = 4
    private const val TAG_RSSI = 5

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : NetworkCommissioningClusterWiFiInterfaceScanResultStruct {
      tlvReader.enterStructure(tlvTag)
      val security = tlvReader.getUInt(ContextSpecificTag(TAG_SECURITY))
      val ssid = tlvReader.getByteArray(ContextSpecificTag(TAG_SSID))
      val bssid = tlvReader.getByteArray(ContextSpecificTag(TAG_BSSID))
      val channel = tlvReader.getUInt(ContextSpecificTag(TAG_CHANNEL))
      val wiFiBand = tlvReader.getUInt(ContextSpecificTag(TAG_WI_FI_BAND))
      val rssi = tlvReader.getInt(ContextSpecificTag(TAG_RSSI))
      
      tlvReader.exitContainer()

      return NetworkCommissioningClusterWiFiInterfaceScanResultStruct(security, ssid, bssid, channel, wiFiBand, rssi)
    }
  }
}
