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
/***************************************************************************//**
 * @file
 * @brief Routines for the Green Power Client plugin.
 *******************************************************************************
   ******************************************************************************/

#include "enums.h"
#include "app/framework/include/af.h"
#include "app/framework/util/af-main.h"
#include "app/framework/util/common.h"
#include "green-power-client.h"
#include "stack/include/gp-types.h"
#include "stack/gp/gp-proxy-table.h"
#include "app/framework/plugin/green-power-common/green-power-common.h"

#ifdef EZSP_HOST
  #define emberGpepIncomingMessageHandler ezspGpepIncomingMessageHandler
  #define emberDGpSentHandler ezspDGpSentHandler
#endif

#define MIN_RESERVED_GPD_COMMAND_ID 0xE4
#define MAX_RESERVED_GPD_COMMAND_ID 0xEF
#define EMBER_ZCL_GP_GPDF_RESERVED_MIN 0xF7
#define EMBER_ZCL_GP_GPDF_RESERVED_MAX  0xFF

#define GP_PROXY_TABLE_RESPONSE_ENTRIES_OFFSET 3
#define GP_NON_MANUFACTURER_ZCL_HEADER_LENGTH 3

// Channel event timeout is 5 seconds
#define GP_CHANNEL_EVENT_TIMEOUT_IN_MSEC  (5 * MILLISECOND_TICKS_PER_SECOND)

#define isNodeIdValid(nodeId) ((nodeId) < EMBER_DISCOVERY_ACTIVE_NODE_ID)

void emSetAddDelay(uint16_t delay);

static EmberAfGreenPowerClientCommissioningState commissioningState;
static EmberAfGreenPowerDuplicateFilter duplicateFilter;
EmberEventControl emberAfPluginGreenPowerClientExpireSeqNumEventControl;
EmberEventControl emberAfPluginGreenPowerClientExitCommissioningEventControl;
EmberEventControl emberAfPluginGreenPowerClientChannelEventControl;

void emberAfPluginGreenPowerClientInitCallback(void)
{
  MEMSET(&commissioningState,
         0x00,
         sizeof(EmberAfGreenPowerClientCommissioningState));
  MEMSET(&duplicateFilter, 0x00, sizeof(EmberAfGreenPowerDuplicateFilter));

  commissioningState.gppCommissioningWindow = EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_GPP_COMMISSIONING_WINDOW;

  // Write the max proxy table entries attribute, since we currently have no way
  // to configure it at compile time.
  uint8_t proxyTableSize = EMBER_GP_PROXY_TABLE_SIZE;
  EmberAfStatus UNUSED status
    = emberAfWriteClientAttribute(GP_ENDPOINT,
                                  ZCL_GREEN_POWER_CLUSTER_ID,
                                  ZCL_GP_CLIENT_GPP_MAX_PROXY_TABLE_ENTRIES_ATTRIBUTE_ID,
                                  &proxyTableSize,
                                  ZCL_INT8U_ATTRIBUTE_TYPE);
}

void emberAfGreenPowerClusterExitCommissioningMode(void)
{
  EmberGpAddress addr;
  commissioningState.inCommissioningMode = false;
  emberAfGreenPowerClusterPrintln("Exit comm for sink %2x", commissioningState.commissioningSink);

  // Use 0xFFFFFFFF wildcard to clear out the gpTxQueue
  addr.applicationId = EMBER_GP_APPLICATION_SOURCE_ID;
  addr.id.sourceId = 0xFFFFFFFF;
  emberDGpSend(false, false, &addr, 0, 0, NULL, 0, 0);
  //  TODO:  remove sender information in stack.
}

void emberAfPluginGreenPowerClientChannelEventHandler(void)
{
  if (commissioningState.channelStatus & GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK) {
    EmberGpAddress addr;
    addr.id.sourceId = 0xFFFFFFFF;
    addr.applicationId = 0;
    emberAfGreenPowerClusterPrintln("return to chan %d", emberAfGetRadioChannel());
    emberStopScan();
    commissioningState.channelStatus = 0;
    //clear the gpTxQueue
    emberDGpSend(false, false, &addr, 0, 0, NULL, 0, 0);
  }
  emberEventControlSetInactive(emberAfPluginGreenPowerClientChannelEventControl);
}

void emberAfPluginGreenPowerClientExitCommissioningEventHandler(void)
{
  emberAfGreenPowerClusterExitCommissioningMode();
  emberEventControlSetInactive(emberAfPluginGreenPowerClientExitCommissioningEventControl);
}

/*
 * Disable default response bit should be set per GP Spec 14-0563-08
 */
bool emberAfGreenPowerClusterGpProxyCommissioningModeCallback(uint8_t options,
                                                              uint16_t commissioningWindow,
                                                              uint8_t channel)
{
  bool enterCommissioningMode = (options
                                 & EMBER_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_ACTION);
  // The exitMode enumeration type is mapped as [b3b2b1] = [b3b2] | [b1] of `options` fields in the Gp Proxy Commissioning Mode
  // command where (b3-b2) for `ExitMode` fields and b1 is the exit on commissioning window time expiry.
  uint8_t exitMode = (options                                                                         \
                      & (EMBER_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_EXIT_MODE                        \
                         | EMBER_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_COMMISSIONING_WINDOW_PRESENT)) \
                     >> 1;

  bool chanPresent = (options
                      & EMBER_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_CHANNEL_PRESENT); // should always be 0
  bool unicastCommunication = (options
                               & EMBER_AF_GP_PROXY_COMMISSIONING_MODE_OPTION_UNICAST_COMMUNICATION);

  if (emberAfCurrentEndpoint() != GP_ENDPOINT) {
  } else if (commissioningState.inCommissioningMode
             && (emberGetSender() != commissioningState.commissioningSink)) {
    // check if current message sender is same as sender that put us in
    // commissioning mode.
    // if not, drop message silently.
  } else if (enterCommissioningMode) {
    commissioningState.commissioningSink = emberGetSender();
    commissioningState.inCommissioningMode = true;
    commissioningState.gppCommissioningWindow = EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_GPP_COMMISSIONING_WINDOW;
    commissioningState.exitMode = (EmberAfGreenPowerClientCommissioningExitMode) exitMode;
    commissioningState.unicastCommunication = unicastCommunication;
    emberAfGreenPowerClusterPrintln("Enter comm for sink %2x", commissioningState.commissioningSink);
    if (commissioningState.exitMode & EMBER_AF_GPC_COMMISSIONING_EXIT_ON_COMMISSIONING_WINDOW_EXP) {
      // store the new commissioning window value and update our local default
      // one.
      emberEventControlSetDelayMS(emberAfPluginGreenPowerClientExitCommissioningEventControl,
                                  (commissioningWindow * MILLISECOND_TICKS_PER_SECOND));
    } else {
      emberEventControlSetDelayMS(emberAfPluginGreenPowerClientExitCommissioningEventControl,
                                  (EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_GPP_COMMISSIONING_WINDOW * MILLISECOND_TICKS_PER_SECOND));
    }
    if (chanPresent) {
      commissioningState.channel = channel;
      // TODO: goto channel
    } else {
      // TODO: goto operational channel
    }
  } else {
    // exit commissioning mode.
    emberAfGreenPowerClusterExitCommissioningMode();
  }
  return true;
}

bool emberAfGreenPowerClusterGpNotificationResponseCallback(uint8_t options,
                                                            uint32_t gpdSrcId,
                                                            uint8_t* gpdIeee,
                                                            uint8_t endpoint,
                                                            uint32_t gpdSecurityFrameCounter)
{
  return false;
}

/*
 * Disable default response should be enabled in the sending message.
 */
bool emberAfGreenPowerClusterGpPairingCallback(uint32_t options, // actually a int24u.
                                               uint32_t gpdSrcId,
                                               uint8_t* gpdIeee,
                                               uint8_t endpoint,
                                               uint8_t* sinkIeeeAddress,
                                               uint16_t sinkNwkAddress,
                                               uint16_t sinkGroupId,
                                               uint8_t deviceId,
                                               uint32_t gpdSecurityFrameCounter,
                                               uint8_t* gpdKey,
                                               uint16_t assignedAlias,
                                               uint8_t groupcastRadius)
{
  uint8_t allZeroesIeeeAddress[17] = { 0 };
  uint8_t securityLevel = (options & EMBER_AF_GP_PAIRING_OPTION_SECURITY_LEVEL)
                          >> EMBER_AF_GP_PAIRING_OPTION_SECURITY_LEVEL_OFFSET;
  uint8_t commMode = ((options & EMBER_AF_GP_PAIRING_OPTION_COMMUNICATION_MODE)
                      >> EMBER_AF_GP_PAIRING_OPTION_COMMUNICATION_MODE_OFFSET);
  uint8_t securityKeyType = ((options & EMBER_AF_GP_PAIRING_OPTION_SECURITY_KEY_TYPE)
                             >> EMBER_AF_GP_PAIRING_OPTION_SECURITY_KEY_TYPE_OFFSET);
  EmberGpAddress addr;
  bool createdPairing = false;
  bool broadcast = false;
  // If "option & addSinkBit(3) == 1" AND "groupcastRadiusBit(17)==0"
  // then groupcastRadius = 0 not 0xFF that was passed by call-command-handler
  if ((options & EMBER_AF_GP_PAIRING_OPTION_ADD_SINK)
      && !(options & EMBER_AF_GP_PAIRING_OPTION_GROUPCAST_RADIUS_PRESENT)) {
    groupcastRadius = 0;
  }

  EmberAfClusterCommand *cmd =  emberAfCurrentCommand();
  if (cmd->type == EMBER_INCOMING_BROADCAST) {
    broadcast = true;
  }

  //handle null args for EZSP
  if (gpdIeee  == NULL) {
    gpdIeee = allZeroesIeeeAddress;
  }
  if (sinkIeeeAddress  == NULL) {
    sinkIeeeAddress = allZeroesIeeeAddress;
  }
  if (gpdKey  == NULL) {
    gpdKey = allZeroesIeeeAddress;
  }

  if (!emGpMakeAddr(&addr,
                    options & EMBER_AF_GP_PAIRING_OPTION_APPLICATION_ID,
                    gpdSrcId,
                    gpdIeee,
                    endpoint)) {
    // GPD Address is invalid, so drop the command
    return true;
  }

  emberAfCorePrint("GP Pairing\n");

  // Commissioning: Step 21a: Proxy finalizes commissioning
  if (emberAfCurrentEndpoint() != GP_ENDPOINT) {
  } else if (securityLevel == EMBER_GP_SECURITY_LEVEL_RESERVED) {
  } else if ((addr.applicationId != EMBER_GP_APPLICATION_SOURCE_ID)
             && (addr.applicationId != EMBER_GP_APPLICATION_IEEE_ADDRESS)) {
  } else if ((addr.applicationId == EMBER_GP_APPLICATION_SOURCE_ID)
             && ((addr.id.sourceId == GP_GPD_SRC_ID_RESERVED_0)
                 || ((addr.id.sourceId >= GP_GPD_SRC_ID_RESERVED_FFFFFF9)
                     && (addr.id.sourceId <= GP_GPD_SRC_ID_RESERVED_FFFFFFE)))) {
  } else if ((addr.applicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS)
             && ((emberAfMemoryByteCompare(addr.id.gpdIeeeAddress, EUI64_SIZE, 0)))) {
  } else if ((options & EMBER_AF_GP_PAIRING_OPTION_ADD_SINK)
             && (options & EMBER_AF_GP_PAIRING_OPTION_REMOVE_GPD)) {
  } else if ((securityKeyType > EMBER_GP_SECURITY_KEY_GPD_OOB)
             &&  (securityKeyType < EMBER_GP_SECURITY_KEY_GPD_DERIVED)) {
  } else if ((commMode == EMBER_ZCL_GP_PAIRING_OPTIONS_COMMUNICATION_MODE_FULL_UNICAST_FORWARDING)
             && (!(options & EMBER_AF_GP_PAIRING_OPTION_REMOVE_GPD))) {
    // CCB #2278 -In case of unicast GP Pairing, the proxy SHALL respond with ZCL Default
    //  Response with relevant error code: INVALID_FIELD for Full unicast;
    //  -In case of broadcast GP Pairing, the proxy SHALL silently drop the frame.
    //  If RemoveGPD options bit is set the comms mode is not valid so ignore this test
    if (broadcast == true) {
      // Silently drop it as its a broadcast
    } else {
      //send default response for unicast pairing
      emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INVALID_FIELD);
    }
  } else {
    // Step b:

    emberAfGreenPowerClusterPrintln("GP Pairing SRCID = %4x, Opt= %u Sink = %2x", addr.id.sourceId, options, sinkNwkAddress);

    createdPairing = emberGpProxyTableProcessGpPairing(options,
                                                       &addr,
                                                       commMode,
                                                       sinkNwkAddress,
                                                       sinkGroupId,
                                                       assignedAlias,
                                                       sinkIeeeAddress,
                                                       (EmberKeyData *) gpdKey,
                                                       gpdSecurityFrameCounter,
                                                       groupcastRadius);

    // step c: optionally, exit commissioning mode
    if (createdPairing
        && (commissioningState.exitMode & EMBER_AF_GPC_COMMISSIONING_EXIT_ON_FIRST_PAIRING_SUCCESS)) {
      emberAfGreenPowerClusterExitCommissioningMode();
    } else if ((options & EMBER_AF_GP_PAIRING_OPTION_ADD_SINK) && !createdPairing) {
#ifndef EZSP_HOST
      uint8_t index = emGpProxyTableGetFreeEntryIndex();  //emGpProxyTableFindOrAllocateEntry(&addr);   //checking if pairing failed because of no
      // space
      if (index == 0xFF) {
        emberAfGreenPowerClusterPrintln("ERR PROXY TABLE FULL");
        if (broadcast != true) {
          // CCB # 2279 - only send default response if not broadcast - broadcasts are dropped
          emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INSUFFICIENT_SPACE);
        }
      }
    }
#else
      // Checking if the proxy table is full is more of a pain on Host
      // We have to search through the whole table.
      // Because of this, we only do it for the non-broadcast case
      // Next time we bump the EZSP version, we should change the
      // return of Process GP Pairing to a status instead of a bool
      if (!broadcast) {
        EmberGpProxyTableEntry entry;
        for (uint8_t entryIndex = 0; entryIndex < EMBER_GP_PROXY_TABLE_SIZE; entryIndex++) {
          emberGpProxyTableGetEntry(entryIndex, &entry);
          if (entry.status == EMBER_GP_PROXY_TABLE_ENTRY_STATUS_UNUSED) {
            return true; // We have an unused entry so we don't need to send a DR for full
          }
        }
        emberAfGreenPowerClusterPrintln("ERR PROXY TABLE FULL");
        emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_INSUFFICIENT_SPACE);
      }
    }
#endif
  }
  return true;
}

bool emberAfGreenPowerClusterGpResponseCallback(uint8_t options,
                                                uint16_t tempMasterShortAddress,
                                                uint8_t tempMasterTxChannel,
                                                uint32_t gpdSrcId,
                                                uint8_t* gpdIeee,
                                                uint8_t endpoint,
                                                uint8_t gpdCommandId,
                                                uint8_t* gpdCommandPayload)
{
  EmberGpAddress addr;
  uint8_t allZeroesIeeeAddress[17] = { 0 };

  //handle null args for EZSP
  if (gpdIeee  == NULL) {
    gpdIeee = allZeroesIeeeAddress;
  }

  if (emberAfCurrentEndpoint() != GP_ENDPOINT) {
    goto kickout;
  }
  emberAfGreenPowerClusterPrintln("GpResponse cmd: %1x", gpdCommandId);

  switch (gpdCommandId) {
    case EMBER_ZCL_GP_GPDF_CHANNEL_CONFIGURATION:
      addr.applicationId = EMBER_GP_APPLICATION_SOURCE_ID;
      addr.id.sourceId = 0x00000000;
      if (gpdSrcId != 0x00000000) {
        return true;
      }

      if (tempMasterShortAddress == emberGetNodeId()) {
        if (!emberEventControlGetActive(emberAfPluginGreenPowerClientChannelEventControl)
            || (commissioningState.channelStatus & GP_CLIENT_TRANSMIT_SAME_AS_OPERATIONAL_CHANNEL_MASK)) {
          //Look at me, I'm the tempMaster now
          //TODO set firstToForward

          uint8_t payload = (emberAfGetRadioChannel() - 11) //channel ID
                            | 0x10;//basic
          emberDGpSend(true, //add
                       false,//no CCA
                       &addr,
                       EMBER_ZCL_GP_GPDF_CHANNEL_CONFIGURATION,
                       1, //length
                       &(payload),
                       0, //gpepHandle
                       0); //entryLifetimeMs
          commissioningState.channelStatus = GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK;
          // based on figure 88 of GP proxy basic spec V1.0 page 191
          // the following condition seems neccessary
          if (emberAfGetRadioChannel() != (tempMasterTxChannel + 11) ) {
            emberAfGreenPowerClusterPrintln("GP switch to chan %d", tempMasterTxChannel + 11);
            emberStartScan(EMBER_STACK_GP_CHANNEL_DELIVERY_SCAN, (1 << (tempMasterTxChannel + 11)), 0 /*duration*/);
            commissioningState.channelStatus &= ~GP_CLIENT_TRANSMIT_SAME_AS_OPERATIONAL_CHANNEL_MASK;
          } else {
            commissioningState.channelStatus |= GP_CLIENT_TRANSMIT_SAME_AS_OPERATIONAL_CHANNEL_MASK;
          }
          // Stared timeout to clear the Tx Queue if the packet is not delivered,
          emberEventControlSetDelayMS(emberAfPluginGreenPowerClientChannelEventControl, GP_CHANNEL_EVENT_TIMEOUT_IN_MSEC);
          return true;
        } else {
          return false;
        }
      } else {
        //TODO clear firstToForward
        emberAfPluginGreenPowerClientChannelEventHandler();
        //emberDGpSend(false, false, &addr, 0, 0, NULL, 0, 0);
      }
      break;
    case EMBER_ZCL_GP_GPDF_COMMISSIONING_REPLY:
      //TODO IEEE GPD
    {
      EmberStatus result;
      EmberGpAddress addr;
      addr.applicationId = options & EMBER_AF_GP_RESPONSE_OPTION_APPLICATION_ID;
      if (addr.applicationId == EMBER_GP_APPLICATION_SOURCE_ID) {
        addr.id.sourceId = gpdSrcId;
      } else {
        addr.applicationId = EMBER_GP_APPLICATION_IEEE_ADDRESS;
        addr.endpoint = endpoint;
        // addr.id is a union so it is fine to dereference it in this was
        MEMCOPY(addr.id.gpdIeeeAddress, gpdIeee, 8);
      }
      if (tempMasterShortAddress == emberGetNodeId()) {
        emberAfGreenPowerClusterPrintln("dgpsend comm reply %s", "A");
        result = emberDGpSend(true, //add
                              false, //skip cca
                              &addr,
                              EMBER_ZCL_GP_GPDF_COMMISSIONING_REPLY,
                              gpdCommandPayload[0],
                              gpdCommandPayload + 1,
                              0, 0
                              );
        if (result == EMBER_SUCCESS) {
          emberAfGreenPowerClusterPrintln("success");
        } else {
          emberAfGreenPowerClusterPrintln("fail %02x", result);
        }
      } else {
        emberAfGreenPowerClusterPrintln("Flush GP TX");
        emberDGpSend(false, false, &addr, 0, 0, NULL, 0, 0);
      }
    }

    break;
    default:
      if ((gpdCommandId >= EMBER_ZCL_GP_GPDF_RESERVED_MIN) //and implicitly check gpdCommandId <= EMBER_ZCL_GP_GPDF_RESERVED_MAX
          || (gpdCommandId >= EMBER_ZCL_GP_GPDF_MFR_DEF_GPD_CMD0
              && gpdCommandId <= EMBER_ZCL_GP_GPDF_MFR_DEF_GPD_CMD_F)) {
        EmberStatus result;
        EmberGpAddress addr;
        addr.applicationId = options & EMBER_GP_APPLICATION_ID_MASK;
        if (addr.applicationId == EMBER_GP_APPLICATION_SOURCE_ID) {
          addr.id.sourceId = gpdSrcId;
        } else {
          addr.applicationId = EMBER_GP_APPLICATION_IEEE_ADDRESS;
          addr.endpoint = endpoint;
          MEMCOPY(addr.id.gpdIeeeAddress, gpdIeee, 8);
        }

        emberAfGreenPowerClusterPrintln("dgpsend comm reply %s", "B");
        result = emberDGpSend(true,   //add
                              false,   //skip cca
                              &addr,
                              gpdCommandId,
                              gpdCommandPayload[0],
                              gpdCommandPayload + 1,
                              0, 0
                              );
        if (result == EMBER_SUCCESS) {
          emberAfGreenPowerClusterPrintln("success");
        } else {
          emberAfGreenPowerClusterPrintln("fail %02x", result);
        }
      }
      break;
  }
  kickout: return false;
}

bool emberAfGreenPowerClusterGpSinkTableResponseCallback(uint8_t status,
                                                         uint8_t totalNumberofNonEmptySinkTableEntries,
                                                         uint8_t startIndex,
                                                         uint8_t entriesCount,
                                                         uint8_t* sinkTableEntry)
{
  return false;
}

uint16_t emberAfGreenPowerClientStoreProxyTableEntry(EmberGpProxyTableEntry *entry,
                                                     uint8_t *buffer)
{
  uint8_t *finger = buffer;
  //uint8_t securityLevel = entry->securityOptions & EMBER_AF_GP_PROXY_TABLE_ENTRY_SECURITY_OPTIONS_SECURITY_LEVEL;
  //uint8_t securityKeyType = entry->securityOptions & EMBER_AF_GP_PROXY_TABLE_ENTRY_SECURITY_OPTIONS_SECURITY_KEY_TYPE;

  emberAfCopyInt16u(finger, 0, (uint16_t)(entry->options & 0xFFFF));
  finger += sizeof(uint16_t);
  if (entry->gpd.applicationId == EMBER_GP_APPLICATION_SOURCE_ID) {
    emberAfCopyInt32u(finger, 0, entry->gpd.id.sourceId);
    finger += sizeof(uint32_t);
  } else if (entry->gpd.applicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    MEMMOVE(finger, entry->gpd.id.gpdIeeeAddress, EUI64_SIZE);
    finger += EUI64_SIZE;
  }

  if (entry->gpd.applicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    emberAfCopyInt8u(finger, 0, entry->gpd.endpoint);
    finger += sizeof(uint8_t);
  }

  if (entry->options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_ASSIGNED_ALIAS) {
    emberAfCopyInt16u(finger, 0, entry->assignedAlias);
    finger += sizeof(uint16_t);
  }

  if (entry->options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_SECURITY_USE) {
    emberAfCopyInt8u(finger, 0, entry->securityOptions);
    finger += sizeof(uint8_t);
    emberAfCopyInt32u(finger, 0, entry->gpdSecurityFrameCounter);
    finger += sizeof(uint32_t);
    MEMMOVE(finger, entry->gpdKey.contents, EMBER_ENCRYPTION_KEY_SIZE);
    finger += EMBER_ENCRYPTION_KEY_SIZE;
  } else if (entry->options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_SEQUENCE_NUMBER_CAP) {
    emberAfCopyInt32u(finger, 0, entry->gpdSecurityFrameCounter);
    finger += sizeof(uint32_t);
  }

  // Lightweight sink address list
  if (entry->options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_LIGHTWEIGHT_UNICAST_GPS) {
    // let's count
    uint8_t index = 0;
    uint8_t *entryCount = finger;
    emberAfCopyInt8u(finger, 0, 0x00);
    finger += sizeof(uint8_t);
    for (index = 0; index < GP_SINK_LIST_ENTRIES; index++) {
      EmberGpSinkListEntry * sinkEntry = &entry->sinkList[index];

      if (sinkEntry->type == EMBER_GP_SINK_TYPE_LW_UNICAST) {
        MEMMOVE(finger, sinkEntry->target.unicast.sinkEUI, EUI64_SIZE);
        finger += EUI64_SIZE;
        emberAfCopyInt16u(finger, 0, sinkEntry->target.unicast.sinkNodeId);
        finger += sizeof(uint16_t);
        (*entryCount)++;
      } else {
        continue;
      }
    }
  }

  // Sink group list
  if (entry->options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_COMMISIONED_GROUP_GPS) {
    // let's count
    uint8_t index = 0;
    uint8_t *entryCount = finger;
    emberAfCopyInt8u(finger, 0, 0x00);
    finger += sizeof(uint8_t);
    for (index = 0; index < GP_SINK_LIST_ENTRIES; index++) {
      EmberGpSinkListEntry * sinkEntry = &entry->sinkList[index];

      if (sinkEntry->type == EMBER_GP_SINK_TYPE_GROUPCAST) {
        emberAfCopyInt16u(finger, 0, sinkEntry->target.groupcast.groupID);
        finger += sizeof(uint16_t);
        emberAfCopyInt16u(finger, 0, sinkEntry->target.groupcast.alias);
        finger += sizeof(uint16_t);
        (*entryCount)++;
      } else {
        continue;
      }
    }
  }

  emberAfCopyInt8u(finger, 0, entry->groupcastRadius);
  finger += sizeof(uint8_t);

  if ((entry->options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_ENTRY_ACTIVE) == 0
      || (entry->options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_ENTRY_VALID) == 0) {
    emberAfCopyInt8u(finger, 0, entry->searchCounter);
    finger += sizeof(uint8_t);
  }

  if (entry->options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_EXTENSION) {
    emberAfCopyInt16u(finger, 0, (entry->options >> 16) & 0xFFFF);
    finger += sizeof(uint16_t);
  }

  // full unicast sink address list
  if (entry->options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_FULL_UNICAST_GPS) {
    // let's count
    uint8_t index = 0;
    uint8_t *len = finger;
    emberAfCopyInt8u(finger, 0, 0); // Add zero length
    finger += sizeof(uint8_t);
    for (index = 0; index < GP_SINK_LIST_ENTRIES; index++) {
      EmberGpSinkListEntry * sinkEntry = &entry->sinkList[index];

      if (sinkEntry->type == EMBER_GP_SINK_TYPE_LW_UNICAST) {
        MEMMOVE(finger, sinkEntry->target.unicast.sinkEUI, EUI64_SIZE);
        finger += EUI64_SIZE;
        emberAfCopyInt16u(finger, 0, sinkEntry->target.unicast.sinkNodeId);
        finger += sizeof(uint16_t);
        *len += EUI64_SIZE + sizeof(uint16_t);
      } else {
        continue;
      }
    }
  }

  return (uint16_t)(finger - buffer);
}

bool emberAfGreenPowerClusterGpProxyTableRequestCallback(uint8_t options,
                                                         uint32_t gpdSrcId,
                                                         uint8_t* gpdIeee,
                                                         uint8_t endpoint,
                                                         uint8_t index)
{
  uint8_t validEntriesCount = 0;
  uint8_t entryIndex = 0;
  uint8_t appId = options & EMBER_AF_GP_PROXY_TABLE_REQUEST_OPTIONS_APPLICATION_ID;
  uint8_t requestType = (options & EMBER_AF_GP_PROXY_TABLE_REQUEST_OPTIONS_REQUEST_TYPE)
                        >> EMBER_AF_GP_PROXY_TABLE_REQUEST_OPTIONS_REQUEST_TYPE_OFFSET;
  EmberGpProxyTableEntry entry;
  uint8_t allZeroesIeeeAddress[17] = { 0 };

  //handle null args for EZSP
  if (gpdIeee  == NULL) {
    gpdIeee = allZeroesIeeeAddress;
  }

  // only respond to unicast messages.
  if (emberAfCurrentCommand()->type != EMBER_INCOMING_UNICAST) {
    goto kickout;
  }

  if (EMBER_GP_PROXY_TABLE_SIZE == 0) {
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND);
    goto kickout;
  }

  if (emberAfCurrentEndpoint() != GP_ENDPOINT) {
    return false;
  }

  for (entryIndex = 0; entryIndex < EMBER_GP_PROXY_TABLE_SIZE; entryIndex++) {
    if (emberGpProxyTableGetEntry(entryIndex, &entry) != EMBER_SUCCESS) {
      return false;
    }

    if (entry.status != EMBER_GP_PROXY_TABLE_ENTRY_STATUS_UNUSED) {
      validEntriesCount++;
    }
  }

  if (requestType == EMBER_ZCL_GP_PROXY_TABLE_REQUEST_OPTIONS_REQUEST_TYPE_BY_GPD_ID) {
    EmberGpAddress addr;
    if (!emGpMakeAddr(&addr,
                      appId,
                      gpdSrcId,
                      gpdIeee,
                      endpoint)) {
      // Malformed GPD address
      goto kickout;
    }
    entryIndex = emberGpProxyTableLookup(&addr);
    if (entryIndex == 0xFF) {
      emberAfFillCommandGreenPowerClusterGpProxyTableResponse(EMBER_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_NOT_FOUND,
                                                              validEntriesCount,
                                                              index,
                                                              0x00,
                                                              NULL,
                                                              0);
      emberAfSendResponse();
      goto kickout;
    } else {
      emberAfFillCommandGreenPowerClusterGpProxyTableResponse(EMBER_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_SUCCESS,
                                                              validEntriesCount,
                                                              0xff,
                                                              1,
                                                              NULL,
                                                              0);
      if (emberGpProxyTableGetEntry(entryIndex, &entry) != EMBER_SUCCESS) {
        return false;
      }

      appResponseLength
        += emberAfGreenPowerClientStoreProxyTableEntry(&entry,
                                                       appResponseData + appResponseLength);
      emberAfSendResponse();
      goto kickout;
    }
  } else if (requestType == EMBER_ZCL_GP_PROXY_TABLE_REQUEST_OPTIONS_REQUEST_TYPE_BY_INDEX) {
    if (index >= validEntriesCount) {
      //Nothing to send
      emberAfFillCommandGreenPowerClusterGpProxyTableResponse(EMBER_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_NOT_FOUND,
                                                              validEntriesCount,
                                                              index,
                                                              0x00,
                                                              NULL,
                                                              0);
      emberAfSendResponse();
      goto kickout;
    } else {
      emberAfFillCommandGreenPowerClusterGpProxyTableResponse(EMBER_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_SUCCESS,
                                                              validEntriesCount,
                                                              index,
                                                              0xff,
                                                              NULL,
                                                              0);

      validEntriesCount = 0;
      uint16_t entriesCount = 0;
      for (entryIndex = 0; entryIndex < EMBER_GP_PROXY_TABLE_SIZE; entryIndex++) {
        if (emberGpProxyTableGetEntry(entryIndex, &entry) != EMBER_SUCCESS) {
          return false;
        }

        uint8_t  tempDatabuffer[EMBER_AF_RESPONSE_BUFFER_LEN];
        uint16_t tempDataLength = 0;

        if (entry.status != EMBER_GP_PROXY_TABLE_ENTRY_STATUS_UNUSED) {
          validEntriesCount++;
          if (validEntriesCount > index) {
            // Copy to a temp buffer and add if there is spoce
            tempDataLength = emberAfGreenPowerClientStoreProxyTableEntry(&entry, tempDatabuffer);

            // If space add to buffer
            if ( sizeof(appResponseData) > (appResponseLength + tempDataLength)) {
              MEMMOVE(&appResponseData[appResponseLength], tempDatabuffer, tempDataLength);
              appResponseLength +=  tempDataLength;
              entriesCount++;
            } else {
              break;
            }
          }
        }
      }

      //Insert the number of entries actually included
      appResponseData[GP_PROXY_TABLE_RESPONSE_ENTRIES_OFFSET + GP_NON_MANUFACTURER_ZCL_HEADER_LENGTH] = entriesCount;

      EmberStatus status = emberAfSendResponse();

      if (status == EMBER_MESSAGE_TOO_LONG) {
        emberAfFillCommandGreenPowerClusterGpProxyTableResponse(EMBER_ZCL_GP_PROXY_TABLE_RESPONSE_STATUS_SUCCESS,
                                                                validEntriesCount,
                                                                index,
                                                                0x00,
                                                                NULL,
                                                                0);
        emberAfSendResponse();
      }
      goto kickout;
    }
  }

  kickout:  return true;
}

bool emAfPluginGreenPowerClientRetrieveAttributeAndCraftResponse(uint8_t endpoint,
                                                                 EmberAfClusterId clusterId,
                                                                 EmberAfAttributeId attrId,
                                                                 uint8_t mask,
                                                                 uint16_t manufacturerCode,
                                                                 uint16_t readLength)
{
  uint8_t proxyTableEntryAppResponseData[EMBER_AF_RESPONSE_BUFFER_LEN];
  uint8_t zclStatus = EMBER_ZCL_STATUS_SUCCESS;
  uint16_t stringDataOffsetStart = 0;
  uint16_t stringLength = 0;
  bool    status = false;

  if (endpoint != GP_ENDPOINT
      || clusterId != ZCL_GREEN_POWER_CLUSTER_ID
      || attrId != ZCL_GP_CLIENT_PROXY_TABLE_ATTRIBUTE_ID
      || mask != CLUSTER_MASK_CLIENT
      || manufacturerCode != EMBER_AF_NULL_MANUFACTURER_CODE) {
    // do nothing but return false
  } else if (readLength < 6) {
    // Can't fit the ZCL header in available length so exit with error
    // do nothing but return true as we are processing the correct type of packet
  } else {
    emberAfPutInt16uInResp(attrId);

    // The proxy table attribute is a long string ZCL attribute type, which means
    // it is encoded starting with a 2-byte length. We fill in the real length
    // after we have encoded the whole proxy table.
    // Fopur bytes extra = 2byte length + 1 byte ZCl Status + 1 byte Attribute Type
    uint16_t stringDataOffset =  appResponseLength + 4;
    stringDataOffsetStart = stringDataOffset;

    // Search the proxy table and respond with entries
    for (uint8_t i = 0; i < EMBER_GP_PROXY_TABLE_SIZE; i++) {
      EmberGpProxyTableEntry entry;

      if (emberGpProxyTableGetEntry(i, &entry) != EMBER_SUCCESS) {
        return false;
      } else if (entry.status != EMBER_GP_PROXY_TABLE_ENTRY_STATUS_UNUSED) {
        // Have a valid entry so encode response in temp buffer and add if it fits
        uint16_t proxyTableEntryLength = emberAfGreenPowerClientStoreProxyTableEntry(&entry, proxyTableEntryAppResponseData);

        if ( (proxyTableEntryLength + stringDataOffset) > readLength) {
          // String is too big so
          zclStatus = EMBER_ZCL_STATUS_INSUFFICIENT_SPACE;
          break;
        } else {
          MEMMOVE(&appResponseData[stringDataOffset], proxyTableEntryAppResponseData, proxyTableEntryLength);
          stringDataOffset += proxyTableEntryLength;
        }
      }
    }

    // calculate string length
    stringLength = stringDataOffset - stringDataOffsetStart;

    if (zclStatus == EMBER_ZCL_STATUS_SUCCESS) {
      emberAfPutInt8uInResp(zclStatus);
      emberAfPutInt8uInResp(ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE);
      emberAfPutInt16uInResp(stringLength);
    } else {
      emberAfPutInt8uInResp(zclStatus);
    }

    appResponseLength += stringLength;
    status = true;
  }

  return status;
}

bool emberAfGreenPowerClusterAutoCommissioningCallback(GP_PARAMS)
{
  uint16_t options = 0;
  EmberApsFrame *apsFrame = NULL;

  if (!commissioningState.inCommissioningMode) {
    return true;
  }

  if (addr->applicationId == EMBER_GP_APPLICATION_SOURCE_ID
      && addr->id.sourceId == 0x00000000) {
    return true;
  }
  if (gpdCommandId == EMBER_ZCL_GP_GPDF_COMMISSIONING || gpdCommandId == EMBER_ZCL_GP_GPDF_SUCCESS) {
    return true;
  }

  if (rxAfterTx) {
    return true;
  }

  /* TODO
     uint8_t UNUSED proxyIndex = 0xFF;
     proxyIndex = emberGpProxyTableLookup(addr);
     if (proxyIndex != 0xFF) {
     //we have a proxy table entry for this GPD
     emGpProxyTableSetInRange(proxyIndex);
     } else {
     //we don't have a proxy table entry
     //optionally create an active invalid entry here
     //not required for proxy basic
     }
   */

  if (status == EMBER_NO_SECURITY) {
    options = 0;
  }
  //TODO handle other cases

  options |= addr->applicationId;
  options |= gpdfSecurityLevel
             << GP_COMMISSIONING_SECURITY_LEVEL_TO_OPTIONS_SHIFT;
  options |= gpdfSecurityKeyType
             << GP_COMMISSIONING_SECURITY_KEY_TYPE_TO_OPTIONS_SHIFT; //security key type
  options |= EMBER_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_PROXY_INFO_PRESENT;
  emberAfFillCommandGreenPowerClusterGpCommissioningNotificationSmart(options,
                                                                      addr->id.sourceId,
                                                                      addr->id.gpdIeeeAddress,
                                                                      addr->endpoint,
                                                                      sequenceNumber,
                                                                      gpdfSecurityLevel,
                                                                      gpdSecurityFrameCounter,
                                                                      gpdCommandId,
                                                                      gpdCommandPayloadLength,
                                                                      gpdCommandPayload,
                                                                      emberGetNodeId(),
                                                                      gpdLink,
                                                                      mic);
  apsFrame = emberAfGetCommandApsFrame();
  apsFrame->sourceEndpoint = GP_ENDPOINT; //emberAfCurrentEndpoint();
  apsFrame->destinationEndpoint = GP_ENDPOINT; //emberAfCurrentEndpoint();
  emSetAddDelay(GP_DMIN_U);
  if (commissioningState.unicastCommunication) {
    emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                              commissioningState.commissioningSink);
  } else {
    apsFrame->sequence = sequenceNumber - EMBER_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET;
    apsFrame->options |= EMBER_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
    emberAfSendCommandBroadcastWithAlias(EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                                         emGpdAlias(addr),
                                         sequenceNumber - EMBER_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET);
  }
  emSetAddDelay(0);
  return true;
}

uint8_t qualityBasedDelay(uint8_t gpdLink)
{
  uint8_t ourLqi = (gpdLink & 0xC0) >> 5;
  return ((3 - ourLqi) << 5);
}

bool emberAfGreenPowerClusterCommissioningGpdfCallback(GP_PARAMS)
{
  EmberApsFrame *apsFrame;

  // Per GP spec 14-0563-08:
  // Commissioning: Step 12a: Proxy receives commissioning command
  // If applicationId == 0b0000 and srcId == 0 or reserved range 0xfffffff9 - 0xffffffffe
  // OR applicationId == 0b0010 and srcId == 0, drop frame.
  if ((addr->applicationId == EMBER_GP_APPLICATION_SOURCE_ID)
      && (addr->id.sourceId == GP_GPD_SRC_ID_RESERVED_0)) {
    // silently drop frame
    goto kickout;
  }

  if ((addr->applicationId == EMBER_GP_APPLICATION_SOURCE_ID)
      && (addr->id.sourceId >=  GP_GPD_SRC_ID_RESERVED_FFFFFF9)
      && (addr->id.sourceId <=  GP_GPD_SRC_ID_RESERVED_FFFFFFE)) {
    // silently drop frame as in reserved range
    goto kickout;
  }

  if (addr->applicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
    uint8_t i;
    for (i = 0; i < EUI64_SIZE; i++) {
      if (addr->id.gpdIeeeAddress[i]) {
        break;
      }
    }
    if (i == EUI64_SIZE) {
      // silently drop frame if EUI is all zeros
      goto kickout;
    }
  }

  if (autoCommissioning) {
    goto kickout;
  }

  // Step b
  if ((gpdfSecurityLevel == EMBER_GP_SECURITY_LEVEL_FC_MIC)
      || (gpdfSecurityLevel == EMBER_GP_SECURITY_LEVEL_FC_MIC_ENCRYPTED)) {
    if ((status == EMBER_AUTH_FAILURE)
        || (status == EMBER_UNPROCESSED)) {
      // fwd frame with Security processing failed bit set as part of GP
      // Commissioning Notification.
      goto send_notification;
    }
  }

  // Step c
  // RxAfterTx is true
  {
    bool frameFound = false;
    if (rxAfterTx && (gpdCommandId == EMBER_ZCL_GP_GPDF_COMMISSIONING)) {
      if (addr->applicationId == EMBER_GP_APPLICATION_IEEE_ADDRESS) {
        // TODO: check endpoint
      }
    }

    if (frameFound) {
      // send at least one Commissioning Reply GPDF
      //XXX compare to where we do this now
    }
  }

  // Step d
  /* TODO
     {
     uint8_t gpProxyTableIndex = emberGpProxyTableLookup(addr);
     if (gpProxyTableIndex != 0xFF) {
      emGpProxyTableSetInRange(gpProxyTableIndex);
     } else {
       //basic proxy doesn't have to do anything in this situation
     }

     }
   */

  // Step e: sending GP Commissioning Notification
  send_notification: {
    uint16_t options = addr->applicationId;
    if (rxAfterTx) {
      options |= EMBER_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_RX_AFTER_TX;
    }

    if ((status == EMBER_AUTH_FAILURE)
        || (status == EMBER_UNPROCESSED)) {
      options |=
        EMBER_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_SECURITY_PROCESSING_FAILED;
      options |= (gpdfSecurityKeyType ? 0x4 : 0) // see 1.0 spec line 3040
                 << GP_COMMISSIONING_SECURITY_KEY_TYPE_TO_OPTIONS_SHIFT;
    } else {
      options |= gpdfSecurityKeyType
                 << GP_COMMISSIONING_SECURITY_KEY_TYPE_TO_OPTIONS_SHIFT;
    }

    options |= gpdfSecurityLevel
               << GP_COMMISSIONING_SECURITY_LEVEL_TO_OPTIONS_SHIFT;
    options |= EMBER_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_PROXY_INFO_PRESENT;
    if (commissioningState.unicastCommunication) {
      emberAfFillCommandGreenPowerClusterGpCommissioningNotificationSmart(options,
                                                                          addr->id.sourceId,
                                                                          addr->id.gpdIeeeAddress,
                                                                          addr->endpoint,
                                                                          sequenceNumber,
                                                                          gpdfSecurityLevel,
                                                                          gpdSecurityFrameCounter,
                                                                          gpdCommandId,
                                                                          gpdCommandPayloadLength,
                                                                          gpdCommandPayload,
                                                                          emberGetNodeId(),
                                                                          gpdLink,
                                                                          mic);
      if (rxAfterTx) {
        emSetAddDelay(GP_DMIN_B);
      } else {
        emSetAddDelay(GP_DMIN_U);
      }
      apsFrame = emberAfGetCommandApsFrame();
      apsFrame->sourceEndpoint = GP_ENDPOINT;  //emberAfCurrentEndpoint();
      apsFrame->destinationEndpoint = GP_ENDPOINT; //emberAfCurrentEndpoint();
      UNUSED EmberStatus retval = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                                            commissioningState.commissioningSink);
      emSetAddDelay(0);
    } else {
      emberAfFillCommandGreenPowerClusterGpCommissioningNotificationSmart(options,
                                                                          addr->id.sourceId,
                                                                          addr->id.gpdIeeeAddress,
                                                                          addr->endpoint,
                                                                          sequenceNumber,
                                                                          gpdfSecurityLevel,
                                                                          gpdSecurityFrameCounter,
                                                                          gpdCommandId,
                                                                          gpdCommandPayloadLength,
                                                                          gpdCommandPayload,
                                                                          emberGetNodeId(),
                                                                          gpdLink,
                                                                          mic);
      apsFrame = emberAfGetCommandApsFrame();
      apsFrame->sourceEndpoint = GP_ENDPOINT;  //emberAfCurrentEndpoint();
      apsFrame->destinationEndpoint = GP_ENDPOINT; //emberAfCurrentEndpoint();
      apsFrame->sequence = sequenceNumber - EMBER_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET;
      apsFrame->options |= EMBER_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
      if (rxAfterTx) {
        emSetAddDelay(GP_DMIN_B + qualityBasedDelay(gpdLink));
      } else {
        emSetAddDelay(GP_DMIN_U);
      }

      UNUSED EmberStatus retval = emberAfSendCommandBroadcastWithAlias(EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                                                                       emGpdAlias(addr),
                                                                       sequenceNumber - EMBER_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET);
      emSetAddDelay(0);
    }
  }

  kickout: return true;
}

bool emberAfGreenPowerClusterDecommissioningGpdfCallback(GP_PARAMS)
{
  return emberAfGreenPowerClusterCommissioningGpdfCallback(GP_ARGS);
}

bool emberAfGreenPowerClusterSuccessGpdfCallback(GP_PARAMS)
{
  return emberAfGreenPowerClusterCommissioningGpdfCallback(GP_ARGS);
}

bool emberAfGreenPowerClusterChannelRequestGpdfCallback(GP_PARAMS)
{
  if (!commissioningState.inCommissioningMode) {
    return true;
  }

  // based on figure 88 of GP proxy basic spec V1.0 page 191
  // the second condition of the following condition seems neccessary
  if ((commissioningState.channelStatus & GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK)
      && (!(commissioningState.channelStatus & GP_CLIENT_TRANSMIT_SAME_AS_OPERATIONAL_CHANNEL_MASK))) {
    //Don't forward if we're on the transmit channel, and the transmit channel
    // is not same as the operational channel
  } else {
    //TODO: check for a reason not to forward (busy, etc)

    uint16_t options = 0; //always uses appID srcID becase srcID is absent was addr.applicationId;

    //rxAfter Tx always set for channel request
    options |=
      EMBER_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_RX_AFTER_TX;

    if (status == EMBER_AUTH_FAILURE) {
      options |=
        EMBER_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_SECURITY_PROCESSING_FAILED;
    }

    options |= gpdfSecurityLevel
               << GP_COMMISSIONING_SECURITY_LEVEL_TO_OPTIONS_SHIFT;
    options |= gpdfSecurityKeyType
               << GP_COMMISSIONING_SECURITY_KEY_TYPE_TO_OPTIONS_SHIFT;
    options |= EMBER_AF_GP_COMMISSIONING_NOTIFICATION_OPTION_PROXY_INFO_PRESENT;

    EmberApsFrame *apsFrame;

    if (commissioningState.unicastCommunication) {
      emberAfFillCommandGreenPowerClusterGpCommissioningNotificationSmart(options,
                                                                          0x00000000, //addr.id.sourceId,
                                                                          NULL, //addr.id.gpdIeeeAddress,
                                                                          0, //addr.endpoint,
                                                                          sequenceNumber,
                                                                          gpdfSecurityLevel,
                                                                          gpdSecurityFrameCounter,
                                                                          gpdCommandId,
                                                                          gpdCommandPayloadLength,
                                                                          gpdCommandPayload,
                                                                          emberGetNodeId(),
                                                                          gpdLink,
                                                                          mic);
      apsFrame = emberAfGetCommandApsFrame();
      apsFrame->sourceEndpoint = GP_ENDPOINT;  //emberAfCurrentEndpoint();
      apsFrame->destinationEndpoint = GP_ENDPOINT; //emberAfCurrentEndpoint();
      emSetAddDelay(GP_DMIN_B);
      UNUSED EmberStatus retval = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                                            commissioningState.commissioningSink);
      emSetAddDelay(0);
    } else {
      emberAfFillCommandGreenPowerClusterGpCommissioningNotificationSmart(options,
                                                                          0x00000000, //addr.id.sourceId,
                                                                          NULL, //addr.id.gpdIeeeAddress,
                                                                          0, //addr.endpoint,
                                                                          sequenceNumber,
                                                                          gpdfSecurityLevel,
                                                                          gpdSecurityFrameCounter,
                                                                          gpdCommandId,
                                                                          gpdCommandPayloadLength,
                                                                          gpdCommandPayload,
                                                                          emberGetNodeId(),
                                                                          gpdLink,
                                                                          mic);
      apsFrame = emberAfGetCommandApsFrame();
      apsFrame->sourceEndpoint = GP_ENDPOINT;  //emberAfCurrentEndpoint();
      apsFrame->destinationEndpoint = GP_ENDPOINT; //emberAfCurrentEndpoint();
      apsFrame->sequence = sequenceNumber - EMBER_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET;
      apsFrame->options |= EMBER_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
      emSetAddDelay(GP_DMIN_B + qualityBasedDelay(gpdLink));
      UNUSED EmberStatus retval = emberAfSendCommandBroadcastWithAlias(EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS,
                                                                       emGpdAlias(addr),
                                                                       sequenceNumber - EMBER_GP_COMMISSIONING_NOTIFICATION_SEQUENCE_NUMBER_OFFSET);
      emSetAddDelay(0);
    }
  }

  return true;
}

bool emberAfGreenPowerClusterGpdfForwardCallback(GP_PARAMS)
{
  uint8_t i;
  uint16_t options = 0;
  uint16_t alias = 0;
  EmberApsFrame *apsFrame;

  if (proxyTableIndex == 0xFF) {
    return true;
  }
  EmberGpProxyTableEntry entry;
  if (proxyTableIndex != 0xFF) {
#ifndef EZSP_HOST
    //TODO, we need to decide how to do this for host
    emGpProxyTableSetFirstToForward(proxyTableIndex);
    emGpProxyTableSetInRange(proxyTableIndex);    //TODO is this the right place for this?
#endif
    if (emberGpProxyTableGetEntry(proxyTableIndex, &entry) != EMBER_SUCCESS) {
      return true;
    }
  }

  options |= addr->applicationId;

  //Traverse the sink list to set the options flags
  for (i = 0; i < GP_SINK_LIST_ENTRIES; i++) {
    if (entry.sinkList[i].type == EMBER_GP_SINK_TYPE_FULL_UNICAST
        || entry.sinkList[i].type == EMBER_GP_SINK_TYPE_LW_UNICAST) {
      options |= EMBER_AF_GP_NOTIFICATION_OPTION_ALSO_UNICAST;
    } else if (entry.sinkList[i].type == EMBER_GP_SINK_TYPE_GROUPCAST) {
      options |= EMBER_AF_GP_NOTIFICATION_OPTION_ALSO_COMMISSIONED_GROUP;
    }
  }

  if (entry.options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_DERIVED_GROUP_GPS) {
    options |= EMBER_AF_GP_NOTIFICATION_OPTION_ALSO_DERIVED_GROUP;
  }

  options |= gpdfSecurityLevel
             << EMBER_AF_GP_NOTIFICATION_OPTION_SECURITY_LEVEL_OFFSET;

  options |= gpdfSecurityKeyType
             << EMBER_AF_GP_NOTIFICATION_OPTION_SECURITY_KEY_TYPE_OFFSET;

  //Basic proxy always sends queue full for data gpdfs (not commissioning)
  options |= EMBER_AF_GP_NOTIFICATION_OPTION_GP_TX_QUEUE_FULL;

  // Basic Proxy implementing the latest spec always sets this
  options |= EMBER_AF_GP_NOTIFICATION_OPTION_PROXY_INFO_PRESENT;

  // comm/decomm frames ignore rx after tx bit in operational mode
  if (((gpdCommandId != EMBER_ZCL_GP_GPDF_COMMISSIONING
        && gpdCommandId != EMBER_ZCL_GP_GPDF_DECOMMISSIONING)
       || commissioningState.inCommissioningMode)
      && rxAfterTx) {
    options |= EMBER_AF_GP_NOTIFICATION_OPTION_RX_AFTER_TX;
  }

  if (  gpdfSecurityLevel == EMBER_GP_SECURITY_KEY_NONE) {
    // If no security use the MAC sequence number instead
    gpdSecurityFrameCounter = (uint32_t)(sequenceNumber & 0x000000ff);
  }

  emberAfFillCommandGreenPowerClusterGpNotificationSmart(options,
                                                         addr->id.sourceId,
                                                         addr->id.gpdIeeeAddress,
                                                         addr->endpoint,
                                                         gpdSecurityFrameCounter,
                                                         gpdCommandId,
                                                         gpdCommandPayloadLength,
                                                         gpdCommandPayload,
                                                         emberGetNodeId(),
                                                         gpdLink);

  for (i = 0; i < GP_SINK_LIST_ENTRIES; i++) {
    if (entry.sinkList[i].type == EMBER_GP_SINK_TYPE_FULL_UNICAST
        || entry.sinkList[i].type == EMBER_GP_SINK_TYPE_LW_UNICAST) {
      apsFrame = emberAfGetCommandApsFrame();
      apsFrame->sourceEndpoint = GP_ENDPOINT;  //emberAfCurrentEndpoint();
      apsFrame->destinationEndpoint = GP_ENDPOINT;  //emberAfCurrentEndpoint();
      if (rxAfterTx) {
        emSetAddDelay(GP_DMIN_B);
      } else {
        emSetAddDelay(GP_DMIN_U);
      }
      // The sinkNodeId is optimised to not be in the proxy table token structure because it can be looked up.
      // Incase the proxy table is updated witha valid address earlier it needs to use that.
      // So, check if the proxy table node id is not valid UCAST id before looking up the map and then forward the notification.
      if (!isNodeIdValid(entry.sinkList[i].target.unicast.sinkNodeId)) {
        EmberNodeId nodeId = emberLookupNodeIdByEui64(entry.sinkList[i].target.unicast.sinkEUI);
        if (isNodeIdValid(nodeId)) {
          entry.sinkList[i].target.unicast.sinkNodeId = nodeId;
        }
      }

      UNUSED EmberStatus retval = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,
                                                            entry.sinkList[i].target.unicast.sinkNodeId);
      emSetAddDelay(0);
    } else if (entry.sinkList[i].type == EMBER_GP_SINK_TYPE_GROUPCAST) {
      apsFrame = emberAfGetCommandApsFrame();
      apsFrame->sourceEndpoint = GP_ENDPOINT;  //emberAfCurrentEndpoint();
      apsFrame->destinationEndpoint = GP_ENDPOINT;  //emberAfCurrentEndpoint();
      apsFrame->sequence = sequenceNumber - EMBER_GP_NOTIFICATION_COMMISSIONED_GROUPCAST_SEQUENCE_NUMBER_OFFSET;
      apsFrame->options |= EMBER_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
      apsFrame->radius = entry.groupcastRadius;
      if (rxAfterTx) {
        emSetAddDelay(GP_DMIN_B + qualityBasedDelay(gpdLink));
      } else {
        emSetAddDelay(GP_DMIN_U);
      }
      UNUSED EmberStatus retval = emberAfSendCommandMulticastWithAlias(entry.sinkList[i].target.groupcast.groupID,
                                                                       (entry.sinkList[i].target.groupcast.alias == 0xFFFF)
                                                                       ? emGpdAlias(addr)
                                                                       : entry.sinkList[i].target.groupcast.alias,
                                                                       sequenceNumber - EMBER_GP_NOTIFICATION_COMMISSIONED_GROUPCAST_SEQUENCE_NUMBER_OFFSET);
      emSetAddDelay(0);
    }
  }
  if (entry.options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_DERIVED_GROUP_GPS) {
    apsFrame = emberAfGetCommandApsFrame();
    apsFrame->sourceEndpoint = GP_ENDPOINT;  //emberAfCurrentEndpoint();
    apsFrame->destinationEndpoint = GP_ENDPOINT;  //emberAfCurrentEndpoint();
    apsFrame->sequence = sequenceNumber;
    apsFrame->options |= EMBER_APS_OPTION_USE_ALIAS_SEQUENCE_NUMBER;
    apsFrame->radius = entry.groupcastRadius;
    if (rxAfterTx) {
      emSetAddDelay(GP_DMIN_B + qualityBasedDelay(gpdLink));
    } else {
      emSetAddDelay(GP_DMIN_U);
    }

    //get derived alias for address alias and group ID alias
    if (entry.options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_ASSIGNED_ALIAS) {
      alias = entry.assignedAlias;
    } else {
      alias = emGpdAlias(addr);
    }

    UNUSED EmberStatus retval = emberAfSendCommandMulticastWithAlias((EmberMulticastId)emGpdAlias(addr),
                                                                     (EmberNodeId)alias,
                                                                     sequenceNumber);
    emSetAddDelay(0);
  }
  return true;
}

/*
 * Check if a EmberGpAddress entry is being used by checking each elemental
 * sequence number's corresponding expiration time field.
 */
bool emAfGreenPowerIsGpAddrUsed(EmberAfGreenPowerDuplicateFilter * filter,
                                uint8_t index)
{
  uint32_t * expirationTimes;
  uint8_t i;
  uint32_t curTime = halCommonGetInt32uMillisecondTick();

  if (index >= EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES) {
    return true;
  }
  expirationTimes = (uint32_t *) &(filter->expirationTimes[index][0]);

  for (i = 0;
       i < EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR;
       i++) {
    if (expirationTimes[i] > curTime) {
      return true;
    }
  }

  return false;
}

uint32_t emAfGreenPowerFindNextExpirationTime(EmberAfGreenPowerDuplicateFilter * filter)
{
  uint32_t expirationTime = MAX_INT32U_VALUE;
  uint8_t addrIndex;
  uint8_t entryIndex;
  uint32_t curTime = halCommonGetInt32uMillisecondTick();
  for (addrIndex = 0;
       addrIndex < EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES;
       addrIndex++) {
    for (entryIndex = 0;
         entryIndex
         < EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR;
         entryIndex++) {
      // The addrIndex cannot be out of bounds as checked by the for loop conditions.
      if ((filter->expirationTimes[addrIndex][entryIndex] > curTime)
          && (filter->expirationTimes[addrIndex][entryIndex] < expirationTime)) {
        expirationTime = filter->expirationTimes[addrIndex][entryIndex];
      }
    }
  }

  return expirationTime;
}

/*
 * return index to the entry with the smallest expiration date.
 */
uint8_t emAfGreenPowerFindEarliestExpirationTimeInAddr(EmberAfGreenPowerDuplicateFilter * filter,
                                                       uint8_t addrIndex)
{
  // addrIndex must be less than EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR
  // so we dont need to check it here. It is done in preceding function calls.
  uint32_t * expirationTimes = filter->expirationTimes[addrIndex];
  uint32_t earliestTimeIndex = 0;
  uint32_t earliestTime = expirationTimes[0];
  uint8_t index;

  for (index = 0;
       index < EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR;
       index++) {
    if (expirationTimes[index] < earliestTime) {
      earliestTime = expirationTimes[index];
      earliestTimeIndex = index;
    }
  }

  return earliestTimeIndex;
}

bool emAfGreenPowerAddRandomMacSeqNum(EmberAfGreenPowerDuplicateFilter * filter,
                                      uint8_t addrIndex,
                                      uint8_t randomSeqNum)
{
  uint8_t * seqNumList = NULL;
  uint32_t * expirationTimeList = NULL;
  uint8_t i;
  uint32_t curTime = halCommonGetInt32uMillisecondTick();
  uint8_t entryIndex;

  // addrIndex must be less than the EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR
  // so we dont need to check it here. It is done in preceding function calls.
  seqNumList = (filter->randomSeqNums[addrIndex]);
  expirationTimeList = (filter->expirationTimes[addrIndex]);

  // check if an existing sequence number with valid expiration date exists
  for (i = 0;
       i < EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_SEQ_NUM_ENTRIES_PER_ADDR;
       i++) {
    if ((randomSeqNum == seqNumList[i]) && (expirationTimeList[i] > curTime)) {
      return false;
    }
  }

  entryIndex = emAfGreenPowerFindEarliestExpirationTimeInAddr(filter,
                                                              addrIndex);

  seqNumList[entryIndex] = randomSeqNum;
  expirationTimeList[entryIndex] = curTime + (2 * MILLISECOND_TICKS_PER_MINUTE);

  return true;
}

/* Helper function for handling duplicateFilter */
uint8_t emAfGreenPowerFindGpAddrIndex(EmberGpAddress * addr,
                                      EmberGpAddress * addrList,
                                      uint8_t sizeOfList)
{
  uint8_t index = 0;
  for (index = 0; index < sizeOfList; index++) {
    if (emGpAddressMatch(addr, &addrList[index])) {
      return index;
    }
  }

  return 0xFF;
}

uint8_t emAfGreenPowerAddGpAddr(EmberAfGreenPowerDuplicateFilter * filter,
                                EmberGpAddress * addr)
{
  uint8_t addrIndex;

  for (addrIndex = 0;
       addrIndex < EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES;
       addrIndex++) {
    if (!emAfGreenPowerIsGpAddrUsed(&duplicateFilter, addrIndex)) {
      MEMCOPY(&filter->addrs[addrIndex], addr, sizeof(EmberGpAddress));
      return addrIndex;
    }
  }

  return 0xFF;
}

/*
 * Check if the incoming message is a duplicate message when the GPD's
 * Mac Sequence Number Capability indicate that random Mac Sequence Number is
 * used.
 *
 *
 * @return
 * If a duplicate mac sequence number has been found, we'll return true in
 * order to reject the message. Otherwise, we'll save the new random mac seq
 * number and return false to accept the message.
 *
 */
bool emAfGreenPowerFindDuplicateMacSeqNum(EmberGpAddress * addr,
                                          uint8_t randomSeqNum)
{
  bool added = false;
  uint8_t addrIndex = emAfGreenPowerFindGpAddrIndex(addr,
                                                    duplicateFilter.addrs,
                                                    EMBER_AF_PLUGIN_GREEN_POWER_CLIENT_MAX_ADDR_ENTRIES);
  if (addrIndex == 0xFF) {
    addrIndex = emAfGreenPowerAddGpAddr(&duplicateFilter, addr);
  }

  if (addrIndex == 0xFF) {
    return false;
  }

  added = emAfGreenPowerAddRandomMacSeqNum(&duplicateFilter,
                                           addrIndex,
                                           randomSeqNum);
  return !added;
}

/*
 * This function implements the "Duplicate checking" and "Freshness check"
 * section of the GP implementation details.
 *
 * @return true if given sequenceNumber is valid and message should be
 *         accepted, false otherwise.
 */
bool emGpMessageChecking(EmberGpAddress *gpAddr, uint8_t sequenceNumber)
{
  /*
     uint8_t proxyTableIndex = emberGpProxyTableLookup(gpAddr);
     EmberGpProxyTableEntry entry;
     EmberGpSecurityLevel securityLevel;
     EmberGpGpdMacSeqNumCap macCap;

     if (proxyTableIndex == 0xFF) {
     return true;
     }

     emberGpProxyTableGetEntry(proxyTableIndex, &entry);
     securityLevel = (EmberGpSecurityLevel) ((entry.options >> 9) & 0x03);
     macCap = (EmberGpGpdMacSeqNumCap) ((entry.options >> 8) & 0x01);

     // Duplicate filtering / Freshness check
     if (securityLevel == EMBER_GP_SECURITY_LEVEL_NONE) {
     // filter by MAC Sequence Number
     if (macCap == EMBER_GP_GPD_MAC_SEQ_NUM_CAP_SEQUENTIAL) {
      uint8_t macSeqNum = (entry.gpdSecurityFrameCounter) & 0xFF;
      if (sequenceNumber <= macSeqNum) {
        emberAfGreenPowerClusterPrintln("Drop frame due to invalid sequence number. incoming(0x%4X), current(0x%4X)", sequenceNumber, macSeqNum);
        return false;
      }
     } else { // EMBER_GP_GPD_MAC_SEQ_NUM_CAP_RANDOM
      if (emAfGreenPowerFindDuplicateMacSeqNum(gpAddr, sequenceNumber)) {
        emberAfGreenPowerClusterPrintln("Drop frame due to duplicate sequence number: 0x%4X", sequenceNumber);
        return false;
      }
     }
     } else if ((securityLevel == EMBER_GP_SECURITY_LEVEL_FC_MIC)
   || (securityLevel == EMBER_GP_SECURITY_LEVEL_FC_MIC_ENCRYPTED)) {
     // filter base on GPD security frame counter
     if (sequenceNumber <= entry.gpdSecurityFrameCounter) {
      emberAfGreenPowerClusterPrintln("Drop frame due to lower sequence number: 0x%4X", sequenceNumber);
      return false;
     }
   */
  return true;
}

//GP-DATA.indication
void emberGpepIncomingMessageHandler(GP_PARAMS)
{
  //emberAfGreenPowerClusterPrintln("Gpep cmd %x status %x SFC %4x", gpdCommandId, status, gpdSecurityFrameCounter);
  if ((commissioningState.channelStatus & GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK)
      && gpdCommandId != EMBER_ZCL_GP_GPDF_CHANNEL_REQUEST) {
    //drop everything but a channel request when we're off on a different channel
    return;
  }

  if ( gpdfSecurityLevel == EMBER_GP_SECURITY_LEVEL_NONE
       && !commissioningState.inCommissioningMode) {
    // earlier a message satisfying this condition would have been
    // dropped in gp-data, but
    // this check is needed here after we added a condition
    // in gp-data to always pass up the unsecure commissioning messages
    // up to this function (EMZIGBEE-1417)
    // when we have a security enabled proxy table entry
    // We only need to forward the unsecure commisioning messages
    // in the commisisong mode, and not in the operational mode
    // (EMZIGBEE-1578)
    bool secureEntry = false;
    uint8_t index = emberGpProxyTableLookup(addr);
    if (index != 0xFF) {
      EmberGpProxyTableEntry entry;
      EmberStatus getStatus = emberGpProxyTableGetEntry(index, &entry);
      if (getStatus != EMBER_SUCCESS) {
        return;
      }
      secureEntry = ((entry.options & EMBER_AF_GP_PROXY_TABLE_ENTRY_OPTIONS_SECURITY_USE)
                     || (entry.securityOptions));
    }

    if (secureEntry) {
      return;
    }
  }

  if (status == EMBER_SUCCESS
      || status == EMBER_NO_SECURITY
      || (status == EMBER_UNPROCESSED && commissioningState.inCommissioningMode)) {
    // duplicate filter
    if (emGpMessageChecking(addr, sequenceNumber)) {
      if (commissioningState.inCommissioningMode) {
        if (autoCommissioning) {
          emberAfGreenPowerClusterAutoCommissioningCallback(GP_ARGS);
        }
        // If status is unprocessed and security is level 3, then gpdCommandId with its payload are encrypted
        // and can not be interpreated by proxy, hence forward it as a GP Commissioning Notification with security
        // processing failed bit set, so that sink can process the commissioning notification.
        if (status == EMBER_UNPROCESSED
            && gpdfSecurityLevel == EMBER_GP_SECURITY_LEVEL_FC_MIC_ENCRYPTED) {
          status = EMBER_AUTH_FAILURE;
          emberAfGreenPowerClusterCommissioningGpdfCallback(GP_ARGS);
          return;
        }
        // Proceed with command processing based on gpdCommandId
        switch (gpdCommandId) {
          case EMBER_ZCL_GP_GPDF_COMMISSIONING:
            emberAfGreenPowerClusterCommissioningGpdfCallback(GP_ARGS);
            break;
          case EMBER_ZCL_GP_GPDF_DECOMMISSIONING:
            emberAfGreenPowerClusterDecommissioningGpdfCallback(GP_ARGS);
            break;
          case EMBER_ZCL_GP_GPDF_SUCCESS:
            emberAfGreenPowerClusterSuccessGpdfCallback(GP_ARGS);
            break;
          case EMBER_ZCL_GP_GPDF_CHANNEL_REQUEST:
            emberAfGreenPowerClusterChannelRequestGpdfCallback(GP_ARGS);
            break;

          default:
            if (status == EMBER_NO_SECURITY
                && ((gpdCommandId >= MIN_RESERVED_GPD_COMMAND_ID
                     && gpdCommandId <= MAX_RESERVED_GPD_COMMAND_ID)
                    || (gpdCommandId >= EMBER_ZCL_GP_GPDF_MFR_DEF_GPD_CMD0
                        && gpdCommandId <= EMBER_ZCL_GP_GPDF_MFR_DEF_GPD_CMD_F))) {
              emberAfGreenPowerClusterCommissioningGpdfCallback(GP_ARGS);
            } else if (status == EMBER_UNPROCESSED) {
              status = EMBER_AUTH_FAILURE;
              emberAfGreenPowerClusterCommissioningGpdfCallback(GP_ARGS);
            } else {
              emberAfGreenPowerClusterGpdfForwardCallback(GP_ARGS);
            }
            break;
        }
      } else {
        //outside of commissioning mode, everything gets forwarded, including commissioning related messages
        // But not success messages (See A3.9.1)
        if (gpdCommandId == EMBER_ZCL_GP_GPDF_SUCCESS
            && !commissioningState.inCommissioningMode) {
          // Drop a success message
        } else {
          emberAfGreenPowerClusterGpdfForwardCallback(GP_ARGS);
        }
      }
    }
  }
}

void emberDGpSentHandler(EmberStatus status, uint8_t gpepHandle)
{
  emberAfGreenPowerClusterPrintln("emberDGpSentHandler Status = %x", status);
  if (status == EMBER_SUCCESS
      && (commissioningState.channelStatus & GP_CLIENT_ON_TRANSMIT_CHANNEL_MASK)) {
    // Here, the sent event is for channel configuration delivery.
    // Clear the 5 sec timer that started while submitted a packet to TxQueue - call handler
    emberAfPluginGreenPowerClientChannelEventHandler();
  }
}

void emberAfPluginGreenPowerClientStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_DOWN
      && emberStackIsPerformingRejoin() == FALSE) {
    // Clear proxy table when leaving the network.
    emberAfPluginGreenPowerClientClearProxyTable();
  }
}
