/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
