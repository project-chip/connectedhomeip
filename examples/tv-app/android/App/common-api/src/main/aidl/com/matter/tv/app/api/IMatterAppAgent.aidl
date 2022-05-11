// IMatterAppAgent.aidl
package com.matter.tv.app.api;

import com.matter.tv.app.api.SetSupportedClustersRequest;
import com.matter.tv.app.api.ReportAttributeChangeRequest;

/*
 * To use this interface, partners should query for and bind to a service that handles the "com.matter.tv.app.api.action.MatterAppAgent" Action.
 * They should verify the host process  holds the "com.matter.tv.app.api.permission.SEND_DATA" permission
 * To bind to this service the client app itself must hold "com.matter.tv.app.api.permission.BIND_SERVICE_PERMISSION".
 */
interface IMatterAppAgent {
    /**
     * Report dynamic clusters to matter agent. Note that this api is not incremental, every time it is called
     * you must report ALL dynamic clusters the app supports. Any dynamic clusters previously reported
     * which are not reported in a subsequent call will be removed. This does NOT impact static clusters
     * declared in app resources; those cannot be removed. However, a dynamic cluster can be used to override
     * and hide a static one based on cluster name.
     *
     * @param SetClustersRequest request object containing the list of clusters to assert for this app.
     * @returns true if successful.
     */
     // TODO : replace the boolean with some kind of enumerated status field
    boolean setSupportedClusters(in SetSupportedClustersRequest request);

    /**
     * Reports the Attribute changes of attributes.
     * @param - ReportAttributeChangeRequest, request object containing all attributes which have changed.
     * @return - ReportAttributeChangeResult, returns success or error code
     */
     // TODO : replace the boolean with some kind of enumerated status field
    boolean reportAttributeChange(in ReportAttributeChangeRequest request);
}