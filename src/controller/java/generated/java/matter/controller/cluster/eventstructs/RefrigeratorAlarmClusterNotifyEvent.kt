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

class RefrigeratorAlarmClusterNotifyEvent(
  val active: UInt,
  val inactive: UInt,
  val state: UInt,
  val mask: UInt
) {
  override fun toString(): String = buildString {
    append("RefrigeratorAlarmClusterNotifyEvent {\n")
    append("\tactive : $active\n")
    append("\tinactive : $inactive\n")
    append("\tstate : $state\n")
    append("\tmask : $mask\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): RefrigeratorAlarmClusterNotifyEvent {
      tlvReader.enterStructure(tlvTag)
      val active = tlvReader.getUInt(ContextSpecificTag(TAG_ACTIVE))
      val inactive = tlvReader.getUInt(ContextSpecificTag(TAG_INACTIVE))
      val state = tlvReader.getUInt(ContextSpecificTag(TAG_STATE))
      val mask = tlvReader.getUInt(ContextSpecificTag(TAG_MASK))

      tlvReader.exitContainer()

      return RefrigeratorAlarmClusterNotifyEvent(active, inactive, state, mask)
    }
  }
}
