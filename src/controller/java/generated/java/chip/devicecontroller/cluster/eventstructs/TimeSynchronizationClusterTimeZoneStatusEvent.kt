/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import java.util.Optional

class TimeSynchronizationClusterTimeZoneStatusEvent(val offset: Long, val name: Optional<String>) {
  override fun toString(): String = buildString {
    append("TimeSynchronizationClusterTimeZoneStatusEvent {\n")
    append("\toffset : $offset\n")
    append("\tname : $name\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_OFFSET), offset)
      if (name.isPresent) {
        val optname = name.get()
        put(ContextSpecificTag(TAG_NAME), optname)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_OFFSET = 0
    private const val TAG_NAME = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): TimeSynchronizationClusterTimeZoneStatusEvent {
      tlvReader.enterStructure(tag)
      val offset = tlvReader.getLong(ContextSpecificTag(TAG_OFFSET))
      val name =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NAME)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return TimeSynchronizationClusterTimeZoneStatusEvent(offset, name)
    }
  }
}
