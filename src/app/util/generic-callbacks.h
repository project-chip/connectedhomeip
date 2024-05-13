/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <app/util/af-types.h>
#include <app/util/attribute-metadata.h>
#include <app/util/basic-types.h>

#include <app/ConcreteAttributePath.h>
#include <protocols/interaction_model/Constants.h>

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

/** @brief Attribute Read Access
 *
 * This function is called whenever the Application Framework needs to check
 * access permission for an attribute read.
 */
bool emberAfAttributeReadAccessCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);

/** @brief Attribute Write Access
 *
 * This function is called whenever the Application Framework needs to check
 * access permission for an attribute write.
 */
bool emberAfAttributeWriteAccessCallback(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);

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
 * value of InteractionModel::Status::Success. Ensure that the size of the externally
 * managed attribute value is smaller than what the buffer can hold. In the case
 * of a buffer overflow throw an appropriate error such as
 * InteractionModel::Status::ResourceExhausted. Any other return value indicates the
 * application was not able to read the attribute.
 */
chip::Protocols::InteractionModel::Status emberAfExternalAttributeReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                               const EmberAfAttributeMetadata * attributeMetadata,
                                                                               uint8_t * buffer, uint16_t maxReadLength);

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
 * write the attribute, it returns a value of InteractionModel::Status::Success. Any
 * other return value indicates the application was not able to write the
 * attribute.
 */
chip::Protocols::InteractionModel::Status emberAfExternalAttributeWriteCallback(chip::EndpointId endpoint,
                                                                                chip::ClusterId clusterId,
                                                                                const EmberAfAttributeMetadata * attributeMetadata,
                                                                                uint8_t * buffer);

/** @brief Pre Attribute Change
 *
 * This function is called by the application framework before it changes an
 * attribute value.  The value passed into this callback is the value to which
 * the attribute is to be set by the framework.  The application should return
 * Protocols::InteractionModel::Status::Success to permit the change or
 * any other code to reject it.
 */
chip::Protocols::InteractionModel::Status MatterPreAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath,
                                                                           uint8_t type, uint16_t size, uint8_t * value);

/** @brief Post Attribute Change
 *
 * This function is called by the application framework after it changes an
 * attribute value. The value passed into this callback is the value to which
 * the attribute was set by the framework.
 */
void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value);
