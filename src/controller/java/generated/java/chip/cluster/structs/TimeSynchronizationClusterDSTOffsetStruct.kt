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

class TimeSynchronizationClusterDSTOffsetStruct (
    val offset: Long,
    val validStarting: Long,
    val validUntil: Long?) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("TimeSynchronizationClusterDSTOffsetStruct {\n")
    builder.append("\toffset : $offset\n")
    builder.append("\tvalidStarting : $validStarting\n")
    builder.append("\tvalidUntil : $validUntil\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), offset)
    tlvWriter.put(ContextSpecificTag(1), validStarting)
    if (validUntil == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      tlvWriter.put(ContextSpecificTag(2), validUntil)
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : TimeSynchronizationClusterDSTOffsetStruct {
      tlvReader.enterStructure(tag)
      val offset: Long = tlvReader.getLong(ContextSpecificTag(0))
      val validStarting: Long = tlvReader.getLong(ContextSpecificTag(1))
      val validUntil: Long? = try {
      tlvReader.getLong(ContextSpecificTag(2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
      
      tlvReader.exitContainer()

      return TimeSynchronizationClusterDSTOffsetStruct(offset, validStarting, validUntil)
    }
  }
}
