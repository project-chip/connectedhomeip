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

class TimeSynchronizationClusterTimeZoneStruct (
    val offset: Long,
    val validAt: Long,
    val name: Optional<String>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("TimeSynchronizationClusterTimeZoneStruct {\n")
    builder.append("\toffset : $offset\n")
    builder.append("\tvalidAt : $validAt\n")
    builder.append("\tname : $name\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), offset)
    tlvWriter.put(ContextSpecificTag(1), validAt)
    if (name.isPresent) {
      val optname = name.get()
      tlvWriter.put(ContextSpecificTag(2), optname)
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterTimeZoneStruct {
      tlvReader.enterStructure(tag)
      val offset: Long = tlvReader.getLong(ContextSpecificTag(0))
      val validAt: Long = tlvReader.getLong(ContextSpecificTag(1))
      val name: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return TimeSynchronizationClusterTimeZoneStruct(offset, validAt, name)
    }
  }
}
