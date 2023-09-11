/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class ActionsClusterActionFailedEvent(
  val actionID: Int,
  val invokeID: Long,
  val newState: Int,
  val error: Int
) {
  override fun toString(): String = buildString {
    append("ActionsClusterActionFailedEvent {\n")
    append("\tactionID : $actionID\n")
    append("\tinvokeID : $invokeID\n")
    append("\tnewState : $newState\n")
    append("\terror : $error\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_ACTION_I_D), actionID)
      put(ContextSpecificTag(TAG_INVOKE_I_D), invokeID)
      put(ContextSpecificTag(TAG_NEW_STATE), newState)
      put(ContextSpecificTag(TAG_ERROR), error)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ACTION_I_D = 0
    private const val TAG_INVOKE_I_D = 1
    private const val TAG_NEW_STATE = 2
    private const val TAG_ERROR = 3

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ActionsClusterActionFailedEvent {
      tlvReader.enterStructure(tag)
      val actionID = tlvReader.getInt(ContextSpecificTag(TAG_ACTION_I_D))
      val invokeID = tlvReader.getLong(ContextSpecificTag(TAG_INVOKE_I_D))
      val newState = tlvReader.getInt(ContextSpecificTag(TAG_NEW_STATE))
      val error = tlvReader.getInt(ContextSpecificTag(TAG_ERROR))

      tlvReader.exitContainer()

      return ActionsClusterActionFailedEvent(actionID, invokeID, newState, error)
    }
  }
}
