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
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AmbientSensingUnionClusterUnionContributorStruct(
  val contributorNodeID: ULong?,
  val contributorEndpointID: UInt?,
  val contributorName: Optional<String>,
  val contributorHealth: UInt,
) {
  override fun toString(): String = buildString {
    append("AmbientSensingUnionClusterUnionContributorStruct {\n")
    append("\tcontributorNodeID : $contributorNodeID\n")
    append("\tcontributorEndpointID : $contributorEndpointID\n")
    append("\tcontributorName : $contributorName\n")
    append("\tcontributorHealth : $contributorHealth\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (contributorNodeID != null) {
        put(ContextSpecificTag(TAG_CONTRIBUTOR_NODE_ID), contributorNodeID)
      } else {
        putNull(ContextSpecificTag(TAG_CONTRIBUTOR_NODE_ID))
      }
      if (contributorEndpointID != null) {
        put(ContextSpecificTag(TAG_CONTRIBUTOR_ENDPOINT_ID), contributorEndpointID)
      } else {
        putNull(ContextSpecificTag(TAG_CONTRIBUTOR_ENDPOINT_ID))
      }
      if (contributorName.isPresent) {
        val optcontributorName = contributorName.get()
        put(ContextSpecificTag(TAG_CONTRIBUTOR_NAME), optcontributorName)
      }
      put(ContextSpecificTag(TAG_CONTRIBUTOR_HEALTH), contributorHealth)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTRIBUTOR_NODE_ID = 0
    private const val TAG_CONTRIBUTOR_ENDPOINT_ID = 1
    private const val TAG_CONTRIBUTOR_NAME = 2
    private const val TAG_CONTRIBUTOR_HEALTH = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientSensingUnionClusterUnionContributorStruct {
      tlvReader.enterStructure(tlvTag)
      val contributorNodeID =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_CONTRIBUTOR_NODE_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CONTRIBUTOR_NODE_ID))
          null
        }
      val contributorEndpointID =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_CONTRIBUTOR_ENDPOINT_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CONTRIBUTOR_ENDPOINT_ID))
          null
        }
      val contributorName =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CONTRIBUTOR_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_CONTRIBUTOR_NAME)))
        } else {
          Optional.empty()
        }
      val contributorHealth = tlvReader.getUInt(ContextSpecificTag(TAG_CONTRIBUTOR_HEALTH))

      tlvReader.exitContainer()

      return AmbientSensingUnionClusterUnionContributorStruct(
        contributorNodeID,
        contributorEndpointID,
        contributorName,
        contributorHealth,
      )
    }
  }
}
