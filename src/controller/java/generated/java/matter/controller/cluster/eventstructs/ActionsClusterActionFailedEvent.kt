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

class ActionsClusterActionFailedEvent(
  val actionID: UShort,
  val invokeID: UInt,
  val newState: UByte,
  val error: UByte,
) {
  override fun toString(): String = buildString {
    append("ActionsClusterActionFailedEvent {\n")
    append("\tactionID : $actionID\n")
    append("\tinvokeID : $invokeID\n")
    append("\tnewState : $newState\n")
    append("\terror : $error\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ACTION_ID), actionID)
      put(ContextSpecificTag(TAG_INVOKE_ID), invokeID)
      put(ContextSpecificTag(TAG_NEW_STATE), newState)
      put(ContextSpecificTag(TAG_ERROR), error)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ACTION_ID = 0
    private const val TAG_INVOKE_ID = 1
    private const val TAG_NEW_STATE = 2
    private const val TAG_ERROR = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ActionsClusterActionFailedEvent {
      tlvReader.enterStructure(tlvTag)
      val actionID = tlvReader.getUShort(ContextSpecificTag(TAG_ACTION_ID))
      val invokeID = tlvReader.getUInt(ContextSpecificTag(TAG_INVOKE_ID))
      val newState = tlvReader.getUByte(ContextSpecificTag(TAG_NEW_STATE))
      val error = tlvReader.getUByte(ContextSpecificTag(TAG_ERROR))

      tlvReader.exitContainer()

      return ActionsClusterActionFailedEvent(actionID, invokeID, newState, error)
    }
  }
}
