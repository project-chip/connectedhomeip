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

class CommodityTariffClusterCalendarPeriodStruct (
    val startDate: ULong?,
    val dayPatternIDs: List<ULong>) {
  override fun toString(): String  = buildString {
    append("CommodityTariffClusterCalendarPeriodStruct {\n")
    append("\tstartDate : $startDate\n")
    append("\tdayPatternIDs : $dayPatternIDs\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (startDate != null) {
      put(ContextSpecificTag(TAG_START_DATE), startDate)
    } else {
      putNull(ContextSpecificTag(TAG_START_DATE))
    }
      startArray(ContextSpecificTag(TAG_DAY_PATTERN_I_DS))
      for (item in dayPatternIDs.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_START_DATE = 0
    private const val TAG_DAY_PATTERN_I_DS = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CommodityTariffClusterCalendarPeriodStruct {
      tlvReader.enterStructure(tlvTag)
      val startDate = if (!tlvReader.isNull()) {
      tlvReader.getULong(ContextSpecificTag(TAG_START_DATE))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_START_DATE))
      null
    }
      val dayPatternIDs = buildList<ULong> {
      tlvReader.enterArray(ContextSpecificTag(TAG_DAY_PATTERN_I_DS))
      while(!tlvReader.isEndOfContainer()) {
        add(tlvReader.getULong(AnonymousTag))
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return CommodityTariffClusterCalendarPeriodStruct(startDate, dayPatternIDs)
    }
  }
}
