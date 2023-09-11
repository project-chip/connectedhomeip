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

class TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct(
  val nodeID: Long,
  val endpoint: Int
) {
  override fun toString(): String = buildString {
    append("TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct {\n")
    append("\tnodeID : $nodeID\n")
    append("\tendpoint : $endpoint\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_NODE_I_D), nodeID)
      put(ContextSpecificTag(TAG_ENDPOINT), endpoint)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE_I_D = 0
    private const val TAG_ENDPOINT = 1

    fun fromTlv(
      tag: Tag,
      tlvReader: TlvReader
    ): TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct {
      tlvReader.enterStructure(tag)
      val nodeID = tlvReader.getLong(ContextSpecificTag(TAG_NODE_I_D))
      val endpoint = tlvReader.getInt(ContextSpecificTag(TAG_ENDPOINT))

      tlvReader.exitContainer()

      return TimeSynchronizationClusterFabricScopedTrustedTimeSourceStruct(nodeID, endpoint)
    }
  }
}
