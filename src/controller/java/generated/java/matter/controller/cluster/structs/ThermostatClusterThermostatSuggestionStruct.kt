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

class ThermostatClusterThermostatSuggestionStruct(
  val uniqueID: UByte,
  val presetHandle: ByteArray,
  val effectiveTime: UInt,
  val expirationTime: UInt,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterThermostatSuggestionStruct {\n")
    append("\tuniqueID : $uniqueID\n")
    append("\tpresetHandle : $presetHandle\n")
    append("\teffectiveTime : $effectiveTime\n")
    append("\texpirationTime : $expirationTime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_UNIQUE_ID), uniqueID)
      put(ContextSpecificTag(TAG_PRESET_HANDLE), presetHandle)
      put(ContextSpecificTag(TAG_EFFECTIVE_TIME), effectiveTime)
      put(ContextSpecificTag(TAG_EXPIRATION_TIME), expirationTime)
      endStructure()
    }
  }

  companion object {
    private const val TAG_UNIQUE_ID = 0
    private const val TAG_PRESET_HANDLE = 1
    private const val TAG_EFFECTIVE_TIME = 2
    private const val TAG_EXPIRATION_TIME = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterThermostatSuggestionStruct {
      tlvReader.enterStructure(tlvTag)
      val uniqueID = tlvReader.getUByte(ContextSpecificTag(TAG_UNIQUE_ID))
      val presetHandle = tlvReader.getByteArray(ContextSpecificTag(TAG_PRESET_HANDLE))
      val effectiveTime = tlvReader.getUInt(ContextSpecificTag(TAG_EFFECTIVE_TIME))
      val expirationTime = tlvReader.getUInt(ContextSpecificTag(TAG_EXPIRATION_TIME))

      tlvReader.exitContainer()

      return ThermostatClusterThermostatSuggestionStruct(
        uniqueID,
        presetHandle,
        effectiveTime,
        expirationTime,
      )
    }
  }
}
