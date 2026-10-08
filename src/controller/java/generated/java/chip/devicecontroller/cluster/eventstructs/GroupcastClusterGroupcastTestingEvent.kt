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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class GroupcastClusterGroupcastTestingEvent(
  val sourceIpAddress: Optional<ByteArray>,
  val destinationIpAddress: Optional<ByteArray>,
  val groupID: Optional<UInt>,
  val endpointID: Optional<UInt>,
  val clusterID: Optional<ULong>,
  val elementID: Optional<ULong>,
  val accessAllowed: Optional<Boolean>,
  val groupcastTestResult: UInt,
  val fabricIndex: UInt,
) {
  override fun toString(): String = buildString {
    append("GroupcastClusterGroupcastTestingEvent {\n")
    append("\tsourceIpAddress : $sourceIpAddress\n")
    append("\tdestinationIpAddress : $destinationIpAddress\n")
    append("\tgroupID : $groupID\n")
    append("\tendpointID : $endpointID\n")
    append("\tclusterID : $clusterID\n")
    append("\telementID : $elementID\n")
    append("\taccessAllowed : $accessAllowed\n")
    append("\tgroupcastTestResult : $groupcastTestResult\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (sourceIpAddress.isPresent) {
        val optsourceIpAddress = sourceIpAddress.get()
        put(ContextSpecificTag(TAG_SOURCE_IP_ADDRESS), optsourceIpAddress)
      }
      if (destinationIpAddress.isPresent) {
        val optdestinationIpAddress = destinationIpAddress.get()
        put(ContextSpecificTag(TAG_DESTINATION_IP_ADDRESS), optdestinationIpAddress)
      }
      if (groupID.isPresent) {
        val optgroupID = groupID.get()
        put(ContextSpecificTag(TAG_GROUP_ID), optgroupID)
      }
      if (endpointID.isPresent) {
        val optendpointID = endpointID.get()
        put(ContextSpecificTag(TAG_ENDPOINT_ID), optendpointID)
      }
      if (clusterID.isPresent) {
        val optclusterID = clusterID.get()
        put(ContextSpecificTag(TAG_CLUSTER_ID), optclusterID)
      }
      if (elementID.isPresent) {
        val optelementID = elementID.get()
        put(ContextSpecificTag(TAG_ELEMENT_ID), optelementID)
      }
      if (accessAllowed.isPresent) {
        val optaccessAllowed = accessAllowed.get()
        put(ContextSpecificTag(TAG_ACCESS_ALLOWED), optaccessAllowed)
      }
      put(ContextSpecificTag(TAG_GROUPCAST_TEST_RESULT), groupcastTestResult)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_SOURCE_IP_ADDRESS = 0
    private const val TAG_DESTINATION_IP_ADDRESS = 1
    private const val TAG_GROUP_ID = 2
    private const val TAG_ENDPOINT_ID = 3
    private const val TAG_CLUSTER_ID = 4
    private const val TAG_ELEMENT_ID = 5
    private const val TAG_ACCESS_ALLOWED = 6
    private const val TAG_GROUPCAST_TEST_RESULT = 7
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): GroupcastClusterGroupcastTestingEvent {
      tlvReader.enterStructure(tlvTag)
      val sourceIpAddress =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_SOURCE_IP_ADDRESS))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_SOURCE_IP_ADDRESS)))
        } else {
          Optional.empty()
        }
      val destinationIpAddress =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DESTINATION_IP_ADDRESS))) {
          Optional.of(tlvReader.getByteArray(ContextSpecificTag(TAG_DESTINATION_IP_ADDRESS)))
        } else {
          Optional.empty()
        }
      val groupID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_GROUP_ID))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_GROUP_ID)))
        } else {
          Optional.empty()
        }
      val endpointID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENDPOINT_ID))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT_ID)))
        } else {
          Optional.empty()
        }
      val clusterID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CLUSTER_ID))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_CLUSTER_ID)))
        } else {
          Optional.empty()
        }
      val elementID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ELEMENT_ID))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_ELEMENT_ID)))
        } else {
          Optional.empty()
        }
      val accessAllowed =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ACCESS_ALLOWED))) {
          Optional.of(tlvReader.getBoolean(ContextSpecificTag(TAG_ACCESS_ALLOWED)))
        } else {
          Optional.empty()
        }
      val groupcastTestResult = tlvReader.getUInt(ContextSpecificTag(TAG_GROUPCAST_TEST_RESULT))
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return GroupcastClusterGroupcastTestingEvent(
        sourceIpAddress,
        destinationIpAddress,
        groupID,
        endpointID,
        clusterID,
        elementID,
        accessAllowed,
        groupcastTestResult,
        fabricIndex,
      )
    }
  }
}
