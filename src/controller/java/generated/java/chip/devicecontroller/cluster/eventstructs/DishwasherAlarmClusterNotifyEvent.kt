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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class DishwasherAlarmClusterNotifyEvent(
  val active: Long,
  val inactive: Long,
  val state: Long,
  val mask: Long
) {
  override fun toString(): String = buildString {
    append("DishwasherAlarmClusterNotifyEvent {\n")
    append("\tactive : $active\n")
    append("\tinactive : $inactive\n")
    append("\tstate : $state\n")
    append("\tmask : $mask\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ACTIVE), active)
      put(ContextSpecificTag(TAG_INACTIVE), inactive)
      put(ContextSpecificTag(TAG_STATE), state)
      put(ContextSpecificTag(TAG_MASK), mask)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ACTIVE = 0
    private const val TAG_INACTIVE = 1
    private const val TAG_STATE = 2
    private const val TAG_MASK = 3

    fun fromTlv(tag: Tag, tlvReader: TlvReader): DishwasherAlarmClusterNotifyEvent {
      tlvReader.enterStructure(tag)
      val active = tlvReader.getLong(ContextSpecificTag(TAG_ACTIVE))
      val inactive = tlvReader.getLong(ContextSpecificTag(TAG_INACTIVE))
      val state = tlvReader.getLong(ContextSpecificTag(TAG_STATE))
      val mask = tlvReader.getLong(ContextSpecificTag(TAG_MASK))

      tlvReader.exitContainer()

      return DishwasherAlarmClusterNotifyEvent(active, inactive, state, mask)
    }
  }
}
