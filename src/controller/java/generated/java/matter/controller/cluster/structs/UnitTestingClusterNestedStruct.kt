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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class UnitTestingClusterNestedStruct(
  val a: UByte,
  val b: Boolean,
  val c: UnitTestingClusterSimpleStruct,
  val d: Optional<UnitTestingClusterTestGlobalStruct>,
) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterNestedStruct {\n")
    append("\ta : $a\n")
    append("\tb : $b\n")
    append("\tc : $c\n")
    append("\td : $d\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_A), a)
      put(ContextSpecificTag(TAG_B), b)
      c.toTlv(ContextSpecificTag(TAG_C), this)
      if (d.isPresent) {
        val optd = d.get()
        optd.toTlv(ContextSpecificTag(TAG_D), this)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_A = 0
    private const val TAG_B = 1
    private const val TAG_C = 2
    private const val TAG_D = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): UnitTestingClusterNestedStruct {
      tlvReader.enterStructure(tlvTag)
      val a = tlvReader.getUByte(ContextSpecificTag(TAG_A))
      val b = tlvReader.getBoolean(ContextSpecificTag(TAG_B))
      val c = UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_C), tlvReader)
      val d =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_D))) {
          Optional.of(
            UnitTestingClusterTestGlobalStruct.fromTlv(ContextSpecificTag(TAG_D), tlvReader)
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return UnitTestingClusterNestedStruct(a, b, c, d)
    }
  }
}
