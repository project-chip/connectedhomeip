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

class AmbientContextSensingCluster(private val controller: MatterController, private val endpointId: UShort) {class AmbientContextTypeAttribute(
    val value: List<AmbientContextSensingClusterAmbientContextTypeStruct>?
  )

  sealed class AmbientContextTypeAttributeSubscriptionState {
    data class Success(
    val value: List<AmbientContextSensingClusterAmbientContextTypeStruct>?
    ) : AmbientContextTypeAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : AmbientContextTypeAttributeSubscriptionState()

    object SubscriptionEstablished : AmbientContextTypeAttributeSubscriptionState()    
  }  
class AmbientContextTypeSupportedAttribute(
    val value: List<AmbientContextSensingClusterSemanticTagStruct>?
  )

  sealed class AmbientContextTypeSupportedAttributeSubscriptionState {
    data class Success(
    val value: List<AmbientContextSensingClusterSemanticTagStruct>?
    ) : AmbientContextTypeSupportedAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : AmbientContextTypeSupportedAttributeSubscriptionState()

    object SubscriptionEstablished : AmbientContextTypeSupportedAttributeSubscriptionState()    
  }  
class HoldTimeLimitsAttribute(
    val value: AmbientContextSensingClusterHoldTimeLimitsStruct?
  )

  sealed class HoldTimeLimitsAttributeSubscriptionState {
    data class Success(
    val value: AmbientContextSensingClusterHoldTimeLimitsStruct?
    ) : HoldTimeLimitsAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : HoldTimeLimitsAttributeSubscriptionState()

    object SubscriptionEstablished : HoldTimeLimitsAttributeSubscriptionState()    
  }  
class PredictedActivityAttribute(
    val value: List<AmbientContextSensingClusterPredictedActivityStruct>?
  )

  sealed class PredictedActivityAttributeSubscriptionState {
    data class Success(
    val value: List<AmbientContextSensingClusterPredictedActivityStruct>?
    ) : PredictedActivityAttributeSubscriptionState()
    
    data class Error(val exception: Exception) : PredictedActivityAttributeSubscriptionState()

    object SubscriptionEstablished : PredictedActivityAttributeSubscriptionState()    
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
suspend fun readHumanActivityDetectedAttribute(): Boolean? {val ATTRIBUTE_ID: UInt = 0u

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
      "Humanactivitydetected attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getBoolean(AnonymousTag)
    } else {
      null
    }


    return decodedValue
  }

  suspend fun subscribeHumanActivityDetectedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
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
          emit(BooleanSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Humanactivitydetected attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getBoolean(AnonymousTag)
    } else {
      null
    }

          decodedValue?.let {
            emit(BooleanSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readObjectIdentifiedAttribute(): Boolean? {val ATTRIBUTE_ID: UInt = 1u

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
      "Objectidentified attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getBoolean(AnonymousTag)
    } else {
      null
    }


    return decodedValue
  }

  suspend fun subscribeObjectIdentifiedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
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
          emit(BooleanSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Objectidentified attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getBoolean(AnonymousTag)
    } else {
      null
    }

          decodedValue?.let {
            emit(BooleanSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readAudioContextDetectedAttribute(): Boolean? {val ATTRIBUTE_ID: UInt = 2u

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
      "Audiocontextdetected attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getBoolean(AnonymousTag)
    } else {
      null
    }


    return decodedValue
  }

  suspend fun subscribeAudioContextDetectedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
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
          emit(BooleanSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Audiocontextdetected attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getBoolean(AnonymousTag)
    } else {
      null
    }

          decodedValue?.let {
            emit(BooleanSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readAmbientContextTypeAttribute(): AmbientContextTypeAttribute {val ATTRIBUTE_ID: UInt = 3u

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
      "Ambientcontexttype attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<AmbientContextSensingClusterAmbientContextTypeStruct>? = if (tlvReader.isNextTag(AnonymousTag)) {
      buildList<AmbientContextSensingClusterAmbientContextTypeStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(AmbientContextSensingClusterAmbientContextTypeStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
    } else {
      null
    }


    return AmbientContextTypeAttribute(decodedValue)
  }

  suspend fun subscribeAmbientContextTypeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<AmbientContextTypeAttributeSubscriptionState> {
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
          emit(AmbientContextTypeAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Ambientcontexttype attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<AmbientContextSensingClusterAmbientContextTypeStruct>? = if (tlvReader.isNextTag(AnonymousTag)) {
      buildList<AmbientContextSensingClusterAmbientContextTypeStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(AmbientContextSensingClusterAmbientContextTypeStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
    } else {
      null
    }

          decodedValue?.let {
            emit(AmbientContextTypeAttributeSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AmbientContextTypeAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readAmbientContextTypeSupportedAttribute(): AmbientContextTypeSupportedAttribute {val ATTRIBUTE_ID: UInt = 4u

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
      "Ambientcontexttypesupported attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<AmbientContextSensingClusterSemanticTagStruct>? = if (tlvReader.isNextTag(AnonymousTag)) {
      buildList<AmbientContextSensingClusterSemanticTagStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(AmbientContextSensingClusterSemanticTagStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
    } else {
      null
    }


    return AmbientContextTypeSupportedAttribute(decodedValue)
  }

  suspend fun subscribeAmbientContextTypeSupportedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<AmbientContextTypeSupportedAttributeSubscriptionState> {
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
          emit(AmbientContextTypeSupportedAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Ambientcontexttypesupported attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<AmbientContextSensingClusterSemanticTagStruct>? = if (tlvReader.isNextTag(AnonymousTag)) {
      buildList<AmbientContextSensingClusterSemanticTagStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(AmbientContextSensingClusterSemanticTagStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
    } else {
      null
    }

          decodedValue?.let {
            emit(AmbientContextTypeSupportedAttributeSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(AmbientContextTypeSupportedAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readSimultaneousDetectionLimitAttribute(): UByte {val ATTRIBUTE_ID: UInt = 5u

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
      "Simultaneousdetectionlimit attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)


    return decodedValue
  }

  suspend fun writeSimultaneousDetectionLimitAttribute(
    value: UByte,
    timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 5u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value) 

    val writeRequests: WriteRequests =
      WriteRequests(
        requests = listOf(
          WriteRequest(
            attributePath = AttributePath(
              endpointId,
              clusterId = CLUSTER_ID,
              attributeId = ATTRIBUTE_ID
            ),
            tlvPayload = tlvWriter.getEncoded()
          )
        ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeSimultaneousDetectionLimitAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UByteSubscriptionState> {
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
          emit(UByteSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Simultaneousdetectionlimit attribute not found in Node State update" 
          }

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
suspend fun readCountThresholdReachedAttribute(): Boolean? {val ATTRIBUTE_ID: UInt = 6u

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
      "Countthresholdreached attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getBoolean(AnonymousTag)
    } else {
      null
    }


    return decodedValue
  }

  suspend fun subscribeCountThresholdReachedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 6u
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
          emit(BooleanSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Countthresholdreached attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getBoolean(AnonymousTag)
    } else {
      null
    }

          decodedValue?.let {
            emit(BooleanSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readCountThresholdAttribute(): UShort? {val ATTRIBUTE_ID: UInt = 7u

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
      "Countthreshold attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getUShort(AnonymousTag)
    } else {
      null
    }


    return decodedValue
  }

  suspend fun writeCountThresholdAttribute(
    value: UShort,
    timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 7u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value) 

    val writeRequests: WriteRequests =
      WriteRequests(
        requests = listOf(
          WriteRequest(
            attributePath = AttributePath(
              endpointId,
              clusterId = CLUSTER_ID,
              attributeId = ATTRIBUTE_ID
            ),
            tlvPayload = tlvWriter.getEncoded()
          )
        ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeCountThresholdAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 7u
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
            "Countthreshold attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getUShort(AnonymousTag)
    } else {
      null
    }

          decodedValue?.let {
            emit(UShortSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readObjectCountAttribute(): UShort? {val ATTRIBUTE_ID: UInt = 8u

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
      "Objectcount attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getUShort(AnonymousTag)
    } else {
      null
    }


    return decodedValue
  }

  suspend fun subscribeObjectCountAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 8u
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
            "Objectcount attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getUShort(AnonymousTag)
    } else {
      null
    }

          decodedValue?.let {
            emit(UShortSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readHoldTimeAttribute(): UShort? {val ATTRIBUTE_ID: UInt = 9u

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
      "Holdtime attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getUShort(AnonymousTag)
    } else {
      null
    }


    return decodedValue
  }

  suspend fun writeHoldTimeAttribute(
    value: UShort,
    timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 9u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value) 

    val writeRequests: WriteRequests =
      WriteRequests(
        requests = listOf(
          WriteRequest(
            attributePath = AttributePath(
              endpointId,
              clusterId = CLUSTER_ID,
              attributeId = ATTRIBUTE_ID
            ),
            tlvPayload = tlvWriter.getEncoded()
          )
        ),
        timedRequest = timedWriteTimeout
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

  suspend fun subscribeHoldTimeAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<UShortSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 9u
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
            "Holdtime attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: UShort? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getUShort(AnonymousTag)
    } else {
      null
    }

          decodedValue?.let {
            emit(UShortSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(UShortSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readHoldTimeLimitsAttribute(): HoldTimeLimitsAttribute {val ATTRIBUTE_ID: UInt = 10u

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
      "Holdtimelimits attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: AmbientContextSensingClusterHoldTimeLimitsStruct? = if (tlvReader.isNextTag(AnonymousTag)) {
      AmbientContextSensingClusterHoldTimeLimitsStruct.fromTlv(AnonymousTag, tlvReader)
    } else {
      null
    }


    return HoldTimeLimitsAttribute(decodedValue)
  }

  suspend fun subscribeHoldTimeLimitsAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<HoldTimeLimitsAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 10u
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
          emit(HoldTimeLimitsAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Holdtimelimits attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: AmbientContextSensingClusterHoldTimeLimitsStruct? = if (tlvReader.isNextTag(AnonymousTag)) {
      AmbientContextSensingClusterHoldTimeLimitsStruct.fromTlv(AnonymousTag, tlvReader)
    } else {
      null
    }

          decodedValue?.let {
            emit(HoldTimeLimitsAttributeSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(HoldTimeLimitsAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readPredictedActivityAttribute(): PredictedActivityAttribute {val ATTRIBUTE_ID: UInt = 11u

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
      "Predictedactivity attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<AmbientContextSensingClusterPredictedActivityStruct>? = if (tlvReader.isNextTag(AnonymousTag)) {
      buildList<AmbientContextSensingClusterPredictedActivityStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(AmbientContextSensingClusterPredictedActivityStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
    } else {
      null
    }


    return PredictedActivityAttribute(decodedValue)
  }

  suspend fun subscribePredictedActivityAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<PredictedActivityAttributeSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 11u
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
          emit(PredictedActivityAttributeSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Predictedactivity attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: List<AmbientContextSensingClusterPredictedActivityStruct>? = if (tlvReader.isNextTag(AnonymousTag)) {
      buildList<AmbientContextSensingClusterPredictedActivityStruct> {
      tlvReader.enterArray(AnonymousTag)
      while(!tlvReader.isEndOfContainer()) {
        add(AmbientContextSensingClusterPredictedActivityStruct.fromTlv(AnonymousTag, tlvReader))
      }
      tlvReader.exitContainer()
    }
    } else {
      null
    }

          decodedValue?.let {
            emit(PredictedActivityAttributeSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(PredictedActivityAttributeSubscriptionState.SubscriptionEstablished)
        }
      }
    }    
  }
suspend fun readPrivacyModeEnabledAttribute(): Boolean? {val ATTRIBUTE_ID: UInt = 12u

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
      "Privacymodeenabled attribute not found in response" 
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getBoolean(AnonymousTag)
    } else {
      null
    }


    return decodedValue
  }

  suspend fun subscribePrivacyModeEnabledAttribute(
    minInterval: Int,
    maxInterval: Int
  ): Flow<BooleanSubscriptionState> {
    val ATTRIBUTE_ID: UInt = 12u
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
          emit(BooleanSubscriptionState.Error(Exception("Subscription terminated with error code: ${subscriptionState.terminationCause}")))
        }
        is SubscriptionState.NodeStateUpdate -> {
          val attributeData =
            subscriptionState.updateState.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
              it.path.attributeId == ATTRIBUTE_ID
            }        
             
          requireNotNull(attributeData) { 
            "Privacymodeenabled attribute not found in Node State update" 
          }

          // Decode the TLV data into the appropriate type
          val tlvReader = TlvReader(attributeData.data)
          val decodedValue: Boolean? = if (tlvReader.isNextTag(AnonymousTag)) {
      tlvReader.getBoolean(AnonymousTag)
    } else {
      null
    }

          decodedValue?.let {
            emit(BooleanSubscriptionState.Success(it))
          }
          
        }
        SubscriptionState.SubscriptionEstablished -> {
          emit(BooleanSubscriptionState.SubscriptionEstablished)
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
    private val logger = Logger.getLogger(AmbientContextSensingCluster::class.java.name)
    const val CLUSTER_ID: UInt = 1073u
  }
}
