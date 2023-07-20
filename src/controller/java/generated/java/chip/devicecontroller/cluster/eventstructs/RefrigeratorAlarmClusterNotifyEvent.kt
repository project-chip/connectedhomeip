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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class RefrigeratorAlarmClusterNotifyEvent (
    val active: Long,
    val inactive: Long,
    val state: Long,
    val mask: Long) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("RefrigeratorAlarmClusterNotifyEvent {\n")
    builder.append("\tactive : $active\n")
    builder.append("\tinactive : $inactive\n")
    builder.append("\tstate : $state\n")
    builder.append("\tmask : $mask\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_ACTIVE), active)
    tlvWriter.put(ContextSpecificTag(TAG_INACTIVE), inactive)
    tlvWriter.put(ContextSpecificTag(TAG_STATE), state)
    tlvWriter.put(ContextSpecificTag(TAG_MASK), mask)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_ACTIVE = 0
    private const val TAG_INACTIVE = 1
    private const val TAG_STATE = 2
    private const val TAG_MASK = 3

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : RefrigeratorAlarmClusterNotifyEvent {
      tlvReader.enterStructure(tag)
      val active: Long = tlvReader.getLong(ContextSpecificTag(TAG_ACTIVE))
      val inactive: Long = tlvReader.getLong(ContextSpecificTag(TAG_INACTIVE))
      val state: Long = tlvReader.getLong(ContextSpecificTag(TAG_STATE))
      val mask: Long = tlvReader.getLong(ContextSpecificTag(TAG_MASK))
      
      tlvReader.exitContainer()

      return RefrigeratorAlarmClusterNotifyEvent(active, inactive, state, mask)
    }
  }
}
