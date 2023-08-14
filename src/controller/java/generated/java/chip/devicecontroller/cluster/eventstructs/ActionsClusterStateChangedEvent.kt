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

class ActionsClusterStateChangedEvent(val actionID: Int, val invokeID: Long, val newState: Int) {
  override fun toString(): String = buildString {
    append("ActionsClusterStateChangedEvent {\n")
    append("\tactionID : $actionID\n")
    append("\tinvokeID : $invokeID\n")
    append("\tnewState : $newState\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ACTION_I_D), actionID)
      put(ContextSpecificTag(TAG_INVOKE_I_D), invokeID)
      put(ContextSpecificTag(TAG_NEW_STATE), newState)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ACTION_I_D = 0
    private const val TAG_INVOKE_I_D = 1
    private const val TAG_NEW_STATE = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ActionsClusterStateChangedEvent {
      tlvReader.enterStructure(tag)
      val actionID = tlvReader.getInt(ContextSpecificTag(TAG_ACTION_I_D))
      val invokeID = tlvReader.getLong(ContextSpecificTag(TAG_INVOKE_I_D))
      val newState = tlvReader.getInt(ContextSpecificTag(TAG_NEW_STATE))

      tlvReader.exitContainer()

      return ActionsClusterStateChangedEvent(actionID, invokeID, newState)
    }
  }
}
