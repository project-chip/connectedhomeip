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

package chip.devicecontroller;

public final class ChipIdLookup {
  /**
   * Translates cluster ID to a cluster name in upper camel case. If no matching ID is found,
   * returns an empty string.
   */
  public static String clusterIdToName(long clusterId) {
    ClusterIDMapping.BaseCluster cluster = ClusterIDMapping.getCluster(clusterId);
    if (cluster == null) {
      return String.format("UNKNOWN_CLUSTER(%d)", clusterId);
    }

    return cluster.getClass().getSimpleName();
  }

  /**
   * Translates cluster ID and attribute ID to an attribute name in upper camel case. If no matching
   * IDs are found, returns an empty string.
   */
  public static String attributeIdToName(long clusterId, long attributeId) {
    ClusterIDMapping.BaseCluster cluster = ClusterIDMapping.getCluster(clusterId);
    if (cluster == null) {
      return String.format("UNKNOWN_CLUSTER(%d)", clusterId);
    }

    try {
      return cluster.getAttributeName(attributeId);
    } catch (NoSuchFieldError e) {
      return String.format("UNKNOWN_ATTRIBUTE(%d, %d)", clusterId, attributeId);
    }
  }

  /**
   * Translates cluster ID and event ID to an attribute name in upper camel case. If no matching IDs
   * are found, returns an empty string.
   */
  public static String eventIdToName(long clusterId, long eventId) {
    ClusterIDMapping.BaseCluster cluster = ClusterIDMapping.getCluster(clusterId);
    if (cluster == null) {
      return String.format("UNKNOWN_CLUSTER(%d)", clusterId);
    }

    try {
      return cluster.getEventName(eventId);
    } catch (NoSuchFieldError e) {
      return String.format("UNKNOWN_EVENT(%d, %d)", clusterId, eventId);
    }
  }
}
