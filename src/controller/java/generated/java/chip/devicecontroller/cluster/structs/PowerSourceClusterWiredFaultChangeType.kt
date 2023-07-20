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

class PowerSourceClusterWiredFaultChangeType (
    val current: List<Int>,
    val previous: List<Int>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("PowerSourceClusterWiredFaultChangeType {\n")
    builder.append("\tcurrent : $current\n")
    builder.append("\tprevious : $previous\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.startList(ContextSpecificTag(TAG_CURRENT))
      val itercurrent = current.iterator()
      while(itercurrent.hasNext()) {
        val next = itercurrent.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.startList(ContextSpecificTag(TAG_PREVIOUS))
      val iterprevious = previous.iterator()
      while(iterprevious.hasNext()) {
        val next = iterprevious.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_CURRENT = 0
    private const val TAG_PREVIOUS = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : PowerSourceClusterWiredFaultChangeType {
      tlvReader.enterStructure(tag)
      val current: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_CURRENT))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      val previous: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_PREVIOUS))
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

      return PowerSourceClusterWiredFaultChangeType(current, previous)
    }
  }
}
