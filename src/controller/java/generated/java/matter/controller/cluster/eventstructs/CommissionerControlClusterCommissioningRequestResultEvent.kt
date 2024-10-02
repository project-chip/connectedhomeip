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

class CommissionerControlClusterCommissioningRequestResultEvent(
  val requestID: ULong,
  val clientNodeID: ULong,
  val statusCode: UByte,
  val fabricIndex: UByte,
) {
  override fun toString(): String = buildString {
    append("CommissionerControlClusterCommissioningRequestResultEvent {\n")
    append("\trequestID : $requestID\n")
    append("\tclientNodeID : $clientNodeID\n")
    append("\tstatusCode : $statusCode\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_REQUEST_ID), requestID)
      put(ContextSpecificTag(TAG_CLIENT_NODE_ID), clientNodeID)
      put(ContextSpecificTag(TAG_STATUS_CODE), statusCode)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_REQUEST_ID = 0
    private const val TAG_CLIENT_NODE_ID = 1
    private const val TAG_STATUS_CODE = 2
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): CommissionerControlClusterCommissioningRequestResultEvent {
      tlvReader.enterStructure(tlvTag)
      val requestID = tlvReader.getULong(ContextSpecificTag(TAG_REQUEST_ID))
      val clientNodeID = tlvReader.getULong(ContextSpecificTag(TAG_CLIENT_NODE_ID))
      val statusCode = tlvReader.getUByte(ContextSpecificTag(TAG_STATUS_CODE))
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return CommissionerControlClusterCommissioningRequestResultEvent(
        requestID,
        clientNodeID,
        statusCode,
        fabricIndex,
      )
    }
  }
}
