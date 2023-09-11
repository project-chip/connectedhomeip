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

class TimeSynchronizationClusterDSTOffsetStruct(
  val offset: Long,
  val validStarting: Long,
  val validUntil: Long?
) {
  override fun toString(): String = buildString {
    append("TimeSynchronizationClusterDSTOffsetStruct {\n")
    append("\toffset : $offset\n")
    append("\tvalidStarting : $validStarting\n")
    append("\tvalidUntil : $validUntil\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
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

    fun fromTlv(tag: Tag, tlvReader: TlvReader): TimeSynchronizationClusterDSTOffsetStruct {
      tlvReader.enterStructure(tag)
      val offset = tlvReader.getLong(ContextSpecificTag(TAG_OFFSET))
      val validStarting = tlvReader.getLong(ContextSpecificTag(TAG_VALID_STARTING))
      val validUntil =
        if (!tlvReader.isNull()) {
          tlvReader.getLong(ContextSpecificTag(TAG_VALID_UNTIL))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_VALID_UNTIL))
          null
        }

      tlvReader.exitContainer()

      return TimeSynchronizationClusterDSTOffsetStruct(offset, validStarting, validUntil)
    }
  }
}
