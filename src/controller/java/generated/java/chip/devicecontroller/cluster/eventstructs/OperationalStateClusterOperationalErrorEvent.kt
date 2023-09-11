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

class OperationalStateClusterOperationalErrorEvent(
  val errorState: chip.devicecontroller.cluster.structs.OperationalStateClusterErrorStateStruct
) {
  override fun toString(): String = buildString {
    append("OperationalStateClusterOperationalErrorEvent {\n")
    append("\terrorState : $errorState\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      errorState.toTlv(ContextSpecificTag(TAG_ERROR_STATE), this)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ERROR_STATE = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): OperationalStateClusterOperationalErrorEvent {
      tlvReader.enterStructure(tag)
      val errorState =
        chip.devicecontroller.cluster.structs.OperationalStateClusterErrorStateStruct.fromTlv(
          ContextSpecificTag(TAG_ERROR_STATE),
          tlvReader
        )

      tlvReader.exitContainer()

      return OperationalStateClusterOperationalErrorEvent(errorState)
    }
  }
}
