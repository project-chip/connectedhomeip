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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class WaterHeaterManagementClusterWaterHeaterBoostInfoStruct(
  val duration: ULong,
  val oneShot: Optional<Boolean>,
  val emergencyBoost: Optional<Boolean>,
  val temporarySetpoint: Optional<Int>,
  val targetPercentage: Optional<UInt>,
  val targetReheat: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("WaterHeaterManagementClusterWaterHeaterBoostInfoStruct {\n")
    append("\tduration : $duration\n")
    append("\toneShot : $oneShot\n")
    append("\temergencyBoost : $emergencyBoost\n")
    append("\ttemporarySetpoint : $temporarySetpoint\n")
    append("\ttargetPercentage : $targetPercentage\n")
    append("\ttargetReheat : $targetReheat\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_DURATION), duration)
      if (oneShot.isPresent) {
        val optoneShot = oneShot.get()
        put(ContextSpecificTag(TAG_ONE_SHOT), optoneShot)
      }
      if (emergencyBoost.isPresent) {
        val optemergencyBoost = emergencyBoost.get()
        put(ContextSpecificTag(TAG_EMERGENCY_BOOST), optemergencyBoost)
      }
      if (temporarySetpoint.isPresent) {
        val opttemporarySetpoint = temporarySetpoint.get()
        put(ContextSpecificTag(TAG_TEMPORARY_SETPOINT), opttemporarySetpoint)
      }
      if (targetPercentage.isPresent) {
        val opttargetPercentage = targetPercentage.get()
        put(ContextSpecificTag(TAG_TARGET_PERCENTAGE), opttargetPercentage)
      }
      if (targetReheat.isPresent) {
        val opttargetReheat = targetReheat.get()
        put(ContextSpecificTag(TAG_TARGET_REHEAT), opttargetReheat)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_DURATION = 0
    private const val TAG_ONE_SHOT = 1
    private const val TAG_EMERGENCY_BOOST = 2
    private const val TAG_TEMPORARY_SETPOINT = 3
    private const val TAG_TARGET_PERCENTAGE = 4
    private const val TAG_TARGET_REHEAT = 5

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): WaterHeaterManagementClusterWaterHeaterBoostInfoStruct {
      tlvReader.enterStructure(tlvTag)
      val duration = tlvReader.getULong(ContextSpecificTag(TAG_DURATION))
      val oneShot =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ONE_SHOT))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_ONE_SHOT)))
        } else {
          Optional.empty()
        }
      val emergencyBoost =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_EMERGENCY_BOOST))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_EMERGENCY_BOOST)))
        } else {
          Optional.empty()
        }
      val temporarySetpoint =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TEMPORARY_SETPOINT))) {
          Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_TEMPORARY_SETPOINT)))
        } else {
          Optional.empty()
        }
      val targetPercentage =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TARGET_PERCENTAGE))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TARGET_PERCENTAGE)))
        } else {
          Optional.empty()
        }
      val targetReheat =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_TARGET_REHEAT))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_TARGET_REHEAT)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return WaterHeaterManagementClusterWaterHeaterBoostInfoStruct(
        duration,
        oneShot,
        emergencyBoost,
        temporarySetpoint,
        targetPercentage,
        targetReheat,
      )
    }
  }
}
