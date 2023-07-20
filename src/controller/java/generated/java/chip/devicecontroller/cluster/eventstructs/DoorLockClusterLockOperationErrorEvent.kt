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

class DoorLockClusterLockOperationErrorEvent (
    val lockOperationType: Int,
    val operationSource: Int,
    val operationError: Int,
    val userIndex: Int?,
    val fabricIndex: Int?,
    val sourceNode: Long?,
    val credentials: Optional<List<chip.devicecontroller.cluster.structs.DoorLockClusterCredentialStruct>>?) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("DoorLockClusterLockOperationErrorEvent {\n")
    builder.append("\tlockOperationType : $lockOperationType\n")
    builder.append("\toperationSource : $operationSource\n")
    builder.append("\toperationError : $operationError\n")
    builder.append("\tuserIndex : $userIndex\n")
    builder.append("\tfabricIndex : $fabricIndex\n")
    builder.append("\tsourceNode : $sourceNode\n")
    builder.append("\tcredentials : $credentials\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_LOCK_OPERATION_TYPE), lockOperationType)
    tlvWriter.put(ContextSpecificTag(TAG_OPERATION_SOURCE), operationSource)
    tlvWriter.put(ContextSpecificTag(TAG_OPERATION_ERROR), operationError)
    if (userIndex == null) { tlvWriter.putNull(ContextSpecificTag(TAG_USER_INDEX)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_USER_INDEX), userIndex)
    }
    if (fabricIndex == null) { tlvWriter.putNull(ContextSpecificTag(TAG_FABRIC_INDEX)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
    }
    if (sourceNode == null) { tlvWriter.putNull(ContextSpecificTag(TAG_SOURCE_NODE)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_SOURCE_NODE), sourceNode)
    }
    if (credentials == null) { tlvWriter.putNull(ContextSpecificTag(TAG_CREDENTIALS)) }
    else {
      if (credentials.isPresent) {
      val optcredentials = credentials.get()
      tlvWriter.startList(ContextSpecificTag(TAG_CREDENTIALS))
      val iteroptcredentials = optcredentials.iterator()
      while(iteroptcredentials.hasNext()) {
        val next = iteroptcredentials.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    }
    }
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_LOCK_OPERATION_TYPE = 0
    private const val TAG_OPERATION_SOURCE = 1
    private const val TAG_OPERATION_ERROR = 2
    private const val TAG_USER_INDEX = 3
    private const val TAG_FABRIC_INDEX = 4
    private const val TAG_SOURCE_NODE = 5
    private const val TAG_CREDENTIALS = 6

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : DoorLockClusterLockOperationErrorEvent {
      tlvReader.enterStructure(tag)
      val lockOperationType: Int = tlvReader.getInt(ContextSpecificTag(TAG_LOCK_OPERATION_TYPE))
      val operationSource: Int = tlvReader.getInt(ContextSpecificTag(TAG_OPERATION_SOURCE))
      val operationError: Int = tlvReader.getInt(ContextSpecificTag(TAG_OPERATION_ERROR))
      val userIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(TAG_USER_INDEX))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_USER_INDEX))
      null
    }
      val fabricIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_FABRIC_INDEX))
      null
    }
      val sourceNode: Long? = try {
      tlvReader.getLong(ContextSpecificTag(TAG_SOURCE_NODE))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_SOURCE_NODE))
      null
    }
      val credentials: Optional<List<chip.devicecontroller.cluster.structs.DoorLockClusterCredentialStruct>>? = try {
      try {
      Optional.of(mutableListOf<chip.devicecontroller.cluster.structs.DoorLockClusterCredentialStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_CREDENTIALS))
      while(true) {
        try {
          this.add(chip.devicecontroller.cluster.structs.DoorLockClusterCredentialStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    })
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_CREDENTIALS))
      null
    }
      
      tlvReader.exitContainer()

      return DoorLockClusterLockOperationErrorEvent(lockOperationType, operationSource, operationError, userIndex, fabricIndex, sourceNode, credentials)
    }
  }
}
