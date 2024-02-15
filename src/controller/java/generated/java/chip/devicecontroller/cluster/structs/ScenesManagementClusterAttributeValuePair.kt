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

class ScenesManagementClusterAttributeValuePair(val attributeID: ULong, val attributeValue: ULong) {
  override fun toString(): String = buildString {
    append("ScenesManagementClusterAttributeValuePair {\n")
    append("\tattributeID : $attributeID\n")
    append("\tattributeValue : $attributeValue\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ATTRIBUTE_I_D), attributeID)
      put(ContextSpecificTag(TAG_ATTRIBUTE_VALUE), attributeValue)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ATTRIBUTE_I_D = 0
    private const val TAG_ATTRIBUTE_VALUE = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ScenesManagementClusterAttributeValuePair {
      tlvReader.enterStructure(tlvTag)
      val attributeID = tlvReader.getULong(ContextSpecificTag(TAG_ATTRIBUTE_I_D))
      val attributeValue = tlvReader.getULong(ContextSpecificTag(TAG_ATTRIBUTE_VALUE))

      tlvReader.exitContainer()

      return ScenesManagementClusterAttributeValuePair(attributeID, attributeValue)
    }
  }
}
