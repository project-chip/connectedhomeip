// ReportAttributeChangeRequest.aidl
package com.matter.tv.app.api;

parcelable ReportAttributeChangeRequest{

    int clusterIdentifier;

    int attributeIdentifier;

    String value;

}