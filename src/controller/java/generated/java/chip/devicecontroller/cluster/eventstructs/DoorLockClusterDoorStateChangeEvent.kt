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

class DoorLockClusterDoorStateChangeEvent(val doorState: Int) {
  override fun toString(): String = buildString {
    append("DoorLockClusterDoorStateChangeEvent {\n")
    append("\tdoorState : $doorState\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_DOOR_STATE), doorState)
      endStructure()
    }
  }

  companion object {
    private const val TAG_DOOR_STATE = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): DoorLockClusterDoorStateChangeEvent {
      tlvReader.enterStructure(tag)
      val doorState = tlvReader.getInt(ContextSpecificTag(TAG_DOOR_STATE))

      tlvReader.exitContainer()

      return DoorLockClusterDoorStateChangeEvent(doorState)
    }
  }
}
