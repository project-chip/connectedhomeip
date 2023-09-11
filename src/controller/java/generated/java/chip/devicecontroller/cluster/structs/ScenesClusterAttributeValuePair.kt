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

class ScenesClusterAttributeValuePair(val attributeID: Long, val attributeValue: Long) {
  override fun toString(): String = buildString {
    append("ScenesClusterAttributeValuePair {\n")
    append("\tattributeID : $attributeID\n")
    append("\tattributeValue : $attributeValue\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ATTRIBUTE_I_D), attributeID)
      put(ContextSpecificTag(TAG_ATTRIBUTE_VALUE), attributeValue)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ATTRIBUTE_I_D = 0
    private const val TAG_ATTRIBUTE_VALUE = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ScenesClusterAttributeValuePair {
      tlvReader.enterStructure(tag)
      val attributeID = tlvReader.getLong(ContextSpecificTag(TAG_ATTRIBUTE_I_D))
      val attributeValue = tlvReader.getLong(ContextSpecificTag(TAG_ATTRIBUTE_VALUE))

      tlvReader.exitContainer()

      return ScenesClusterAttributeValuePair(attributeID, attributeValue)
    }
  }
}
