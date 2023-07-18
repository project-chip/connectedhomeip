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

class RvcRunModeClusterModeOptionStruct (
    val label: String,
    val mode: Int,
    val modeTags: List<RvcRunModeClusterModeTagStruct>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("RvcRunModeClusterModeOptionStruct {\n")
    builder.append("\tlabel : $label\n")
    builder.append("\tmode : $mode\n")
    builder.append("\tmodeTags : $modeTags\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), label)
    tlvWriter.put(ContextSpecificTag(1), mode)
    tlvWriter.startList(ContextSpecificTag(2))
      val iter_modeTags = modeTags.iterator()
      while(iter_modeTags.hasNext()) {
        val next = iter_modeTags.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : RvcRunModeClusterModeOptionStruct {
      tlvReader.enterStructure(tag)
      val label: String = tlvReader.getString(ContextSpecificTag(0))
      val mode: Int = tlvReader.getInt(ContextSpecificTag(1))
      val modeTags: List<RvcRunModeClusterModeTagStruct> = mutableListOf<RvcRunModeClusterModeTagStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(2))
      while(true) {
        try {
          this.add(RvcRunModeClusterModeTagStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return RvcRunModeClusterModeOptionStruct(label, mode, modeTags)
    }
  }
}
