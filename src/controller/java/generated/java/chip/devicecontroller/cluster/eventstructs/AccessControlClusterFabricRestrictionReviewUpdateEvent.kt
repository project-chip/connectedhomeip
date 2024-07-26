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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AccessControlClusterFabricRestrictionReviewUpdateEvent(
  val token: ULong,
  val instruction: String?,
  val redirectURL: String?,
  val fabricIndex: UInt,
) {
  override fun toString(): String = buildString {
    append("AccessControlClusterFabricRestrictionReviewUpdateEvent {\n")
    append("\ttoken : $token\n")
    append("\tinstruction : $instruction\n")
    append("\tredirectURL : $redirectURL\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_TOKEN), token)
      if (instruction != null) {
        put(ContextSpecificTag(TAG_INSTRUCTION), instruction)
      } else {
        putNull(ContextSpecificTag(TAG_INSTRUCTION))
      }
      if (redirectURL != null) {
        put(ContextSpecificTag(TAG_REDIRECT_U_R_L), redirectURL)
      } else {
        putNull(ContextSpecificTag(TAG_REDIRECT_U_R_L))
      }
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_TOKEN = 0
    private const val TAG_INSTRUCTION = 1
    private const val TAG_REDIRECT_U_R_L = 2
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AccessControlClusterFabricRestrictionReviewUpdateEvent {
      tlvReader.enterStructure(tlvTag)
      val token = tlvReader.getULong(ContextSpecificTag(TAG_TOKEN))
      val instruction =
        if (!tlvReader.isNull()) {
          tlvReader.getString(ContextSpecificTag(TAG_INSTRUCTION))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_INSTRUCTION))
          null
        }
      val redirectURL =
        if (!tlvReader.isNull()) {
          tlvReader.getString(ContextSpecificTag(TAG_REDIRECT_U_R_L))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_REDIRECT_U_R_L))
          null
        }
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return AccessControlClusterFabricRestrictionReviewUpdateEvent(
        token,
        instruction,
        redirectURL,
        fabricIndex,
      )
    }
  }
}
