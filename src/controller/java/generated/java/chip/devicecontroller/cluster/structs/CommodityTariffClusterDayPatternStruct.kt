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

class CommodityTariffClusterDayPatternStruct (
    val dayPatternID: ULong,
    val daysOfWeek: UInt,
    val dayEntryIDs: List<ULong>) {
  override fun toString(): String  = buildString {
    append("CommodityTariffClusterDayPatternStruct {\n")
    append("\tdayPatternID : $dayPatternID\n")
    append("\tdaysOfWeek : $daysOfWeek\n")
    append("\tdayEntryIDs : $dayEntryIDs\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_DAY_PATTERN_ID), dayPatternID)
      put(ContextSpecificTag(TAG_DAYS_OF_WEEK), daysOfWeek)
      startArray(ContextSpecificTag(TAG_DAY_ENTRY_I_DS))
      for (item in dayEntryIDs.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_DAY_PATTERN_ID = 0
    private const val TAG_DAYS_OF_WEEK = 1
    private const val TAG_DAY_ENTRY_I_DS = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CommodityTariffClusterDayPatternStruct {
      tlvReader.enterStructure(tlvTag)
      val dayPatternID = tlvReader.getULong(ContextSpecificTag(TAG_DAY_PATTERN_ID))
      val daysOfWeek = tlvReader.getUInt(ContextSpecificTag(TAG_DAYS_OF_WEEK))
      val dayEntryIDs = buildList<ULong> {
      tlvReader.enterArray(ContextSpecificTag(TAG_DAY_ENTRY_I_DS))
      while(!tlvReader.isEndOfContainer()) {
        add(tlvReader.getULong(AnonymousTag))
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return CommodityTariffClusterDayPatternStruct(dayPatternID, daysOfWeek, dayEntryIDs)
    }
  }
}
