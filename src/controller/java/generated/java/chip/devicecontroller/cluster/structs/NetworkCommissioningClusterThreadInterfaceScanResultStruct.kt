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

class NetworkCommissioningClusterThreadInterfaceScanResultStruct(
  val panId: UInt,
  val extendedPanId: ULong,
  val networkName: String,
  val channel: UInt,
  val version: UInt,
  val extendedAddress: ByteArray,
  val rssi: Int,
  val lqi: UInt
) {
  override fun toString(): String = buildString {
    append("NetworkCommissioningClusterThreadInterfaceScanResultStruct {\n")
    append("\tpanId : $panId\n")
    append("\textendedPanId : $extendedPanId\n")
    append("\tnetworkName : $networkName\n")
    append("\tchannel : $channel\n")
    append("\tversion : $version\n")
    append("\textendedAddress : $extendedAddress\n")
    append("\trssi : $rssi\n")
    append("\tlqi : $lqi\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PAN_ID), panId)
      put(ContextSpecificTag(TAG_EXTENDED_PAN_ID), extendedPanId)
      put(ContextSpecificTag(TAG_NETWORK_NAME), networkName)
      put(ContextSpecificTag(TAG_CHANNEL), channel)
      put(ContextSpecificTag(TAG_VERSION), version)
      put(ContextSpecificTag(TAG_EXTENDED_ADDRESS), extendedAddress)
      put(ContextSpecificTag(TAG_RSSI), rssi)
      put(ContextSpecificTag(TAG_LQI), lqi)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PAN_ID = 0
    private const val TAG_EXTENDED_PAN_ID = 1
    private const val TAG_NETWORK_NAME = 2
    private const val TAG_CHANNEL = 3
    private const val TAG_VERSION = 4
    private const val TAG_EXTENDED_ADDRESS = 5
    private const val TAG_RSSI = 6
    private const val TAG_LQI = 7

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): NetworkCommissioningClusterThreadInterfaceScanResultStruct {
      tlvReader.enterStructure(tlvTag)
      val panId = tlvReader.getUInt(ContextSpecificTag(TAG_PAN_ID))
      val extendedPanId = tlvReader.getULong(ContextSpecificTag(TAG_EXTENDED_PAN_ID))
      val networkName = tlvReader.getString(ContextSpecificTag(TAG_NETWORK_NAME))
      val channel = tlvReader.getUInt(ContextSpecificTag(TAG_CHANNEL))
      val version = tlvReader.getUInt(ContextSpecificTag(TAG_VERSION))
      val extendedAddress = tlvReader.getByteArray(ContextSpecificTag(TAG_EXTENDED_ADDRESS))
      val rssi = tlvReader.getInt(ContextSpecificTag(TAG_RSSI))
      val lqi = tlvReader.getUInt(ContextSpecificTag(TAG_LQI))

      tlvReader.exitContainer()

      return NetworkCommissioningClusterThreadInterfaceScanResultStruct(
        panId,
        extendedPanId,
        networkName,
        channel,
        version,
        extendedAddress,
        rssi,
        lqi
      )
    }
  }
}
