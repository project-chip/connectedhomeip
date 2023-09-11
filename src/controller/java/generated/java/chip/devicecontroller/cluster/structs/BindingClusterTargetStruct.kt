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
import java.util.Optional

class BindingClusterTargetStruct(
  val node: Optional<Long>,
  val group: Optional<Int>,
  val endpoint: Optional<Int>,
  val cluster: Optional<Long>,
  val fabricIndex: Int
) {
  override fun toString(): String = buildString {
    append("BindingClusterTargetStruct {\n")
    append("\tnode : $node\n")
    append("\tgroup : $group\n")
    append("\tendpoint : $endpoint\n")
    append("\tcluster : $cluster\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      if (node.isPresent) {
        val optnode = node.get()
        put(ContextSpecificTag(TAG_NODE), optnode)
      }
      if (group.isPresent) {
        val optgroup = group.get()
        put(ContextSpecificTag(TAG_GROUP), optgroup)
      }
      if (endpoint.isPresent) {
        val optendpoint = endpoint.get()
        put(ContextSpecificTag(TAG_ENDPOINT), optendpoint)
      }
      if (cluster.isPresent) {
        val optcluster = cluster.get()
        put(ContextSpecificTag(TAG_CLUSTER), optcluster)
      }
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_NODE = 1
    private const val TAG_GROUP = 2
    private const val TAG_ENDPOINT = 3
    private const val TAG_CLUSTER = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tag: Tag, tlvReader: TlvReader): BindingClusterTargetStruct {
      tlvReader.enterStructure(tag)
      val node =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NODE))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_NODE)))
        } else {
          Optional.empty()
        }
      val group =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_GROUP))) {
          Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_GROUP)))
        } else {
          Optional.empty()
        }
      val endpoint =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENDPOINT))) {
          Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_ENDPOINT)))
        } else {
          Optional.empty()
        }
      val cluster =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CLUSTER))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_CLUSTER)))
        } else {
          Optional.empty()
        }
      val fabricIndex = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return BindingClusterTargetStruct(node, group, endpoint, cluster, fabricIndex)
    }
  }
}
