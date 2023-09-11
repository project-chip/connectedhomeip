/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import java.util.Optional

class SoftwareDiagnosticsClusterSoftwareFaultEvent(
  val id: Long,
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

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
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

    fun fromTlv(tag: Tag, tlvReader: TlvReader): SoftwareDiagnosticsClusterSoftwareFaultEvent {
      tlvReader.enterStructure(tag)
      val id = tlvReader.getLong(ContextSpecificTag(TAG_ID))
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
