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
package chip.devicecontroller

import chip.devicecontroller.ClusterIDMapping.BaseCluster

object ChipIdLookup {
    /**
     * Translates cluster ID to a cluster name in upper camel case. If no matching ID is found,
     * returns an empty string.
     */
    fun clusterIdToName(clusterId: Long): String {
        val cluster: BaseCluster = ClusterIDMapping.getCluster(clusterId)
            ?: return String.format("UNKNOWN_CLUSTER(%d)", clusterId)
        return cluster.javaClass.simpleName
    }

    /**
     * Translates cluster ID and attribute ID to an attribute name in upper camel case. If no matching
     * IDs are found, returns an empty string.
     */
    fun attributeIdToName(clusterId: Long, attributeId: Long): String {
        val cluster: BaseCluster = ClusterIDMapping.getCluster(clusterId)
            ?: return "UNKNOWN_CLUSTER($clusterId)"
        return try {
            cluster.getAttributeName(attributeId)
        } catch (e: NoSuchFieldError) {
            "UNKNOWN_ATTRIBUTE($clusterId, $attributeId)"
        }
    }

    /**
     * Translates cluster ID and event ID to an attribute name in upper camel case. If no matching IDs
     * are found, returns an empty string.
     */
    fun eventIdToName(clusterId: Long, eventId: Long): String {
        val cluster: BaseCluster = ClusterIDMapping.getCluster(clusterId)
            ?: return "UNKNOWN_CLUSTER($clusterId)"
        return try {
            cluster.getEventName(eventId)
        } catch (e: NoSuchFieldError) {
            "UNKNOWN_EVENT($clusterId, $eventId)"
        }
    }
}