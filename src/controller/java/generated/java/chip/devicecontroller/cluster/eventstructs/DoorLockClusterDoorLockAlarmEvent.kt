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

class DoorLockClusterDoorLockAlarmEvent(val alarmCode: Int) {
  override fun toString(): String = buildString {
    append("DoorLockClusterDoorLockAlarmEvent {\n")
    append("\talarmCode : $alarmCode\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ALARM_CODE), alarmCode)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ALARM_CODE = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): DoorLockClusterDoorLockAlarmEvent {
      tlvReader.enterStructure(tag)
      val alarmCode = tlvReader.getInt(ContextSpecificTag(TAG_ALARM_CODE))

      tlvReader.exitContainer()

      return DoorLockClusterDoorLockAlarmEvent(alarmCode)
    }
  }
}
