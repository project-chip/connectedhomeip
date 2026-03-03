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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class EcosystemInformationClusterEcosystemDeviceStruct(
  val deviceName: Optional<String>,
  val deviceNameLastEdit: Optional<ULong>,
  val bridgedEndpoint: UInt,
  val originalEndpoint: UInt,
  val deviceTypes: List<EcosystemInformationClusterDeviceTypeStruct>,
  val uniqueLocationIDs: List<String>,
  val uniqueLocationIDsLastEdit: ULong,
  val fabricIndex: UInt,
) {
  override fun toString(): String = buildString {
    append("EcosystemInformationClusterEcosystemDeviceStruct {\n")
    append("\tdeviceName : $deviceName\n")
    append("\tdeviceNameLastEdit : $deviceNameLastEdit\n")
    append("\tbridgedEndpoint : $bridgedEndpoint\n")
    append("\toriginalEndpoint : $originalEndpoint\n")
    append("\tdeviceTypes : $deviceTypes\n")
    append("\tuniqueLocationIDs : $uniqueLocationIDs\n")
    append("\tuniqueLocationIDsLastEdit : $uniqueLocationIDsLastEdit\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (deviceName.isPresent) {
        val optdeviceName = deviceName.get()
        put(ContextSpecificTag(TAG_DEVICE_NAME), optdeviceName)
      }
      if (deviceNameLastEdit.isPresent) {
        val optdeviceNameLastEdit = deviceNameLastEdit.get()
        put(ContextSpecificTag(TAG_DEVICE_NAME_LAST_EDIT), optdeviceNameLastEdit)
      }
      put(ContextSpecificTag(TAG_BRIDGED_ENDPOINT), bridgedEndpoint)
      put(ContextSpecificTag(TAG_ORIGINAL_ENDPOINT), originalEndpoint)
      startArray(ContextSpecificTag(TAG_DEVICE_TYPES))
      for (item in deviceTypes.iterator()) {
        item.toTlv(AnonymousTag, this)
      }
      endArray()
      startArray(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS))
      for (item in uniqueLocationIDs.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      put(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS_LAST_EDIT), uniqueLocationIDsLastEdit)
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_DEVICE_NAME = 0
    private const val TAG_DEVICE_NAME_LAST_EDIT = 1
    private const val TAG_BRIDGED_ENDPOINT = 2
    private const val TAG_ORIGINAL_ENDPOINT = 3
    private const val TAG_DEVICE_TYPES = 4
    private const val TAG_UNIQUE_LOCATION_I_DS = 5
    private const val TAG_UNIQUE_LOCATION_I_DS_LAST_EDIT = 6
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): EcosystemInformationClusterEcosystemDeviceStruct {
      tlvReader.enterStructure(tlvTag)
      val deviceName =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DEVICE_NAME))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_DEVICE_NAME)))
        } else {
          Optional.empty()
        }
      val deviceNameLastEdit =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DEVICE_NAME_LAST_EDIT))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_DEVICE_NAME_LAST_EDIT)))
        } else {
          Optional.empty()
        }
      val bridgedEndpoint = tlvReader.getUInt(ContextSpecificTag(TAG_BRIDGED_ENDPOINT))
      val originalEndpoint = tlvReader.getUInt(ContextSpecificTag(TAG_ORIGINAL_ENDPOINT))
      val deviceTypes =
        buildList<EcosystemInformationClusterDeviceTypeStruct> {
          tlvReader.enterArray(ContextSpecificTag(TAG_DEVICE_TYPES))
          while (!tlvReader.isEndOfContainer()) {
            add(EcosystemInformationClusterDeviceTypeStruct.fromTlv(AnonymousTag, tlvReader))
          }
          tlvReader.exitContainer()
        }
      val uniqueLocationIDs =
        buildList<String> {
          tlvReader.enterArray(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getString(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val uniqueLocationIDsLastEdit =
        tlvReader.getULong(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS_LAST_EDIT))
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return EcosystemInformationClusterEcosystemDeviceStruct(
        deviceName,
        deviceNameLastEdit,
        bridgedEndpoint,
        originalEndpoint,
        deviceTypes,
        uniqueLocationIDs,
        uniqueLocationIDsLastEdit,
        fabricIndex,
      )
    }
  }
}
