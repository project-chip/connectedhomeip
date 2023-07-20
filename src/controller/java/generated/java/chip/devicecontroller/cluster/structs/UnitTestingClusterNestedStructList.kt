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

class UnitTestingClusterNestedStructList (
    val a: Int,
    val b: Boolean,
    val c: UnitTestingClusterSimpleStruct,
    val d: List<UnitTestingClusterSimpleStruct>,
    val e: List<Long>,
    val f: List<ByteArray>,
    val g: List<Int>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("UnitTestingClusterNestedStructList {\n")
    builder.append("\ta : $a\n")
    builder.append("\tb : $b\n")
    builder.append("\tc : $c\n")
    builder.append("\td : $d\n")
    builder.append("\te : $e\n")
    builder.append("\tf : $f\n")
    builder.append("\tg : $g\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_A), a)
    tlvWriter.put(ContextSpecificTag(TAG_B), b)
    c.toTlv(ContextSpecificTag(TAG_C), tlvWriter)
    tlvWriter.startList(ContextSpecificTag(TAG_D))
      val iterd = d.iterator()
      while(iterd.hasNext()) {
        val next = iterd.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    tlvWriter.startList(ContextSpecificTag(TAG_E))
      val itere = e.iterator()
      while(itere.hasNext()) {
        val next = itere.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.startList(ContextSpecificTag(TAG_F))
      val iterf = f.iterator()
      while(iterf.hasNext()) {
        val next = iterf.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.startList(ContextSpecificTag(TAG_G))
      val iterg = g.iterator()
      while(iterg.hasNext()) {
        val next = iterg.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_A = 0
    private const val TAG_B = 1
    private const val TAG_C = 2
    private const val TAG_D = 3
    private const val TAG_E = 4
    private const val TAG_F = 5
    private const val TAG_G = 6

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterNestedStructList {
      tlvReader.enterStructure(tag)
      val a: Int = tlvReader.getInt(ContextSpecificTag(TAG_A))
      val b: Boolean = tlvReader.getBoolean(ContextSpecificTag(TAG_B))
      val c: UnitTestingClusterSimpleStruct = UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_C), tlvReader)
      val d: List<UnitTestingClusterSimpleStruct> = mutableListOf<UnitTestingClusterSimpleStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_D))
      while(true) {
        try {
          this.add(UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      val e: List<Long> = mutableListOf<Long>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_E))
      while(true) {
        try {
          this.add(tlvReader.getLong(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      val f: List<ByteArray> = mutableListOf<ByteArray>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_F))
      while(true) {
        try {
          this.add(tlvReader.getByteArray(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      val g: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_G))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return UnitTestingClusterNestedStructList(a, b, c, d, e, f, g)
    }
  }
}
