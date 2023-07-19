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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class ScenesClusterExtensionFieldSet (
    val clusterID: Long,
    val attributeValueList: List<ScenesClusterAttributeValuePair>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("ScenesClusterExtensionFieldSet {\n")
    builder.append("\tclusterID : $clusterID\n")
    builder.append("\tattributeValueList : $attributeValueList\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), clusterID)
    tlvWriter.startList(ContextSpecificTag(1))
      val iterattributeValueList = attributeValueList.iterator()
      while(iterattributeValueList.hasNext()) {
        val next = iterattributeValueList.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : ScenesClusterExtensionFieldSet {
      tlvReader.enterStructure(tag)
      val clusterID: Long = tlvReader.getLong(ContextSpecificTag(0))
      val attributeValueList: List<ScenesClusterAttributeValuePair> = mutableListOf<ScenesClusterAttributeValuePair>().apply {
      tlvReader.enterList(ContextSpecificTag(1))
      while(true) {
        try {
          this.add(ScenesClusterAttributeValuePair.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return ScenesClusterExtensionFieldSet(clusterID, attributeValueList)
    }
  }
}
