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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ScenesManagementClusterExtensionFieldSet(
  val clusterID: ULong,
  val attributeValueList: List<ScenesManagementClusterAttributeValuePairStruct>,
) {
  override fun toString(): String = buildString {
    append("ScenesManagementClusterExtensionFieldSet {\n")
    append("\tclusterID : $clusterID\n")
    append("\tattributeValueList : $attributeValueList\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CLUSTER_ID), clusterID)
      startArray(ContextSpecificTag(TAG_ATTRIBUTE_VALUE_LIST))
      for (item in attributeValueList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_CLUSTER_ID = 0
    private const val TAG_ATTRIBUTE_VALUE_LIST = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ScenesManagementClusterExtensionFieldSet {
      tlvReader.enterStructure(tlvTag)
      val clusterID = tlvReader.getULong(ContextSpecificTag(TAG_CLUSTER_ID))
      val attributeValueList =
        buildList<ScenesManagementClusterAttributeValuePairStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_ATTRIBUTE_VALUE_LIST))
          while (!tlvReader.isEndOfContainer()) {
            add(ScenesManagementClusterAttributeValuePairStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return ScenesManagementClusterExtensionFieldSet(clusterID, attributeValueList)
    }
  }
}
