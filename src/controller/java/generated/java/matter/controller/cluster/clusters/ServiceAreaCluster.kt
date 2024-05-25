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

import java.util.logging.Level
import java.util.logging.Logger
import java.time.Duration
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.transform
import matter.controller.MatterController
import matter.controller.ReadRequest
import matter.controller.ReadData
import matter.controller.ReadFailure
import matter.controller.ReadResponse
import matter.controller.SubscribeRequest
import matter.controller.SubscriptionState
import matter.controller.ByteSubscriptionState
import matter.controller.ShortSubscriptionState
import matter.controller.IntSubscriptionState
import matter.controller.LongSubscriptionState
import matter.controller.FloatSubscriptionState
import matter.controller.DoubleSubscriptionState
import matter.controller.CharSubscriptionState
import matter.controller.BooleanSubscriptionState
import matter.controller.UByteSubscriptionState
import matter.controller.UShortSubscriptionState
import matter.controller.UIntSubscriptionState
import matter.controller.ULongSubscriptionState
import matter.controller.StringSubscriptionState
import matter.controller.ByteArraySubscriptionState
import matter.controller.WriteRequest
import matter.controller.WriteRequests
import matter.controller.WriteResponse
import matter.controller.AttributeWriteError
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.controller.cluster.structs.*
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ServiceAreaCluster(private val controller: MatterController, private val endpointId: UShort) {
  class SelectLocationsResponse(
    val status: UByte, 
    val statusText: String
  )

  class SkipCurrentResponse(
    val status: UByte, 
    val statusText: String
  )
class SupportedLocationsAttribute(
    val value: List<ServiceAreaClusterLocationStruct>
  )

  sealed class SupportedLocationsAttributeSubscriptionState {
    data class Success(
    val value: List<ServiceAreaClusterLocationStruct>
    ) : SupportedLocationsAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : SupportedLocationsAttributeSubscriptionState()

    object SubscriptionEstablished : SupportedLocationsAttributeSubscriptionState()    
  }  
class SupportedMapsAttribute(
    val value: List<ServiceAreaClusterMapStruct>?
  )

  sealed class SupportedMapsAttributeSubscriptionState {
    data class Success(
    val value: List<ServiceAreaClusterMapStruct>?
    ) : SupportedMapsAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : SupportedMapsAttributeSubscriptionState()

    object SubscriptionEstablished : SupportedMapsAttributeSubscriptionState()    
  }  
class SelectedLocationsAttribute(
    val value: List<UInt>?
  )

  sealed class SelectedLocationsAttributeSubscriptionState {
    data class Success(
    val value: List<UInt>?
    ) : SelectedLocationsAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : SelectedLocationsAttributeSubscriptionState()

    object SubscriptionEstablished : SelectedLocationsAttributeSubscriptionState()    
  }  
class CurrentLocationAttribute(
    val value: UInt?
  )

  sealed class CurrentLocationAttributeSubscriptionState {
    data class Success(
    val value: UInt?
    ) : CurrentLocationAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : CurrentLocationAttributeSubscriptionState()

    object SubscriptionEstablished : CurrentLocationAttributeSubscriptionState()    
  }  
class EstimatedEndTimeAttribute(
    val value: UInt?
  )

  sealed class EstimatedEndTimeAttributeSubscriptionState {
    data class Success(
    val value: UInt?
    ) : EstimatedEndTimeAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : EstimatedEndTimeAttributeSubscriptionState()

    object SubscriptionEstablished : EstimatedEndTimeAttributeSubscriptionState()    
  }  
class ProgressAttribute(
    val value: List<ServiceAreaClusterProgressStruct>?
  )

  sealed class ProgressAttributeSubscriptionState {
    data class Success(
    val value: List<ServiceAreaClusterProgressStruct>?
    ) : ProgressAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : ProgressAttributeSubscriptionState()

    object SubscriptionEstablished : ProgressAttributeSubscriptionState()    
  }  
class GeneratedCommandListAttribute(
    val value: List<UInt>
  )

  sealed class GeneratedCommandListAttributeSubscriptionState {
    data class Success(
    val value: List<UInt>
    ) : GeneratedCommandListAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : GeneratedCommandListAttributeSubscriptionState()

    object SubscriptionEstablished : GeneratedCommandListAttributeSubscriptionState()    
  }  
class AcceptedCommandListAttribute(
    val value: List<UInt>
  )

  sealed class AcceptedCommandListAttributeSubscriptionState {
    data class Success(
    val value: List<UInt>
    ) : AcceptedCommandListAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : AcceptedCommandListAttributeSubscriptionState()

    object SubscriptionEstablished : AcceptedCommandListAttributeSubscriptionState()    
  }  
class EventListAttribute(
    val value: List<UInt>
  )

  sealed class EventListAttributeSubscriptionState {
    data class Success(
    val value: List<UInt>
    ) : EventListAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : EventListAttributeSubscriptionState()

    object SubscriptionEstablished : EventListAttributeSubscriptionState()    
  }  
class AttributeListAttribute(
    val value: List<UInt>
  )

  sealed class AttributeListAttributeSubscriptionState {
    data class Success(
    val value: List<UInt>
    ) : AttributeListAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : AttributeListAttributeSubscriptionState()

    object SubscriptionEstablished : AttributeListAttributeSubscriptionState()    
  }  

  suspend fun selectLocations(newLocations: List<UInt>?
      ,timedInvokeTimeout: Duration? = null): SelectLocationsResponse {
    val commandId: UInt = 0u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NEW_LOCATIONS_REQ: Int = 0
    newLocations?.let {
      tlvWriter.startArray(ContextSpecificTag(TAG_NEW_LOCATIONS_REQ))
      for (item in newLocations.iterator()) {
        tlvWriter.put(AnonymousTag, item)
      }
      tlvWriter.endArray()
    }    
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null
    
    val TAG_STATUS_TEXT: Int = 1
    var statusText_decoded: String? = null
    

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      
      if (tag == ContextSpecificTag(TAG_STATUS)) {status_decoded = tlvReader.getUByte(tag)}  
      
      if (tag == ContextSpecificTag(TAG_STATUS_TEXT)) {statusText_decoded = tlvReader.getString(tag)}  
      

      else {
        tlvReader.skipElement()
      }
    }

    
    
    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")  
    }
    
    
    if (statusText_decoded == null) {
      throw IllegalStateException("statusText not found in TLV")  
    }
    

    tlvReader.exitContainer()

    return SelectLocationsResponse(
      status_decoded,
      statusText_decoded
    )
  }

  suspend fun skipCurrent(timedInvokeTimeout: Duration? = null): SkipCurrentResponse {
    val commandId: UInt = 2u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)    
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_STATUS: Int = 0
    var status_decoded: UByte? = null
    
    val TAG_STATUS_TEXT: Int = 1
    var statusText_decoded: String? = null
    

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      
      if (tag == ContextSpecificTag(TAG_STATUS)) {status_decoded = tlvReader.getUByte(tag)}  
      
      if (tag == ContextSpecificTag(TAG_STATUS_TEXT)) {statusText_decoded = tlvReader.getString(tag)}  
      

      else {
        tlvReader.skipElement()
      }
    }

    
    
    if (status_decoded == null) {
      throw IllegalStateException("status not found in TLV")  
    }
    
    
    if (statusText_decoded == null) {
      throw IllegalStateException("statusText not found in TLV")  
    }
    

    tlvReader.exitContainer()

    return SkipCurrentResponse(
      status_decoded,
      statusText_decoded
    )
  }
suspend fun readSupportedLocationsAttribute(): SupportedLocationsAttribute {val ATTRIBUTE_ID: UInt = 0u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Supportedlocations attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<ServiceAreaClusterLocationStruct> = buildList<ServiceAreaClusterLocationStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(ServiceAreaClusterLocationStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }


    return SupportedLocationsAttribute(decodedValue)
  }

  suspend fun subscribeSupportedLocationsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<SupportedLocationsAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 0u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(SupportedLocationsAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Supportedlocations attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<ServiceAreaClusterLocationStruct> = buildList<ServiceAreaClusterLocationStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(ServiceAreaClusterLocationStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }

          emit(SupportedLocationsAttributeSubscriptionState.Success(decodedValue))
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(SupportedLocationsAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readSupportedMapsAttribute(): SupportedMapsAttribute {val ATTRIBUTE_ID: UInt = 1u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Supportedmaps attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<ServiceAreaClusterMapStruct>? = if (!tlvReader.isNull()) {
      buildList<ServiceAreaClusterMapStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(ServiceAreaClusterMapStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
    } else {
      tlvReader.getNull(AnonymousTag)
      null
    }


    return SupportedMapsAttribute(decodedValue)
  }

  suspend fun subscribeSupportedMapsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<SupportedMapsAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 1u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(SupportedMapsAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Supportedmaps attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<ServiceAreaClusterMapStruct>? = if (!tlvReader.isNull()) {
      buildList<ServiceAreaClusterMapStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(ServiceAreaClusterMapStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
    } else {
      tlvReader.getNull(AnonymousTag)
      null
    }

          decodedValue?.let {
            emit(SupportedMapsAttributeSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(SupportedMapsAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readSelectedLocationsAttribute(): SelectedLocationsAttribute {val ATTRIBUTE_ID: UInt = 2u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Selectedlocations attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt>? = if (!tlvReader.isNull()) {
      buildList<UInt> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(tlvReader.getUInt(AnonymousTag))
      }
      tlvReader.exitContainer()
    }
    } else {
      tlvReader.getNull(AnonymousTag)
      null
    }


    return SelectedLocationsAttribute(decodedValue)
  }

  suspend fun subscribeSelectedLocationsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<SelectedLocationsAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 2u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(SelectedLocationsAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Selectedlocations attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt>? = if (!tlvReader.isNull()) {
      buildList<UInt> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(tlvReader.getUInt(AnonymousTag))
      }
      tlvReader.exitContainer()
    }
    } else {
      tlvReader.getNull(AnonymousTag)
      null
    }

          decodedValue?.let {
            emit(SelectedLocationsAttributeSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(SelectedLocationsAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readCurrentLocationAttribute(): CurrentLocationAttribute {val ATTRIBUTE_ID: UInt = 3u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Currentlocation attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? = if (!tlvReader.isNull()) {
      if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getUInt(AnonymousTag)
    } else {
      null
    }
    } else {
      tlvReader.getNull(AnonymousTag)
      null
    }


    return CurrentLocationAttribute(decodedValue)
  }

  suspend fun subscribeCurrentLocationAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<CurrentLocationAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 3u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(CurrentLocationAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Currentlocation attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt? = if (!tlvReader.isNull()) {
      if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getUInt(AnonymousTag)
    } else {
      null
    }
    } else {
      tlvReader.getNull(AnonymousTag)
      null
    }

          decodedValue?.let {
            emit(CurrentLocationAttributeSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(CurrentLocationAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readEstimatedEndTimeAttribute(): EstimatedEndTimeAttribute {val ATTRIBUTE_ID: UInt = 4u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Estimatedendtime attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? = if (!tlvReader.isNull()) {
      if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getUInt(AnonymousTag)
    } else {
      null
    }
    } else {
      tlvReader.getNull(AnonymousTag)
      null
    }


    return EstimatedEndTimeAttribute(decodedValue)
  }

  suspend fun subscribeEstimatedEndTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<EstimatedEndTimeAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 4u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(EstimatedEndTimeAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Estimatedendtime attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UInt? = if (!tlvReader.isNull()) {
      if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getUInt(AnonymousTag)
    } else {
      null
    }
    } else {
      tlvReader.getNull(AnonymousTag)
      null
    }

          decodedValue?.let {
            emit(EstimatedEndTimeAttributeSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(EstimatedEndTimeAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readProgressAttribute(): ProgressAttribute {val ATTRIBUTE_ID: UInt = 5u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Progress attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<ServiceAreaClusterProgressStruct>? = if (!tlvReader.isNull()) {
      if (tlvReader.isNextTag(AnonymousTag)) {
      buildList<ServiceAreaClusterProgressStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(ServiceAreaClusterProgressStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
    } else {
      null
    }
    } else {
      tlvReader.getNull(AnonymousTag)
      null
    }


    return ProgressAttribute(decodedValue)
  }

  suspend fun subscribeProgressAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<ProgressAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 5u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(ProgressAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Progress attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<ServiceAreaClusterProgressStruct>? = if (!tlvReader.isNull()) {
      if (tlvReader.isNextTag(AnonymousTag)) {
      buildList<ServiceAreaClusterProgressStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(ServiceAreaClusterProgressStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
    } else {
      null
    }
    } else {
      tlvReader.getNull(AnonymousTag)
      null
    }

          decodedValue?.let {
            emit(ProgressAttributeSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(ProgressAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {val ATTRIBUTE_ID: UInt = 65528u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Generatedcommandlist attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> = buildList<UInt> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(tlvReader.getUInt(AnonymousTag))
      }
      tlvReader.exitContainer()
    }


    return GeneratedCommandListAttribute(decodedValue)
  }

  suspend fun subscribeGeneratedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<GeneratedCommandListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65528u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(GeneratedCommandListAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Generatedcommandlist attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> = buildList<UInt> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
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
suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {val ATTRIBUTE_ID: UInt = 65529u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Acceptedcommandlist attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> = buildList<UInt> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(tlvReader.getUInt(AnonymousTag))
      }
      tlvReader.exitContainer()
    }


    return AcceptedCommandListAttribute(decodedValue)
  }

  suspend fun subscribeAcceptedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<AcceptedCommandListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65529u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(AcceptedCommandListAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Acceptedcommandlist attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> = buildList<UInt> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
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
suspend fun readEventListAttribute(): EventListAttribute {val ATTRIBUTE_ID: UInt = 65530u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Eventlist attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> = buildList<UInt> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(tlvReader.getUInt(AnonymousTag))
      }
      tlvReader.exitContainer()
    }


    return EventListAttribute(decodedValue)
  }

  suspend fun subscribeEventListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<EventListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65530u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(EventListAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Eventlist attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> = buildList<UInt> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
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
suspend fun readAttributeListAttribute(): AttributeListAttribute {val ATTRIBUTE_ID: UInt = 65531u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Attributelist attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> = buildList<UInt> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(tlvReader.getUInt(AnonymousTag))
      }
      tlvReader.exitContainer()
    }


    return AttributeListAttribute(decodedValue)
  }

  suspend fun subscribeAttributeListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<AttributeListAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65531u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(AttributeListAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Attributelist attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<UInt> = buildList<UInt> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
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
suspend fun readFeatureMapAttribute(): UInt {val ATTRIBUTE_ID: UInt = 65532u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Featuremap attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)


    return decodedValue
  }

  suspend fun subscribeFeatureMapAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UIntSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65532u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(UIntSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Featuremap attribute not found in Node State update" 
          }

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
suspend fun readClusterRevisionAttribute(): UShort {val ATTRIBUTE_ID: UInt = 65533u

    val attributePath = AttributePath(
      endpointId = endpointId, 
      clusterId = CLUSTER_ID,
      attributeId = ATTRIBUTE_ID
    )

    val readRequest = ReadRequest(
      eventPaths = emptyList(),
      attributePaths = listOf(attributePath)
    )
    
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
       
    requireNotNull(attributeData) { 
      "Clusterrevision attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)


    return decodedValue
  }

  suspend fun subscribeClusterRevisionAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 65533u
    val attributePaths = listOf(
      AttributePath(
          endpointId = endpointId,
          clusterId = CLUSTER_ID,
          attributeId = ATTRIBUTE_ID
      )
    )

    val subscribeRequest: SubscribeRequest = SubscribeRequest(
      eventPaths = emptyList(), 
      attributePaths = attributePaths, 
      minInterval = Duration.ofSeconds(minInterval.toLong()), 
      maxInterval = Duration.ofSeconds(maxInterval.toLong())
    )

    return controller.subscribe(subscribeRequest).transform { subscriptionState ->
      when (subscriptionState) {
        is SubscriptionState.SubscriptionErrorNotification -> {
          emit(UShortSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
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
    private val logger = Logger.getLogger(ServiceAreaCluster::class.java.name)
    const val CLUSTER_ID: UInt = 336u
  }
}
