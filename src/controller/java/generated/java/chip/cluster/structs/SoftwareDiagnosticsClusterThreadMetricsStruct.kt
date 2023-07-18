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

class SoftwareDiagnosticsClusterThreadMetricsStruct (
    val id: Long,
    val name: Optional<String>,
    val stackFreeCurrent: Optional<Long>,
    val stackFreeMinimum: Optional<Long>,
    val stackSize: Optional<Long>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("SoftwareDiagnosticsClusterThreadMetricsStruct {\n")
    builder.append("\tid : $id\n")
    builder.append("\tname : $name\n")
    builder.append("\tstackFreeCurrent : $stackFreeCurrent\n")
    builder.append("\tstackFreeMinimum : $stackFreeMinimum\n")
    builder.append("\tstackSize : $stackSize\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(0), id)
    if (name.isPresent) {
      val opt_name = name.get()
      tlvWriter.put(ContextSpecificTag(1), opt_name)
    }
    if (stackFreeCurrent.isPresent) {
      val opt_stackFreeCurrent = stackFreeCurrent.get()
      tlvWriter.put(ContextSpecificTag(2), opt_stackFreeCurrent)
    }
    if (stackFreeMinimum.isPresent) {
      val opt_stackFreeMinimum = stackFreeMinimum.get()
      tlvWriter.put(ContextSpecificTag(3), opt_stackFreeMinimum)
    }
    if (stackSize.isPresent) {
      val opt_stackSize = stackSize.get()
      tlvWriter.put(ContextSpecificTag(4), opt_stackSize)
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : SoftwareDiagnosticsClusterThreadMetricsStruct {
      tlvReader.enterStructure(tag)
      val id: Long = tlvReader.getLong(ContextSpecificTag(0))
      val name: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val stackFreeCurrent: Optional<Long> = try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val stackFreeMinimum: Optional<Long> = try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(3)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val stackSize: Optional<Long> = try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(4)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return SoftwareDiagnosticsClusterThreadMetricsStruct(id, name, stackFreeCurrent, stackFreeMinimum, stackSize)
    }
  }
}
