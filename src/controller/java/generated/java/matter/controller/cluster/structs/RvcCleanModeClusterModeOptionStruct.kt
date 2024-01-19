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
package matter.controller.cluster.structs

import matter.controller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class RvcCleanModeClusterModeOptionStruct(
  val label: String,
  val mode: UByte,
  val modeTags: List<RvcCleanModeClusterModeTagStruct>
) {
  override fun toString(): String = buildString {
    append("RvcCleanModeClusterModeOptionStruct {\n")
    append("\tlabel : $label\n")
    append("\tmode : $mode\n")
    append("\tmodeTags : $modeTags\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LABEL), label)
      put(ContextSpecificTag(TAG_MODE), mode)
      startArray(ContextSpecificTag(TAG_MODE_TAGS))
      for (item in modeTags.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_LABEL = 0
    private const val TAG_MODE = 1
    private const val TAG_MODE_TAGS = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): RvcCleanModeClusterModeOptionStruct {
      tlvReader.enterStructure(tlvTag)
      val label = tlvReader.getString(ContextSpecificTag(TAG_LABEL))
      val mode = tlvReader.getUByte(ContextSpecificTag(TAG_MODE))
      val modeTags =
        buildList<RvcCleanModeClusterModeTagStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_MODE_TAGS))
          while (!tlvReader.isEndOfContainer()) {
            add(RvcCleanModeClusterModeTagStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return RvcCleanModeClusterModeOptionStruct(label, mode, modeTags)
    }
  }
}
