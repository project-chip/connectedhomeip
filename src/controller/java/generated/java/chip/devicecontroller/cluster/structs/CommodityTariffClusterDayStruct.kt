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

class CommodityTariffClusterDayStruct (
    val date: ULong,
    val dayType: UInt,
    val dayEntryIDs: List<ULong>) {
  override fun toString(): String  = buildString {
    append("CommodityTariffClusterDayStruct {\n")
    append("\tdate : $date\n")
    append("\tdayType : $dayType\n")
    append("\tdayEntryIDs : $dayEntryIDs\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_DATE), date)
      put(ContextSpecificTag(TAG_DAY_TYPE), dayType)
      startArray(ContextSpecificTag(TAG_DAY_ENTRY_I_DS))
      for (item in dayEntryIDs.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_DATE = 0
    private const val TAG_DAY_TYPE = 1
    private const val TAG_DAY_ENTRY_I_DS = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CommodityTariffClusterDayStruct {
      tlvReader.enterStructure(tlvTag)
      val date = tlvReader.getULong(ContextSpecificTag(TAG_DATE))
      val dayType = tlvReader.getUInt(ContextSpecificTag(TAG_DAY_TYPE))
      val dayEntryIDs = buildList<ULong> {
      tlvReader.enterArray(ContextSpecificTag(TAG_DAY_ENTRY_I_DS))
      while(!tlvReader.isEndOfContainer()) {
        add(tlvReader.getULong(AnonymousTag))
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return CommodityTariffClusterDayStruct(date, dayType, dayEntryIDs)
    }
  }
}
