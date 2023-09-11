/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class PowerSourceClusterWiredFaultChangeType(val current: List<Int>, val previous: List<Int>) {
  override fun toString(): String = buildString {
    append("PowerSourceClusterWiredFaultChangeType {\n")
    append("\tcurrent : $current\n")
    append("\tprevious : $previous\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      startList(ContextSpecificTag(TAG_CURRENT))
      for (item in current.iterator()) {
        put(AnonymousTag, item)
      }
      endList()
      startList(ContextSpecificTag(TAG_PREVIOUS))
      for (item in previous.iterator()) {
        put(AnonymousTag, item)
      }
      endList()
      endStructure()
    }
  }

  companion object {
    private const val TAG_CURRENT = 0
    private const val TAG_PREVIOUS = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): PowerSourceClusterWiredFaultChangeType {
      tlvReader.enterStructure(tag)
      val current =
        buildList<Int> {
          tlvReader.enterList(ContextSpecificTag(TAG_CURRENT))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val previous =
        buildList<Int> {
          tlvReader.enterList(ContextSpecificTag(TAG_PREVIOUS))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return PowerSourceClusterWiredFaultChangeType(current, previous)
    }
  }
}
