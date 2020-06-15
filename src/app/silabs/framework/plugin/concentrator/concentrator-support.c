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
 * @brief Code common to SOC and host to handle periodically broadcasting
 * many-to-one route requests (MTORRs).
 *******************************************************************************
   ******************************************************************************/

// *****************************************************************************
// * concentrator-support.c
// *
// * Code common to SOC and host to handle periodically broadcasting
// * many-to-one route requests (MTORRs).
// *
// * Copyright 2012 by Ember Corporation. All rights reserved.              *80*
// *****************************************************************************

#include "app/framework/include/af.h"
#include "stack/include/zigbee-device-stack.h"
#include "concentrator-support.h"
#include "stack/include/source-route.h"
// *****************************************************************************
// Globals

#define MIN_QS (EMBER_AF_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS << 2)
#define MAX_QS (EMBER_AF_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS << 2)

#if (MIN_QS > MAX_QS)
  #error "Minimum broadcast time must be less than max (EMBER_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS < EMBER_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS)"
#endif

// Handy values to make the code more readable.

typedef enum {
  USE_MIN_TIME = 0,
  USE_MAX_TIME = 1,
  USE_ADDRESS_DISCOVERY_TIME = 2,
} QueueDelay;

EmberEventControl emberAfPluginConcentratorUpdateEventControl;

// Use a shorter name to make the code more readable
#define myEvent emberAfPluginConcentratorUpdateEventControl

#ifndef EMBER_AF_HAS_ROUTER_NETWORK
  #error "Concentrator support only allowed on routers and coordinators."
#endif

//This is used to store the sourceRouteOverhead to our last sender
//It defaults to 0xFF if no valid sourceRoute is found. When available, it
//is used once to prevent the overhead of calling ezspGetSourceRouteOverhead()
//and cleared subsequently.
#if defined(EZSP_HOST)
static EmberNodeId targetId          = EMBER_UNKNOWN_NODE_ID;
static uint8_t sourceRouteOverhead     = EZSP_SOURCE_ROUTE_OVERHEAD_UNKNOWN;
#endif

// EMINSIGHT-2484 - allow applications to set whether or not they want
// routers to send mtorrs.
EmberAfPluginConcentratorRouterBehavior emAfPluginConcentratorRouterBehavior
  = EMBER_AF_PLUGIN_CONCENTRATOR_DEFAULT_ROUTER_BEHAVIOR;

// *****************************************************************************
// Functions

uint32_t emberAfPluginConcentratorQueueDiscovery(void)
{
  return emberSetSourceRouteDiscoveryMode(EMBER_SOURCE_ROUTE_DISCOVERY_RESCHEDULE);
}

void emberAfPluginConcentratorMessageSentCallback(EmberOutgoingMessageType type,
                                                  uint16_t indexOrDestination,
                                                  EmberApsFrame *apsFrame,
                                                  EmberStatus status,
                                                  uint16_t messageLength,
                                                  uint8_t *messageContents)
{
  // Handling of message sent failure is now done in the source route library.
  // the callback however, is left here in case there is still consumer side code depening on it
}

// We only store one valid overhead for one destination. We don't want to overwrite that with
// an invalid source route to another destination. We do however want to invalidate an
// overhead to our destination if it is now unknown.
void emberAfSetSourceRouteOverheadCallback(EmberNodeId destination, uint8_t overhead)
{
  #if defined(EZSP_HOST)
  if (!(destination != targetId && overhead == EZSP_SOURCE_ROUTE_OVERHEAD_UNKNOWN)) {
    targetId = destination;
    sourceRouteOverhead = overhead;
  }
  #endif
}

// In an effort to reduce the traffic between the host and NCP, for each incoming message,
// the sourceRouteOverhead to that particular destination is sent from the NCP to the host
// as a part of the incomingMessageHandler(). This information is cached and can be used
// once to calculate the MaximumPayload() to that same destination. It is invalidated after
// one use.
uint8_t emberAfGetSourceRouteOverheadCallback(EmberNodeId destination)
{
#if defined(EZSP_HOST)
  if (targetId == destination && sourceRouteOverhead != EZSP_SOURCE_ROUTE_OVERHEAD_UNKNOWN) {
    emberAfDebugPrintln("ValidSourceRouteFound %u ", sourceRouteOverhead);
    return sourceRouteOverhead;
  } else {
    return ezspGetSourceRouteOverhead(destination);
  }
#else
  return emberGetSourceRouteOverhead(destination);
#endif
}

void emberIncomingRouteErrorHandler(EmberStatus status, EmberNodeId target)
{
}

void ezspIncomingRouteErrorHandler(EmberStatus status, EmberNodeId target)
{
}

void emberAfPluginConcentratorStackStatusCallback(EmberStatus status)
{
  EmberNodeType nodeType;
  if (status == EMBER_NETWORK_DOWN
      && !emberStackIsPerformingRejoin()) {
    //now we clear/init the source route table everytime the network is up , therefore we do clear the source route table on rejoin.
  } else if (status == EMBER_NETWORK_UP) {
    if ((emAfPluginConcentratorRouterBehavior == FULL)
        || (emberAfGetNodeType(&nodeType) == EMBER_SUCCESS
            && nodeType == EMBER_COORDINATOR)) {
      emberSetConcentrator(true,
                           EMBER_AF_PLUGIN_CONCENTRATOR_CONCENTRATOR_TYPE,
                           EMBER_AF_PLUGIN_CONCENTRATOR_MIN_TIME_BETWEEN_BROADCASTS_SECONDS,
                           EMBER_AF_PLUGIN_CONCENTRATOR_MAX_TIME_BETWEEN_BROADCASTS_SECONDS,
                           EMBER_AF_PLUGIN_CONCENTRATOR_ROUTE_ERROR_THRESHOLD,
                           EMBER_AF_PLUGIN_CONCENTRATOR_DELIVERY_FAILURE_THRESHOLD,
                           EMBER_AF_PLUGIN_CONCENTRATOR_MAX_HOPS
                           );
    }
  }
}
