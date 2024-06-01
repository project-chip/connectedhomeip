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

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ContentControlClusterTimePeriodStruct(
  val startHour: UByte,
  val startMinute: UByte,
  val endHour: UByte,
  val endMinute: UByte
) {
  override fun toString(): String = buildString {
    append("ContentControlClusterTimePeriodStruct {\n")
    append("\tstartHour : $startHour\n")
    append("\tstartMinute : $startMinute\n")
    append("\tendHour : $endHour\n")
    append("\tendMinute : $endMinute\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_START_HOUR), startHour)
      put(ContextSpecificTag(TAG_START_MINUTE), startMinute)
      put(ContextSpecificTag(TAG_END_HOUR), endHour)
      put(ContextSpecificTag(TAG_END_MINUTE), endMinute)
      endStructure()
    }
  }

  companion object {
    private const val TAG_START_HOUR = 0
    private const val TAG_START_MINUTE = 1
    private const val TAG_END_HOUR = 2
    private const val TAG_END_MINUTE = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ContentControlClusterTimePeriodStruct {
      tlvReader.enterStructure(tlvTag)
      val startHour = tlvReader.getUByte(ContextSpecificTag(TAG_START_HOUR))
      val startMinute = tlvReader.getUByte(ContextSpecificTag(TAG_START_MINUTE))
      val endHour = tlvReader.getUByte(ContextSpecificTag(TAG_END_HOUR))
      val endMinute = tlvReader.getUByte(ContextSpecificTag(TAG_END_MINUTE))

      tlvReader.exitContainer()

      return ContentControlClusterTimePeriodStruct(startHour, startMinute, endHour, endMinute)
    }
  }
}
