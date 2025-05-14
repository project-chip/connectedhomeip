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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class CommodityTariffClusterDayEntryStruct (
    val dayEntryID: ULong,
    val startTime: UInt,
    val duration: Optional<UInt>,
    val randomizationOffset: Optional<Int>,
    val randomizationType: Optional<UInt>) {
  override fun toString(): String  = buildString {
    append("CommodityTariffClusterDayEntryStruct {\n")
    append("\tdayEntryID : $dayEntryID\n")
    append("\tstartTime : $startTime\n")
    append("\tduration : $duration\n")
    append("\trandomizationOffset : $randomizationOffset\n")
    append("\trandomizationType : $randomizationType\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_DAY_ENTRY_ID), dayEntryID)
      put(ContextSpecificTag(TAG_START_TIME), startTime)
      if (duration.isPresent) {
      val optduration = duration.get()
      put(ContextSpecificTag(TAG_DURATION), optduration)
    }
      if (randomizationOffset.isPresent) {
      val optrandomizationOffset = randomizationOffset.get()
      put(ContextSpecificTag(TAG_RANDOMIZATION_OFFSET), optrandomizationOffset)
    }
      if (randomizationType.isPresent) {
      val optrandomizationType = randomizationType.get()
      put(ContextSpecificTag(TAG_RANDOMIZATION_TYPE), optrandomizationType)
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_DAY_ENTRY_ID = 0
    private const val TAG_START_TIME = 1
    private const val TAG_DURATION = 2
    private const val TAG_RANDOMIZATION_OFFSET = 3
    private const val TAG_RANDOMIZATION_TYPE = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CommodityTariffClusterDayEntryStruct {
      tlvReader.enterStructure(tlvTag)
      val dayEntryID = tlvReader.getULong(ContextSpecificTag(TAG_DAY_ENTRY_ID))
      val startTime = tlvReader.getUInt(ContextSpecificTag(TAG_START_TIME))
      val duration = if (tlvReader.isNextTag(ContextSpecificTag(TAG_DURATION))) {
      Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_DURATION)))
    } else {
      Optional.empty()
    }
      val randomizationOffset = if (tlvReader.isNextTag(ContextSpecificTag(TAG_RANDOMIZATION_OFFSET))) {
      Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_RANDOMIZATION_OFFSET)))
    } else {
      Optional.empty()
    }
      val randomizationType = if (tlvReader.isNextTag(ContextSpecificTag(TAG_RANDOMIZATION_TYPE))) {
      Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_RANDOMIZATION_TYPE)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return CommodityTariffClusterDayEntryStruct(dayEntryID, startTime, duration, randomizationOffset, randomizationType)
    }
  }
}
