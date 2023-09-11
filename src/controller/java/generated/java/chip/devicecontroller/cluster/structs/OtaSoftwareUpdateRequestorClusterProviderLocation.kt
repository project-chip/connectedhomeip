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

class OtaSoftwareUpdateRequestorClusterProviderLocation(
  val providerNodeID: Long,
  val endpoint: Int,
  val fabricIndex: Int
) {
  override fun toString(): String = buildString {
    append("OtaSoftwareUpdateRequestorClusterProviderLocation {\n")
    append("\tproviderNodeID : $providerNodeID\n")
    append("\tendpoint : $endpoint\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_PROVIDER_NODE_I_D), providerNodeID)
      put(ContextSpecificTag(TAG_ENDPOINT), endpoint)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PROVIDER_NODE_I_D = 1
    private const val TAG_ENDPOINT = 2
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tag: Tag, tlvReader: TlvReader): OtaSoftwareUpdateRequestorClusterProviderLocation {
      tlvReader.enterStructure(tag)
      val providerNodeID = tlvReader.getLong(ContextSpecificTag(TAG_PROVIDER_NODE_I_D))
      val endpoint = tlvReader.getInt(ContextSpecificTag(TAG_ENDPOINT))
      val fabricIndex = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return OtaSoftwareUpdateRequestorClusterProviderLocation(
        providerNodeID,
        endpoint,
        fabricIndex
      )
    }
  }
}
