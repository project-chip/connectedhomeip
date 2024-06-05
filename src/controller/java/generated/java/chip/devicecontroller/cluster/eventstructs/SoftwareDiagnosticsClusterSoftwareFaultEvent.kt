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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class SoftwareDiagnosticsClusterSoftwareFaultEvent(
  val id: ULong,
  val name: Optional<String>,
  val faultRecording: Optional<ByteArray>
) {
  override fun toString(): String = buildString {
    append("SoftwareDiagnosticsClusterSoftwareFaultEvent {\n")
    append("\tid : $id\n")
    append("\tname : $name\n")
    append("\tfaultRecording : $faultRecording\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ID), id)
      if (name.isPresent) {
        val optname = name.get()
        put(ContextSpecificTag(TAG_NAME), optname)
      }
      if (faultRecording.isPresent) {
        val optfaultRecording = faultRecording.get()
        put(ContextSpecificTag(TAG_FAULT_RECORDING), optfaultRecording)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ID = 0
    private const val TAG_NAME = 1
    private const val TAG_FAULT_RECORDING = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): SoftwareDiagnosticsClusterSoftwareFaultEvent {
      tlvReader.enterStructure(tlvTag)
      val id = tlvReader.getULong(ContextSpecificTag(TAG_ID))
      val name =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NAME)))
        } else {
          Optional.empty()
        }
      val faultRecording =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_FAULT_RECORDING))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_FAULT_RECORDING)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return SoftwareDiagnosticsClusterSoftwareFaultEvent(id, name, faultRecording)
    }
  }
}
