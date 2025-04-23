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
package matter.controller.cluster.structs

import matter.controller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class UnitTestingClusterNestedStructList(
  val a: UByte,
  val b: Boolean,
  val c: UnitTestingClusterSimpleStruct,
  val d: List<UnitTestingClusterSimpleStruct>,
  val e: List<UInt>,
  val f: List<ByteArray>,
  val g: List<UByte>,
) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterNestedStructList {\n")
    append("\ta : $a\n")
    append("\tb : $b\n")
    append("\tc : $c\n")
    append("\td : $d\n")
    append("\te : $e\n")
    append("\tf : $f\n")
    append("\tg : $g\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_A), a)
      put(ContextSpecificTag(TAG_B), b)
      c.toTlv(ContextSpecificTag(TAG_C), this)
      startArray(ContextSpecificTag(TAG_D))
      for (item in d.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_E))
      for (item in e.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_F))
      for (item in f.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_G))
      for (item in g.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): UnitTestingClusterNestedStructList {
      tlvReader.enterStructure(tlvTag)
      val a = tlvReader.getUByte(ContextSpecificTag(TAG_A))
      val b = tlvReader.getBoolean(ContextSpecificTag(TAG_B))
      val c = UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_C), tlvReader)
      val d =
        buildList<UnitTestingClusterSimpleStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_D))
          while (!tlvReader.isEndOfContainer()) {
            add(UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val e =
        buildList<UInt> {
          tlvReader.enterArray(ContextSpecificTag(TAG_E))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val f =
        buildList<ByteArray> {
          tlvReader.enterArray(ContextSpecificTag(TAG_F))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getByteArray(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val g =
        buildList<UByte> {
          tlvReader.enterArray(ContextSpecificTag(TAG_G))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUByte(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return UnitTestingClusterNestedStructList(a, b, c, d, e, f, g)
    }
  }
}
