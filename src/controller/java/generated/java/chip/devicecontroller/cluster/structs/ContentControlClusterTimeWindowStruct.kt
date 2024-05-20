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
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ContentControlClusterTimeWindowStruct(
  val timeWindowIndex: UInt?,
  val dayOfWeek: UInt,
  val timePeriod: List<ContentControlClusterTimePeriodStruct>
) {
  override fun toString(): String = buildString {
    append("ContentControlClusterTimeWindowStruct {\n")
    append("\ttimeWindowIndex : $timeWindowIndex\n")
    append("\tdayOfWeek : $dayOfWeek\n")
    append("\ttimePeriod : $timePeriod\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (timeWindowIndex != null) {
        put(ContextSpecificTag(TAG_TIME_WINDOW_INDEX), timeWindowIndex)
      } else {
        putNull(ContextSpecificTag(TAG_TIME_WINDOW_INDEX))
      }
      put(ContextSpecificTag(TAG_DAY_OF_WEEK), dayOfWeek)
      startArray(ContextSpecificTag(TAG_TIME_PERIOD))
      for (item in timePeriod.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      endStructure()
    }
  }

  companion object {
    private const val TAG_TIME_WINDOW_INDEX = 0
    private const val TAG_DAY_OF_WEEK = 1
    private const val TAG_TIME_PERIOD = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentControlClusterTimeWindowStruct {
      tlvReader.enterStructure(tlvTag)
      val timeWindowIndex =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_TIME_WINDOW_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TIME_WINDOW_INDEX))
          null
        }
      val dayOfWeek = tlvReader.getUInt(ContextSpecificTag(TAG_DAY_OF_WEEK))
      val timePeriod =
        buildList<ContentControlClusterTimePeriodStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_TIME_PERIOD))
          while (!tlvReader.isEndOfContainer()) {
            add(ContentControlClusterTimePeriodStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }

      tlvReader.exitContainer()

      return ContentControlClusterTimeWindowStruct(timeWindowIndex, dayOfWeek, timePeriod)
    }
  }
}
