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

/** @brief On/off Cluster Init
 *
 * Cluster Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfOnOffClusterInitCallback(chip::EndpointId endpoint);

// Cluster Server/Client Init Functions

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

// Cluster Commands Callback

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
