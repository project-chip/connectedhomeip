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

package matter.devicecontroller.cluster.clusters

import java.util.ArrayList

class ContentLauncherCluster(private val endpointId: UShort) {
  class LauncherResponse(val status: UInt, val data: String?)

  class AcceptHeaderAttribute(val value: ArrayList<String>?)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun launchContent(
    search: ChipStructs.ContentLauncherClusterContentSearchStruct,
    autoPlay: Boolean,
    data: String?
  ): LauncherResponse {
    // Implementation needs to be added here
  }

  suspend fun launchContent(
    search: ChipStructs.ContentLauncherClusterContentSearchStruct,
    autoPlay: Boolean,
    data: String?,
    timedInvokeTimeoutMs: Int
  ): LauncherResponse {
    // Implementation needs to be added here
  }

  suspend fun launchURL(
    contentURL: String,
    displayString: String?,
    brandingInformation: ChipStructs.ContentLauncherClusterBrandingInformationStruct?
  ): LauncherResponse {
    // Implementation needs to be added here
  }

  suspend fun launchURL(
    contentURL: String,
    displayString: String?,
    brandingInformation: ChipStructs.ContentLauncherClusterBrandingInformationStruct?,
    timedInvokeTimeoutMs: Int
  ): LauncherResponse {
    // Implementation needs to be added here
  }

  suspend fun readAcceptHeaderAttribute(): AcceptHeaderAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcceptHeaderAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AcceptHeaderAttribute {
    // Implementation needs to be added here
  }

  suspend fun readSupportedStreamingProtocolsAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun writeSupportedStreamingProtocolsAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeSupportedStreamingProtocolsAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeSupportedStreamingProtocolsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Long {
    // Implementation needs to be added here
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeGeneratedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcceptedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEventListAttribute(minInterval: Int, maxInterval: Int): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAttributeListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readFeatureMapAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): Integer {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): Integer {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 1290u
  }
}
