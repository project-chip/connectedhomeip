// SupportedCluster.aidl
package com.matter.tv.app.api;

parcelable SupportedCluster {

    int clusterIdentifier;

    int features;

    int[] optionalCommandIdentifiers;

    int[] optionalAttributesIdentifiers;
}