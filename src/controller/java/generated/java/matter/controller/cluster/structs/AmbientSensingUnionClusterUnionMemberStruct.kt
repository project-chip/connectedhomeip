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

class AmbientSensingUnionClusterUnionMemberStruct(
  val contributorNodeID: ULong,
  val contributorEndpointID: UShort,
  val contributorHealth: UByte,
) {
  override fun toString(): String = buildString {
    append("AmbientSensingUnionClusterUnionMemberStruct {\n")
    append("\tcontributorNodeID : $contributorNodeID\n")
    append("\tcontributorEndpointID : $contributorEndpointID\n")
    append("\tcontributorHealth : $contributorHealth\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CONTRIBUTOR_NODE_ID), contributorNodeID)
      put(ContextSpecificTag(TAG_CONTRIBUTOR_ENDPOINT_ID), contributorEndpointID)
      put(ContextSpecificTag(TAG_CONTRIBUTOR_HEALTH), contributorHealth)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTRIBUTOR_NODE_ID = 0
    private const val TAG_CONTRIBUTOR_ENDPOINT_ID = 1
    private const val TAG_CONTRIBUTOR_HEALTH = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): AmbientSensingUnionClusterUnionMemberStruct {
      tlvReader.enterStructure(tlvTag)
      val contributorNodeID = tlvReader.getULong(ContextSpecificTag(TAG_CONTRIBUTOR_NODE_ID))
      val contributorEndpointID =
        tlvReader.getUShort(ContextSpecificTag(TAG_CONTRIBUTOR_ENDPOINT_ID))
      val contributorHealth = tlvReader.getUByte(ContextSpecificTag(TAG_CONTRIBUTOR_HEALTH))

      tlvReader.exitContainer()

      return AmbientSensingUnionClusterUnionMemberStruct(
        contributorNodeID,
        contributorEndpointID,
        contributorHealth,
      )
    }
  }
}
