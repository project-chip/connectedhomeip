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

class UnitTestingClusterSimpleStruct(
  val a: Int,
  val b: Boolean,
  val c: Int,
  val d: ByteArray,
  val e: String,
  val f: Int,
  val g: Float,
  val h: Double
) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterSimpleStruct {\n")
    append("\ta : $a\n")
    append("\tb : $b\n")
    append("\tc : $c\n")
    append("\td : $d\n")
    append("\te : $e\n")
    append("\tf : $f\n")
    append("\tg : $g\n")
    append("\th : $h\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_A), a)
      put(ContextSpecificTag(TAG_B), b)
      put(ContextSpecificTag(TAG_C), c)
      put(ContextSpecificTag(TAG_D), d)
      put(ContextSpecificTag(TAG_E), e)
      put(ContextSpecificTag(TAG_F), f)
      put(ContextSpecificTag(TAG_G), g)
      put(ContextSpecificTag(TAG_H), h)
      endStructure()
    }
  }

  companion object {
    private const val TAG_A = 0
    private const val TAG_B = 1
    private const val TAG_C = 2
    private const val TAG_D = 3
    private const val TAG_E = 4
    private const val TAG_F = 5
    private const val TAG_G = 6
    private const val TAG_H = 7

    fun fromTlv(tag: Tag, tlvReader: TlvReader): UnitTestingClusterSimpleStruct {
      tlvReader.enterStructure(tag)
      val a = tlvReader.getInt(ContextSpecificTag(TAG_A))
      val b = tlvReader.getBoolean(ContextSpecificTag(TAG_B))
      val c = tlvReader.getInt(ContextSpecificTag(TAG_C))
      val d = tlvReader.getByteArray(ContextSpecificTag(TAG_D))
      val e = tlvReader.getString(ContextSpecificTag(TAG_E))
      val f = tlvReader.getInt(ContextSpecificTag(TAG_F))
      val g = tlvReader.getFloat(ContextSpecificTag(TAG_G))
      val h = tlvReader.getDouble(ContextSpecificTag(TAG_H))

      tlvReader.exitContainer()

      return UnitTestingClusterSimpleStruct(a, b, c, d, e, f, g, h)
    }
  }
}
