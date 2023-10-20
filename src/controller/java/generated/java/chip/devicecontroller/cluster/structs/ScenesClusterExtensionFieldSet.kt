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
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class ScenesClusterExtensionFieldSet (
    val clusterID: ULong,
    val attributeValueList: List<ScenesClusterAttributeValuePair>) {
  override fun toString(): String  = buildString {
    append("ScenesClusterExtensionFieldSet {\n")
    append("\tclusterID : $clusterID\n")
    append("\tattributeValueList : $attributeValueList\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CLUSTER_I_D), clusterID)
      startArray(ContextSpecificTag(TAG_ATTRIBUTE_VALUE_LIST))
      for (item in attributeValueList.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_CLUSTER_I_D = 0
    private const val TAG_ATTRIBUTE_VALUE_LIST = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ScenesClusterExtensionFieldSet {
      tlvReader.enterStructure(tlvTag)
      val clusterID = tlvReader.getULong(ContextSpecificTag(TAG_CLUSTER_I_D))
      val attributeValueList = buildList<ScenesClusterAttributeValuePair> {
      tlvReader.enterArray(ContextSpecificTag(TAG_ATTRIBUTE_VALUE_LIST))
      while(!tlvReader.isEndOfContainer()) {
        add(ScenesClusterAttributeValuePair.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return ScenesClusterExtensionFieldSet(clusterID, attributeValueList)
    }
  }
}
