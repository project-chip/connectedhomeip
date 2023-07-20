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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class SoftwareDiagnosticsClusterSoftwareFaultEvent (
    val id: Long,
    val name: Optional<String>,
    val faultRecording: Optional<ByteArray>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("SoftwareDiagnosticsClusterSoftwareFaultEvent {\n")
    builder.append("\tid : $id\n")
    builder.append("\tname : $name\n")
    builder.append("\tfaultRecording : $faultRecording\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_ID), id)
    if (name.isPresent) {
      val optname = name.get()
      tlvWriter.put(ContextSpecificTag(TAG_NAME), optname)
    }
    if (faultRecording.isPresent) {
      val optfaultRecording = faultRecording.get()
      tlvWriter.put(ContextSpecificTag(TAG_FAULT_RECORDING), optfaultRecording)
    }
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_ID = 0
    private const val TAG_NAME = 1
    private const val TAG_FAULT_RECORDING = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : SoftwareDiagnosticsClusterSoftwareFaultEvent {
      tlvReader.enterStructure(tag)
      val id: Long = tlvReader.getLong(ContextSpecificTag(TAG_ID))
      val name: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NAME)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val faultRecording: Optional<ByteArray> = try {
      Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_FAULT_RECORDING)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return SoftwareDiagnosticsClusterSoftwareFaultEvent(id, name, faultRecording)
    }
  }
}
