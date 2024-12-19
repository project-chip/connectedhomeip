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
package matter.controller.cluster.eventstructs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThermostatClusterActiveScheduleChangeEvent(
  val previousScheduleHandle: ByteArray,
  val currentScheduleHandle: ByteArray,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterActiveScheduleChangeEvent {\n")
    append("\tpreviousScheduleHandle : $previousScheduleHandle\n")
    append("\tcurrentScheduleHandle : $currentScheduleHandle\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PREVIOUS_SCHEDULE_HANDLE), previousScheduleHandle)
      put(ContextSpecificTag(TAG_CURRENT_SCHEDULE_HANDLE), currentScheduleHandle)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PREVIOUS_SCHEDULE_HANDLE = 0
    private const val TAG_CURRENT_SCHEDULE_HANDLE = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterActiveScheduleChangeEvent {
      tlvReader.enterStructure(tlvTag)
      val previousScheduleHandle =
        tlvReader.getByteArray(ContextSpecificTag(TAG_PREVIOUS_SCHEDULE_HANDLE))
      val currentScheduleHandle =
        tlvReader.getByteArray(ContextSpecificTag(TAG_CURRENT_SCHEDULE_HANDLE))

      tlvReader.exitContainer()

      return ThermostatClusterActiveScheduleChangeEvent(
        previousScheduleHandle,
        currentScheduleHandle,
      )
    }
  }
}
