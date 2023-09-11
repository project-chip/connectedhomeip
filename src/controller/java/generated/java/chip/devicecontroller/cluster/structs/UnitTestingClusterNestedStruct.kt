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

class UnitTestingClusterNestedStruct(
  val a: Int,
  val b: Boolean,
  val c: UnitTestingClusterSimpleStruct
) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterNestedStruct {\n")
    append("\ta : $a\n")
    append("\tb : $b\n")
    append("\tc : $c\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_A), a)
      put(ContextSpecificTag(TAG_B), b)
      c.toTlv(ContextSpecificTag(TAG_C), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_A = 0
    private const val TAG_B = 1
    private const val TAG_C = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader): UnitTestingClusterNestedStruct {
      tlvReader.enterStructure(tag)
      val a = tlvReader.getInt(ContextSpecificTag(TAG_A))
      val b = tlvReader.getBoolean(ContextSpecificTag(TAG_B))
      val c = UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_C), tlvReader)

      tlvReader.exitContainer()

      return UnitTestingClusterNestedStruct(a, b, c)
    }
  }
}
