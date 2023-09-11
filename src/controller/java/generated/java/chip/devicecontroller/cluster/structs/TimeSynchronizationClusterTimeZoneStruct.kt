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
import java.util.Optional

class TimeSynchronizationClusterTimeZoneStruct(
  val offset: Long,
  val validAt: Long,
  val name: Optional<String>
) {
  override fun toString(): String = buildString {
    append("TimeSynchronizationClusterTimeZoneStruct {\n")
    append("\toffset : $offset\n")
    append("\tvalidAt : $validAt\n")
    append("\tname : $name\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
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

    fun fromTlv(tag: Tag, tlvReader: TlvReader): TimeSynchronizationClusterTimeZoneStruct {
      tlvReader.enterStructure(tag)
      val offset = tlvReader.getLong(ContextSpecificTag(TAG_OFFSET))
      val validAt = tlvReader.getLong(ContextSpecificTag(TAG_VALID_AT))
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
