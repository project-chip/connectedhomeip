// SupportedCluster.aidl
package com.matter.tv.app.api;

parcelable SupportedCluster {

    int clusterIdentifier;

    String[] features;

    int[] optionalCommandIdentifiers;

}