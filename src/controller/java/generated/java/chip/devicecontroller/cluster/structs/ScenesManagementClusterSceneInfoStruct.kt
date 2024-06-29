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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class ScenesManagementClusterSceneInfoStruct (
    val sceneCount: UInt,
    val currentScene: UInt,
    val currentGroup: UInt,
    val sceneValid: Boolean,
    val remainingCapacity: UInt,
    val fabricIndex: UInt) {
  override fun toString(): String  = buildString {
    append("ScenesManagementClusterSceneInfoStruct {\n")
    append("\tsceneCount : $sceneCount\n")
    append("\tcurrentScene : $currentScene\n")
    append("\tcurrentGroup : $currentGroup\n")
    append("\tsceneValid : $sceneValid\n")
    append("\tremainingCapacity : $remainingCapacity\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SCENE_COUNT), sceneCount)
      put(ContextSpecificTag(TAG_CURRENT_SCENE), currentScene)
      put(ContextSpecificTag(TAG_CURRENT_GROUP), currentGroup)
      put(ContextSpecificTag(TAG_SCENE_VALID), sceneValid)
      put(ContextSpecificTag(TAG_REMAINING_CAPACITY), remainingCapacity)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_SCENE_COUNT = 0
    private const val TAG_CURRENT_SCENE = 1
    private const val TAG_CURRENT_GROUP = 2
    private const val TAG_SCENE_VALID = 3
    private const val TAG_REMAINING_CAPACITY = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : ScenesManagementClusterSceneInfoStruct {
      tlvReader.enterStructure(tlvTag)
      val sceneCount = tlvReader.getUInt(ContextSpecificTag(TAG_SCENE_COUNT))
      val currentScene = tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_SCENE))
      val currentGroup = tlvReader.getUInt(ContextSpecificTag(TAG_CURRENT_GROUP))
      val sceneValid = tlvReader.getBoolean(ContextSpecificTag(TAG_SCENE_VALID))
      val remainingCapacity = tlvReader.getUInt(ContextSpecificTag(TAG_REMAINING_CAPACITY))
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))
      
      tlvReader.exitContainer()

      return ScenesManagementClusterSceneInfoStruct(sceneCount, currentScene, currentGroup, sceneValid, remainingCapacity, fabricIndex)
    }
  }
}
