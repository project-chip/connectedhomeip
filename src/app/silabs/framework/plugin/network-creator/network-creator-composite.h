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
 * @brief APIs and defines for the Network Creator plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_NETWORK_CREATOR_COMPOSITE_H
#define SILABS_NETWORK_CREATOR_COMPOSITE_H

#include "app/framework/include/af.h"

// A composite struct for how "friendly" a channel is. This struct is used by
// the network creator to decide on which channel it should form a network.
// The lower the values, the better the channel.
typedef struct {
  uint8_t beaconsHeard;
  int8_t maxRssiHeard;
} EmAfPluginNetworkCreatorChannelComposite;

// The minimum for a signed 8 bit integer.
#define EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_INVALID_RSSI (-128)

enum {
  EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_METRIC_BEACONS  = 0,
  EM_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_COMPOSITE_METRIC_RSSI     = 1,
};
typedef uint8_t EmAfPluginNetworkCreatorChannelCompositeMetric;

// Here are the threshold values for the channel composite struct. These enable
// the network creator to use discrete logic to find the best channel on which
// to form a network. If a channel has a composite where one of the values is
// higher than the corresponding threshold, then the network creator will not
// form a network on this channel.
#ifndef EMBER_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_BEACONS_THRESHOLD
  #define EMBER_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_BEACONS_THRESHOLD 20
#endif

#define emAfPluginNetworkCreatorChannelCompositeIsAboveThreshold(composite) \
  (((composite).beaconsHeard                                                \
    > EMBER_AF_PLUGIN_NETWORK_CREATOR_CHANNEL_BEACONS_THRESHOLD))

// This is the number of 802.15.4 channels that the internal logic
// looks at in the 'form' part of the formation process. It is recommended
// that this level is not set too low, since that leads to a higher probability
// that a lot of coordinators would form a network on the same channel if they
// acted at the same time.
#define EM_AF_PLUGIN_NETWORK_CREATOR_CHANNELS_TO_CONSIDER 4

#endif /* __NETWORK_CREATOR_COMPOSITE_H__ */
