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

class OtaSoftwareUpdateRequestorClusterStateTransitionEvent (
    val previousState: Int,
    val newState: Int,
    val reason: Int,
    val targetSoftwareVersion: Long?) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("OtaSoftwareUpdateRequestorClusterStateTransitionEvent {\n")
    builder.append("\tpreviousState : $previousState\n")
    builder.append("\tnewState : $newState\n")
    builder.append("\treason : $reason\n")
    builder.append("\ttargetSoftwareVersion : $targetSoftwareVersion\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_PREVIOUS_STATE), previousState)
    tlvWriter.put(ContextSpecificTag(TAG_NEW_STATE), newState)
    tlvWriter.put(ContextSpecificTag(TAG_REASON), reason)
    if (targetSoftwareVersion == null) { tlvWriter.putNull(ContextSpecificTag(TAG_TARGET_SOFTWARE_VERSION)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_TARGET_SOFTWARE_VERSION), targetSoftwareVersion)
    }
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_PREVIOUS_STATE = 0
    private const val TAG_NEW_STATE = 1
    private const val TAG_REASON = 2
    private const val TAG_TARGET_SOFTWARE_VERSION = 3

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : OtaSoftwareUpdateRequestorClusterStateTransitionEvent {
      tlvReader.enterStructure(tag)
      val previousState: Int = tlvReader.getInt(ContextSpecificTag(TAG_PREVIOUS_STATE))
      val newState: Int = tlvReader.getInt(ContextSpecificTag(TAG_NEW_STATE))
      val reason: Int = tlvReader.getInt(ContextSpecificTag(TAG_REASON))
      val targetSoftwareVersion: Long? = try {
      tlvReader.getLong(ContextSpecificTag(TAG_TARGET_SOFTWARE_VERSION))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_TARGET_SOFTWARE_VERSION))
      null
    }
      
      tlvReader.exitContainer()

      return OtaSoftwareUpdateRequestorClusterStateTransitionEvent(previousState, newState, reason, targetSoftwareVersion)
    }
  }
}
