/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

// Prevent multiple inclusion
#pragma once

#include "af-structs.h"
#include "af-types.h"
#include "basic-types.h"

/** @brief Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 */
void emberAfClusterInitCallback(chip::EndpointId endpoint, chip::ClusterId clusterId);

// Cluster Init Functions

/** @brief Barrier Control Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfBarrierControlClusterInitCallback(chip::EndpointId endpoint);

/** @brief Basic Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfBasicClusterInitCallback(chip::EndpointId endpoint);

/** @brief Binding Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfBindingClusterInitCallback(chip::EndpointId endpoint);

/** @brief Color Control Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfColorControlClusterInitCallback(chip::EndpointId endpoint);

/** @brief Door Lock Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfDoorLockClusterInitCallback(chip::EndpointId endpoint);

/** @brief Groups Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfGroupsClusterInitCallback(chip::EndpointId endpoint);

/** @brief IAS Zone Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfIasZoneClusterInitCallback(chip::EndpointId endpoint);

/** @brief Identify Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfIdentifyClusterInitCallback(chip::EndpointId endpoint);

/** @brief Level Control Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfLevelControlClusterInitCallback(chip::EndpointId endpoint);

/** @brief On/off Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfOnOffClusterInitCallback(chip::EndpointId endpoint);

/** @brief Scenes Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfScenesClusterInitCallback(chip::EndpointId endpoint);

/** @brief Temperature Measurement Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfTemperatureMeasurementClusterInitCallback(chip::EndpointId endpoint);

// Cluster Server/Client Init Functions

//
// Barrier Control Cluster client
//

/** @brief Barrier Control Cluster Client Init
 *
 * Client Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfBarrierControlClusterClientInitCallback(chip::EndpointId endpoint);

/** @brief Barrier Control Cluster Client Attribute Changed
 *
 * Client Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfBarrierControlClusterClientAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Barrier Control Cluster Client Manufacturer Specific Attribute Changed
 *
 * Client Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfBarrierControlClusterClientManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                                    chip::AttributeId attributeId,
                                                                                    uint16_t manufacturerCode);

/** @brief Barrier Control Cluster Client Message Sent
 *
 * Client Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfBarrierControlClusterClientMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                           EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                           EmberStatus status);

/** @brief Barrier Control Cluster Client Pre Attribute Changed
 *
 * client Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfBarrierControlClusterClientPreAttributeChangedCallback(chip::EndpointId endpoint,
                                                                            chip::AttributeId attributeId,
                                                                            EmberAfAttributeType attributeType, uint8_t size,
                                                                            uint8_t * value);

/** @brief Barrier Control Cluster Client Tick
 *
 * client Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfBarrierControlClusterClientTickCallback(chip::EndpointId endpoint);

//
// Barrier Control Cluster server
//

/** @brief Barrier Control Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfBarrierControlClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief Barrier Control Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfBarrierControlClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Barrier Control Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfBarrierControlClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                                    chip::AttributeId attributeId,
                                                                                    uint16_t manufacturerCode);

/** @brief Barrier Control Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfBarrierControlClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                           EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                           EmberStatus status);

/** @brief Barrier Control Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfBarrierControlClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint,
                                                                            chip::AttributeId attributeId,
                                                                            EmberAfAttributeType attributeType, uint8_t size,
                                                                            uint8_t * value);

/** @brief Barrier Control Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfBarrierControlClusterServerTickCallback(chip::EndpointId endpoint);

//
// Basic Cluster client
//

/** @brief Basic Cluster Client Init
 *
 * Client Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfBasicClusterClientInitCallback(chip::EndpointId endpoint);

/** @brief Basic Cluster Client Attribute Changed
 *
 * Client Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfBasicClusterClientAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Basic Cluster Client Manufacturer Specific Attribute Changed
 *
 * Client Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfBasicClusterClientManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                           uint16_t manufacturerCode);

/** @brief Basic Cluster Client Message Sent
 *
 * Client Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfBasicClusterClientMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                  EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message, EmberStatus status);

/** @brief Basic Cluster Client Pre Attribute Changed
 *
 * client Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfBasicClusterClientPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                   EmberAfAttributeType attributeType, uint8_t size,
                                                                   uint8_t * value);

/** @brief Basic Cluster Client Tick
 *
 * client Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfBasicClusterClientTickCallback(chip::EndpointId endpoint);

//
// Basic Cluster server
//

/** @brief Basic Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfBasicClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief Basic Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfBasicClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Basic Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfBasicClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                           uint16_t manufacturerCode);

/** @brief Basic Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfBasicClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                  EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message, EmberStatus status);

/** @brief Basic Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfBasicClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                   EmberAfAttributeType attributeType, uint8_t size,
                                                                   uint8_t * value);

/** @brief Basic Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfBasicClusterServerTickCallback(chip::EndpointId endpoint);

//
// Binding Cluster client
//

/** @brief Binding Cluster Client Init
 *
 * Client Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfBindingClusterClientInitCallback(chip::EndpointId endpoint);

/** @brief Binding Cluster Client Attribute Changed
 *
 * Client Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfBindingClusterClientAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Binding Cluster Client Manufacturer Specific Attribute Changed
 *
 * Client Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfBindingClusterClientManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                             chip::AttributeId attributeId,
                                                                             uint16_t manufacturerCode);

/** @brief Binding Cluster Client Message Sent
 *
 * Client Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfBindingClusterClientMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                    EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                    EmberStatus status);

/** @brief Binding Cluster Client Pre Attribute Changed
 *
 * client Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfBindingClusterClientPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                     EmberAfAttributeType attributeType, uint8_t size,
                                                                     uint8_t * value);

/** @brief Binding Cluster Client Tick
 *
 * client Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfBindingClusterClientTickCallback(chip::EndpointId endpoint);

//
// Binding Cluster server
//

/** @brief Binding Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfBindingClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief Binding Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfBindingClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Binding Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfBindingClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                             chip::AttributeId attributeId,
                                                                             uint16_t manufacturerCode);

/** @brief Binding Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfBindingClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                    EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                    EmberStatus status);

/** @brief Binding Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfBindingClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                     EmberAfAttributeType attributeType, uint8_t size,
                                                                     uint8_t * value);

/** @brief Binding Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfBindingClusterServerTickCallback(chip::EndpointId endpoint);

//
// Color Control Cluster client
//

/** @brief Color Control Cluster Client Init
 *
 * Client Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfColorControlClusterClientInitCallback(chip::EndpointId endpoint);

/** @brief Color Control Cluster Client Attribute Changed
 *
 * Client Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfColorControlClusterClientAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Color Control Cluster Client Manufacturer Specific Attribute Changed
 *
 * Client Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfColorControlClusterClientManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                                  chip::AttributeId attributeId,
                                                                                  uint16_t manufacturerCode);

/** @brief Color Control Cluster Client Message Sent
 *
 * Client Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfColorControlClusterClientMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                         EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                         EmberStatus status);

/** @brief Color Control Cluster Client Pre Attribute Changed
 *
 * client Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfColorControlClusterClientPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                          EmberAfAttributeType attributeType, uint8_t size,
                                                                          uint8_t * value);

/** @brief Color Control Cluster Client Tick
 *
 * client Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfColorControlClusterClientTickCallback(chip::EndpointId endpoint);

//
// Color Control Cluster server
//

/** @brief Color Control Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfColorControlClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief Color Control Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfColorControlClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Color Control Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfColorControlClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                                  chip::AttributeId attributeId,
                                                                                  uint16_t manufacturerCode);

/** @brief Color Control Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfColorControlClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                         EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                         EmberStatus status);

/** @brief Color Control Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfColorControlClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                          EmberAfAttributeType attributeType, uint8_t size,
                                                                          uint8_t * value);

/** @brief Color Control Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfColorControlClusterServerTickCallback(chip::EndpointId endpoint);

//
// Door Lock Cluster client
//

/** @brief Door Lock Cluster Client Init
 *
 * Client Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfDoorLockClusterClientInitCallback(chip::EndpointId endpoint);

/** @brief Door Lock Cluster Client Attribute Changed
 *
 * Client Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfDoorLockClusterClientAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Door Lock Cluster Client Manufacturer Specific Attribute Changed
 *
 * Client Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfDoorLockClusterClientManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                              chip::AttributeId attributeId,
                                                                              uint16_t manufacturerCode);

/** @brief Door Lock Cluster Client Message Sent
 *
 * Client Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfDoorLockClusterClientMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                     EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                     EmberStatus status);

/** @brief Door Lock Cluster Client Pre Attribute Changed
 *
 * client Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfDoorLockClusterClientPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                      EmberAfAttributeType attributeType, uint8_t size,
                                                                      uint8_t * value);

/** @brief Door Lock Cluster Client Tick
 *
 * client Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfDoorLockClusterClientTickCallback(chip::EndpointId endpoint);

//
// Door Lock Cluster server
//

/** @brief Door Lock Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfDoorLockClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief Door Lock Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfDoorLockClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Door Lock Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfDoorLockClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                              chip::AttributeId attributeId,
                                                                              uint16_t manufacturerCode);

/** @brief Door Lock Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfDoorLockClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                     EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                     EmberStatus status);

/** @brief Door Lock Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfDoorLockClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                      EmberAfAttributeType attributeType, uint8_t size,
                                                                      uint8_t * value);

/** @brief Door Lock Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfDoorLockClusterServerTickCallback(chip::EndpointId endpoint);

//
// Groups Cluster client
//

/** @brief Groups Cluster Client Init
 *
 * Client Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfGroupsClusterClientInitCallback(chip::EndpointId endpoint);

/** @brief Groups Cluster Client Attribute Changed
 *
 * Client Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfGroupsClusterClientAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Groups Cluster Client Manufacturer Specific Attribute Changed
 *
 * Client Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfGroupsClusterClientManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                            chip::AttributeId attributeId,
                                                                            uint16_t manufacturerCode);

/** @brief Groups Cluster Client Message Sent
 *
 * Client Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfGroupsClusterClientMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                   EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                   EmberStatus status);

/** @brief Groups Cluster Client Pre Attribute Changed
 *
 * client Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfGroupsClusterClientPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                    EmberAfAttributeType attributeType, uint8_t size,
                                                                    uint8_t * value);

/** @brief Groups Cluster Client Tick
 *
 * client Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfGroupsClusterClientTickCallback(chip::EndpointId endpoint);

//
// Groups Cluster server
//

/** @brief Groups Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfGroupsClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief Groups Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfGroupsClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Groups Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfGroupsClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                            chip::AttributeId attributeId,
                                                                            uint16_t manufacturerCode);

/** @brief Groups Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfGroupsClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                   EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                   EmberStatus status);

/** @brief Groups Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfGroupsClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                    EmberAfAttributeType attributeType, uint8_t size,
                                                                    uint8_t * value);

/** @brief Groups Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfGroupsClusterServerTickCallback(chip::EndpointId endpoint);

//
// IAS Zone Cluster server
//

/** @brief IAS Zone Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfIasZoneClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief IAS Zone Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfIasZoneClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief IAS Zone Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfIasZoneClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                             chip::AttributeId attributeId,
                                                                             uint16_t manufacturerCode);

/** @brief IAS Zone Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfIasZoneClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                    EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                    EmberStatus status);

/** @brief IAS Zone Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfIasZoneClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                     EmberAfAttributeType attributeType, uint8_t size,
                                                                     uint8_t * value);

/** @brief IAS Zone Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfIasZoneClusterServerTickCallback(chip::EndpointId endpoint);

//
// Identify Cluster client
//

/** @brief Identify Cluster Client Init
 *
 * Client Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfIdentifyClusterClientInitCallback(chip::EndpointId endpoint);

/** @brief Identify Cluster Client Attribute Changed
 *
 * Client Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfIdentifyClusterClientAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Identify Cluster Client Manufacturer Specific Attribute Changed
 *
 * Client Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfIdentifyClusterClientManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                              chip::AttributeId attributeId,
                                                                              uint16_t manufacturerCode);

/** @brief Identify Cluster Client Message Sent
 *
 * Client Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfIdentifyClusterClientMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                     EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                     EmberStatus status);

/** @brief Identify Cluster Client Pre Attribute Changed
 *
 * client Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfIdentifyClusterClientPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                      EmberAfAttributeType attributeType, uint8_t size,
                                                                      uint8_t * value);

/** @brief Identify Cluster Client Tick
 *
 * client Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfIdentifyClusterClientTickCallback(chip::EndpointId endpoint);

//
// Identify Cluster server
//

/** @brief Identify Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfIdentifyClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief Identify Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfIdentifyClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Identify Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfIdentifyClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                              chip::AttributeId attributeId,
                                                                              uint16_t manufacturerCode);

/** @brief Identify Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfIdentifyClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                     EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                     EmberStatus status);

/** @brief Identify Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfIdentifyClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                      EmberAfAttributeType attributeType, uint8_t size,
                                                                      uint8_t * value);

/** @brief Identify Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfIdentifyClusterServerTickCallback(chip::EndpointId endpoint);

//
// Level Control Cluster client
//

/** @brief Level Control Cluster Client Init
 *
 * Client Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfLevelControlClusterClientInitCallback(chip::EndpointId endpoint);

/** @brief Level Control Cluster Client Attribute Changed
 *
 * Client Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfLevelControlClusterClientAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Level Control Cluster Client Manufacturer Specific Attribute Changed
 *
 * Client Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfLevelControlClusterClientManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                                  chip::AttributeId attributeId,
                                                                                  uint16_t manufacturerCode);

/** @brief Level Control Cluster Client Message Sent
 *
 * Client Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfLevelControlClusterClientMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                         EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                         EmberStatus status);

/** @brief Level Control Cluster Client Pre Attribute Changed
 *
 * client Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfLevelControlClusterClientPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                          EmberAfAttributeType attributeType, uint8_t size,
                                                                          uint8_t * value);

/** @brief Level Control Cluster Client Tick
 *
 * client Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfLevelControlClusterClientTickCallback(chip::EndpointId endpoint);

//
// Level Control Cluster server
//

/** @brief Level Control Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfLevelControlClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief Level Control Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfLevelControlClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Level Control Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfLevelControlClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                                  chip::AttributeId attributeId,
                                                                                  uint16_t manufacturerCode);

/** @brief Level Control Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfLevelControlClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                         EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                         EmberStatus status);

/** @brief Level Control Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfLevelControlClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                          EmberAfAttributeType attributeType, uint8_t size,
                                                                          uint8_t * value);

/** @brief Level Control Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfLevelControlClusterServerTickCallback(chip::EndpointId endpoint);

//
// On/off Cluster client
//

/** @brief On/off Cluster Client Init
 *
 * Client Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfOnOffClusterClientInitCallback(chip::EndpointId endpoint);

/** @brief On/off Cluster Client Attribute Changed
 *
 * Client Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfOnOffClusterClientAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief On/off Cluster Client Manufacturer Specific Attribute Changed
 *
 * Client Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfOnOffClusterClientManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                           uint16_t manufacturerCode);

/** @brief On/off Cluster Client Message Sent
 *
 * Client Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfOnOffClusterClientMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                  EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message, EmberStatus status);

/** @brief On/off Cluster Client Pre Attribute Changed
 *
 * client Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfOnOffClusterClientPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                   EmberAfAttributeType attributeType, uint8_t size,
                                                                   uint8_t * value);

/** @brief On/off Cluster Client Tick
 *
 * client Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfOnOffClusterClientTickCallback(chip::EndpointId endpoint);

//
// On/off Cluster server
//

/** @brief On/off Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfOnOffClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief On/off Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfOnOffClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief On/off Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfOnOffClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                           uint16_t manufacturerCode);

/** @brief On/off Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfOnOffClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                  EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message, EmberStatus status);

/** @brief On/off Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfOnOffClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                   EmberAfAttributeType attributeType, uint8_t size,
                                                                   uint8_t * value);

/** @brief On/off Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfOnOffClusterServerTickCallback(chip::EndpointId endpoint);

//
// Scenes Cluster client
//

/** @brief Scenes Cluster Client Init
 *
 * Client Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfScenesClusterClientInitCallback(chip::EndpointId endpoint);

/** @brief Scenes Cluster Client Attribute Changed
 *
 * Client Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfScenesClusterClientAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Scenes Cluster Client Manufacturer Specific Attribute Changed
 *
 * Client Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfScenesClusterClientManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                            chip::AttributeId attributeId,
                                                                            uint16_t manufacturerCode);

/** @brief Scenes Cluster Client Message Sent
 *
 * Client Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfScenesClusterClientMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                   EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                   EmberStatus status);

/** @brief Scenes Cluster Client Pre Attribute Changed
 *
 * client Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfScenesClusterClientPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                    EmberAfAttributeType attributeType, uint8_t size,
                                                                    uint8_t * value);

/** @brief Scenes Cluster Client Tick
 *
 * client Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfScenesClusterClientTickCallback(chip::EndpointId endpoint);

//
// Scenes Cluster server
//

/** @brief Scenes Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfScenesClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief Scenes Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfScenesClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Scenes Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfScenesClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                            chip::AttributeId attributeId,
                                                                            uint16_t manufacturerCode);

/** @brief Scenes Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfScenesClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                   EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                   EmberStatus status);

/** @brief Scenes Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfScenesClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId,
                                                                    EmberAfAttributeType attributeType, uint8_t size,
                                                                    uint8_t * value);

/** @brief Scenes Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfScenesClusterServerTickCallback(chip::EndpointId endpoint);

//
// Temperature Measurement Cluster server
//

/** @brief Temperature Measurement Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfTemperatureMeasurementClusterServerInitCallback(chip::EndpointId endpoint);

/** @brief Temperature Measurement Cluster Server Attribute Changed
 *
 * Server Attribute Changed
 *
 * @param endpoint    Endpoint that is being initialized
 * @param attributeId Attribute that changed
 */
void emberAfTemperatureMeasurementClusterServerAttributeChangedCallback(chip::EndpointId endpoint, chip::AttributeId attributeId);

/** @brief Temperature Measurement Cluster Server Manufacturer Specific Attribute Changed
 *
 * Server Manufacturer Specific Attribute Changed
 *
 * @param endpoint          Endpoint that is being initialized
 * @param attributeId       Attribute that changed
 * @param manufacturerCode  Manufacturer Code of the attribute that changed
 */
void emberAfTemperatureMeasurementClusterServerManufacturerSpecificAttributeChangedCallback(chip::EndpointId endpoint,
                                                                                            chip::AttributeId attributeId,
                                                                                            uint16_t manufacturerCode);

/** @brief Temperature Measurement Cluster Server Message Sent
 *
 * Server Message Sent
 *
 * @param type               The type of message sent
 * @param indexOrDestination The destination or address to which the message was sent
 * @param apsFrame           The APS frame for the message
 * @param msgLen             The length of the message
 * @param message            The message that was sent
 * @param status             The status of the sent message
 */
void emberAfTemperatureMeasurementClusterServerMessageSentCallback(EmberOutgoingMessageType type, uint64_t indexOrDestination,
                                                                   EmberApsFrame * apsFrame, uint16_t msgLen, uint8_t * message,
                                                                   EmberStatus status);

/** @brief Temperature Measurement Cluster Server Pre Attribute Changed
 *
 * server Pre Attribute Changed
 *
 * @param endpoint      Endpoint that is being initialized
 * @param attributeId   Attribute to be changed
 * @param attributeType Attribute type
 * @param size          Attribute size
 * @param value         Attribute value
 */
EmberAfStatus emberAfTemperatureMeasurementClusterServerPreAttributeChangedCallback(chip::EndpointId endpoint,
                                                                                    chip::AttributeId attributeId,
                                                                                    EmberAfAttributeType attributeType,
                                                                                    uint8_t size, uint8_t * value);

/** @brief Temperature Measurement Cluster Server Tick
 *
 * server Tick
 *
 * @param endpoint  Endpoint that is being served
 */
void emberAfTemperatureMeasurementClusterServerTickCallback(chip::EndpointId endpoint);

// Cluster Commands Callback

/**
 * @brief Barrier Control Cluster BarrierControlGoToPercent Command callback
 * @param percentOpen
 */

bool emberAfBarrierControlClusterBarrierControlGoToPercentCallback(uint8_t percentOpen);

/**
 * @brief Barrier Control Cluster BarrierControlStop Command callback
 */

bool emberAfBarrierControlClusterBarrierControlStopCallback();

/**
 * @brief Basic Cluster MfgSpecificPing Command callback
 */

bool emberAfBasicClusterMfgSpecificPingCallback();

/**
 * @brief Basic Cluster ResetToFactoryDefaults Command callback
 */

bool emberAfBasicClusterResetToFactoryDefaultsCallback();

/**
 * @brief Binding Cluster Bind Command callback
 * @param nodeId
 * @param groupId
 * @param endpointId
 * @param clusterId
 */

bool emberAfBindingClusterBindCallback(chip::NodeId nodeId, chip::GroupId groupId, chip::EndpointId endpointId,
                                       chip::ClusterId clusterId);

/**
 * @brief Binding Cluster Unbind Command callback
 * @param nodeId
 * @param groupId
 * @param endpointId
 * @param clusterId
 */

bool emberAfBindingClusterUnbindCallback(chip::NodeId nodeId, chip::GroupId groupId, chip::EndpointId endpointId,
                                         chip::ClusterId clusterId);

/**
 * @brief Color Control Cluster MoveColor Command callback
 * @param rateX
 * @param rateY
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterMoveColorCallback(int16_t rateX, int16_t rateY, uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief Color Control Cluster MoveColorTemperature Command callback
 * @param moveMode
 * @param rate
 * @param colorTemperatureMinimum
 * @param colorTemperatureMaximum
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterMoveColorTemperatureCallback(uint8_t moveMode, uint16_t rate, uint16_t colorTemperatureMinimum,
                                                            uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                                            uint8_t optionsOverride);

/**
 * @brief Color Control Cluster MoveHue Command callback
 * @param moveMode
 * @param rate
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterMoveHueCallback(uint8_t moveMode, uint8_t rate, uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief Color Control Cluster MoveSaturation Command callback
 * @param moveMode
 * @param rate
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterMoveSaturationCallback(uint8_t moveMode, uint8_t rate, uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief Color Control Cluster MoveToColor Command callback
 * @param colorX
 * @param colorY
 * @param transitionTime
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterMoveToColorCallback(uint16_t colorX, uint16_t colorY, uint16_t transitionTime, uint8_t optionsMask,
                                                   uint8_t optionsOverride);

/**
 * @brief Color Control Cluster MoveToColorTemperature Command callback
 * @param colorTemperature
 * @param transitionTime
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterMoveToColorTemperatureCallback(uint16_t colorTemperature, uint16_t transitionTime,
                                                              uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief Color Control Cluster MoveToHue Command callback
 * @param hue
 * @param direction
 * @param transitionTime
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterMoveToHueCallback(uint8_t hue, uint8_t direction, uint16_t transitionTime, uint8_t optionsMask,
                                                 uint8_t optionsOverride);

/**
 * @brief Color Control Cluster MoveToHueAndSaturation Command callback
 * @param hue
 * @param saturation
 * @param transitionTime
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterMoveToHueAndSaturationCallback(uint8_t hue, uint8_t saturation, uint16_t transitionTime,
                                                              uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief Color Control Cluster MoveToSaturation Command callback
 * @param saturation
 * @param transitionTime
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterMoveToSaturationCallback(uint8_t saturation, uint16_t transitionTime, uint8_t optionsMask,
                                                        uint8_t optionsOverride);

/**
 * @brief Color Control Cluster StepColor Command callback
 * @param stepX
 * @param stepY
 * @param transitionTime
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterStepColorCallback(int16_t stepX, int16_t stepY, uint16_t transitionTime, uint8_t optionsMask,
                                                 uint8_t optionsOverride);

/**
 * @brief Color Control Cluster StepColorTemperature Command callback
 * @param stepMode
 * @param stepSize
 * @param transitionTime
 * @param colorTemperatureMinimum
 * @param colorTemperatureMaximum
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterStepColorTemperatureCallback(uint8_t stepMode, uint16_t stepSize, uint16_t transitionTime,
                                                            uint16_t colorTemperatureMinimum, uint16_t colorTemperatureMaximum,
                                                            uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief Color Control Cluster StepHue Command callback
 * @param stepMode
 * @param stepSize
 * @param transitionTime
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterStepHueCallback(uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime, uint8_t optionsMask,
                                               uint8_t optionsOverride);

/**
 * @brief Color Control Cluster StepSaturation Command callback
 * @param stepMode
 * @param stepSize
 * @param transitionTime
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterStepSaturationCallback(uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime,
                                                      uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief Color Control Cluster StopMoveStep Command callback
 * @param optionsMask
 * @param optionsOverride
 */

bool emberAfColorControlClusterStopMoveStepCallback(uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief Door Lock Cluster LockDoorResponse Command callback
 * @param status
 */

bool emberAfDoorLockClusterLockDoorResponseCallback(uint8_t status);

/**
 * @brief Door Lock Cluster UnlockDoorResponse Command callback
 * @param status
 */

bool emberAfDoorLockClusterUnlockDoorResponseCallback(uint8_t status);

/**
 * @brief Door Lock Cluster ClearAllPins Command callback
 */

bool emberAfDoorLockClusterClearAllPinsCallback();

/**
 * @brief Door Lock Cluster ClearAllRfids Command callback
 */

bool emberAfDoorLockClusterClearAllRfidsCallback();

/**
 * @brief Door Lock Cluster ClearHolidaySchedule Command callback
 * @param scheduleId
 */

bool emberAfDoorLockClusterClearHolidayScheduleCallback(uint8_t scheduleId);

/**
 * @brief Door Lock Cluster ClearPin Command callback
 * @param userId
 */

bool emberAfDoorLockClusterClearPinCallback(uint16_t userId);

/**
 * @brief Door Lock Cluster ClearRfid Command callback
 * @param userId
 */

bool emberAfDoorLockClusterClearRfidCallback(uint16_t userId);

/**
 * @brief Door Lock Cluster ClearWeekdaySchedule Command callback
 * @param scheduleId
 * @param userId
 */

bool emberAfDoorLockClusterClearWeekdayScheduleCallback(uint8_t scheduleId, uint16_t userId);

/**
 * @brief Door Lock Cluster ClearYeardaySchedule Command callback
 * @param scheduleId
 * @param userId
 */

bool emberAfDoorLockClusterClearYeardayScheduleCallback(uint8_t scheduleId, uint16_t userId);

/**
 * @brief Door Lock Cluster GetHolidaySchedule Command callback
 * @param scheduleId
 */

bool emberAfDoorLockClusterGetHolidayScheduleCallback(uint8_t scheduleId);

/**
 * @brief Door Lock Cluster GetLogRecord Command callback
 * @param logIndex
 */

bool emberAfDoorLockClusterGetLogRecordCallback(uint16_t logIndex);

/**
 * @brief Door Lock Cluster GetPin Command callback
 * @param userId
 */

bool emberAfDoorLockClusterGetPinCallback(uint16_t userId);

/**
 * @brief Door Lock Cluster GetRfid Command callback
 * @param userId
 */

bool emberAfDoorLockClusterGetRfidCallback(uint16_t userId);

/**
 * @brief Door Lock Cluster GetUserType Command callback
 * @param userId
 */

bool emberAfDoorLockClusterGetUserTypeCallback(uint16_t userId);

/**
 * @brief Door Lock Cluster GetWeekdaySchedule Command callback
 * @param scheduleId
 * @param userId
 */

bool emberAfDoorLockClusterGetWeekdayScheduleCallback(uint8_t scheduleId, uint16_t userId);

/**
 * @brief Door Lock Cluster GetYeardaySchedule Command callback
 * @param scheduleId
 * @param userId
 */

bool emberAfDoorLockClusterGetYeardayScheduleCallback(uint8_t scheduleId, uint16_t userId);

/**
 * @brief Door Lock Cluster LockDoor Command callback
 * @param pin
 */

bool emberAfDoorLockClusterLockDoorCallback(uint8_t * PIN);

/**
 * @brief Door Lock Cluster SetHolidaySchedule Command callback
 * @param scheduleId
 * @param localStartTime
 * @param localEndTime
 * @param operatingModeDuringHoliday
 */

bool emberAfDoorLockClusterSetHolidayScheduleCallback(uint8_t scheduleId, uint32_t localStartTime, uint32_t localEndTime,
                                                      uint8_t operatingModeDuringHoliday);

/**
 * @brief Door Lock Cluster SetPin Command callback
 * @param userId
 * @param userStatus
 * @param userType
 * @param pin
 */

bool emberAfDoorLockClusterSetPinCallback(uint16_t userId, uint8_t userStatus, uint8_t userType, uint8_t * pin);

/**
 * @brief Door Lock Cluster SetRfid Command callback
 * @param userId
 * @param userStatus
 * @param userType
 * @param id
 */

bool emberAfDoorLockClusterSetRfidCallback(uint16_t userId, uint8_t userStatus, uint8_t userType, uint8_t * id);

/**
 * @brief Door Lock Cluster SetUserType Command callback
 * @param userId
 * @param userType
 */

bool emberAfDoorLockClusterSetUserTypeCallback(uint16_t userId, uint8_t userType);

/**
 * @brief Door Lock Cluster SetWeekdaySchedule Command callback
 * @param scheduleId
 * @param userId
 * @param daysMask
 * @param startHour
 * @param startMinute
 * @param endHour
 * @param endMinute
 */

bool emberAfDoorLockClusterSetWeekdayScheduleCallback(uint8_t scheduleId, uint16_t userId, uint8_t daysMask, uint8_t startHour,
                                                      uint8_t startMinute, uint8_t endHour, uint8_t endMinute);

/**
 * @brief Door Lock Cluster SetYeardaySchedule Command callback
 * @param scheduleId
 * @param userId
 * @param localStartTime
 * @param localEndTime
 */

bool emberAfDoorLockClusterSetYeardayScheduleCallback(uint8_t scheduleId, uint16_t userId, uint32_t localStartTime,
                                                      uint32_t localEndTime);

/**
 * @brief Door Lock Cluster UnlockDoor Command callback
 * @param pin
 */

bool emberAfDoorLockClusterUnlockDoorCallback(uint8_t * PIN);

/**
 * @brief Door Lock Cluster UnlockWithTimeout Command callback
 * @param timeoutInSeconds
 * @param pin
 */

bool emberAfDoorLockClusterUnlockWithTimeoutCallback(uint16_t timeoutInSeconds, uint8_t * pin);

/**
 * @brief Groups Cluster AddGroupResponse Command callback
 * @param status
 * @param groupId
 */

bool emberAfGroupsClusterAddGroupResponseCallback(uint8_t status, uint16_t groupId);

/**
 * @brief Groups Cluster GetGroupMembershipResponse Command callback
 * @param capacity
 * @param groupCount
 * @param groupList
 */

bool emberAfGroupsClusterGetGroupMembershipResponseCallback(uint8_t capacity, uint8_t groupCount,
                                                            /* TYPE WARNING: array array defaults to */ uint8_t * groupList);

/**
 * @brief Groups Cluster RemoveGroupResponse Command callback
 * @param status
 * @param groupId
 */

bool emberAfGroupsClusterRemoveGroupResponseCallback(uint8_t status, uint16_t groupId);

/**
 * @brief Groups Cluster ViewGroupResponse Command callback
 * @param status
 * @param groupId
 * @param groupName
 */

bool emberAfGroupsClusterViewGroupResponseCallback(uint8_t status, uint16_t groupId, uint8_t * groupName);

/**
 * @brief Groups Cluster AddGroup Command callback
 * @param groupId
 * @param groupName
 */

bool emberAfGroupsClusterAddGroupCallback(uint16_t groupId, uint8_t * groupName);

/**
 * @brief Groups Cluster AddGroupIfIdentifying Command callback
 * @param groupId
 * @param groupName
 */

bool emberAfGroupsClusterAddGroupIfIdentifyingCallback(uint16_t groupId, uint8_t * groupName);

/**
 * @brief Groups Cluster GetGroupMembership Command callback
 * @param groupCount
 * @param groupList
 */

bool emberAfGroupsClusterGetGroupMembershipCallback(uint8_t groupCount,
                                                    /* TYPE WARNING: array array defaults to */ uint8_t * groupList);

/**
 * @brief Groups Cluster RemoveAllGroups Command callback
 */

bool emberAfGroupsClusterRemoveAllGroupsCallback();

/**
 * @brief Groups Cluster RemoveGroup Command callback
 * @param groupId
 */

bool emberAfGroupsClusterRemoveGroupCallback(uint16_t groupId);

/**
 * @brief Groups Cluster ViewGroup Command callback
 * @param groupId
 */

bool emberAfGroupsClusterViewGroupCallback(uint16_t groupId);

/**
 * @brief IAS Zone Cluster ZoneEnrollResponse Command callback
 * @param enrollResponseCode
 * @param zoneId
 */

bool emberAfIasZoneClusterZoneEnrollResponseCallback(uint8_t enrollResponseCode, uint8_t zoneId);

/**
 * @brief Identify Cluster IdentifyQueryResponse Command callback
 * @param timeout
 */

bool emberAfIdentifyClusterIdentifyQueryResponseCallback(uint16_t timeout);

/**
 * @brief Identify Cluster Identify Command callback
 * @param identifyTime
 */

bool emberAfIdentifyClusterIdentifyCallback(uint16_t identifyTime);

/**
 * @brief Identify Cluster IdentifyQuery Command callback
 */

bool emberAfIdentifyClusterIdentifyQueryCallback();

/**
 * @brief Level Control Cluster Move Command callback
 * @param moveMode
 * @param rate
 * @param optionMask
 * @param optionOverride
 */

bool emberAfLevelControlClusterMoveCallback(uint8_t moveMode, uint8_t rate, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Level Control Cluster MoveToLevel Command callback
 * @param level
 * @param transitionTime
 * @param optionMask
 * @param optionOverride
 */

bool emberAfLevelControlClusterMoveToLevelCallback(uint8_t level, uint16_t transitionTime, uint8_t optionMask,
                                                   uint8_t optionOverride);

/**
 * @brief Level Control Cluster MoveToLevelWithOnOff Command callback
 * @param level
 * @param transitionTime
 */

bool emberAfLevelControlClusterMoveToLevelWithOnOffCallback(uint8_t level, uint16_t transitionTime);

/**
 * @brief Level Control Cluster MoveWithOnOff Command callback
 * @param moveMode
 * @param rate
 */

bool emberAfLevelControlClusterMoveWithOnOffCallback(uint8_t moveMode, uint8_t rate);

/**
 * @brief Level Control Cluster Step Command callback
 * @param stepMode
 * @param stepSize
 * @param transitionTime
 * @param optionMask
 * @param optionOverride
 */

bool emberAfLevelControlClusterStepCallback(uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask,
                                            uint8_t optionOverride);

/**
 * @brief Level Control Cluster StepWithOnOff Command callback
 * @param stepMode
 * @param stepSize
 * @param transitionTime
 */

bool emberAfLevelControlClusterStepWithOnOffCallback(uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime);

/**
 * @brief Level Control Cluster Stop Command callback
 * @param optionMask
 * @param optionOverride
 */

bool emberAfLevelControlClusterStopCallback(uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Level Control Cluster StopWithOnOff Command callback
 */

bool emberAfLevelControlClusterStopWithOnOffCallback();

/**
 * @brief On/off Cluster Off Command callback
 */

bool emberAfOnOffClusterOffCallback();

/**
 * @brief On/off Cluster On Command callback
 */

bool emberAfOnOffClusterOnCallback();

/**
 * @brief On/off Cluster Toggle Command callback
 */

bool emberAfOnOffClusterToggleCallback();

/**
 * @brief Scenes Cluster AddSceneResponse Command callback
 * @param status
 * @param groupId
 * @param sceneId
 */

bool emberAfScenesClusterAddSceneResponseCallback(uint8_t status, uint16_t groupId, uint8_t sceneId);

/**
 * @brief Scenes Cluster GetSceneMembershipResponse Command callback
 * @param status
 * @param capacity
 * @param groupId
 * @param sceneCount
 * @param sceneList
 */

bool emberAfScenesClusterGetSceneMembershipResponseCallback(uint8_t status, uint8_t capacity, uint16_t groupId, uint8_t sceneCount,
                                                            /* TYPE WARNING: array array defaults to */ uint8_t * sceneList);

/**
 * @brief Scenes Cluster RemoveAllScenesResponse Command callback
 * @param status
 * @param groupId
 */

bool emberAfScenesClusterRemoveAllScenesResponseCallback(uint8_t status, uint16_t groupId);

/**
 * @brief Scenes Cluster RemoveSceneResponse Command callback
 * @param status
 * @param groupId
 * @param sceneId
 */

bool emberAfScenesClusterRemoveSceneResponseCallback(uint8_t status, uint16_t groupId, uint8_t sceneId);

/**
 * @brief Scenes Cluster StoreSceneResponse Command callback
 * @param status
 * @param groupId
 * @param sceneId
 */

bool emberAfScenesClusterStoreSceneResponseCallback(uint8_t status, uint16_t groupId, uint8_t sceneId);

/**
 * @brief Scenes Cluster ViewSceneResponse Command callback
 * @param status
 * @param groupId
 * @param sceneId
 * @param transitionTime
 * @param sceneName
 * @param extensionFieldSets
 */

bool emberAfScenesClusterViewSceneResponseCallback(uint8_t status, uint16_t groupId, uint8_t sceneId, uint16_t transitionTime,
                                                   uint8_t * sceneName,
                                                   /* TYPE WARNING: array array defaults to */ uint8_t * extensionFieldSets);

/**
 * @brief Scenes Cluster AddScene Command callback
 * @param groupId
 * @param sceneId
 * @param transitionTime
 * @param sceneName
 * @param extensionFieldSets
 */

bool emberAfScenesClusterAddSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime, uint8_t * sceneName,
                                          /* TYPE WARNING: array array defaults to */ uint8_t * extensionFieldSets);

/**
 * @brief Scenes Cluster GetSceneMembership Command callback
 * @param groupId
 */

bool emberAfScenesClusterGetSceneMembershipCallback(uint16_t groupId);

/**
 * @brief Scenes Cluster RecallScene Command callback
 * @param groupId
 * @param sceneId
 * @param transitionTime
 */

bool emberAfScenesClusterRecallSceneCallback(uint16_t groupId, uint8_t sceneId, uint16_t transitionTime);

/**
 * @brief Scenes Cluster RemoveAllScenes Command callback
 * @param groupId
 */

bool emberAfScenesClusterRemoveAllScenesCallback(uint16_t groupId);

/**
 * @brief Scenes Cluster RemoveScene Command callback
 * @param groupId
 * @param sceneId
 */

bool emberAfScenesClusterRemoveSceneCallback(uint16_t groupId, uint8_t sceneId);

/**
 * @brief Scenes Cluster StoreScene Command callback
 * @param groupId
 * @param sceneId
 */

bool emberAfScenesClusterStoreSceneCallback(uint16_t groupId, uint8_t sceneId);

/**
 * @brief Scenes Cluster ViewScene Command callback
 * @param groupId
 * @param sceneId
 */

bool emberAfScenesClusterViewSceneCallback(uint16_t groupId, uint8_t sceneId);

//
// Non-Cluster Related Callbacks
//

/** @brief Add To Current App Tasks
 *
 * This function is only useful to sleepy end devices.  This function will note
 * the passed item as part of a set of tasks the application has outstanding
 * (e.g. message sent requiring APS acknwoledgement).  This will affect how the
 * application behaves with regard to sleeping and polling.  Until the
 * outstanding task is completed, the device may poll more frequently and sleep
 * less often.
 *
 * @param tasks   Ver.: always
 */
void emberAfAddToCurrentAppTasksCallback(EmberAfApplicationTask tasks);

/** @brief Remove From Current App Tasks
 *
 * This function is only useful to sleepy end devices.  This function will
 * remove the passed item from the set of tasks the application has outstanding
 * (e.g. message sent requiring APS acknwoledgement).  This will affect how the
 * application behaves with regard to sleeping and polling.  Removing the item
 * from the list of outstanding tasks may allow the device to sleep longer and
 * poll less frequently.  If there are other outstanding tasks the system may
 * still have to stay away and poll more often.
 *
 * @param tasks   Ver.: always
 */
void emberAfRemoveFromCurrentAppTasksCallback(EmberAfApplicationTask tasks);

/** @brief Allow Network Write Attribute
 *
 * This function is called by the application framework before it writes an
 * attribute in response to a write attribute request from an external device.
 * The value passed into this callback is the value to which the attribute is to
 * be set by the framework.
        Example:	In mirroring simple metering data
 * on an Energy Services Interface (ESI) (formerly called Energy Service Portal
 * (ESP) in SE 1.0).), a mirrored simple meter needs to write read-only
 * attributes on its mirror. The-meter-mirror sample application, located in
 * app/framework/sample-apps, uses this callback to allow the mirrored device to
 * write simple metering attributes on the mirror regardless of the fact that
 * most simple metering attributes are defined as read-only by the ZigBee
 * specification.
        Note:	The ZCL specification does not (as of this
 * writing) specify any permission-level security for writing writeable
 * attributes. As far as the ZCL specification is concerned, if an attribute is
 * writeable, any device that has a link key for the device should be able to
 * write that attribute. Furthermore if an attribute is read only, it should not
 * be written over the air. Thus, if you implement permissions for writing
 * attributes as a feature, you MAY be operating outside the specification. This
 * is unlikely to be a problem for writing read-only attributes, but it may be a
 * problem for attributes that are writeable according to the specification but
 * restricted by the application implementing this callback.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeId   Ver.: always
 * @param mask   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param value   Ver.: always
 * @param type   Ver.: always
 */
EmberAfAttributeWritePermission emberAfAllowNetworkWriteAttributeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                          chip::AttributeId attributeId, uint8_t mask,
                                                                          uint16_t manufacturerCode, uint8_t * value, uint8_t type);

/** @brief Attribute Read Access
 *
 * This function is called whenever the Application Framework needs to check
 * access permission for an attribute read.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param attributeId   Ver.: always
 */
bool emberAfAttributeReadAccessCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, uint16_t manufacturerCode,
                                        chip::AttributeId attributeId);

/** @brief Attribute Write Access
 *
 * This function is called whenever the Application Framework needs to check
 * access permission for an attribute write.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param attributeId   Ver.: always
 */
bool emberAfAttributeWriteAccessCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, uint16_t manufacturerCode,
                                         chip::AttributeId attributeId);

/** @brief Default Response
 *
 * This function is called by the application framework when a Default Response
 * command is received from an external device.  The application should return
 * true if the message was processed or false if it was not.
 *
 * @param clusterId The cluster identifier of this response.  Ver.: always
 * @param commandId The command identifier to which this is a response.  Ver.:
 * always
 * @param status Specifies either SUCCESS or the nature of the error that was
 * detected in the received command.  Ver.: always
 */
bool emberAfDefaultResponseCallback(chip::ClusterId clusterId, chip::CommandId commandId, EmberAfStatus status);

/** @brief Discover Attributes Response
 *
 * This function is called by the application framework when a Discover
 * Attributes Response or Discover Attributes Extended Response command is
 * received from an external device.  The Discover Attributes Response command
 * contains a bool indicating if discovery is complete and a list of zero or
 * more attribute identifier/type records. The final argument indicates whether
 * the response is in the extended format or not.  The application should return
 * true if the message was processed or false if it was not.
 *
 * @param clusterId The cluster identifier of this response.  Ver.: always
 * @param discoveryComplete Indicates whether there are more attributes to be
 * discovered.  true if there are no more attributes to be discovered.  Ver.:
 * always
 * @param buffer Buffer containing the list of attribute identifier/type
 * records.  Ver.: always
 * @param bufLen The length in bytes of the list.  Ver.: always
 * @param extended Indicates whether the response is in the extended format or
 * not.  Ver.: always
 */
bool emberAfDiscoverAttributesResponseCallback(chip::ClusterId clusterId, bool discoveryComplete, uint8_t * buffer, uint16_t bufLen,
                                               bool extended);

/** @brief Discover Commands Generated Response
 *
 * This function is called by the framework when Discover Commands Generated
 * Response is received.
 *
 * @param clusterId The cluster identifier of this response.  Ver.: always
 * @param manufacturerCode Manufacturer code  Ver.: always
 * @param discoveryComplete Indicates whether there are more commands to be
 * discovered.  Ver.: always
 * @param commandIds Buffer containing the list of command identifiers.  Ver.:
 * always
 * @param commandIdCount The length of bytes of the list, whish is the same as
 * the number of identifiers.  Ver.: always
 */
bool emberAfDiscoverCommandsGeneratedResponseCallback(chip::ClusterId clusterId, uint16_t manufacturerCode, bool discoveryComplete,
                                                      chip::CommandId * commandIds, uint16_t commandIdCount);

/** @brief Discover Commands Received Response
 *
 * This function is called by the framework when Discover Commands Received
 * Response is received.
 *
 * @param clusterId The cluster identifier of this response.  Ver.: always
 * @param manufacturerCode Manufacturer code  Ver.: always
 * @param discoveryComplete Indicates whether there are more commands to be
 * discovered.  Ver.: always
 * @param commandIds Buffer containing the list of command identifiers.  Ver.:
 * always
 * @param commandIdCount The length of bytes of the list, whish is the same as
 * the number of identifiers.  Ver.: always
 */
bool emberAfDiscoverCommandsReceivedResponseCallback(chip::ClusterId clusterId, uint16_t manufacturerCode, bool discoveryComplete,
                                                     chip::CommandId * commandIds, uint16_t commandIdCount);

/** @brief Pre Command Received
 *
 * This callback is the second in the Application Framework's message processing
 * chain. At this point in the processing of incoming over-the-air messages, the
 * application has determined that the incoming message is a ZCL command. It
 * parses enough of the message to populate an EmberAfClusterCommand struct. The
 * Application Framework defines this struct value in a local scope to the
 * command processing but also makes it available through a global pointer
 * called emberAfCurrentCommand, in app/framework/util/util.c. When command
 * processing is complete, this pointer is cleared.
 *
 * @param cmd   Ver.: always
 */
bool emberAfPreCommandReceivedCallback(EmberAfClusterCommand * cmd);

/** @brief Pre Message Send
 *
 * This function is called by the framework when it is about to pass a message
 * to the stack primitives for sending.   This message may or may not be ZCL,
 * ZDO, or some other protocol.  This is called prior to
        any ZigBee
 * fragmentation that may be done.  If the function returns true it is assumed
 * the callback has consumed and processed the message.  The callback must also
 * set the EmberStatus status code to be passed back to the caller.  The
 * framework will do no further processing on the message.
        If the
 * function returns false then it is assumed that the callback has not processed
 * the mesasge and the framework will continue to process accordingly.
 *
 * @param messageStruct The structure containing the parameters of the APS
 * message to be sent.  Ver.: always
 * @param status A pointer to the status code value that will be returned to the
 * caller.  Ver.: always
 */
bool emberAfPreMessageSendCallback(EmberAfMessageStruct * messageStruct, EmberStatus * status);

/** @brief Message Sent
 *
 * This function is called by the application framework from the message sent
 * handler, when it is informed by the stack regarding the message sent status.
 * All of the values passed to the emberMessageSentHandler are passed on to this
 * callback. This provides an opportunity for the application to verify that its
 * message has been sent successfully and take the appropriate action. This
 * callback should return a bool value of true or false. A value of true
 * indicates that the message sent notification has been handled and should not
 * be handled by the application framework.
 *
 * @param type   Ver.: always
 * @param indexOrDestination   Ver.: always
 * @param apsFrame   Ver.: always
 * @param msgLen   Ver.: always
 * @param message   Ver.: always
 * @param status   Ver.: always
 */
bool emberAfMessageSentCallback(EmberOutgoingMessageType type, uint16_t indexOrDestination, EmberApsFrame * apsFrame,
                                uint16_t msgLen, uint8_t * message, EmberStatus status);

/** @brief Pre Attribute Change
 *
 * This function is called by the application framework before it changes an
 * attribute value.  The value passed into this callback is the value to which
 * the attribute is to be set by the framework.  The application should return
 * ::EMBER_ZCL_STATUS_SUCCESS to permit the change or any other ::EmberAfStatus
 * to reject it.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeId   Ver.: always
 * @param mask   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param type   Ver.: always
 * @param size   Ver.: always
 * @param value   Ver.: always
 */
EmberAfStatus emberAfPreAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                                uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint8_t size,
                                                uint8_t * value);

/** @brief Post Attribute Change
 *
 * This function is called by the application framework after it changes an
 * attribute value. The value passed into this callback is the value to which
 * the attribute was set by the framework.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeId   Ver.: always
 * @param mask   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param type   Ver.: always
 * @param size   Ver.: always
 * @param value   Ver.: always
 */
void emberAfPostAttributeChangeCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                        uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value);

/** @brief Read Attributes Response
 *
 * This function is called by the application framework when a Read Attributes
 * Response command is received from an external device.  The application should
 * return true if the message was processed or false if it was not.
 *
 * @param clusterId The cluster identifier of this response.  Ver.: always
 * @param buffer Buffer containing the list of read attribute status records.
 * Ver.: always
 * @param bufLen The length in bytes of the list.  Ver.: always
 */
bool emberAfReadAttributesResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen);

/** @brief External Attribute Read
 *
 * Like emberAfExternalAttributeWriteCallback above, this function is called
 * when the framework needs to read an attribute that is not stored within the
 * Application Framework's data structures.
        All of the important
 * information about the attribute itself is passed as a pointer to an
 * EmberAfAttributeMetadata struct, which is stored within the application and
 * used to manage the attribute. A complete description of the
 * EmberAfAttributeMetadata struct is provided in
 * app/framework/include/af-types.h
        This function assumes that the
 * application is able to read the attribute, write it into the passed buffer,
 * and return immediately. Any attributes that require a state machine for
 * reading and writing are not really candidates for externalization at the
 * present time. The Application Framework does not currently include a state
 * machine for reading or writing attributes that must take place across a
 * series of application ticks. Attributes that cannot be read in a timely
 * manner should be stored within the Application Framework and updated
 * occasionally by the application code from within the
 * emberAfMainTickCallback.
        If the application was successfully able to
 * read the attribute and write it into the passed buffer, it should return a
 * value of EMBER_ZCL_STATUS_SUCCESS. Ensure that the size of the externally
 * managed attribute value is smaller than what the buffer can hold. In the case
 * of a buffer overflow throw an appropriate error such as
 * EMBER_ZCL_STATUS_INSUFFICIENT_SPACE. Any other return value indicates the
 * application was not able to read the attribute.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeMetadata   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param buffer   Ver.: always
 * @param maxReadLength   Ver.: always
 */
EmberAfStatus emberAfExternalAttributeReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                   EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                   uint8_t * buffer, uint16_t maxReadLength);

/** @brief Write Attributes Response
 *
 * This function is called by the application framework when a Write Attributes
 * Response command is received from an external device.  The application should
 * return true if the message was processed or false if it was not.
 *
 * @param clusterId The cluster identifier of this response.  Ver.: always
 * @param buffer Buffer containing the list of write attribute status records.
 * Ver.: always
 * @param bufLen The length in bytes of the list.  Ver.: always
 */
bool emberAfWriteAttributesResponseCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen);

/** @brief External Attribute Write
 *
 * This function is called whenever the Application Framework needs to write an
 * attribute which is not stored within the data structures of the Application
 * Framework itself. One of the new features in Version 2 is the ability to
 * store attributes outside the Framework. This is particularly useful for
 * attributes that do not need to be stored because they can be read off the
 * hardware when they are needed, or are stored in some central location used by
 * many modules within the system. In this case, you can indicate that the
 * attribute is stored externally. When the framework needs to write an external
 * attribute, it makes a call to this callback.
        This callback is very
 * useful for host micros which need to store attributes in persistent memory.
 * Because each host micro (used with an Ember NCP) has its own type of
 * persistent memory storage, the Application Framework does not include the
 * ability to mark attributes as stored in flash the way that it does for Ember
 * SoCs like the EM35x. On a host micro, any attributes that need to be stored
 * in persistent memory should be marked as external and accessed through the
 * external read and write callbacks. Any host code associated with the
 * persistent storage should be implemented within this callback.
        All of
 * the important information about the attribute itself is passed as a pointer
 * to an EmberAfAttributeMetadata struct, which is stored within the application
 * and used to manage the attribute. A complete description of the
 * EmberAfAttributeMetadata struct is provided in
 * app/framework/include/af-types.h.
        This function assumes that the
 * application is able to write the attribute and return immediately. Any
 * attributes that require a state machine for reading and writing are not
 * candidates for externalization at the present time. The Application Framework
 * does not currently include a state machine for reading or writing attributes
 * that must take place across a series of application ticks. Attributes that
 * cannot be written immediately should be stored within the Application
 * Framework and updated occasionally by the application code from within the
 * emberAfMainTickCallback.
        If the application was successfully able to
 * write the attribute, it returns a value of EMBER_ZCL_STATUS_SUCCESS. Any
 * other return value indicates the application was not able to write the
 * attribute.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 * @param attributeMetadata   Ver.: always
 * @param manufacturerCode   Ver.: always
 * @param buffer   Ver.: always
 */
EmberAfStatus emberAfExternalAttributeWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                    EmberAfAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                    uint8_t * buffer);

/** @brief Report Attributes
 *
 * This function is called by the application framework when a Report Attributes
 * command is received from an external device.  The application should return
 * true if the message was processed or false if it was not.
 *
 * @param clusterId The cluster identifier of this command.  Ver.: always
 * @param buffer Buffer containing the list of attribute report records.  Ver.:
 * always
 * @param bufLen The length in bytes of the list.  Ver.: always
 */
bool emberAfReportAttributesCallback(chip::ClusterId clusterId, uint8_t * buffer, uint16_t bufLen);

/** @brief Get Current Time
 *
 * This callback is called when device attempts to get current time from the
 * hardware. If this device has means to retrieve exact time, then this method
 * should implement it. If the callback can't provide the exact time it should
 * return 0 to indicate failure. Default action is to return 0, which indicates
 * that device does not have access to real time.
 *
 */
uint32_t emberAfGetCurrentTimeCallback();

/** @brief Get Endpoint Info
 *
 * This function is a callback to an application implemented endpoint that
 * operates outside the normal application framework.  When the framework wishes
 * to perform operations with that endpoint it uses this callback to retrieve
 * the endpoint's information.  If the endpoint exists and the application can
 * provide data then true shall be returned.  Otherwise the callback must return
 * false.
 *
 * @param endpoint The endpoint to retrieve data for.  Ver.: always
 * @param returnNetworkIndex The index corresponding to the ZigBee network the
 * endpoint belongs to.  If not using a multi-network device, 0 must be
 * returned.  Otherwise on a multi-network device the stack will switch to this
 * network before sending the message.  Ver.: always
 * @param returnEndpointInfo A pointer to a data struct that will be written
 * with information about the endpoint.  Ver.: always
 */
bool emberAfGetEndpointInfoCallback(chip::EndpointId endpoint, uint8_t * returnNetworkIndex,
                                    EmberAfEndpointInfoStruct * returnEndpointInfo);

/** @brief Get Source Route Overhead
 *
 * This function is called by the framework to determine the overhead required
 * in the network frame for source routing to a particular destination.
 *
 * @param destination The node id of the destination  Ver.: always
 */
uint8_t emberAfGetSourceRouteOverheadCallback(EmberNodeId destination);

/** @brief Registration Abort
 *
 * This callback is called when the device should abort the registration
 * process.
 *
 */
void emberAfRegistrationAbortCallback();

/** @brief Interpan Send Message
 *
 * This function will send a raw MAC message with interpan frame format using
 * the passed parameters.
 *
 * @param header Interpan header info  Ver.: always
 * @param messageLength The length of the message received or to send  Ver.:
 * always
 * @param message The message data received or to send.  Ver.: always
 */
EmberStatus emberAfInterpanSendMessageCallback(EmberAfInterpanHeader * header, uint16_t messageLength, uint8_t * message);

/** @brief Start Move
 *
 * This function is called to initiate the process for a device to move (rejoin)
 * to a new parent.
 *
 */
bool emberAfStartMoveCallback();
