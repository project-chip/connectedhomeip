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

package matter.controller.cluster.clusters

import java.time.Duration
import java.util.logging.Level
import java.util.logging.Logger
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.transform
import matter.controller.BooleanSubscriptionState
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.MatterController
import matter.controller.ReadData
import matter.controller.ReadRequest
import matter.controller.SubscribeRequest
import matter.controller.SubscriptionState
import matter.controller.UByteSubscriptionState
import matter.controller.UIntSubscriptionState
import matter.controller.UShortSubscriptionState
import matter.controller.WriteRequest
import matter.controller.WriteRequests
import matter.controller.WriteResponse
import matter.controller.cluster.structs.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class NetworkCommissioningCluster(
  private val controller: MatterController,
  private val endpointId: UShort,
) {
  class ScanNetworksResponse(
    val networkingStatus: UByte,
    val debugText: String?,
    val wiFiScanResults: List<NetworkCommissioningClusterWiFiInterfaceScanResultStruct>?,
    val threadScanResults: List<NetworkCommissioningClusterThreadInterfaceScanResultStruct>?,
  )

  class NetworkConfigResponse(
    val networkingStatus: UByte,
    val debugText: String?,
    val networkIndex: UByte?,
    val clientIdentity: ByteArray?,
    val possessionSignature: ByteArray?,
  )

  class ConnectNetworkResponse(
    val networkingStatus: UByte,
    val debugText: String?,
    val errorValue: Int?,
  )

  class QueryIdentityResponse(val identity: ByteArray, val possessionSignature: ByteArray?)

  class NetworksAttribute(val value: List<NetworkCommissioningClusterNetworkInfoStruct>)

  sealed class NetworksAttributeSubscriptionState {
    data class Success(val value: List<NetworkCommissioningClusterNetworkInfoStruct>) :
      NetworksAttributeSubscriptionState()

    data class Error(val exception: Exception) : NetworksAttributeSubscriptionState()

    object SubscriptionEstablished : NetworksAttributeSubscriptionState()
  }

  class LastNetworkingStatusAttribute(val value: UByte?)

  sealed class LastNetworkingStatusAttributeSubscriptionState {
    data class Success(val value: UByte?) : LastNetworkingStatusAttributeSubscriptionState()

    data class Error(val exception: Exception) : LastNetworkingStatusAttributeSubscriptionState()

    object SubscriptionEstablished : LastNetworkingStatusAttributeSubscriptionState()
  }

  class LastNetworkIDAttribute(val value: ByteArray?)

  sealed class LastNetworkIDAttributeSubscriptionState {
    data class Success(val value: ByteArray?) : LastNetworkIDAttributeSubscriptionState()

    data class Error(val exception: Exception) : LastNetworkIDAttributeSubscriptionState()

    object SubscriptionEstablished : LastNetworkIDAttributeSubscriptionState()
  }

  class LastConnectErrorValueAttribute(val value: Int?)

  sealed class LastConnectErrorValueAttributeSubscriptionState {
    data class Success(val value: Int?) : LastConnectErrorValueAttributeSubscriptionState()

    data class Error(val exception: Exception) : LastConnectErrorValueAttributeSubscriptionState()

    object SubscriptionEstablished : LastConnectErrorValueAttributeSubscriptionState()
  }

  class SupportedWiFiBandsAttribute(val value: List<UByte>?)

  sealed class SupportedWiFiBandsAttributeSubscriptionState {
    data class Success(val value: List<UByte>?) : SupportedWiFiBandsAttributeSubscriptionState()

    data class Error(val exception: Exception) : SupportedWiFiBandsAttributeSubscriptionState()

    object SubscriptionEstablished : SupportedWiFiBandsAttributeSubscriptionState()
  }

  class GeneratedCommandListAttribute(val value: List<UInt>)

  sealed class GeneratedCommandListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : GeneratedCommandListAttributeSubscriptionState()

    data class Error(val exception: Exception) : GeneratedCommandListAttributeSubscriptionState()

    object SubscriptionEstablished : GeneratedCommandListAttributeSubscriptionState()
  }

  class AcceptedCommandListAttribute(val value: List<UInt>)

  sealed class AcceptedCommandListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : AcceptedCommandListAttributeSubscriptionState()

    data class Error(val exception: Exception) : AcceptedCommandListAttributeSubscriptionState()

    object SubscriptionEstablished : AcceptedCommandListAttributeSubscriptionState()
  }

  class EventListAttribute(val value: List<UInt>)

  sealed class EventListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : EventListAttributeSubscriptionState()

    data class Error(val exception: Exception) : EventListAttributeSubscriptionState()

    object SubscriptionEstablished : EventListAttributeSubscriptionState()
  }

  class AttributeListAttribute(val value: List<UInt>)

  sealed class AttributeListAttributeSubscriptionState {
    data class Success(val value: List<UInt>) : AttributeListAttributeSubscriptionState()

    data class Error(val exception: Exception) : AttributeListAttributeSubscriptionState()

    object SubscriptionEstablished : AttributeListAttributeSubscriptionState()
  }

  suspend fun scanNetworks(
    ssid: ByteArray?,
    breadcrumb: ULong?,
    timedInvokeTimeout: Duration? = null,
  ): ScanNetworksResponse {
    val commandId: UInt = 0u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_SSID_REQ: Int = 0
    ssid?.let { tlvWriter.put(ContextSpecificTag(TAG_SSID_REQ), ssid) }

    val TAG_BREADCRUMB_REQ: Int = 1
    breadcrumb?.let { tlvWriter.put(ContextSpecificTag(TAG_BREADCRUMB_REQ), breadcrumb) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_NETWORKING_STATUS: Int = 0
    var networkingStatus_decoded: UByte? = null

    val TAG_DEBUG_TEXT: Int = 1
    var debugText_decoded: String? = null

    val TAG_WI_FI_SCAN_RESULTS: Int = 2
    var wiFiScanResults_decoded: List<NetworkCommissioningClusterWiFiInterfaceScanResultStruct>? =
      null

    val TAG_THREAD_SCAN_RESULTS: Int = 3
    var threadScanResults_decoded:
      List<NetworkCommissioningClusterThreadInterfaceScanResultStruct>? =
      null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_NETWORKING_STATUS)) {
        networkingStatus_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_DEBUG_TEXT)) {
        debugText_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_WI_FI_SCAN_RESULTS)) {
        wiFiScanResults_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              buildList<NetworkCommissioningClusterWiFiInterfaceScanResultStruct> {
                tlvReader.enterArray(tag)
                while (!tlvReader.isEndOfContainer()) {
                  add(
                    NetworkCommissioningClusterWiFiInterfaceScanResultStruct.fromTlv(
                      AnonymousTag,
                      tlvReader,
                    )
                  )
                }
                tlvReader.exitContainer()
              }
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_THREAD_SCAN_RESULTS)) {
        threadScanResults_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              buildList<NetworkCommissioningClusterThreadInterfaceScanResultStruct> {
                tlvReader.enterArray(tag)
                while (!tlvReader.isEndOfContainer()) {
                  add(
                    NetworkCommissioningClusterThreadInterfaceScanResultStruct.fromTlv(
                      AnonymousTag,
                      tlvReader,
                    )
                  )
                }
                tlvReader.exitContainer()
              }
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (networkingStatus_decoded == null) {
      throw IllegalStateException("networkingStatus not found in TLV")
    }

    tlvReader.exitContainer()

    return ScanNetworksResponse(
      networkingStatus_decoded,
      debugText_decoded,
      wiFiScanResults_decoded,
      threadScanResults_decoded,
    )
  }

  suspend fun addOrUpdateWiFiNetwork(
    ssid: ByteArray,
    credentials: ByteArray,
    breadcrumb: ULong?,
    networkIdentity: ByteArray?,
    clientIdentifier: ByteArray?,
    possessionNonce: ByteArray?,
    timedInvokeTimeout: Duration? = null,
  ): NetworkConfigResponse {
    val commandId: UInt = 2u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_SSID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_SSID_REQ), ssid)

    val TAG_CREDENTIALS_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_CREDENTIALS_REQ), credentials)

    val TAG_BREADCRUMB_REQ: Int = 2
    breadcrumb?.let { tlvWriter.put(ContextSpecificTag(TAG_BREADCRUMB_REQ), breadcrumb) }

    val TAG_NETWORK_IDENTITY_REQ: Int = 3
    networkIdentity?.let {
      tlvWriter.put(ContextSpecificTag(TAG_NETWORK_IDENTITY_REQ), networkIdentity)
    }

    val TAG_CLIENT_IDENTIFIER_REQ: Int = 4
    clientIdentifier?.let {
      tlvWriter.put(ContextSpecificTag(TAG_CLIENT_IDENTIFIER_REQ), clientIdentifier)
    }

    val TAG_POSSESSION_NONCE_REQ: Int = 5
    possessionNonce?.let {
      tlvWriter.put(ContextSpecificTag(TAG_POSSESSION_NONCE_REQ), possessionNonce)
    }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_NETWORKING_STATUS: Int = 0
    var networkingStatus_decoded: UByte? = null

    val TAG_DEBUG_TEXT: Int = 1
    var debugText_decoded: String? = null

    val TAG_NETWORK_INDEX: Int = 2
    var networkIndex_decoded: UByte? = null

    val TAG_CLIENT_IDENTITY: Int = 3
    var clientIdentity_decoded: ByteArray? = null

    val TAG_POSSESSION_SIGNATURE: Int = 4
    var possessionSignature_decoded: ByteArray? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_NETWORKING_STATUS)) {
        networkingStatus_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_DEBUG_TEXT)) {
        debugText_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NETWORK_INDEX)) {
        networkIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_CLIENT_IDENTITY)) {
        clientIdentity_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_POSSESSION_SIGNATURE)) {
        possessionSignature_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (networkingStatus_decoded == null) {
      throw IllegalStateException("networkingStatus not found in TLV")
    }

    tlvReader.exitContainer()

    return NetworkConfigResponse(
      networkingStatus_decoded,
      debugText_decoded,
      networkIndex_decoded,
      clientIdentity_decoded,
      possessionSignature_decoded,
    )
  }

  suspend fun addOrUpdateThreadNetwork(
    operationalDataset: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeout: Duration? = null,
  ): NetworkConfigResponse {
    val commandId: UInt = 3u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_OPERATIONAL_DATASET_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_OPERATIONAL_DATASET_REQ), operationalDataset)

    val TAG_BREADCRUMB_REQ: Int = 1
    breadcrumb?.let { tlvWriter.put(ContextSpecificTag(TAG_BREADCRUMB_REQ), breadcrumb) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_NETWORKING_STATUS: Int = 0
    var networkingStatus_decoded: UByte? = null

    val TAG_DEBUG_TEXT: Int = 1
    var debugText_decoded: String? = null

    val TAG_NETWORK_INDEX: Int = 2
    var networkIndex_decoded: UByte? = null

    val TAG_CLIENT_IDENTITY: Int = 3
    var clientIdentity_decoded: ByteArray? = null

    val TAG_POSSESSION_SIGNATURE: Int = 4
    var possessionSignature_decoded: ByteArray? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_NETWORKING_STATUS)) {
        networkingStatus_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_DEBUG_TEXT)) {
        debugText_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NETWORK_INDEX)) {
        networkIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_CLIENT_IDENTITY)) {
        clientIdentity_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_POSSESSION_SIGNATURE)) {
        possessionSignature_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (networkingStatus_decoded == null) {
      throw IllegalStateException("networkingStatus not found in TLV")
    }

    tlvReader.exitContainer()

    return NetworkConfigResponse(
      networkingStatus_decoded,
      debugText_decoded,
      networkIndex_decoded,
      clientIdentity_decoded,
      possessionSignature_decoded,
    )
  }

  suspend fun removeNetwork(
    networkID: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeout: Duration? = null,
  ): NetworkConfigResponse {
    val commandId: UInt = 4u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NETWORK_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NETWORK_ID_REQ), networkID)

    val TAG_BREADCRUMB_REQ: Int = 1
    breadcrumb?.let { tlvWriter.put(ContextSpecificTag(TAG_BREADCRUMB_REQ), breadcrumb) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_NETWORKING_STATUS: Int = 0
    var networkingStatus_decoded: UByte? = null

    val TAG_DEBUG_TEXT: Int = 1
    var debugText_decoded: String? = null

    val TAG_NETWORK_INDEX: Int = 2
    var networkIndex_decoded: UByte? = null

    val TAG_CLIENT_IDENTITY: Int = 3
    var clientIdentity_decoded: ByteArray? = null

    val TAG_POSSESSION_SIGNATURE: Int = 4
    var possessionSignature_decoded: ByteArray? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_NETWORKING_STATUS)) {
        networkingStatus_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_DEBUG_TEXT)) {
        debugText_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NETWORK_INDEX)) {
        networkIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_CLIENT_IDENTITY)) {
        clientIdentity_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_POSSESSION_SIGNATURE)) {
        possessionSignature_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (networkingStatus_decoded == null) {
      throw IllegalStateException("networkingStatus not found in TLV")
    }

    tlvReader.exitContainer()

    return NetworkConfigResponse(
      networkingStatus_decoded,
      debugText_decoded,
      networkIndex_decoded,
      clientIdentity_decoded,
      possessionSignature_decoded,
    )
  }

  suspend fun connectNetwork(
    networkID: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeout: Duration? = null,
  ): ConnectNetworkResponse {
    val commandId: UInt = 6u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NETWORK_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NETWORK_ID_REQ), networkID)

    val TAG_BREADCRUMB_REQ: Int = 1
    breadcrumb?.let { tlvWriter.put(ContextSpecificTag(TAG_BREADCRUMB_REQ), breadcrumb) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_NETWORKING_STATUS: Int = 0
    var networkingStatus_decoded: UByte? = null

    val TAG_DEBUG_TEXT: Int = 1
    var debugText_decoded: String? = null

    val TAG_ERROR_VALUE: Int = 2
    var errorValue_decoded: Int? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_NETWORKING_STATUS)) {
        networkingStatus_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_DEBUG_TEXT)) {
        debugText_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_ERROR_VALUE)) {
        errorValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              tlvReader.getInt(tag)
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (networkingStatus_decoded == null) {
      throw IllegalStateException("networkingStatus not found in TLV")
    }

    tlvReader.exitContainer()

    return ConnectNetworkResponse(networkingStatus_decoded, debugText_decoded, errorValue_decoded)
  }

  suspend fun reorderNetwork(
    networkID: ByteArray,
    networkIndex: UByte,
    breadcrumb: ULong?,
    timedInvokeTimeout: Duration? = null,
  ): NetworkConfigResponse {
    val commandId: UInt = 8u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NETWORK_ID_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NETWORK_ID_REQ), networkID)

    val TAG_NETWORK_INDEX_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_NETWORK_INDEX_REQ), networkIndex)

    val TAG_BREADCRUMB_REQ: Int = 2
    breadcrumb?.let { tlvWriter.put(ContextSpecificTag(TAG_BREADCRUMB_REQ), breadcrumb) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_NETWORKING_STATUS: Int = 0
    var networkingStatus_decoded: UByte? = null

    val TAG_DEBUG_TEXT: Int = 1
    var debugText_decoded: String? = null

    val TAG_NETWORK_INDEX: Int = 2
    var networkIndex_decoded: UByte? = null

    val TAG_CLIENT_IDENTITY: Int = 3
    var clientIdentity_decoded: ByteArray? = null

    val TAG_POSSESSION_SIGNATURE: Int = 4
    var possessionSignature_decoded: ByteArray? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_NETWORKING_STATUS)) {
        networkingStatus_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_DEBUG_TEXT)) {
        debugText_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NETWORK_INDEX)) {
        networkIndex_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_CLIENT_IDENTITY)) {
        clientIdentity_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_POSSESSION_SIGNATURE)) {
        possessionSignature_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (networkingStatus_decoded == null) {
      throw IllegalStateException("networkingStatus not found in TLV")
    }

    tlvReader.exitContainer()

    return NetworkConfigResponse(
      networkingStatus_decoded,
      debugText_decoded,
      networkIndex_decoded,
      clientIdentity_decoded,
      possessionSignature_decoded,
    )
  }

  suspend fun queryIdentity(
    keyIdentifier: ByteArray,
    possessionNonce: ByteArray?,
    timedInvokeTimeout: Duration? = null,
  ): QueryIdentityResponse {
    val commandId: UInt = 9u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_KEY_IDENTIFIER_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_KEY_IDENTIFIER_REQ), keyIdentifier)

    val TAG_POSSESSION_NONCE_REQ: Int = 1
    possessionNonce?.let {
      tlvWriter.put(ContextSpecificTag(TAG_POSSESSION_NONCE_REQ), possessionNonce)
    }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout,
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_IDENTITY: Int = 0
    var identity_decoded: ByteArray? = null

    val TAG_POSSESSION_SIGNATURE: Int = 1
    var possessionSignature_decoded: ByteArray? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_IDENTITY)) {
        identity_decoded = tlvReader.getByteArray(tag)
      }

      if (tag == ContextSpecificTag(TAG_POSSESSION_SIGNATURE)) {
        possessionSignature_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getByteArray(tag)
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (identity_decoded == null) {
      throw IllegalStateException("identity not found in TLV")
    }

    tlvReader.exitContainer()

    return QueryIdentityResponse(identity_decoded, possessionSignature_decoded)
  }

  suspend fun readMaxNetworksAttribute(): UByte {
    val ATTRIBUTE_ID: UInt = 0u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Maxnetworks attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeMaxNetworksAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 0u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UByteSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Maxnetworks attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

          emit(UByteSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readNetworksAttribute(): NetworksAttribute {
    val ATTRIBUTE_ID: UInt = 1u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Networks attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<NetworkCommissioningClusterNetworkInfoStruct> =
      buildList<NetworkCommissioningClusterNetworkInfoStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(NetworkCommissioningClusterNetworkInfoStruct.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    return NetworksAttribute(decodedValue)
  }

  suspend fun subscribeNetworksAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<NetworksAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 1u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            NetworksAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Networks attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<NetworkCommissioningClusterNetworkInfoStruct> =
            buildList<NetworkCommissioningClusterNetworkInfoStruct> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(NetworkCommissioningClusterNetworkInfoStruct.fromTlv(AnonymousTag, tlvReader))
              }
              tlvReader.exitContainer()
            }

          emit(NetworksAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(NetworksAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readScanMaxTimeSecondsAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 2u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Scanmaxtimeseconds attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeScanMaxTimeSecondsAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 2u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UByteSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Scanmaxtimeseconds attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readConnectMaxTimeSecondsAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 3u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Connectmaxtimeseconds attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeConnectMaxTimeSecondsAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UByteSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 3u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UByteSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Connectmaxtimeseconds attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UByteSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UByteSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readInterfaceEnabledAttribute(): Boolean {
    val ATTRIBUTE_ID: UInt = 4u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Interfaceenabled attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInterfaceEnabledAttribute(value: Boolean, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 4u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded(),
            )
          ),
        timedRequest = timedWriteTimeout,
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun subscribeInterfaceEnabledAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<BooleanSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 4u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            BooleanSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Interfaceenabled attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

          emit(BooleanSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readLastNetworkingStatusAttribute(): LastNetworkingStatusAttribute {
    val ATTRIBUTE_ID: UInt = 5u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Lastnetworkingstatus attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (!tlvReader.isNull()) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return LastNetworkingStatusAttribute(decodedValue)
  }

  suspend fun subscribeLastNetworkingStatusAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<LastNetworkingStatusAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 5u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            LastNetworkingStatusAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Lastnetworkingstatus attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UByte? =
            if (!tlvReader.isNull()) {
              tlvReader.getUByte(AnonymousTag)
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(LastNetworkingStatusAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(LastNetworkingStatusAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readLastNetworkIDAttribute(): LastNetworkIDAttribute {
    val ATTRIBUTE_ID: UInt = 6u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Lastnetworkid attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ByteArray? =
      if (!tlvReader.isNull()) {
        tlvReader.getByteArray(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return LastNetworkIDAttribute(decodedValue)
  }

  suspend fun subscribeLastNetworkIDAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<LastNetworkIDAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 6u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            LastNetworkIDAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Lastnetworkid attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: ByteArray? =
            if (!tlvReader.isNull()) {
              tlvReader.getByteArray(AnonymousTag)
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(LastNetworkIDAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(LastNetworkIDAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readLastConnectErrorValueAttribute(): LastConnectErrorValueAttribute {
    val ATTRIBUTE_ID: UInt = 7u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Lastconnecterrorvalue attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Int? =
      if (!tlvReader.isNull()) {
        tlvReader.getInt(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return LastConnectErrorValueAttribute(decodedValue)
  }

  suspend fun subscribeLastConnectErrorValueAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<LastConnectErrorValueAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 7u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            LastConnectErrorValueAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Lastconnecterrorvalue attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Int? =
            if (!tlvReader.isNull()) {
              tlvReader.getInt(AnonymousTag)
            } else {
              tlvReader.getNull(AnonymousTag)
              null
            }

          decodedValue?.let { emit(LastConnectErrorValueAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(LastConnectErrorValueAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readSupportedWiFiBandsAttribute(): SupportedWiFiBandsAttribute {
    val ATTRIBUTE_ID: UInt = 8u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Supportedwifibands attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UByte>? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        buildList<UByte> {
          tlvReader.enterArray(AnonymousTag)
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUByte(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      } else {
        null
      }

    return SupportedWiFiBandsAttribute(decodedValue)
  }

  suspend fun subscribeSupportedWiFiBandsAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<SupportedWiFiBandsAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 8u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            SupportedWiFiBandsAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Supportedwifibands attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UByte>? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              buildList<UByte> {
                tlvReader.enterArray(AnonymousTag)
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getUByte(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
            } else {
              null
            }

          decodedValue?.let { emit(SupportedWiFiBandsAttributeSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(SupportedWiFiBandsAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readSupportedThreadFeaturesAttribute(): UShort? {
    val ATTRIBUTE_ID: UInt = 9u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Supportedthreadfeatures attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeSupportedThreadFeaturesAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 9u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UShortSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Supportedthreadfeatures attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUShort(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UShortSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readThreadVersionAttribute(): UShort? {
    val ATTRIBUTE_ID: UInt = 10u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Threadversion attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun subscribeThreadVersionAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 10u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UShortSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Threadversion attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? =
            if (tlvReader.isNextTag(AnonymousTag)) {
              tlvReader.getUShort(AnonymousTag)
            } else {
              null
            }

          decodedValue?.let { emit(UShortSubscriptionState.Success(it)) }
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    val ATTRIBUTE_ID: UInt = 65528u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Generatedcommandlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return GeneratedCommandListAttribute(decodedValue)
  }

  suspend fun subscribeGeneratedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<GeneratedCommandListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65528u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            GeneratedCommandListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Generatedcommandlist attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> =
            buildList<UInt> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getUInt(AnonymousTag))
              }
              tlvReader.exitContainer()
            }

          emit(GeneratedCommandListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(GeneratedCommandListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    val ATTRIBUTE_ID: UInt = 65529u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Acceptedcommandlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return AcceptedCommandListAttribute(decodedValue)
  }

  suspend fun subscribeAcceptedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<AcceptedCommandListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65529u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AcceptedCommandListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Acceptedcommandlist attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> =
            buildList<UInt> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getUInt(AnonymousTag))
              }
              tlvReader.exitContainer()
            }

          emit(AcceptedCommandListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AcceptedCommandListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    val ATTRIBUTE_ID: UInt = 65530u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Eventlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return EventListAttribute(decodedValue)
  }

  suspend fun subscribeEventListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<EventListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65530u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            EventListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Eventlist attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> =
            buildList<UInt> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getUInt(AnonymousTag))
              }
              tlvReader.exitContainer()
            }

          emit(EventListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(EventListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    val ATTRIBUTE_ID: UInt = 65531u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Attributelist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return AttributeListAttribute(decodedValue)
  }

  suspend fun subscribeAttributeListAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<AttributeListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65531u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            AttributeListAttributeSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Attributelist attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> =
            buildList<UInt> {
              tlvReader.enterArray(AnonymousTag)
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getUInt(AnonymousTag))
              }
              tlvReader.exitContainer()
            }

          emit(AttributeListAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AttributeListAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readFeatureMapAttribute(): UInt {
    val ATTRIBUTE_ID: UInt = 65532u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Featuremap attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeFeatureMapAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UIntSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65532u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UIntSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) { "Featuremap attribute not found in Node State update" }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

          emit(UIntSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UIntSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    val ATTRIBUTE_ID: UInt = 65533u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Clusterrevision attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  suspend fun subscribeClusterRevisionAttribute(
    minInterval: Int,
    maxInterval: Int,
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65533u
    val attributePaths =
      listOf(
        AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)
      )

    val subscribeRequest: SubscribeRequest =
      SubscribeRequest(
        eventPaths = emptyList(),
        attributePaths = attributePaths,
        minInterval = Duration.ofSeconds(minInterval.toLong()),
        maxInterval = Duration.ofSeconds(maxInterval.toLong()),
      )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(
            UShortSubscriptionState.Error(
              Exception(
                "Subscription terminated with error code: ${subscriptionState.terminationCause}"
              )
            )
          )
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes
              .filterIsInstance<ReadData.Attribute>()
              .firstOrNull { it.path.attributeId == ATTRIBUTE_ID }

          requireNotNull(attributeData) {
            "Clusterrevision attribute not found in Node State update"
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

          emit(UShortSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }
  }

  companion object {
    private val logger = Logger.getLogger(NetworkCommissioningCluster::class.java.name)
    const val CLUSTER_ID: UInt = 49u
  }
}
