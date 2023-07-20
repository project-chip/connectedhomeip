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
    tlvWriter.put(ContextSpecificTag(TAG_PAN_ID), panId)
    tlvWriter.put(ContextSpecificTag(TAG_EXTENDED_PAN_ID), extendedPanId)
    tlvWriter.put(ContextSpecificTag(TAG_NETWORK_NAME), networkName)
    tlvWriter.put(ContextSpecificTag(TAG_CHANNEL), channel)
    tlvWriter.put(ContextSpecificTag(TAG_VERSION), version)
    tlvWriter.put(ContextSpecificTag(TAG_EXTENDED_ADDRESS), extendedAddress)
    tlvWriter.put(ContextSpecificTag(TAG_RSSI), rssi)
    tlvWriter.put(ContextSpecificTag(TAG_LQI), lqi)
    tlvWriter.endStructure()
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

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : NetworkCommissioningClusterThreadInterfaceScanResultStruct {
      tlvReader.enterStructure(tag)
      val panId: Int = tlvReader.getInt(ContextSpecificTag(TAG_PAN_ID))
      val extendedPanId: Long = tlvReader.getLong(ContextSpecificTag(TAG_EXTENDED_PAN_ID))
      val networkName: String = tlvReader.getString(ContextSpecificTag(TAG_NETWORK_NAME))
      val channel: Int = tlvReader.getInt(ContextSpecificTag(TAG_CHANNEL))
      val version: Int = tlvReader.getInt(ContextSpecificTag(TAG_VERSION))
      val extendedAddress: ByteArray = tlvReader.getByteArray(ContextSpecificTag(TAG_EXTENDED_ADDRESS))
      val rssi: Int = tlvReader.getInt(ContextSpecificTag(TAG_RSSI))
      val lqi: Int = tlvReader.getInt(ContextSpecificTag(TAG_LQI))
      
      tlvReader.exitContainer()

      return NetworkCommissioningClusterThreadInterfaceScanResultStruct(panId, extendedPanId, networkName, channel, version, extendedAddress, rssi, lqi)
    }
  }
}
