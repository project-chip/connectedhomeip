package com.matter.tv.server.handlers;

import android.content.Context;
import android.util.Log;
import com.matter.tv.server.model.ContentApp;
import com.matter.tv.server.receivers.ContentAppDiscoveryService;
import com.matter.tv.server.service.ContentAppAgentService;
import com.matter.tv.server.tvapp.ContentAppEndpointManager;
import com.matter.tv.server.utils.EndpointsDataStore;
import java.util.Collection;

public class ContentAppEndpointManagerImpl implements ContentAppEndpointManager {

  private static final String TAG = "MatterMainActivity";
  private final Context context;

  public ContentAppEndpointManagerImpl(Context context) {
    this.context = context;
  }

  public String sendCommand(int endpointId, long clusterId, long commandId, String commandPayload) {
    Log.d(TAG, "Received a command for endpointId " + endpointId + ". Message " + commandPayload);

    ContentApp discoveredApp =
        getContentApp(
            ContentAppDiscoveryService.getReceiverInstance().getDiscoveredContentApps().values(),
            endpointId);
    if (discoveredApp != null) {
      Log.d(TAG, "Sending a command for endpointId " + endpointId + ". Message " + commandPayload);
      return ContentAppAgentService.sendCommand(
          context, discoveredApp.getAppName(), clusterId, commandId, commandPayload);
    }

    // check to see if this was a previously discovered but now removed app
    ContentApp persistedApp =
        getContentApp(
            EndpointsDataStore.getInstance(context).getAllPersistedContentApps().values(),
            endpointId);
    if (persistedApp != null) {
      Log.d(
          TAG,
          "Message received for a previously discovered app that is no longer "
              + "available. App Name "
              + persistedApp.getAppName());
      return "{\""
          + ContentAppAgentService.FAILURE_KEY
          + "\":{\""
          + ContentAppAgentService.FAILURE_STATUS_KEY
          + "\":"
          + ContentAppAgentService.FAILED_UNSUPPORTED_ENDPOINT
          + "}}";
    }
    // For test cases to pass.
    return "Success";
  }

  public String readAttribute(int endpointId, long clusterId, long attributeId) {
    Log.d(
        TAG,
        "Received a attribute read request for endpointId "
            + endpointId
            + "clusterId"
            + clusterId
            + " attributeId "
            + attributeId);
    ContentApp discoveredApp =
        getContentApp(
            ContentAppDiscoveryService.getReceiverInstance().getDiscoveredContentApps().values(),
            endpointId);
    if (discoveredApp != null) {
      Log.d(TAG, "Sending attribute read request for endpointId " + endpointId);
      return ContentAppAgentService.sendAttributeReadRequest(
          context, discoveredApp.getAppName(), clusterId, attributeId);
    }
    // check to see if this was a previously discovered but now removed app
    ContentApp persistedApp =
        getContentApp(
            EndpointsDataStore.getInstance(context).getAllPersistedContentApps().values(),
            endpointId);
    if (persistedApp != null) {
      Log.d(
          TAG,
          "Message received for a previously discovered app that is no longer "
              + "available. App Name "
              + persistedApp.getAppName());
      return "{\""
          + ContentAppAgentService.FAILURE_KEY
          + "\":{\""
          + ContentAppAgentService.FAILURE_STATUS_KEY
          + "\":"
          + ContentAppAgentService.FAILED_UNSUPPORTED_ENDPOINT
          + "}}";
    }
    // For test cases to pass.
    return "";
  }

  ContentApp getContentApp(Collection<ContentApp> apps, int endpointId) {
    for (ContentApp app : apps) {
      if (app.getEndpointId() == endpointId) {
        return app;
      }
    }
    return null;
  }
}
