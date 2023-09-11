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

class TimeSynchronizationClusterTrustedTimeSourceStruct(
  val fabricIndex: Int,
  val nodeID: Long,
  val endpoint: Int
) {
  override fun toString(): String = buildString {
    append("TimeSynchronizationClusterTrustedTimeSourceStruct {\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("\tnodeID : $nodeID\n")
    append("\tendpoint : $endpoint\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      put(ContextSpecificTag(TAG_NODE_I_D), nodeID)
      put(ContextSpecificTag(TAG_ENDPOINT), endpoint)
      endStructure()
    }
  }

  companion object {
    private const val TAG_FABRIC_INDEX = 0
    private const val TAG_NODE_I_D = 1
    private const val TAG_ENDPOINT = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader): TimeSynchronizationClusterTrustedTimeSourceStruct {
      tlvReader.enterStructure(tag)
      val fabricIndex = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))
      val nodeID = tlvReader.getLong(ContextSpecificTag(TAG_NODE_I_D))
      val endpoint = tlvReader.getInt(ContextSpecificTag(TAG_ENDPOINT))

      tlvReader.exitContainer()

      return TimeSynchronizationClusterTrustedTimeSourceStruct(fabricIndex, nodeID, endpoint)
    }
  }
}
