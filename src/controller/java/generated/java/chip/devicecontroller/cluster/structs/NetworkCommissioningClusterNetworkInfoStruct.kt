/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class NetworkCommissioningClusterNetworkInfoStruct(
  val networkID: ByteArray,
  val connected: Boolean
) {
  override fun toString(): String = buildString {
    append("NetworkCommissioningClusterNetworkInfoStruct {\n")
    append("\tnetworkID : $networkID\n")
    append("\tconnected : $connected\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_NETWORK_I_D), networkID)
      put(ContextSpecificTag(TAG_CONNECTED), connected)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NETWORK_I_D = 0
    private const val TAG_CONNECTED = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): NetworkCommissioningClusterNetworkInfoStruct {
      tlvReader.enterStructure(tag)
      val networkID = tlvReader.getByteArray(ContextSpecificTag(TAG_NETWORK_I_D))
      val connected = tlvReader.getBoolean(ContextSpecificTag(TAG_CONNECTED))

      tlvReader.exitContainer()

      return NetworkCommissioningClusterNetworkInfoStruct(networkID, connected)
    }
  }
}
