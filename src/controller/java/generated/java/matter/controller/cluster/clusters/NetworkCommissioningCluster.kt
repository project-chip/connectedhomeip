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
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.MatterController
import matter.controller.ReadData
import matter.controller.ReadRequest
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
  private val endpointId: UShort
) {
  class ScanNetworksResponse(
    val networkingStatus: UByte,
    val debugText: String?,
    val wiFiScanResults: List<NetworkCommissioningClusterWiFiInterfaceScanResultStruct>?,
    val threadScanResults: List<NetworkCommissioningClusterThreadInterfaceScanResultStruct>?
  )

  class NetworkConfigResponse(
    val networkingStatus: UByte,
    val debugText: String?,
    val networkIndex: UByte?,
    val clientIdentity: ByteArray?,
    val possessionSignature: ByteArray?
  )

  class ConnectNetworkResponse(
    val networkingStatus: UByte,
    val debugText: String?,
    val errorValue: Int?
  )

  class QueryIdentityResponse(val identity: ByteArray, val possessionSignature: ByteArray?)

  class NetworksAttribute(val value: List<NetworkCommissioningClusterNetworkInfoStruct>)

  class LastNetworkingStatusAttribute(val value: UByte?)

  class LastNetworkIDAttribute(val value: ByteArray?)

  class LastConnectErrorValueAttribute(val value: Int?)

  class SupportedWiFiBandsAttribute(val value: List<UByte>?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun scanNetworks(
    ssid: ByteArray?,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int? = null
  ): ScanNetworksResponse {
    val commandId: UInt = 0u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
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
                      tlvReader
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
                      tlvReader
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
        // Skip unknown tags
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
      threadScanResults_decoded
    )
  }

  suspend fun addOrUpdateWiFiNetwork(
    ssid: ByteArray,
    credentials: ByteArray,
    breadcrumb: ULong?,
    networkIdentity: ByteArray?,
    clientIdentifier: ByteArray?,
    possessionNonce: ByteArray?,
    timedInvokeTimeoutMs: Int? = null
  ): NetworkConfigResponse {
    val commandId: UInt = 2u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
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
        // Skip unknown tags
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
      possessionSignature_decoded
    )
  }

  suspend fun addOrUpdateThreadNetwork(
    operationalDataset: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int? = null
  ): NetworkConfigResponse {
    val commandId: UInt = 3u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
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
        // Skip unknown tags
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
      possessionSignature_decoded
    )
  }

  suspend fun removeNetwork(
    networkID: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int? = null
  ): NetworkConfigResponse {
    val commandId: UInt = 4u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NETWORK_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NETWORK_I_D_REQ), networkID)

    val TAG_BREADCRUMB_REQ: Int = 1
    breadcrumb?.let { tlvWriter.put(ContextSpecificTag(TAG_BREADCRUMB_REQ), breadcrumb) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
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
        // Skip unknown tags
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
      possessionSignature_decoded
    )
  }

  suspend fun connectNetwork(
    networkID: ByteArray,
    breadcrumb: ULong?,
    timedInvokeTimeoutMs: Int? = null
  ): ConnectNetworkResponse {
    val commandId: UInt = 6u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NETWORK_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NETWORK_I_D_REQ), networkID)

    val TAG_BREADCRUMB_REQ: Int = 1
    breadcrumb?.let { tlvWriter.put(ContextSpecificTag(TAG_BREADCRUMB_REQ), breadcrumb) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
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
        // Skip unknown tags
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
    timedInvokeTimeoutMs: Int? = null
  ): NetworkConfigResponse {
    val commandId: UInt = 8u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NETWORK_I_D_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_NETWORK_I_D_REQ), networkID)

    val TAG_NETWORK_INDEX_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_NETWORK_INDEX_REQ), networkIndex)

    val TAG_BREADCRUMB_REQ: Int = 2
    breadcrumb?.let { tlvWriter.put(ContextSpecificTag(TAG_BREADCRUMB_REQ), breadcrumb) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
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
        // Skip unknown tags
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
      possessionSignature_decoded
    )
  }

  suspend fun queryIdentity(
    keyIdentifier: ByteArray,
    possessionNonce: ByteArray?,
    timedInvokeTimeoutMs: Int? = null
  ): QueryIdentityResponse {
    val commandId: UInt = 9u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
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
        // Skip unknown tags
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

  suspend fun writeInterfaceEnabledAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID: UInt = 4u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
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

  companion object {
    private val logger = Logger.getLogger(NetworkCommissioningCluster::class.java.name)
    const val CLUSTER_ID: UInt = 49u
  }
}
