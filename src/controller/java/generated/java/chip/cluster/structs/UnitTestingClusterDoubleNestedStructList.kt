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
import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class UnitTestingClusterDoubleNestedStructList (
    val a: List<UnitTestingClusterNestedStructList>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("UnitTestingClusterDoubleNestedStructList {\n")
    builder.append("\ta : $a\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.startList(ContextSpecificTag(0))
      val iter_a = a.iterator()
      while(iter_a.hasNext()) {
        val next = iter_a.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterDoubleNestedStructList {
      tlvReader.enterStructure(tag)
      val a: List<UnitTestingClusterNestedStructList> = mutableListOf<UnitTestingClusterNestedStructList>().apply {
      tlvReader.enterList(ContextSpecificTag(0))
      while(true) {
        try {
          this.add(UnitTestingClusterNestedStructList.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return UnitTestingClusterDoubleNestedStructList(a)
    }
  }
}
