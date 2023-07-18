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
import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
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
    tlvWriter.put(ContextSpecificTag(0), lockOperationType)
    tlvWriter.put(ContextSpecificTag(1), operationSource)
    tlvWriter.put(ContextSpecificTag(2), operationError)
    if (userIndex == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), userIndex)
    }
    if (fabricIndex == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      tlvWriter.put(ContextSpecificTag(4), fabricIndex)
    }
    if (sourceNode == null) { tlvWriter.putNull(ContextSpecificTag(5)) }
    else {
      tlvWriter.put(ContextSpecificTag(5), sourceNode)
    }
    if (credentials == null) { tlvWriter.putNull(ContextSpecificTag(6)) }
    else {
      if (credentials.isPresent) {
      val opt_credentials = credentials.get()
      tlvWriter.startList(ContextSpecificTag(6))
      val iter_opt_credentials = opt_credentials.iterator()
      while(iter_opt_credentials.hasNext()) {
        val next = iter_opt_credentials.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    }
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : DoorLockClusterLockOperationErrorEvent {
      tlvReader.enterStructure(tag)
      val lockOperationType: Int = tlvReader.getInt(ContextSpecificTag(0))
      val operationSource: Int = tlvReader.getInt(ContextSpecificTag(1))
      val operationError: Int = tlvReader.getInt(ContextSpecificTag(2))
      val userIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
      val fabricIndex: Int? = try {
      tlvReader.getInt(ContextSpecificTag(4))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
      val sourceNode: Long? = try {
      tlvReader.getLong(ContextSpecificTag(5))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(5))
      null
    }
      val credentials: Optional<List<chip.devicecontroller.cluster.structs.DoorLockClusterCredentialStruct>>? = try {
      try {
      Optional.of(mutableListOf<chip.devicecontroller.cluster.structs.DoorLockClusterCredentialStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(6))
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
      tlvReader.getNull(ContextSpecificTag(6))
      null
    }
      
      tlvReader.exitContainer()

      return DoorLockClusterLockOperationErrorEvent(lockOperationType, operationSource, operationError, userIndex, fabricIndex, sourceNode, credentials)
    }
  }
}
