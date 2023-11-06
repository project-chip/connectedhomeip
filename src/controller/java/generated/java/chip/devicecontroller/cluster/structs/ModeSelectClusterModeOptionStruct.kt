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

class ModeSelectClusterModeOptionStruct(
  val label: String,
  val mode: UInt,
  val semanticTags: List<ModeSelectClusterSemanticTagStruct>
) {
  override fun toString(): String = buildString {
    append("ModeSelectClusterModeOptionStruct {\n")
    append("\tlabel : $label\n")
    append("\tmode : $mode\n")
    append("\tsemanticTags : $semanticTags\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LABEL), label)
      put(ContextSpecificTag(TAG_MODE), mode)
      startArray(ContextSpecificTag(TAG_SEMANTIC_TAGS))
      for (item in semanticTags.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_LABEL = 0
    private const val TAG_MODE = 1
    private const val TAG_SEMANTIC_TAGS = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ModeSelectClusterModeOptionStruct {
      tlvReader.enterStructure(tlvTag)
      val label = tlvReader.getString(ContextSpecificTag(TAG_LABEL))
      val mode = tlvReader.getUInt(ContextSpecificTag(TAG_MODE))
      val semanticTags =
        buildList<ModeSelectClusterSemanticTagStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_SEMANTIC_TAGS))
          while (!tlvReader.isEndOfContainer()) {
            add(ModeSelectClusterSemanticTagStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return ModeSelectClusterModeOptionStruct(label, mode, semanticTags)
    }
  }
}
