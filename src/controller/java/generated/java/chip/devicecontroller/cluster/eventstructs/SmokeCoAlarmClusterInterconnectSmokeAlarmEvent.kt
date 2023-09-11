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

class SmokeCoAlarmClusterInterconnectSmokeAlarmEvent(val alarmSeverityLevel: Int) {
  override fun toString(): String = buildString {
    append("SmokeCoAlarmClusterInterconnectSmokeAlarmEvent {\n")
    append("\talarmSeverityLevel : $alarmSeverityLevel\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ALARM_SEVERITY_LEVEL), alarmSeverityLevel)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ALARM_SEVERITY_LEVEL = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): SmokeCoAlarmClusterInterconnectSmokeAlarmEvent {
      tlvReader.enterStructure(tag)
      val alarmSeverityLevel = tlvReader.getInt(ContextSpecificTag(TAG_ALARM_SEVERITY_LEVEL))

      tlvReader.exitContainer()

      return SmokeCoAlarmClusterInterconnectSmokeAlarmEvent(alarmSeverityLevel)
    }
  }
}
