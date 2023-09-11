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

class LaundryWasherModeClusterModeOptionStruct(
  val label: String,
  val mode: Int,
  val modeTags: List<LaundryWasherModeClusterModeTagStruct>
) {
  override fun toString(): String = buildString {
    append("LaundryWasherModeClusterModeOptionStruct {\n")
    append("\tlabel : $label\n")
    append("\tmode : $mode\n")
    append("\tmodeTags : $modeTags\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_LABEL), label)
      put(ContextSpecificTag(TAG_MODE), mode)
      startList(ContextSpecificTag(TAG_MODE_TAGS))
      for (item in modeTags.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endList()
      endStructure()
    }
  }

  companion object {
    private const val TAG_LABEL = 0
    private const val TAG_MODE = 1
    private const val TAG_MODE_TAGS = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader): LaundryWasherModeClusterModeOptionStruct {
      tlvReader.enterStructure(tag)
      val label = tlvReader.getString(ContextSpecificTag(TAG_LABEL))
      val mode = tlvReader.getInt(ContextSpecificTag(TAG_MODE))
      val modeTags =
        buildList<LaundryWasherModeClusterModeTagStruct> {
          tlvReader.enterList(ContextSpecificTag(TAG_MODE_TAGS))
          while (!tlvReader.isEndOfContainer()) {
            add(LaundryWasherModeClusterModeTagStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return LaundryWasherModeClusterModeOptionStruct(label, mode, modeTags)
    }
  }
}
