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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class PushAvStreamTransportClusterTransportTriggerOptionsStruct(
  val triggerType: UInt,
  val motionZones: Optional<List<PushAvStreamTransportClusterTransportZoneOptionsStruct>>?,
  val motionSensitivity: Optional<UInt>?,
  val motionTimeControl:
    Optional<PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct>,
  val maxPreRollLen: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterTransportTriggerOptionsStruct {\n")
    append("\ttriggerType : $triggerType\n")
    append("\tmotionZones : $motionZones\n")
    append("\tmotionSensitivity : $motionSensitivity\n")
    append("\tmotionTimeControl : $motionTimeControl\n")
    append("\tmaxPreRollLen : $maxPreRollLen\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_TRIGGER_TYPE), triggerType)
      if (motionZones != null) {
        if (motionZones.isPresent) {
          val optmotionZones = motionZones.get()
          startArray(ContextSpecificTag(TAG_MOTION_ZONES))
          for (item in optmotionZones.iterator()) {
            item.toTlv(AnonymousTag, this)
          }
          endArray()
        }
      } else {
        putNull(ContextSpecificTag(TAG_MOTION_ZONES))
      }
      if (motionSensitivity != null) {
        if (motionSensitivity.isPresent) {
          val optmotionSensitivity = motionSensitivity.get()
          put(ContextSpecificTag(TAG_MOTION_SENSITIVITY), optmotionSensitivity)
        }
      } else {
        putNull(ContextSpecificTag(TAG_MOTION_SENSITIVITY))
      }
      if (motionTimeControl.isPresent) {
        val optmotionTimeControl = motionTimeControl.get()
        optmotionTimeControl.toTlv(ContextSpecificTag(TAG_MOTION_TIME_CONTROL), this)
      }
      if (maxPreRollLen.isPresent) {
        val optmaxPreRollLen = maxPreRollLen.get()
        put(ContextSpecificTag(TAG_MAX_PRE_ROLL_LEN), optmaxPreRollLen)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_TRIGGER_TYPE = 0
    private const val TAG_MOTION_ZONES = 1
    private const val TAG_MOTION_SENSITIVITY = 2
    private const val TAG_MOTION_TIME_CONTROL = 3
    private const val TAG_MAX_PRE_ROLL_LEN = 4

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterTransportTriggerOptionsStruct {
      tlvReader.enterStructure(tlvTag)
      val triggerType = tlvReader.getUInt(ContextSpecificTag(TAG_TRIGGER_TYPE))
      val motionZones =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_MOTION_ZONES))) {
            Optional.of(
              buildList<PushAvStreamTransportClusterTransportZoneOptionsStruct> {
                tlvReader.enterArray(ContextSpecificTag(TAG_MOTION_ZONES))
                while (!tlvReader.isEndOfContainer()) {
                  add(
                    PushAvStreamTransportClusterTransportZoneOptionsStruct.fromTlv(
                      AnonymousTag,
                      tlvReader,
                    )
                  )
                }
                tlvReader.exitContainer()
              }
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MOTION_ZONES))
          null
        }
      val motionSensitivity =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_MOTION_SENSITIVITY))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MOTION_SENSITIVITY)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MOTION_SENSITIVITY))
          null
        }
      val motionTimeControl =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MOTION_TIME_CONTROL))) {
          Optional.of(
            PushAvStreamTransportClusterTransportMotionTriggerTimeControlStruct.fromTlv(
              ContextSpecificTag(TAG_MOTION_TIME_CONTROL),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }
      val maxPreRollLen =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_MAX_PRE_ROLL_LEN))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MAX_PRE_ROLL_LEN)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterTransportTriggerOptionsStruct(
        triggerType,
        motionZones,
        motionSensitivity,
        motionTimeControl,
        maxPreRollLen,
      )
    }
  }
}
