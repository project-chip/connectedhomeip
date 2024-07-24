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

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class EcosystemInformationClusterEcosystemDeviceStruct(
  val deviceName: Optional<String>?,
  val deviceNameLastEdit: Optional<ULong>?,
  val bridgedEndpoint: UShort?,
  val originalEndpoint: UShort?,
  val deviceTypes: List<EcosystemInformationClusterDeviceTypeStruct>?,
  val uniqueLocationIDs: List<String>?,
  val uniqueLocationIDsLastEdit: ULong?,
  val fabricIndex: UByte,
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
      if (deviceName != null) {
        if (deviceName.isPresent) {
          val optdeviceName = deviceName.get()
          put(ContextSpecificTag(TAG_DEVICE_NAME), optdeviceName)
        }
      } else {
        putNull(ContextSpecificTag(TAG_DEVICE_NAME))
      }
      if (deviceNameLastEdit != null) {
        if (deviceNameLastEdit.isPresent) {
          val optdeviceNameLastEdit = deviceNameLastEdit.get()
          put(ContextSpecificTag(TAG_DEVICE_NAME_LAST_EDIT), optdeviceNameLastEdit)
        }
      } else {
        putNull(ContextSpecificTag(TAG_DEVICE_NAME_LAST_EDIT))
      }
      if (bridgedEndpoint != null) {
        put(ContextSpecificTag(TAG_BRIDGED_ENDPOINT), bridgedEndpoint)
      } else {
        putNull(ContextSpecificTag(TAG_BRIDGED_ENDPOINT))
      }
      if (originalEndpoint != null) {
        put(ContextSpecificTag(TAG_ORIGINAL_ENDPOINT), originalEndpoint)
      } else {
        putNull(ContextSpecificTag(TAG_ORIGINAL_ENDPOINT))
      }
      if (deviceTypes != null) {
        startArray(ContextSpecificTag(TAG_DEVICE_TYPES))
        for (item in deviceTypes.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      } else {
        putNull(ContextSpecificTag(TAG_DEVICE_TYPES))
      }
      if (uniqueLocationIDs != null) {
        startArray(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS))
        for (item in uniqueLocationIDs.iterator()) {
          put(AnonymousTag, item)
        }
        endArray()
      } else {
        putNull(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS))
      }
      if (uniqueLocationIDsLastEdit != null) {
        put(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS_LAST_EDIT), uniqueLocationIDsLastEdit)
      } else {
        putNull(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS_LAST_EDIT))
      }
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
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_DEVICE_NAME))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_DEVICE_NAME)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DEVICE_NAME))
          null
        }
      val deviceNameLastEdit =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_DEVICE_NAME_LAST_EDIT))) {
            Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_DEVICE_NAME_LAST_EDIT)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DEVICE_NAME_LAST_EDIT))
          null
        }
      val bridgedEndpoint =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_BRIDGED_ENDPOINT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_BRIDGED_ENDPOINT))
          null
        }
      val originalEndpoint =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_ORIGINAL_ENDPOINT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ORIGINAL_ENDPOINT))
          null
        }
      val deviceTypes =
        if (!tlvReader.isNull()) {
          buildList<EcosystemInformationClusterDeviceTypeStruct> {
            tlvReader.enterArray(ContextSpecificTag(TAG_DEVICE_TYPES))
            while (!tlvReader.isEndOfContainer()) {
              add(EcosystemInformationClusterDeviceTypeStruct.fromTlv(AnonymousTag, tlvReader))
            }
            tlvReader.exitContainer()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_DEVICE_TYPES))
          null
        }
      val uniqueLocationIDs =
        if (!tlvReader.isNull()) {
          buildList<String> {
            tlvReader.enterArray(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS))
            while (!tlvReader.isEndOfContainer()) {
              add(tlvReader.getString(AnonymousTag))
            }
            tlvReader.exitContainer()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS))
          null
        }
      val uniqueLocationIDsLastEdit =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS_LAST_EDIT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_UNIQUE_LOCATION_I_DS_LAST_EDIT))
          null
        }
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

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
