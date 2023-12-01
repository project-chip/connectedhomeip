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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ChannelClusterPageTokenStruct(
  val limit: Optional<UInt>,
  val after: Optional<String>,
  val before: Optional<String>
) {
  override fun toString(): String = buildString {
    append("ChannelClusterPageTokenStruct {\n")
    append("\tlimit : $limit\n")
    append("\tafter : $after\n")
    append("\tbefore : $before\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (limit.isPresent) {
        val optlimit = limit.get()
        put(ContextSpecificTag(TAG_LIMIT), optlimit)
      }
      if (after.isPresent) {
        val optafter = after.get()
        put(ContextSpecificTag(TAG_AFTER), optafter)
      }
      if (before.isPresent) {
        val optbefore = before.get()
        put(ContextSpecificTag(TAG_BEFORE), optbefore)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_LIMIT = 0
    private const val TAG_AFTER = 1
    private const val TAG_BEFORE = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ChannelClusterPageTokenStruct {
      tlvReader.enterStructure(tlvTag)
      val limit =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_LIMIT))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_LIMIT)))
        } else {
          Optional.empty()
        }
      val after =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_AFTER))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_AFTER)))
        } else {
          Optional.empty()
        }
      val before =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_BEFORE))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_BEFORE)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ChannelClusterPageTokenStruct(limit, after, before)
    }
  }
}
