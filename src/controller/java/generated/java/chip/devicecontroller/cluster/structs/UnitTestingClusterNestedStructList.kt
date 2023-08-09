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
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class UnitTestingClusterNestedStructList(
  val a: Int,
  val b: Boolean,
  val c: UnitTestingClusterSimpleStruct,
  val d: List<UnitTestingClusterSimpleStruct>,
  val e: List<Long>,
  val f: List<ByteArray>,
  val g: List<Int>
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

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_A), a)
      put(ContextSpecificTag(TAG_B), b)
      c.toTlv(ContextSpecificTag(TAG_C), this)
      startList(ContextSpecificTag(TAG_D))
      for (item in d.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endList()
      startList(ContextSpecificTag(TAG_E))
      for (item in e.iterator()) {
        put(AnonymousTag, item)
      }
      endList()
      startList(ContextSpecificTag(TAG_F))
      for (item in f.iterator()) {
        put(AnonymousTag, item)
      }
      endList()
      startList(ContextSpecificTag(TAG_G))
      for (item in g.iterator()) {
        put(AnonymousTag, item)
      }
      endList()
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

    fun fromTlv(tag: Tag, tlvReader: TlvReader): UnitTestingClusterNestedStructList {
      tlvReader.enterStructure(tag)
      val a = tlvReader.getInt(ContextSpecificTag(TAG_A))
      val b = tlvReader.getBoolean(ContextSpecificTag(TAG_B))
      val c = UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_C), tlvReader)
      val d =
        buildList<UnitTestingClusterSimpleStruct> {
          tlvReader.enterList(ContextSpecificTag(TAG_D))
          while (!tlvReader.isEndOfContainer()) {
            add(UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val e =
        buildList<Long> {
          tlvReader.enterList(ContextSpecificTag(TAG_E))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getLong(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val f =
        buildList<ByteArray> {
          tlvReader.enterList(ContextSpecificTag(TAG_F))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getByteArray(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val g =
        buildList<Int> {
          tlvReader.enterList(ContextSpecificTag(TAG_G))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return UnitTestingClusterNestedStructList(a, b, c, d, e, f, g)
    }
  }
}
