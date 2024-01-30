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

class ThermostatClusterQueuedPresetStruct(
  val presetHandle: ByteArray?,
  val transitionTimestamp: UInt?
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterQueuedPresetStruct {\n")
    append("\tpresetHandle : $presetHandle\n")
    append("\ttransitionTimestamp : $transitionTimestamp\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (presetHandle != null) {
        put(ContextSpecificTag(TAG_PRESET_HANDLE), presetHandle)
      } else {
        putNull(ContextSpecificTag(TAG_PRESET_HANDLE))
      }
      if (transitionTimestamp != null) {
        put(ContextSpecificTag(TAG_TRANSITION_TIMESTAMP), transitionTimestamp)
      } else {
        putNull(ContextSpecificTag(TAG_TRANSITION_TIMESTAMP))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_PRESET_HANDLE = 0
    private const val TAG_TRANSITION_TIMESTAMP = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterQueuedPresetStruct {
      tlvReader.enterStructure(tlvTag)
      val presetHandle =
        if (!tlvReader.isNull()) {
          tlvReader.getByteArray(ContextSpecificTag(TAG_PRESET_HANDLE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PRESET_HANDLE))
          null
        }
      val transitionTimestamp =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_TRANSITION_TIMESTAMP))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TRANSITION_TIMESTAMP))
          null
        }

      tlvReader.exitContainer()

      return ThermostatClusterQueuedPresetStruct(presetHandle, transitionTimestamp)
    }
  }
}
