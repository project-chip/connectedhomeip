/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class ScenesClusterExtensionFieldSet(
  val clusterID: Long,
  val attributeValueList: List<ScenesClusterAttributeValuePair>
) {
  override fun toString(): String = buildString {
    append("ScenesClusterExtensionFieldSet {\n")
    append("\tclusterID : $clusterID\n")
    append("\tattributeValueList : $attributeValueList\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_CLUSTER_I_D), clusterID)
      startList(ContextSpecificTag(TAG_ATTRIBUTE_VALUE_LIST))
      for (item in attributeValueList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endList()
      endStructure()
    }
  }

  companion object {
    private const val TAG_CLUSTER_I_D = 0
    private const val TAG_ATTRIBUTE_VALUE_LIST = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ScenesClusterExtensionFieldSet {
      tlvReader.enterStructure(tag)
      val clusterID = tlvReader.getLong(ContextSpecificTag(TAG_CLUSTER_I_D))
      val attributeValueList =
        buildList<ScenesClusterAttributeValuePair> {
          tlvReader.enterList(ContextSpecificTag(TAG_ATTRIBUTE_VALUE_LIST))
          while (!tlvReader.isEndOfContainer()) {
            add(ScenesClusterAttributeValuePair.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return ScenesClusterExtensionFieldSet(clusterID, attributeValueList)
    }
  }
}
