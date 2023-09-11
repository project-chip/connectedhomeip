/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import java.util.Optional

class OperationalStateClusterOperationalStateStruct(
  val operationalStateID: Int,
  val operationalStateLabel: Optional<String>
) {
  override fun toString(): String = buildString {
    append("OperationalStateClusterOperationalStateStruct {\n")
    append("\toperationalStateID : $operationalStateID\n")
    append("\toperationalStateLabel : $operationalStateLabel\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_OPERATIONAL_STATE_I_D), operationalStateID)
      if (operationalStateLabel.isPresent) {
        val optoperationalStateLabel = operationalStateLabel.get()
        put(ContextSpecificTag(TAG_OPERATIONAL_STATE_LABEL), optoperationalStateLabel)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_OPERATIONAL_STATE_I_D = 0
    private const val TAG_OPERATIONAL_STATE_LABEL = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): OperationalStateClusterOperationalStateStruct {
      tlvReader.enterStructure(tag)
      val operationalStateID = tlvReader.getInt(ContextSpecificTag(TAG_OPERATIONAL_STATE_I_D))
      val operationalStateLabel =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPERATIONAL_STATE_LABEL))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_OPERATIONAL_STATE_LABEL)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return OperationalStateClusterOperationalStateStruct(
        operationalStateID,
        operationalStateLabel
      )
    }
  }
}
