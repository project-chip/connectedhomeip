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

class ThermostatClusterThermostatScheduleTransition(
  val transitionTime: Int,
  val heatSetpoint: Int?,
  val coolSetpoint: Int?
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterThermostatScheduleTransition {\n")
    append("\ttransitionTime : $transitionTime\n")
    append("\theatSetpoint : $heatSetpoint\n")
    append("\tcoolSetpoint : $coolSetpoint\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_TRANSITION_TIME), transitionTime)
      if (heatSetpoint != null) {
        put(ContextSpecificTag(TAG_HEAT_SETPOINT), heatSetpoint)
      } else {
        putNull(ContextSpecificTag(TAG_HEAT_SETPOINT))
      }
      if (coolSetpoint != null) {
        put(ContextSpecificTag(TAG_COOL_SETPOINT), coolSetpoint)
      } else {
        putNull(ContextSpecificTag(TAG_COOL_SETPOINT))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TRANSITION_TIME = 0
    private const val TAG_HEAT_SETPOINT = 1
    private const val TAG_COOL_SETPOINT = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ThermostatClusterThermostatScheduleTransition {
      tlvReader.enterStructure(tag)
      val transitionTime = tlvReader.getInt(ContextSpecificTag(TAG_TRANSITION_TIME))
      val heatSetpoint =
        if (!tlvReader.isNull()) {
          tlvReader.getInt(ContextSpecificTag(TAG_HEAT_SETPOINT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_HEAT_SETPOINT))
          null
        }
      val coolSetpoint =
        if (!tlvReader.isNull()) {
          tlvReader.getInt(ContextSpecificTag(TAG_COOL_SETPOINT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_COOL_SETPOINT))
          null
        }

      tlvReader.exitContainer()

      return ThermostatClusterThermostatScheduleTransition(
        transitionTime,
        heatSetpoint,
        coolSetpoint
      )
    }
  }
}
