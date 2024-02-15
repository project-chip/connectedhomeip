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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class TimeSynchronizationClusterDSTOffsetStruct(
  val offset: Int,
  val validStarting: ULong,
  val validUntil: ULong?
) {
  override fun toString(): String = buildString {
    append("TimeSynchronizationClusterDSTOffsetStruct {\n")
    append("\toffset : $offset\n")
    append("\tvalidStarting : $validStarting\n")
    append("\tvalidUntil : $validUntil\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_OFFSET), offset)
      put(ContextSpecificTag(TAG_VALID_STARTING), validStarting)
      if (validUntil != null) {
        put(ContextSpecificTag(TAG_VALID_UNTIL), validUntil)
      } else {
        putNull(ContextSpecificTag(TAG_VALID_UNTIL))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_OFFSET = 0
    private const val TAG_VALID_STARTING = 1
    private const val TAG_VALID_UNTIL = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): TimeSynchronizationClusterDSTOffsetStruct {
      tlvReader.enterStructure(tlvTag)
      val offset = tlvReader.getInt(ContextSpecificTag(TAG_OFFSET))
      val validStarting = tlvReader.getULong(ContextSpecificTag(TAG_VALID_STARTING))
      val validUntil =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_VALID_UNTIL))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_VALID_UNTIL))
          null
        }

      tlvReader.exitContainer()

      return TimeSynchronizationClusterDSTOffsetStruct(offset, validStarting, validUntil)
    }
  }
}
