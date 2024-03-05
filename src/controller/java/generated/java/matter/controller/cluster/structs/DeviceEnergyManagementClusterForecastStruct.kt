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
package matter.controller.cluster.structs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DeviceEnergyManagementClusterForecastStruct(
  val forecastId: UShort,
  val activeSlotNumber: UShort?,
  val startTime: UInt,
  val endTime: UInt,
  val earliestStartTime: Optional<UInt>?,
  val latestEndTime: Optional<UInt>,
  val isPauseable: Boolean,
  val slots: List<DeviceEnergyManagementClusterSlotStruct>,
  val forecastUpdateReason: UByte
) {
  override fun toString(): String = buildString {
    append("DeviceEnergyManagementClusterForecastStruct {\n")
    append("\tforecastId : $forecastId\n")
    append("\tactiveSlotNumber : $activeSlotNumber\n")
    append("\tstartTime : $startTime\n")
    append("\tendTime : $endTime\n")
    append("\tearliestStartTime : $earliestStartTime\n")
    append("\tlatestEndTime : $latestEndTime\n")
    append("\tisPauseable : $isPauseable\n")
    append("\tslots : $slots\n")
    append("\tforecastUpdateReason : $forecastUpdateReason\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_FORECAST_ID), forecastId)
      if (activeSlotNumber != null) {
        put(ContextSpecificTag(TAG_ACTIVE_SLOT_NUMBER), activeSlotNumber)
      } else {
        putNull(ContextSpecificTag(TAG_ACTIVE_SLOT_NUMBER))
      }
      put(ContextSpecificTag(TAG_START_TIME), startTime)
      put(ContextSpecificTag(TAG_END_TIME), endTime)
      if (earliestStartTime != null) {
        if (earliestStartTime.isPresent) {
          val optearliestStartTime = earliestStartTime.get()
          put(ContextSpecificTag(TAG_EARLIEST_START_TIME), optearliestStartTime)
        }
      } else {
        putNull(ContextSpecificTag(TAG_EARLIEST_START_TIME))
      }
      if (latestEndTime.isPresent) {
        val optlatestEndTime = latestEndTime.get()
        put(ContextSpecificTag(TAG_LATEST_END_TIME), optlatestEndTime)
      }
      put(ContextSpecificTag(TAG_IS_PAUSEABLE), isPauseable)
      startArray(ContextSpecificTag(TAG_SLOTS))
      for (item in slots.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      put(ContextSpecificTag(TAG_FORECAST_UPDATE_REASON), forecastUpdateReason)
      endStructure()
    }
  }

  companion object {
    private const val TAG_FORECAST_ID = 0
    private const val TAG_ACTIVE_SLOT_NUMBER = 1
    private const val TAG_START_TIME = 2
    private const val TAG_END_TIME = 3
    private const val TAG_EARLIEST_START_TIME = 4
    private const val TAG_LATEST_END_TIME = 5
    private const val TAG_IS_PAUSEABLE = 6
    private const val TAG_SLOTS = 7
    private const val TAG_FORECAST_UPDATE_REASON = 8

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): DeviceEnergyManagementClusterForecastStruct {
      tlvReader.enterStructure(tlvTag)
      val forecastId = tlvReader.getUShort(ContextSpecificTag(TAG_FORECAST_ID))
      val activeSlotNumber =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_ACTIVE_SLOT_NUMBER))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ACTIVE_SLOT_NUMBER))
          null
        }
      val startTime = tlvReader.getUInt(ContextSpecificTag(TAG_START_TIME))
      val endTime = tlvReader.getUInt(ContextSpecificTag(TAG_END_TIME))
      val earliestStartTime =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_EARLIEST_START_TIME))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_EARLIEST_START_TIME)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_EARLIEST_START_TIME))
          null
        }
      val latestEndTime =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_LATEST_END_TIME))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_LATEST_END_TIME)))
        } else {
          Optional.empty()
        }
      val isPauseable = tlvReader.getBoolean(ContextSpecificTag(TAG_IS_PAUSEABLE))
      val slots =
        buildList<DeviceEnergyManagementClusterSlotStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_SLOTS))
          while (!tlvReader.isEndOfContainer()) {
            add(DeviceEnergyManagementClusterSlotStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val forecastUpdateReason = tlvReader.getUByte(ContextSpecificTag(TAG_FORECAST_UPDATE_REASON))

      tlvReader.exitContainer()

      return DeviceEnergyManagementClusterForecastStruct(
        forecastId,
        activeSlotNumber,
        startTime,
        endTime,
        earliestStartTime,
        latestEndTime,
        isPauseable,
        slots,
        forecastUpdateReason
      )
    }
  }
}
