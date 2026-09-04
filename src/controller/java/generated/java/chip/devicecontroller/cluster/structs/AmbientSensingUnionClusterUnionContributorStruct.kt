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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AmbientSensingUnionClusterUnionContributorStruct(
  val contributorNodeID: ULong?,
  val contributorEndpointID: UInt?,
  val contributorName: String?,
  val contributorStatus: UInt,
) {
  override fun toString(): String = buildString {
    append("AmbientSensingUnionClusterUnionContributorStruct {\n")
    append("\tcontributorNodeID : $contributorNodeID\n")
    append("\tcontributorEndpointID : $contributorEndpointID\n")
    append("\tcontributorName : $contributorName\n")
    append("\tcontributorStatus : $contributorStatus\n")
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
      if (contributorName != null) {
        put(ContextSpecificTag(TAG_CONTRIBUTOR_NAME), contributorName)
      } else {
        putNull(ContextSpecificTag(TAG_CONTRIBUTOR_NAME))
      }
      put(ContextSpecificTag(TAG_CONTRIBUTOR_STATUS), contributorStatus)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTRIBUTOR_NODE_ID = 0
    private const val TAG_CONTRIBUTOR_ENDPOINT_ID = 1
    private const val TAG_CONTRIBUTOR_NAME = 2
    private const val TAG_CONTRIBUTOR_STATUS = 3

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
        if (!tlvReader.isNull()) {
          tlvReader.getString(ContextSpecificTag(TAG_CONTRIBUTOR_NAME))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CONTRIBUTOR_NAME))
          null
        }
      val contributorStatus = tlvReader.getUInt(ContextSpecificTag(TAG_CONTRIBUTOR_STATUS))

      tlvReader.exitContainer()

      return AmbientSensingUnionClusterUnionContributorStruct(
        contributorNodeID,
        contributorEndpointID,
        contributorName,
        contributorStatus,
      )
    }
  }
}
