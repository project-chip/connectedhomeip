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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class TimeSynchronizationClusterTimeZoneStatusEvent (
    val offset: Long,
    val name: Optional<String>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("TimeSynchronizationClusterTimeZoneStatusEvent {\n")
    builder.append("\toffset : $offset\n")
    builder.append("\tname : $name\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), offset)
    if (name.isPresent) {
      val opt_name = name.get()
      tlvWriter.put(ContextSpecificTag(1), opt_name)
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterTimeZoneStatusEvent {
      tlvReader.enterStructure(tag)
      val offset: Long = tlvReader.getLong(ContextSpecificTag(0))
      val name: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return TimeSynchronizationClusterTimeZoneStatusEvent(offset, name)
    }
  }
}
