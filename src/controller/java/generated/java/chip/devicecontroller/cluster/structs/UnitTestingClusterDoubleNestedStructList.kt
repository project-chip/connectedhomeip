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

class UnitTestingClusterDoubleNestedStructList(val a: List<UnitTestingClusterNestedStructList>) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterDoubleNestedStructList {\n")
    append("\ta : $a\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      startList(ContextSpecificTag(TAG_A))
      for (item in a.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endList()
      endStructure()
    }
  }

  companion object {
    private const val TAG_A = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): UnitTestingClusterDoubleNestedStructList {
      tlvReader.enterStructure(tag)
      val a =
        buildList<UnitTestingClusterNestedStructList> {
          tlvReader.enterList(ContextSpecificTag(TAG_A))
          while (!tlvReader.isEndOfContainer()) {
            add(UnitTestingClusterNestedStructList.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return UnitTestingClusterDoubleNestedStructList(a)
    }
  }
}
