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

class TimeSynchronizationClusterTimeZoneStruct(
  val offset: Long,
  val validAt: ULong,
  val name: Optional<String>,
) {
  override fun toString(): String = buildString {
    append("TimeSynchronizationClusterTimeZoneStruct {\n")
    append("\toffset : $offset\n")
    append("\tvalidAt : $validAt\n")
    append("\tname : $name\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_OFFSET), offset)
      put(ContextSpecificTag(TAG_VALID_AT), validAt)
      if (name.isPresent) {
        val optname = name.get()
        put(ContextSpecificTag(TAG_NAME), optname)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_OFFSET = 0
    private const val TAG_VALID_AT = 1
    private const val TAG_NAME = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): TimeSynchronizationClusterTimeZoneStruct {
      tlvReader.enterStructure(tlvTag)
      val offset = tlvReader.getLong(ContextSpecificTag(TAG_OFFSET))
      val validAt = tlvReader.getULong(ContextSpecificTag(TAG_VALID_AT))
      val name =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NAME)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return TimeSynchronizationClusterTimeZoneStruct(offset, validAt, name)
    }
  }
}
