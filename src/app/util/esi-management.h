/**
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************/
/**
 * @file
 * @brief It implements and manages the ESI table. The
 *ESI table is shared among other plugins.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include "af.h"

#ifndef EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE
#define EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE 3
#endif // EMBER_AF_PLUGIN_ESI_MANAGEMENT_ESI_TABLE_SIZE

#ifndef EMBER_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE
#define EMBER_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE 3
#endif // EMBER_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE

#ifndef EMBER_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE
#define EMBER_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE 5
#endif // EMBER_AF_PLUGIN_ESI_MANAGEMENT_PLUGIN_CALLBACK_TABLE_SIZE

typedef struct
{
    EmberEUI64 eui64;
    chip::NodeId nodeId;
    uint8_t networkIndex;
    chip::EndpointId endpoint;
    uint8_t age; // The number of discovery cycles the ESI has not been discovered.
} EmberAfPluginEsiManagementEsiEntry;

typedef void (*EmberAfEsiManagementDeletionCallback)(uint8_t);

/**
 * Searches in the ESI table by the pair node (short ID, endopoint).
 *
 * Returns a pointer to the entry if a matching entry was found, otherwise it
 * returns NULL.
 */
EmberAfPluginEsiManagementEsiEntry * emberAfPluginEsiManagementEsiLookUpByShortIdAndEndpoint(EmberNodeId shortId,
                                                                                             chip::EndpointId endpoint);

/**
 * Searches in the ESI table by the pair node (long ID, endopoint).
 *
 * Returns a pointer to the entry if a matching entry was found, otherwise it
 * returns NULL.
 */
EmberAfPluginEsiManagementEsiEntry * emberAfPluginEsiManagementEsiLookUpByLongIdAndEndpoint(EmberEUI64 longId,
                                                                                            chip::EndpointId endpoint);

/**
 * Allows retrieving the index of an entry that matches the passed short ID
 * and endpoint.
 *
 * It returns the index of the matching entry if a matching entry was found,
 * otherwise it returns 0xFF.
 */
uint8_t emberAfPluginEsiManagementIndexLookUpByShortIdAndEndpoint(EmberNodeId shortId, chip::EndpointId endpoint);

/**
 * Allows retrieving the index of an entry that matches the passed long ID
 * and endpoint.
 *
 * It returns the index of the matching entry if a matching entry was found,
 * otherwise it returns 0xFF.
 */
uint8_t emberAfPluginEsiManagementIndexLookUpByLongIdAndEndpoint(EmberEUI64 longId, chip::EndpointId endpoint);

/**
 * Searches in the ESI table by the table index.
 *
 * Returns a pointer to the ESI entry stored at the index passed as
 * parameter.
 */
EmberAfPluginEsiManagementEsiEntry * emberAfPluginEsiManagementEsiLookUpByIndex(uint8_t index);

/**
 * Iterates through the entries in the
 * table that are within a certain age threshold.
 *
 * If the passed pointer is NULL, it returns the first active entry with age
 * lesser or equal than the passed age parameter (if any). Otherwise, it returns
 * the next active entry that satisfy the age requirement. If the are no entries
 * after the passed entry that satisfy the age requirement, it returns NULL.
 */
EmberAfPluginEsiManagementEsiEntry * emberAfPluginEsiManagementGetNextEntry(EmberAfPluginEsiManagementEsiEntry * entry,
                                                                            uint8_t age);

/**
 * Allows obtaining a free entry in the ESI table. It is the
 * requester responsibility to properly set all the fields in the obtained free
 * entry such as nodeId, age, and so on to avoid inconsistencies in the
 * table.
 *
 * Returns a free entry (if any), otherwise it clears the oldest entry whose age
 * is at least EMBER_AF_PLUGIN_ESI_MANAGEMENT_MIN_ERASING_AGE (if any) and
 * returns it, otherwise it returns NULL.
 */
EmberAfPluginEsiManagementEsiEntry * emberAfPluginEsiManagementGetFreeEntry(void);

/**
 * Deletes the entry indicated by the parameter 'index' from the
 * ESI table.
 */
void emberAfPluginEsiManagementDeleteEntry(uint8_t index);

/**
 * Increases the age of all the active entries in the table. A
 * non-active entry is an entry whose short ID is set to EMBER_NULL_NODE_ID.
 */
void emberAfPluginEsiManagementAgeAllEntries(void);

/**
 * Clears the ESI table, i.e., it sets the short ID of each entry
 * to EMBER_NULL_NODE_ID.
 */
void emberAfPluginEsiManagementClearTable(void);

/**
 * Allows a plugin to subscribe to ESI entries deletion
 * announcements by passing its own deletion callback function. Upon an entry
 * deletion, all the deletion callback function are called passing the index
 * of the deleted entry.
 *
 * It returns true if the subscription was successful, false otherwise.
 */
bool emberAfPluginEsiManagementSubscribeToDeletionAnnouncements(EmberAfEsiManagementDeletionCallback callback);

/**
 * Performs the following steps:
 *  - Search for the source node of the passed command in the ESI table.
 *  - Adds a new entry in the ESI table if the source node is not present in the
 *    ESI table yet, or updates the current entry if needed.
 *
 *  @return The index of the source node of the passed command in the ESI
 *  table, or it returns 0xFF if the ESI was not present in the table and a new
 *  entry could not be added since the table was full.
 **/
uint8_t emberAfPluginEsiManagementUpdateEsiAndGetIndex(const EmberAfClusterCommand * cmd);
