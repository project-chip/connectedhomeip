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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class UnitTestingClusterSimpleStruct (
    val a: Int,
    val b: Boolean,
    val c: Int,
    val d: ByteArray,
    val e: String,
    val f: Int,
    val g: Float,
    val h: Double) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("UnitTestingClusterSimpleStruct {\n")
    builder.append("\ta : $a\n")
    builder.append("\tb : $b\n")
    builder.append("\tc : $c\n")
    builder.append("\td : $d\n")
    builder.append("\te : $e\n")
    builder.append("\tf : $f\n")
    builder.append("\tg : $g\n")
    builder.append("\th : $h\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), a)
    tlvWriter.put(ContextSpecificTag(1), b)
    tlvWriter.put(ContextSpecificTag(2), c)
    tlvWriter.put(ContextSpecificTag(3), d)
    tlvWriter.put(ContextSpecificTag(4), e)
    tlvWriter.put(ContextSpecificTag(5), f)
    tlvWriter.put(ContextSpecificTag(6), g)
    tlvWriter.put(ContextSpecificTag(7), h)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterSimpleStruct {
      tlvReader.enterStructure(tag)
      val a: Int = tlvReader.getInt(ContextSpecificTag(0))
      val b: Boolean = tlvReader.getBoolean(ContextSpecificTag(1))
      val c: Int = tlvReader.getInt(ContextSpecificTag(2))
      val d: ByteArray = tlvReader.getByteArray(ContextSpecificTag(3))
      val e: String = tlvReader.getString(ContextSpecificTag(4))
      val f: Int = tlvReader.getInt(ContextSpecificTag(5))
      val g: Float = tlvReader.getFloat(ContextSpecificTag(6))
      val h: Double = tlvReader.getDouble(ContextSpecificTag(7))
      
      tlvReader.exitContainer()

      return UnitTestingClusterSimpleStruct(a, b, c, d, e, f, g, h)
    }
  }
}
